/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * This file is part of PvZ-Portable.
 *
 * PvZ-Portable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PvZ-Portable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PvZ-Portable. If not, see <https://www.gnu.org/licenses/>.
 */

#include "../Board.h"
#include "../Plant.h"
#include "../Zombie.h"
#include "GameButton.h"
#include "../SeedPacket.h"
#include "../../LawnApp.h"
#include "AlmanacDialog.h"
#include "../../Resources.h"
#include "../System/Music.h"
#include "../../GameConstants.h"
#include "../System/PlayerInfo.h"
#include "../System/PoolEffect.h"
#include "../System/ReanimationLawn.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "widget/WidgetManager.h"
#include "graphics/Font.h"
#include <unordered_map>  // Mod API: gZombieDefeated 改用 map 支持自定义僵尸

// Mod API: gZombieDefeated 改用 unordered_map，支持自定义僵尸类型（>= NUM_ZOMBIE_TYPES）
// 原版僵尸仍用数组快速查询，自定义僵尸走 map
namespace {
    bool gZombieDefeatedBuiltin[NUM_ZOMBIE_TYPES] = { false };
    std::unordered_map<int, bool> gZombieDefeatedCustom;
}

bool gZombieDefeated_Get(ZombieType z) {
    int idx = static_cast<int>(z);
    if (idx < NUM_ZOMBIE_TYPES) return gZombieDefeatedBuiltin[idx];
    auto it = gZombieDefeatedCustom.find(idx);
    return it != gZombieDefeatedCustom.end() ? it->second : false;
}

void gZombieDefeated_Set(ZombieType z, bool v) {
    int idx = static_cast<int>(z);
    if (idx < NUM_ZOMBIE_TYPES) gZombieDefeatedBuiltin[idx] = v;
    else gZombieDefeatedCustom[idx] = v;
}

AlmanacDialog::AlmanacDialog(LawnApp* theApp) : LawnDialog(theApp, DIALOG_ALMANAC, true, theApp->GetString("ALMANAC_HEADER", "Almanac"), "", "", BUTTONS_NONE)
{
	mApp = (LawnApp*)gSexyAppBase;
	mOpenPage = ALMANAC_PAGE_INDEX;
	mSelectedSeed = SEED_PEASHOOTER;
	mSelectedZombie = ZOMBIE_NORMAL;
	mZombie = nullptr;
	mPlant = nullptr;
	mDrawStandardBack = false;
	mLoadedResourceNames.push_back("DelayLoad_Almanac");
	for (size_t i = 0; i < LENGTH(mZombiePerfTest); i++) mZombiePerfTest[i] = nullptr;
	LawnDialog::Resize(0, 0, BOARD_WIDTH, BOARD_HEIGHT);
	// Mod API: 初始化分页状态
	mPlantPage = 0;
	mZombiePage = 0;

	for (std::string& resource : mLoadedResourceNames)
		TodLoadResources(resource.c_str());

	mCloseButton = new GameButton(AlmanacDialog::ALMANAC_BUTTON_CLOSE);
	mCloseButton->SetLabel("[CLOSE_BUTTON]");
	mCloseButton->mButtonImage = Sexy::IMAGE_ALMANAC_CLOSEBUTTON;
	mCloseButton->mOverImage = Sexy::IMAGE_ALMANAC_CLOSEBUTTONHIGHLIGHT;
	mCloseButton->mDownImage = nullptr;
	mCloseButton->SetFont(Sexy::FONT_BRIANNETOD12);
	Color aColor = Color(42, 42, 90);
	mCloseButton->mColors[ButtonWidget::COLOR_LABEL] = aColor;
	mCloseButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = aColor;
	mCloseButton->Resize(676, 567, 89, 26);
	mCloseButton->mParentWidget = this;
	mCloseButton->mTextOffsetX = -8;
	mCloseButton->mTextOffsetY = 1;

	mIndexButton = new GameButton(AlmanacDialog::ALMANAC_BUTTON_INDEX);
	mIndexButton->SetLabel("[ALMANAC_INDEX]");
	mIndexButton->mButtonImage = Sexy::IMAGE_ALMANAC_INDEXBUTTON;
	mIndexButton->mOverImage = Sexy::IMAGE_ALMANAC_INDEXBUTTONHIGHLIGHT;
	mIndexButton->mDownImage = nullptr;
	mIndexButton->SetFont(Sexy::FONT_BRIANNETOD12);
	mIndexButton->mColors[ButtonWidget::COLOR_LABEL] = aColor;
	mIndexButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = aColor;
	mIndexButton->Resize(32, 567, 164, 26);
	mIndexButton->mParentWidget = this;
	mIndexButton->mTextOffsetX = 8;
	mIndexButton->mTextOffsetY = 1;

	mPlantButton = new GameButton(AlmanacDialog::ALMANAC_BUTTON_PLANT);
	mPlantButton->SetLabel("[VIEW_PLANTS]");
	mPlantButton->mButtonImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON;
	mPlantButton->mOverImage = nullptr;
	mPlantButton->mDownImage = nullptr;
	mPlantButton->mDisabledImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON_DISABLED;
	mPlantButton->mOverOverlayImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON_GLOW;
	mPlantButton->SetFont(Sexy::FONT_DWARVENTODCRAFT18YELLOW);
	mPlantButton->mColors[ButtonWidget::COLOR_LABEL] = Color::White;
	mPlantButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = Color::White;
	mPlantButton->Resize(130, 345, 156, 42);
	mPlantButton->mTextOffsetY = -1;
	mPlantButton->mParentWidget = this;

	mZombieButton = new GameButton(AlmanacDialog::ALMANAC_BUTTON_ZOMBIE);
	mZombieButton->SetLabel("[VIEW_ZOMBIES]");
	mZombieButton->Resize(487, 345, 210, 48);
	mZombieButton->mDrawStoneButton = true;
	mZombieButton->mParentWidget = this;

	// Mod API: 翻页按钮（使用 ZOMBATAR 风格图片，放在底部 mIndexButton 和 mCloseButton 之间）
	// mIndexButton 右边界=196，mCloseButton 左边界=676，中间 480px 可用空间
	// Prev 放 (350, 565)，Next 放 (430, 565)，页码文字在中间
	mPrevButton = new GameButton(AlmanacDialog::ALMANAC_BUTTON_PREV);
	mPrevButton->SetLabel("");
	mPrevButton->mButtonImage = Sexy::IMAGE_ZOMBATAR_PREV_BUTTON;
	mPrevButton->mOverImage = Sexy::IMAGE_ZOMBATAR_PREV_BUTTON_HIGHLIGHT;
	mPrevButton->mDownImage = nullptr;
	mPrevButton->mParentWidget = this;
	if (Sexy::IMAGE_ZOMBATAR_PREV_BUTTON) {
		int bw = Sexy::IMAGE_ZOMBATAR_PREV_BUTTON->mWidth;
		int bh = Sexy::IMAGE_ZOMBATAR_PREV_BUTTON->mHeight;
		mPrevButton->Resize(350, 565, bw, bh);
		mPrevButton->mBtnNoDraw = true;  // 默认隐藏，仅有多页时显示
	}

	mNextButton = new GameButton(AlmanacDialog::ALMANAC_BUTTON_NEXT);
	mNextButton->SetLabel("");
	mNextButton->mButtonImage = Sexy::IMAGE_ZOMBATAR_NEXT_BUTTON;
	mNextButton->mOverImage = Sexy::IMAGE_ZOMBATAR_NEXT_BUTTON_HIGHLIGHT;
	mNextButton->mDownImage = nullptr;
	mNextButton->mParentWidget = this;
	if (Sexy::IMAGE_ZOMBATAR_NEXT_BUTTON) {
		int bw = Sexy::IMAGE_ZOMBATAR_NEXT_BUTTON->mWidth;
		int bh = Sexy::IMAGE_ZOMBATAR_NEXT_BUTTON->mHeight;
		// Next 按钮放在 Prev 右侧，中间留 30px 给页码文字
		int nextX = 350 + (Sexy::IMAGE_ZOMBATAR_PREV_BUTTON ? Sexy::IMAGE_ZOMBATAR_PREV_BUTTON->mWidth : 0) + 30;
		mNextButton->Resize(nextX, 565, bw, bh);
		mNextButton->mBtnNoDraw = true;
	}

	SetPage(ALMANAC_PAGE_INDEX);
	if (!mApp->mBoard || !mApp->mBoard->mPaused)
		mApp->mMusic->MakeSureMusicIsPlaying(MUSIC_TUNE_CHOOSE_YOUR_SEEDS);
}

