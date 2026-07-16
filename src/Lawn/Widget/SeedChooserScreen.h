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

#ifndef __SEEDCHOOSERSCREEN_H__
#define __SEEDCHOOSERSCREEN_H__

#include "../../ConstEnums.h"
#include "../../Sexy.TodLib/TodCommon.h"
#include "widget/Widget.h"
#include <vector>
using namespace Sexy;

class Board;
class LawnApp;
class GameButton;
class ToolTipWidget;
namespace Sexy
{
    class MTRand;
}

class ChosenSeed
{
public:
    int                     mX;
    int                     mY;
    int                     mTimeStartMotion;
    int                     mTimeEndMotion;
    int                     mStartX;
    int                     mStartY;
    int                     mEndX;
    int                     mEndY;
    SeedType                mSeedType;
    ChosenSeedState         mSeedState;
    int                     mSeedIndexInBank;
    bool                    mRefreshing;
    int                     mRefreshCounter;
    SeedType                mImitaterType;
    bool                    mCrazyDavePicked;
};

class SeedChooserScreen : public Widget
{
private:
    enum
    {
        SeedChooserScreen_Start = 100,
        SeedChooserScreen_Random = 101,
        SeedChooserScreen_ViewLawn = 102,
        SeedChooserScreen_Almanac = 103,
        SeedChooserScreen_Menu = 104,
        SeedChooserScreen_Store = 105,
        SeedChooserScreen_Imitater = 106,
        // Mod API: 翻页按钮
        SeedChooserScreen_PrevPage = 107,
        SeedChooserScreen_NextPage = 108
    };

public:
    GameButton*             mStartButton;
    GameButton*             mRandomButton;
    GameButton*             mViewLawnButton;
    GameButton*             mStoreButton;
    GameButton*             mAlmanacButton;
    GameButton*             mMenuButton;
    GameButton*             mImitaterButton;
    // Mod API: 翻页按钮（仅当有自定义植物且总页数 > 1 时可见）
    GameButton*             mPrevButton;
    GameButton*             mNextButton;
    // Mod API: 改为 vector 以支持运行时注册的自定义植物（SeedType >= NUM_SEED_TYPES）
    std::vector<ChosenSeed> mChosenSeeds;
    LawnApp*                mApp;
    Board*                  mBoard;
    int                     mNumSeedsToChoose;
    int                     mSeedChooserAge;
    int                     mSeedsInFlight;
    int                     mSeedsInBank;
    ToolTipWidget*          mToolTip;
    int                     mToolTipSeed;
    int                     mLastMouseX;
    int                     mLastMouseY;
    SeedChooserState        mChooseState;
    int                     mViewLawnTime;
    // Mod API: 翻页模式当前页码（从 0 开始）
    int                     mSeedPage;

public:
    SeedChooserScreen();
    ~SeedChooserScreen() override;

    static /*inline*/ int   PickFromWeightedArrayUsingSpecialRandSeed(TodWeightedArray* theArray, int theCount, MTRand& theLevelRNG);
    void                    CrazyDavePickSeeds();
    bool                    Has7Rows();
    void                    GetSeedPositionInChooser(int theIndex, int& x, int& y);
    /*inline*/ void         GetSeedPositionInBank(int theIndex, int& x, int& y);
    /*inline*/ unsigned int SeedNotRecommendedToPick(SeedType theSeedType);
    /*inline*/ bool         SeedNotAllowedToPick(SeedType theSeedType);
    /*inline*/ bool         SeedNotAllowedDuringTrial(SeedType theSeedType);
    void                    Draw(Graphics* g) override;
    void                    UpdateViewLawn();
    void                    LandFlyingSeed(ChosenSeed& theChosenSeed);
    void                    UpdateCursor();
    void                    Update() override;
    /*inline*/ bool         DisplayRepickWarningDialog(const char* theMessage);
    bool                    FlyersAreComming();
    bool                    FlyProtectionCurrentlyPlanted();
    bool                    CheckSeedUpgrade(SeedType theSeedTypeTo, SeedType theSeedTypeFrom);
    void                    OnStartButton();
    void                    PickRandomSeeds();
    virtual void            ButtonDepress(int theId);
    SeedType                SeedHitTest(int x, int y);
    SeedType                FindSeedInBank(int theIndexInBank);
    /*inline*/ void         EnableStartButton(bool theEnabled);
    void                    ClickedSeedInBank(ChosenSeed& theChosenSeed);
    void                    ClickedSeedInChooser(ChosenSeed& theChosenSeed);
    void                    ShowToolTip();
    /*inline*/ void         RemoveToolTip();
    /*inline*/ void         CancelLawnView();
    void                    MouseUp(int x, int y, int theClickCount) override;
    void                    UpdateImitaterButton();
    void                    MouseDown(int x, int y, int theClickCount) override;
    /*inline*/ bool         PickedPlantType(SeedType theSeedType);
    void                    CloseSeedChooser();
    void                    KeyDown(KeyCode theKey) override;
    void                    KeyChar(char theChar) override;
    void                    UpdateAfterPurchase();
    // Mod API: 翻页相关
    int                     GetSeedPageCount();   // 总页数
    int                     GetSeedPageStart();   // 当前页起始索引（绝对索引）
    void                    PrevSeedPage();       // 上一页
    void                    NextSeedPage();       // 下一页
    void                    UpdatePageButtons();  // 更新翻页按钮可见性/可用状态
};

#endif
