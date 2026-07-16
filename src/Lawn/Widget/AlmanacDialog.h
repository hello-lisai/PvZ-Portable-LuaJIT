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

#ifndef __ALMANACDIALOG_H__
#define __ALMANACDIALOG_H__

#include "LawnDialog.h"

#define NUM_ALMANAC_SEEDS 49
#define NUM_ALMANAC_ZOMBIES 26
// Mod API: 图鉴分页，每页显示的植物/僵尸数量
#define ALMANAC_PLANTS_PER_PAGE  NUM_ALMANAC_SEEDS
#define ALMANAC_ZOMBIES_PER_PAGE NUM_ALMANAC_ZOMBIES

constexpr const float			ALMANAC_PLANT_POSITION_X		= 578.0f;
constexpr const float			ALMANAC_PLANT_POSITION_Y		= 140.0f;
constexpr const float			ALMANAC_ZOMBIE_POSITION_X		= 559.0f;
constexpr const float			ALMANAC_ZOMBIE_POSITION_Y		= 175.0f;
constexpr const int				ALMANAC_INDEXPLANT_POSITION_X	= 167;
constexpr const int				ALMANAC_INDEXPLANT_POSITION_Y	= 255;
constexpr const float			ALMANAC_INDEXZOMBIE_POSITION_X	= 535.0f;
constexpr const float			ALMANAC_INDEXZOMBIE_POSITION_Y	= 215.0f;

class Plant;
class Zombie;
class LawnApp;
class GameButton;
class Reanimation;
class AlmanacDialog : public LawnDialog
{
private:
	enum
	{
		ALMANAC_BUTTON_CLOSE = 0,
		ALMANAC_BUTTON_PLANT = 1,
		ALMANAC_BUTTON_ZOMBIE = 2,
		ALMANAC_BUTTON_INDEX = 3,
		// Mod API: 翻页按钮
		ALMANAC_BUTTON_PREV = 4,
		ALMANAC_BUTTON_NEXT = 5
	};

public:
	LawnApp*					mApp;
	GameButton*					mCloseButton;
	GameButton*					mIndexButton;
	GameButton*					mPlantButton;
	GameButton*					mZombieButton;
	// Mod API: 翻页按钮（仅当有自定义植物/僵尸时显示）
	GameButton*					mPrevButton;
	GameButton*					mNextButton;
	AlmanacPage					mOpenPage;
	Reanimation*				mReanim[4];
	SeedType					mSelectedSeed;
	ZombieType					mSelectedZombie;
	Plant*						mPlant;
	Zombie*						mZombie;
	Zombie*						mZombiePerfTest[400];
	// Mod API: 分页状态
	int							mPlantPage;     // 当前植物页（0-based）
	int							mZombiePage;    // 当前僵尸页（0-based）
	
public:
	AlmanacDialog(LawnApp* theApp);
	~AlmanacDialog() override;

	void						ClearPlantsAndZombies();
	void						RemovedFromManager(WidgetManager* theWidgetManager) override;
	void						SetupPlant();
	void						SetupZombie();
	void						SetPage(AlmanacPage thePage);
	void						Update() override;
	void						DrawIndex(Graphics* g);
	void						DrawPlants(Graphics* g);
	void						DrawZombies(Graphics* g);
	void						Draw(Graphics* g) override;
	void						GetSeedPosition(SeedType theSeedType, int& x, int& y);
	SeedType					SeedHitTest(int x, int y);
	/*inline*/ bool				ZombieHasSilhouette(ZombieType theZombieType);
	bool						ZombieIsShown(ZombieType theZombieType);
	bool						ZombieHasDescription(ZombieType theZombieType);
	void						GetZombiePosition(ZombieType theZombieType, int& x, int& y);
	ZombieType					ZombieHitTest(int x, int y);
	void						MouseUp(int x, int y, int theClickCount) override;
	void						MouseDown(int x, int y, int theClickCount) override;
	void						KeyDown(KeyCode theKey) override;
	// Mod API: 分页辅助
	int							GetPlantPageCount() const;   // 植物总页数
	int							GetZombiePageCount() const;   // 僵尸总页数
	int							GetPlantPageStart() const;    // 当前页起始索引
	int							GetZombiePageStart() const;   // 当前页起始索引
	void						PrevPlantPage();
	void						NextPlantPage();
	void						PrevZombiePage();
	void						NextZombiePage();
	void						UpdatePageButtons();         // 根据页码更新按钮启用/禁用/可见状态
	void						DrawPageNumber(Graphics* g, int current, int total); // 绘制页码
	// Mod API: 按页内索引获取网格位置（分页绘制用，保持原版布局）
	void						GetSeedPositionByIndex(int pageOffset, int& x, int& y);
	void						GetZombiePositionByIndex(int pageOffset, int& x, int& y);
	// Mod API: 过滤隐藏植物（SEED_EXPLODE_O_NUT/GIANT_WALLNUT/SPROUT/LEFTPEATER 不在图鉴显示）
	static bool					IsAlmanacHiddenSeed(SeedType theSeedType);
	int							GetAlmanacVisibleSeedCount() const;  // 图鉴可见植物总数（排除隐藏植物）
	SeedType					GetAlmanacSeedByVisibleIndex(int visibleIdx) const;  // 可见索引 → SeedType
	// Mod API: 过滤隐藏僵尸（小游戏僵尸 ZOMBIE_PEA_HEAD~REDEYE_GARGANTUAR 不在图鉴显示）
	static bool					IsAlmanacHiddenZombie(ZombieType theZombieType);
	int							GetAlmanacVisibleZombieCount() const;  // 图鉴可见僵尸总数（排除隐藏僵尸）
	ZombieType					GetAlmanacZombieByVisibleIndex(int visibleIdx) const;  // 可见索引 → ZombieType
//	virtual void				KeyChar(char theChar);

	static ZombieType			GetZombieType(int theIndex);
	/*inline*/ void				ShowPlant(SeedType theSeedType);
	/*inline*/ void				ShowZombie(ZombieType theZombieType);
};

// Mod API: gZombieDefeated 改用函数接口，支持自定义僵尸类型（>= NUM_ZOMBIE_TYPES）
// 原版僵尸仍用静态数组快速查询，自定义僵尸用 unordered_map
bool gZombieDefeated_Get(ZombieType z);
void gZombieDefeated_Set(ZombieType z, bool v);

/*inline*/ void					AlmanacInitForPlayer();
/*inline*/ void					AlmanacPlayerDefeatedZombie(ZombieType theZombieType);

#endif