AlmanacDialog::~AlmanacDialog()
{
	if (mCloseButton)	delete mCloseButton;
	if (mIndexButton)	delete mIndexButton;
	if (mPlantButton)	delete mPlantButton;
	if (mZombieButton)	delete mZombieButton;
	if (mPrevButton)	delete mPrevButton;
	if (mNextButton)	delete mNextButton;

	ClearPlantsAndZombies();
}

void AlmanacDialog::ClearPlantsAndZombies()
{
	if (mPlant)
	{
		mPlant->Die();
		delete mPlant;
		mPlant = nullptr;
	}
	if (mZombie)
	{
		mZombie->DieNoLoot();
		delete mZombie;
		mZombie = nullptr;
	}
	for (Zombie* &aZombie : mZombiePerfTest)
	{
		if (aZombie)
		{
			aZombie->DieNoLoot();
			delete aZombie;
		}
		aZombie = nullptr;
	}
}

void AlmanacDialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
	LawnDialog::RemovedFromManager(theWidgetManager);
	ClearPlantsAndZombies();
}

// GOTY @Patoke: 0x402C50
void AlmanacDialog::SetupPlant()
{
	ClearPlantsAndZombies();

	float aPosX = ALMANAC_PLANT_POSITION_X;
	float aPosY = ALMANAC_PLANT_POSITION_Y;
	if (mSelectedSeed == SEED_TALLNUT)				aPosY += 18;
	else if (mSelectedSeed == SEED_COBCANNON)		aPosX -= 40;
	else if (mSelectedSeed == SEED_FLOWERPOT)		aPosY -= 20;
	else if (mSelectedSeed == SEED_INSTANT_COFFEE)	aPosY += 20;
	else if (mSelectedSeed == SEED_GRAVEBUSTER)		aPosY += 55;

	mPlant = new Plant();
	mPlant->mBoard = nullptr;
	mPlant->mIsOnBoard = false;
	mPlant->PlantInitialize(0, 0, mSelectedSeed, SEED_NONE);
	mPlant->mX = aPosX;
	mPlant->mY = aPosY;
}

// GOTY @Patoke: 0x402D90
void AlmanacDialog::SetupZombie()
{
	ClearPlantsAndZombies();

	mZombie = new Zombie();
	mZombie->mBoard = nullptr;
	mZombie->ZombieInitialize(0, mSelectedZombie, false, nullptr, Zombie::ZOMBIE_WAVE_UI);
	mZombie->mPosX = ALMANAC_ZOMBIE_POSITION_X;
	mZombie->mPosY = ALMANAC_ZOMBIE_POSITION_Y;
}

void AlmanacDialog::SetPage(AlmanacPage thePage)
{
	mOpenPage = thePage;
	ClearPlantsAndZombies();

	if (mOpenPage == AlmanacPage::ALMANAC_PAGE_INDEX)
	{
		mPlant = new Plant();
		mPlant->mBoard = nullptr;
		mPlant->mIsOnBoard = false;
		mPlant->PlantInitialize(0, 0, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
		mPlant->mX = ALMANAC_INDEXPLANT_POSITION_X;
		mPlant->mY = ALMANAC_INDEXPLANT_POSITION_Y;

		mZombie = new Zombie();
		mZombie->mBoard = nullptr;
		mZombie->ZombieInitialize(0, ZombieType::ZOMBIE_NORMAL, false, nullptr, Zombie::ZOMBIE_WAVE_UI);
		mZombie->mPosX = ALMANAC_INDEXZOMBIE_POSITION_X;
		mZombie->mPosY = ALMANAC_INDEXZOMBIE_POSITION_Y;

		mIndexButton->mBtnNoDraw = true;
		mPlantButton->mBtnNoDraw = false;
		mZombieButton->mBtnNoDraw = false;
	}
	else
	{
		if (mOpenPage == AlmanacPage::ALMANAC_PAGE_PLANTS)
			SetupPlant();
		else if (mOpenPage == AlmanacPage::ALMANAC_PAGE_ZOMBIES)
			SetupZombie();
		else return;

		mIndexButton->mBtnNoDraw = false;
		mPlantButton->mBtnNoDraw = true;
		mZombieButton->mBtnNoDraw = true;
	}
}

void AlmanacDialog::ShowPlant(SeedType theSeedType)
{
	mSelectedSeed = theSeedType;
	// Mod API: 确保选中植物在当前页
	int seedIdx = static_cast<int>(theSeedType);
	int page = seedIdx / ALMANAC_PLANTS_PER_PAGE;
	if (page != mPlantPage) mPlantPage = page;
	SetPage(ALMANAC_PAGE_PLANTS);
}

void AlmanacDialog::ShowZombie(ZombieType theZombieType)
{
	mSelectedZombie = theZombieType;
	// Mod API: 确保选中僵尸在当前页
	int totalZombies = GetTotalZombieCount();
	for (int i = 0; i < totalZombies; i++) {
		if (GetZombieType(i) == theZombieType) {
			int page = i / ALMANAC_ZOMBIES_PER_PAGE;
			if (page != mZombiePage) mZombiePage = page;
			break;
		}
	}
	SetPage(ALMANAC_PAGE_ZOMBIES);
}

void AlmanacDialog::Update()
{
	mCloseButton->Update();
	mIndexButton->Update();
	mPlantButton->Update();
	mZombieButton->Update();
	if (mPrevButton) mPrevButton->Update();
	if (mNextButton) mNextButton->Update();
	if (mPlant) mPlant->Update();
	if (mZombie) mZombie->Update();
	for (Zombie* aZombie : mZombiePerfTest)
	{
		if (aZombie)
		{
			aZombie->Update();
		}
	}

	int aMouseX = mApp->mWidgetManager->mLastMouseX;
	int aMouseY = mApp->mWidgetManager->mLastMouseY;
	if (SeedHitTest(aMouseX, aMouseY) != SeedType::SEED_NONE || ZombieHitTest(aMouseX, aMouseY) != ZombieType::ZOMBIE_INVALID ||
		mCloseButton->IsMouseOver() || mIndexButton->IsMouseOver() || mPlantButton->IsMouseOver() || mZombieButton->IsMouseOver() ||
		(mPrevButton && !mPrevButton->mBtnNoDraw && mPrevButton->IsMouseOver()) ||
		(mNextButton && !mNextButton->mBtnNoDraw && mNextButton->IsMouseOver()))
	{
		mApp->SetCursor(CURSOR_HAND);
	}
	else
	{
		mApp->SetCursor(CURSOR_POINTER);
	}

	mApp->mPoolEffect->PoolEffectUpdate();
	MarkDirty();
}

ZombieType AlmanacDialog::GetZombieType(int theIndex)
{
	// Mod API: 支持自定义僵尸（索引 >= NUM_ZOMBIE_TYPES 时从 gCustomZombieDefs 取）
	if (theIndex < NUM_ZOMBIE_TYPES) return (ZombieType)theIndex;
	int customIdx = theIndex - NUM_ZOMBIE_TYPES;
	if (customIdx < GetCustomZombieCount()) {
		return gCustomZombieDefs[customIdx].mZombieType;
	}
	return ZOMBIE_INVALID;
}

void AlmanacDialog::DrawIndex(Graphics* g)
{
	g->DrawImage(Sexy::IMAGE_ALMANAC_INDEXBACK, 0, 0);
	TodDrawString(g, "[SUBURBAN_ALMANAC_INDEX]", BOARD_WIDTH / 2, 60, Sexy::FONT_HOUSEOFTERROR28, Color(220, 220, 220), DrawStringJustification::DS_ALIGN_CENTER);
	
	if (mPlant)
	{
		Graphics aPlantGraphics = Graphics(*g);
		mPlant->BeginDraw(&aPlantGraphics);
		mPlant->Draw(&aPlantGraphics);
	}
	if (mZombie)
	{
		Graphics aZombieGraphics = Graphics(*g);
		mZombie->BeginDraw(&aZombieGraphics);
		mZombie->Draw(&aZombieGraphics);
	}
}

void AlmanacDialog::DrawPlants(Graphics* g)
{
	g->DrawImage(Sexy::IMAGE_ALMANAC_PLANTBACK, 0, 0);
	TodDrawString(g, "[SUBURBAN_ALMANAC_PLANTS]", BOARD_WIDTH / 2, 48, Sexy::FONT_HOUSEOFTERROR20, Color(213, 159, 43), DrawStringJustification::DS_ALIGN_CENTER);

	SeedType aSeedMouseOn = SeedHitTest(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY);
	// Mod API: 分页遍历——只绘制当前页的植物
	int totalSeeds = GetTotalPlantCount();
	int pageStart = GetPlantPageStart();
	int pageEnd = std::min(pageStart + ALMANAC_PLANTS_PER_PAGE, totalSeeds);
	for (int aSeedIdx = pageStart; aSeedIdx < pageEnd; aSeedIdx++)
	{
		SeedType aSeedType = static_cast<SeedType>(aSeedIdx);
		int aPosX, aPosY;
		// 当前页内的索引映射回网格位置（页内相对索引，保持原版布局）
		int pageOffset = aSeedIdx - pageStart;
		GetSeedPositionByIndex(pageOffset, aPosX, aPosY);
		if (mApp->HasSeedType(aSeedType))
		{
			if (aSeedType == SeedType::SEED_IMITATER)
			{
				if (aSeedType == aSeedMouseOn)
					g->DrawImage(Sexy::IMAGE_ALMANAC_IMITATER, aPosX, aPosY);
				g->DrawImage(Sexy::IMAGE_ALMANAC_IMITATER, aPosX, aPosY);
			}
			else
			{
				DrawSeedPacket(g, aPosX, aPosY, aSeedType, SeedType::SEED_NONE, 0, 255, true, false);
				if (aSeedType == aSeedMouseOn)
					g->DrawImage(Sexy::IMAGE_SEEDPACKETFLASH, aPosX, aPosY);
			}
		}
	}

	if (mSelectedSeed == SeedType::SEED_LILYPAD || mSelectedSeed == SeedType::SEED_TANGLEKELP || 
		mSelectedSeed == SeedType::SEED_CATTAIL || mSelectedSeed == SeedType::SEED_SEASHROOM)
	{
		bool aNight = mSelectedSeed == SeedType::SEED_SEASHROOM;
		g->DrawImage(aNight ? Sexy::IMAGE_ALMANAC_GROUNDNIGHTPOOL : Sexy::IMAGE_ALMANAC_GROUNDPOOL, 521, 107);

		if (mApp->Is3DAccelerated())
		{
			g->SetClipRect(475, 0, 397, 500);
			g->mTransY -= 145;
			mApp->mPoolEffect->PoolEffectDraw(g, aNight);
			g->mTransY += 145;
			g->ClearClipRect();
		}
	}
	else
	{
		g->DrawImage(
			Plant::IsNocturnal(mSelectedSeed) || mSelectedSeed == SeedType::SEED_GRAVEBUSTER || mSelectedSeed == SeedType::SEED_PLANTERN ? Sexy::IMAGE_ALMANAC_GROUNDNIGHT :
			mSelectedSeed == SeedType::SEED_FLOWERPOT ? Sexy::IMAGE_ALMANAC_GROUNDROOF : Sexy::IMAGE_ALMANAC_GROUNDDAY,
			521, 107
		);
	}
	
	if (mPlant)
	{
		Graphics aPlantGraphics = Graphics(*g);
		mPlant->BeginDraw(&aPlantGraphics);
		mPlant->Draw(&aPlantGraphics);
	}

	g->DrawImage(Sexy::IMAGE_ALMANAC_PLANTCARD, 459, 86);
	const PlantDefinition& aPlantDef = GetPlantDefinition(mSelectedSeed);
	// Mod API: 自定义植物优先用 mAlmanacName/mAlmanacDescription，原版植物走资源文件
	std::string aName;
	std::string aDescriptionName;
	if (IsCustomSeedType(mSelectedSeed) && !aPlantDef.mAlmanacName.empty()) {
		aName = aPlantDef.mAlmanacName;
		aDescriptionName = aPlantDef.mAlmanacDescription;
	} else {
		aName = Plant::GetNameString(mSelectedSeed, SEED_NONE);
		aDescriptionName = StrFormat("[%s_DESCRIPTION]", aPlantDef.mPlantName.c_str());
	}
	TodDrawString(g, aName, 617, 288, Sexy::FONT_DWARVENTODCRAFT18YELLOW, Color::White, DS_ALIGN_CENTER);
	TodDrawStringWrapped(g, aDescriptionName, Rect(485, 309, 258, 230), Sexy::FONT_BRIANNETOD12, Color(40, 50, 90), DS_ALIGN_LEFT);

	if (mSelectedSeed != SeedType::SEED_IMITATER)
	{
		std::string aCostStr = TodReplaceString(StrFormat("{KEYWORD}{COST}:{STAT} %d", aPlantDef.mSeedCost), "{COST}", "[COST]");
		TodDrawStringWrapped(g, aCostStr, Rect(485, 520, 134, 50), Sexy::FONT_BRIANNETOD12, Color::White, DS_ALIGN_LEFT);

		std::string aRechargeStr = TodReplaceString(
			"{KEYWORD}{WAIT_TIME}: {STAT}{WAIT_TIME_LENGTH}", 
			"{WAIT_TIME_LENGTH}",
			aPlantDef.mRefreshTime == 750 ? "[WAIT_TIME_SHORT]" : aPlantDef.mRefreshTime == 3000 ? "[WAIT_TIME_LONG]" : "[WAIT_TIME_VERY_LONG]" // @Patoke: fix typo XD
		);
		aRechargeStr = TodReplaceString(aRechargeStr, "{WAIT_TIME}", "[WAIT_TIME]");
		TodDrawStringWrapped(g, aRechargeStr, Rect(600, 520, 139, 50), Sexy::FONT_BRIANNETOD12, Color(40, 50, 90), DS_ALIGN_RIGHT);
	}
}

// GOTY @Patoke: 0x403DE0
void AlmanacDialog::DrawZombies(Graphics* g)
{
	g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEBACK, 0, 0);
	TodDrawString(g, "[SUBURBAN_ALMANAC_ZOMBIES]", BOARD_WIDTH / 2, 54, Sexy::FONT_DWARVENTODCRAFT24, Color(0, 196, 0), DS_ALIGN_CENTER);

	ZombieType aZombieMouseOn = ZombieHitTest(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY);
	// Mod API: 分页遍历——只绘制当前页的僵尸
	int totalZombies = GetTotalZombieCount();
	int pageStart = GetZombiePageStart();
	int pageEnd = std::min(pageStart + ALMANAC_ZOMBIES_PER_PAGE, totalZombies);
	for (int i = pageStart; i < pageEnd; i++)
	{
		ZombieType aZombieType = GetZombieType(i);
		int aPosX, aPosY;
		// 当前页内的索引映射回网格位置（页内相对索引，保持原版布局）
		int pageOffset = i - pageStart;
		GetZombiePositionByIndex(pageOffset, aPosX, aPosY);
		if (aZombieType != ZombieType::ZOMBIE_INVALID)
		{
			if (!ZombieIsShown(aZombieType))
				g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEBLANK, aPosX, aPosY);
			else
			{
				g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW, aPosX, aPosY);
				if (aZombieType == aZombieMouseOn)
				{
					g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
					g->SetColor(Color(255, 255, 255, 48));
					g->SetColorizeImages(true);
					g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW, aPosX, aPosY);
					g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
					g->SetColorizeImages(false);
				}

				ZombieType aZombieTypeToDraw = aZombieType;
				Graphics aZombieGraphics = Graphics(*g);
				aZombieGraphics.SetClipRect(aPosX + 2, aPosY + 2, 72, 72);
				aZombieGraphics.Translate(aPosX + 1, aPosY - 6);
				aZombieGraphics.mScaleX = 0.5f;
				aZombieGraphics.mScaleY = 0.5f;
				switch (aZombieType)
				{
				case ZombieType::ZOMBIE_POLEVAULTER:
					aZombieGraphics.TranslateF(2, -3);
					aZombieTypeToDraw = ZombieType::ZOMBIE_CACHED_POLEVAULTER_WITH_POLE;		break;
				case ZombieType::ZOMBIE_FLAG:			aZombieGraphics.TranslateF(2, 10);	break;
				case ZombieType::ZOMBIE_TRAFFIC_CONE:	aZombieGraphics.TranslateF(0, 12);	break;
				case ZombieType::ZOMBIE_PAIL:			aZombieGraphics.TranslateF(0, 9);		break;
				case ZombieType::ZOMBIE_FOOTBALL:		aZombieGraphics.TranslateF(-15, -1);	break;
				case ZombieType::ZOMBIE_ZAMBONI:		aZombieGraphics.TranslateF(0, 3);		break;
				case ZombieType::ZOMBIE_DOLPHIN_RIDER:	aZombieGraphics.TranslateF(-2, -10);	break;
				case ZombieType::ZOMBIE_POGO:			aZombieGraphics.TranslateF(0, -3);	break;
				case ZombieType::ZOMBIE_GARGANTUAR:		aZombieGraphics.TranslateF(15, 17);	break;
				case ZombieType::ZOMBIE_IMP:			aZombieGraphics.TranslateF(-8, -7);	break;
				case ZombieType::ZOMBIE_BUNGEE:			aZombieGraphics.TranslateF(-4, 3);	break;
				case ZombieType::ZOMBIE_DANCER:			aZombieGraphics.TranslateF(0, 15);	break;
				case ZombieType::ZOMBIE_BACKUP_DANCER:	aZombieGraphics.TranslateF(-4, 20);	break;
				case ZombieType::ZOMBIE_SNORKEL:		aZombieGraphics.TranslateF(-10, 0);	break;
				case ZombieType::ZOMBIE_YETI:			aZombieGraphics.TranslateF(0, 4);		break;
				case ZombieType::ZOMBIE_CATAPULT:		aZombieGraphics.TranslateF(-24, -1);	break;
				case ZombieType::ZOMBIE_BOBSLED:		aZombieGraphics.TranslateF(0, -8);	break;
				case ZombieType::ZOMBIE_LADDER:			aZombieGraphics.TranslateF(0, -3);	break;
				default: break;
				}
				if (ZombieHasSilhouette(aZombieType))
				{
					aZombieGraphics.SetColor(Color(0, 0, 0, 40));
					aZombieGraphics.SetColorizeImages(true);
				}
				mApp->mReanimatorCache->DrawCachedZombie(&aZombieGraphics, 0, 0, aZombieTypeToDraw);
				aZombieGraphics.SetColorizeImages(false);

				g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW2, aPosX, aPosY);
				if (aZombieType == aZombieMouseOn)
				{
					g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
					g->SetColor(Color(255, 255, 255, 48));
					g->SetColorizeImages(true);
					g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW2, aPosX, aPosY);
					g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
					g->SetColorizeImages(false);
				}
			}
		}
	}

	g->DrawImage(mZombie->mZombieType == ZombieType::ZOMBIE_ZAMBONI || mZombie->mZombieType == ZombieType::ZOMBIE_BOBSLED ?
		Sexy::IMAGE_ALMANAC_GROUNDICE : Sexy::IMAGE_ALMANAC_GROUNDDAY, 518, 110);
	if (mZombie && !ZombieHasSilhouette(mZombie->mZombieType))
	{
		Graphics aZombieGraphics = Graphics(*g);
		mZombie->BeginDraw(&aZombieGraphics);
		aZombieGraphics.SetClipRect(-42, -51, 197, 187);
		switch (mZombie->mZombieType)
		{
		case ZombieType::ZOMBIE_ZAMBONI:		aZombieGraphics.TranslateF(-30, 5);		break;
		case ZombieType::ZOMBIE_GARGANTUAR:		aZombieGraphics.TranslateF(0, 40);		break;
		case ZombieType::ZOMBIE_FOOTBALL:		aZombieGraphics.TranslateF(-10, 0);		break;
		case ZombieType::ZOMBIE_BALLOON:		aZombieGraphics.TranslateF(0, -20);		break;
		case ZombieType::ZOMBIE_BUNGEE:			aZombieGraphics.TranslateF(15, 0);		break;
		case ZombieType::ZOMBIE_CATAPULT:		aZombieGraphics.TranslateF(-10, 0);		break;
		case ZombieType::ZOMBIE_BOSS:			aZombieGraphics.TranslateF(-540, -175);	break;
		default: break;
		}
		if (mZombie->mZombieType != ZombieType::ZOMBIE_BUNGEE && mZombie->mZombieType != ZombieType::ZOMBIE_BOSS &&
			mZombie->mZombieType != ZombieType::ZOMBIE_ZAMBONI && mZombie->mZombieType != ZombieType::ZOMBIE_CATAPULT)
			mZombie->DrawShadow(&aZombieGraphics);
		mZombie->Draw(&aZombieGraphics);
	}
	g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIECARD, 455, 78);

	const ZombieDefinition& aZombieDef = GetZombieDefinition(mSelectedZombie);
	// Mod API: 自定义僵尸优先用 mAlmanacName/mAlmanacDescription，原版僵尸走资源文件
	std::string aName;
	if (IsCustomZombieType(mSelectedZombie) && !aZombieDef.mAlmanacName.empty()) {
		aName = aZombieDef.mAlmanacName;
	} else {
		aName = ZombieHasSilhouette(mSelectedZombie) ? "???" : StrFormat("[%s]", aZombieDef.mZombieName.c_str());
	}
	TodDrawString(g, aName, 613, 362, Sexy::FONT_DWARVENTODCRAFT18GREENINSET, Color(190, 255, 235, 255), DS_ALIGN_CENTER);

	std::string aDescription;
	DrawStringJustification aAlign;
	if (ZombieHasDescription(mSelectedZombie))
	{
		// Mod API: 自定义僵尸用 mAlmanacDescription
		if (IsCustomZombieType(mSelectedZombie) && !aZombieDef.mAlmanacDescription.empty()) {
			aDescription = aZombieDef.mAlmanacDescription;
		} else {
			aDescription = TodStringTranslate(StrFormat("[%s_DESCRIPTION]", aZombieDef.mZombieName.c_str()));
		}
		aAlign = DS_ALIGN_LEFT;
	}
	else
	{
		aDescription = "[NOT_ENCOUNTERED_YET]";
		aAlign = DS_ALIGN_CENTER_VERTICAL_MIDDLE;
	}
	for (TodStringListFormat& aFormat : gLawnStringFormats)
	{
		if (TestBit(aFormat.mFormatFlags, TodStringFormatFlag::TOD_FORMAT_HIDE_UNTIL_MAGNETSHROOM))
		{
			if (mApp->HasSeedType(SeedType::SEED_MAGNETSHROOM))
			{
				aFormat.mNewColor.mAlpha = 255;
				aFormat.mLineSpacingOffset = 0;
			}
			else
			{
				aFormat.mNewColor.mAlpha = 0;
				aFormat.mLineSpacingOffset = -17;
			}
		}
	}
	// todo @Patoke: fix stuff that have another formatter after them, ex: "{KEYWORD}Weakness:{STAT} fume-shroom{METAL} and magnet-shroom{KEYWORD}" (magnet-shroom will show with the {KEYWORD} colors)
	// @Patoke: added extra check for the zamboni zombie
	TodDrawStringWrapped(g, aDescription, Rect(484, mSelectedZombie == ZombieType::ZOMBIE_ZAMBONI ? 372 : 377, 258, 170), Sexy::FONT_BRIANNETOD12, Color(40, 50, 90), aAlign);
}

void AlmanacDialog::Draw(Graphics* g)
{
	g->SetLinearBlend(true);
	switch (mOpenPage)
	{
	case AlmanacPage::ALMANAC_PAGE_INDEX:	DrawIndex(g);	break;
	case AlmanacPage::ALMANAC_PAGE_PLANTS:	DrawPlants(g);	break;
	case AlmanacPage::ALMANAC_PAGE_ZOMBIES:	DrawZombies(g);	break;
	}

	for (Zombie* aZombie : mZombiePerfTest)
	{
		if (aZombie)
		{
			Graphics aTestGraphics = Graphics(*g);
			aZombie->Draw(&aTestGraphics);
		}
	}

	mCloseButton->Draw(g);
	mIndexButton->Draw(g);
	mPlantButton->Draw(g);
	mZombieButton->Draw(g);
	// Mod API: 翻页按钮 + 页码绘制
	if (mOpenPage == ALMANAC_PAGE_PLANTS && GetPlantPageCount() > 1) {
		if (mPrevButton) { mPrevButton->mBtnNoDraw = false; mPrevButton->Draw(g); }
		if (mNextButton) { mNextButton->mBtnNoDraw = false; mNextButton->Draw(g); }
		DrawPageNumber(g, mPlantPage + 1, GetPlantPageCount());
	} else if (mOpenPage == ALMANAC_PAGE_ZOMBIES && GetZombiePageCount() > 1) {
		if (mPrevButton) { mPrevButton->mBtnNoDraw = false; mPrevButton->Draw(g); }
		if (mNextButton) { mNextButton->mBtnNoDraw = false; mNextButton->Draw(g); }
		DrawPageNumber(g, mZombiePage + 1, GetZombiePageCount());
	} else {
		if (mPrevButton) mPrevButton->mBtnNoDraw = true;
		if (mNextButton) mNextButton->mBtnNoDraw = true;
	}
}

void AlmanacDialog::GetSeedPosition(SeedType theSeedType, int& x, int& y)
{
	if (theSeedType == SeedType::SEED_IMITATER)
	{
		x = 20, y = 23;
	}
	else
	{
		// Mod API: 自定义植物的 SeedType 可能 >= NUM_ALMANAC_SEEDS，
		// 仍用 % 8 / / 8 的网格布局，从 Imitater 旁的位置往下排列
		int idx = static_cast<int>(theSeedType);
		x = idx % 8 * 52 + 26;
		y = idx / 8 * 78 + 92;
	}
}

// Mod API: 按页内索引获取网格位置（用于分页绘制，保持原版布局）
// 第 0 页时 pageOffset 等于原版 idx，布局完全一致；翻页后自定义植物复用相同网格
void AlmanacDialog::GetSeedPositionByIndex(int pageOffset, int& x, int& y)
{
	if (pageOffset == static_cast<int>(SeedType::SEED_IMITATER))
	{
		// Imitater 在第 0 页的固定位置
		x = 20, y = 23;
	}
	else
	{
		x = pageOffset % 8 * 52 + 26;
		y = pageOffset / 8 * 78 + 92;
	}
}

SeedType AlmanacDialog::SeedHitTest(int x, int y)
{
	if (mMouseVisible && mOpenPage == AlmanacPage::ALMANAC_PAGE_PLANTS)
	{
		// Mod API: 分页命中测试——只检测当前页的植物
		int totalSeeds = GetTotalPlantCount();
		int pageStart = GetPlantPageStart();
		int pageEnd = std::min(pageStart + ALMANAC_PLANTS_PER_PAGE, totalSeeds);
		for (int aSeedIdx = pageStart; aSeedIdx < pageEnd; aSeedIdx++)
		{
			SeedType aSeedType = static_cast<SeedType>(aSeedIdx);
			if (mApp->HasSeedType(aSeedType))
			{
				int aSeedX, aSeedY;
				int pageOffset = aSeedIdx - pageStart;
				GetSeedPositionByIndex(pageOffset, aSeedX, aSeedY);
				Rect aSeedRect = aSeedType == SeedType::SEED_IMITATER ? Rect(aSeedX, aSeedY, 34, 46) : Rect(aSeedX, aSeedY, SEED_PACKET_WIDTH, SEED_PACKET_HEIGHT);
				if (aSeedRect.Contains(x, y)) return aSeedType;
			}
		}
	}
	return SeedType::SEED_NONE;
}

bool AlmanacDialog::ZombieHasSilhouette(ZombieType theZombieType)
{
	// 除雪人僵尸以外的其他僵尸，或者雪人僵尸已经可以刷出（已经到达或完成冒险模式二周目 4-10 关卡），则不会显示为剪影
	if (theZombieType != ZombieType::ZOMBIE_YETI || mApp->CanSpawnYetis())
		return false;

	// 排除上述情况后，若已完成雪人僵尸出现的关卡（冒险模式一周目 4-10 关卡），则雪人僵尸显示为剪影
	return mApp->HasFinishedAdventure() || mApp->mPlayerInfo->GetLevel() > GetZombieDefinition(ZombieType::ZOMBIE_YETI).mStartingLevel;
}

// GOTY @Patoke: 0x404C50
bool AlmanacDialog::ZombieIsShown(ZombieType theZombieType)
{
	// Mod API: 自定义僵尸默认在图鉴中显示（无解锁进度限制）
	if (IsCustomZombieType(theZombieType)) return true;

	// 试玩模式下，仅展示潜水僵尸及其之前出现的僵尸
	if (mApp->IsTrialStageLocked() && theZombieType > ZombieType::ZOMBIE_SNORKEL)
		return false;

	// 对于雪人僵尸，要求其可以在刷怪中出现（已经到达或完成冒险模式二周目 4-10 关卡），
	// 或已得知其存在但未解锁其形象（已经完成冒险模式一周目 4-10 关卡，但未到达二周目 4-10 关卡）
	if (theZombieType == ZombieType::ZOMBIE_YETI)
		return mApp->CanSpawnYetis() || ZombieHasSilhouette(ZombieType::ZOMBIE_YETI);

	// 对于冒险模式中出现的僵尸
	if (theZombieType <= ZombieType::ZOMBIE_BOSS)
	{
		// 冒险模式一周目完成后，图鉴展示所有僵尸
		if (mApp->HasFinishedAdventure())
			return true;

		int aLevel = mApp->mPlayerInfo->GetLevel();
		int aStart = GetZombieDefinition(theZombieType).mStartingLevel;
		// 要求已经达到僵尸首次出现的关卡
		// 对于不能通过自然刷怪出现的僵尸（小鬼僵尸、雪橇僵尸小队、伴舞僵尸），额外要求已通过其首次出现的关卡或已击败过该僵尸
		return aStart <= aLevel && (aStart != aLevel || !Board::IsZombieTypeSpawnedOnly(theZombieType) || gZombieDefeated_Get(theZombieType));
	}

	return false;
}

// GOTY @Patoke: 0x404D50
bool AlmanacDialog::ZombieHasDescription(ZombieType theZombieType)
{
	// Mod API: 自定义僵尸默认有描述（用 mAlmanacDescription）
	if (IsCustomZombieType(theZombieType)) return true;

	int aLevel = mApp->mPlayerInfo->GetLevel();
	int aStart = GetZombieDefinition(theZombieType).mStartingLevel;

	// 对于雪人僵尸
	if (theZombieType == ZombieType::ZOMBIE_YETI)
	{
		// 当雪人僵尸不可在刷怪中出现时（冒险模式二周目 4-10 关卡之前），不显示僵尸描述
		if (!mApp->CanSpawnYetis())
			return false;
		// 从第三周目开始，总是显示雪人僵尸的描述
		if (mApp->mPlayerInfo->mFinishedAdventure >= 2)
			return true;
	}
	// 对于雪人僵尸外的其他僵尸，当冒险模式已完成时，总是显示僵尸的描述
	else if (mApp->HasFinishedAdventure())
		return true;

	// 雪人僵尸在二周目 4-10 关卡至三周目之间，或其他僵尸在冒险模式一周目中的情况，
	// 要求已经达到僵尸首次出现的关卡，且已通过其首次出现的关卡或已击败过该僵尸
	return aStart <= aLevel && (aStart != aLevel || gZombieDefeated_Get(theZombieType));
}

void AlmanacDialog::GetZombiePosition(ZombieType theZombieType, int& x, int& y)
{
	if (theZombieType == ZombieType::ZOMBIE_BOSS)
	{
		x = 192, y = 486;
	}
	else
	{
		// Mod API: 自定义僵尸用与原版相同的网格布局
		int idx = static_cast<int>(theZombieType);
		x = idx % 5 * 85 + 22;
		y = idx / 5 * 80 + 86;
	}
}

// Mod API: 按页内索引获取僵尸网格位置（用于分页绘制，保持原版布局）
// BOSS 在每页的固定位置（pageOffset == NUM_ALMANAC_ZOMBIES-1）
void AlmanacDialog::GetZombiePositionByIndex(int pageOffset, int& x, int& y)
{
	// BOSS 僵尸固定在最后一个位置 (192, 486)
	if (pageOffset == ALMANAC_ZOMBIES_PER_PAGE - 1)
	{
		x = 192, y = 486;
	}
	else
	{
		x = pageOffset % 5 * 85 + 22;
		y = pageOffset / 5 * 80 + 86;
	}
}

// GOTY @Patoke: 0x404DD0
ZombieType AlmanacDialog::ZombieHitTest(int x, int y)
{
	if (mMouseVisible && mOpenPage == AlmanacPage::ALMANAC_PAGE_ZOMBIES)
	{
		// Mod API: 分页命中测试——只检测当前页的僵尸
		int totalZombies = GetTotalZombieCount();
		int pageStart = GetZombiePageStart();
		int pageEnd = std::min(pageStart + ALMANAC_ZOMBIES_PER_PAGE, totalZombies);
		for (int i = pageStart; i < pageEnd; i++)
		{
			ZombieType aZombieType = GetZombieType(i);
			// @Patoke: added IsShown check
			if (aZombieType != ZombieType::ZOMBIE_INVALID && ZombieIsShown(aZombieType))
			{
				int aZombieX, aZombieY;
				int pageOffset = i - pageStart;
				GetZombiePositionByIndex(pageOffset, aZombieX, aZombieY);
				if (Rect(aZombieX, aZombieY, 76, 76).Contains(x, y))
					return aZombieType;
			}
		}
	}
	return ZombieType::ZOMBIE_INVALID;
}

void AlmanacDialog::MouseUp(int x, int y, int theClickCount)
{
	(void)x;(void)y;(void)theClickCount;
	if (mPlantButton->IsMouseOver())		SetPage(ALMANAC_PAGE_PLANTS);
	else if (mZombieButton->IsMouseOver())	SetPage(ALMANAC_PAGE_ZOMBIES);
	else if (mCloseButton->IsMouseOver())	mApp->KillAlmanacDialog();
	else if (mIndexButton->IsMouseOver())	SetPage(ALMANAC_PAGE_INDEX);
	// Mod API: 翻页按钮处理
	else if (mPrevButton && !mPrevButton->mBtnNoDraw && mPrevButton->IsMouseOver())
	{
		if (mOpenPage == ALMANAC_PAGE_PLANTS)     PrevPlantPage();
		else if (mOpenPage == ALMANAC_PAGE_ZOMBIES) PrevZombiePage();
		mApp->PlaySample(Sexy::SOUND_TAP);
	}
	else if (mNextButton && !mNextButton->mBtnNoDraw && mNextButton->IsMouseOver())
	{
		if (mOpenPage == ALMANAC_PAGE_PLANTS)     NextPlantPage();
		else if (mOpenPage == ALMANAC_PAGE_ZOMBIES) NextZombiePage();
		mApp->PlaySample(Sexy::SOUND_TAP);
	}
}

// GOTY @Patoke: 0x404F10
void AlmanacDialog::MouseDown(int x, int y, int theClickCount)
{
	(void)theClickCount;
	if (mPlantButton->IsMouseOver() || mCloseButton->IsMouseOver() || mIndexButton->IsMouseOver())
		mApp->PlaySample(Sexy::SOUND_TAP);
	if (mZombieButton->IsMouseOver())
		mApp->PlaySample(Sexy::SOUND_GRAVEBUTTON);
	// Mod API: 翻页按钮点击声
	if (mPrevButton && !mPrevButton->mBtnNoDraw && mPrevButton->IsMouseOver())
		mApp->PlaySample(Sexy::SOUND_TAP);
	if (mNextButton && !mNextButton->mBtnNoDraw && mNextButton->IsMouseOver())
		mApp->PlaySample(Sexy::SOUND_TAP);

	SeedType aSeedType = SeedHitTest(x, y);
	if (aSeedType != SeedType::SEED_NONE && aSeedType != mSelectedSeed)
	{
		mSelectedSeed = aSeedType;
		SetupPlant();
		mApp->PlaySample(Sexy::SOUND_TAP);
	}
	ZombieType aZombieType = ZombieHitTest(x, y);
	if (aZombieType != ZombieType::ZOMBIE_INVALID && aZombieType != mSelectedZombie)
	{
		mSelectedZombie = aZombieType;
		SetupZombie();
		mApp->PlaySample(Sexy::SOUND_TAP);
	}
}

void AlmanacDialog::KeyDown(KeyCode theKey)
{
	if (theKey == KeyCode::KEYCODE_ESCAPE)
	{
		if (mOpenPage == AlmanacPage::ALMANAC_PAGE_INDEX)
			mApp->KillAlmanacDialog();
		else
			SetPage(AlmanacPage::ALMANAC_PAGE_INDEX);
		return;
	}

	LawnDialog::KeyDown(theKey);
}

// ====== Mod API: 分页辅助方法 ======

int AlmanacDialog::GetPlantPageCount() const
{
	int total = GetTotalPlantCount();
	return (total + ALMANAC_PLANTS_PER_PAGE - 1) / ALMANAC_PLANTS_PER_PAGE;
}

int AlmanacDialog::GetZombiePageCount() const
{
	int total = GetTotalZombieCount();
	return (total + ALMANAC_ZOMBIES_PER_PAGE - 1) / ALMANAC_ZOMBIES_PER_PAGE;
}

int AlmanacDialog::GetPlantPageStart() const
{
	return mPlantPage * ALMANAC_PLANTS_PER_PAGE;
}

int AlmanacDialog::GetZombiePageStart() const
{
	return mZombiePage * ALMANAC_ZOMBIES_PER_PAGE;
}

void AlmanacDialog::PrevPlantPage()
{
	int pages = GetPlantPageCount();
	if (pages <= 1) return;
	mPlantPage = (mPlantPage - 1 + pages) % pages;
	// 切页后选中第一个植物
	mSelectedSeed = static_cast<SeedType>(GetPlantPageStart());
	SetupPlant();
}

void AlmanacDialog::NextPlantPage()
{
	int pages = GetPlantPageCount();
	if (pages <= 1) return;
	mPlantPage = (mPlantPage + 1) % pages;
	mSelectedSeed = static_cast<SeedType>(GetPlantPageStart());
	SetupPlant();
}

void AlmanacDialog::PrevZombiePage()
{
	int pages = GetZombiePageCount();
	if (pages <= 1) return;
	mZombiePage = (mZombiePage - 1 + pages) % pages;
	mSelectedZombie = GetZombieType(GetZombiePageStart());
	SetupZombie();
}

void AlmanacDialog::NextZombiePage()
{
	int pages = GetZombiePageCount();
	if (pages <= 1) return;
	mZombiePage = (mZombiePage + 1) % pages;
	mSelectedZombie = GetZombieType(GetZombiePageStart());
	SetupZombie();
}

void AlmanacDialog::UpdatePageButtons()
{
	// 当前仅控制可见性，实际可见性在 Draw 中根据页数动态设置
	// 此方法预留供未来扩展（如禁用边界按钮）
}

void AlmanacDialog::DrawPageNumber(Graphics* g, int current, int total)
{
	// 在 Prev 和 Next 按钮之间绘制页码 "current/total"
	if (!mPrevButton || !mNextButton || mPrevButton->mBtnNoDraw) return;
	std::string pageStr = StrFormat("%d/%d", current, total);
	int textW = Sexy::FONT_BRIANNETOD12->StringWidth(pageStr);
	// Prev 右边界和 Next 左边界之间的中心
	int prevRight = mPrevButton->mX + mPrevButton->mWidth;
	int nextLeft  = mNextButton->mX;
	int centerX = (prevRight + nextLeft) / 2;
	int posY = mPrevButton->mY + (mPrevButton->mHeight - 12) / 2;
	TodDrawString(g, pageStr, centerX, posY, Sexy::FONT_BRIANNETOD12, Color(42, 42, 90), DrawStringJustification::DS_ALIGN_CENTER);
}

void AlmanacInitForPlayer()
{
	for (int i = 0; i < ZombieType::NUM_ZOMBIE_TYPES; i++)
		gZombieDefeatedBuiltin[i] = false;
	// Mod API: 同时清空自定义僵尸的击败记录
	gZombieDefeatedCustom.clear();
}

void AlmanacPlayerDefeatedZombie(ZombieType theZombieType)
{
	gZombieDefeated_Set(theZombieType, true);
}
