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

#pragma once

#include <cstdint>

#include "../ConstEnums.h"
#include "../SexyAppFramework/graphics/Graphics.h"

using namespace Sexy;

class LawnApp;
class Board;

class GameObject
{
public:
	LawnApp*                        mApp;
	Board*                          mBoard;
    int32_t                         mX;
    int32_t                         mY;
    int32_t                         mWidth;
    int32_t                         mHeight;
    bool                            mVisible;
    int32_t                         mRow;
    int32_t                         mRenderOrder;

public:
    /*inline*/                      GameObject();
    /*inline*/ bool                 BeginDraw(Graphics* g);
    /*inline*/ void                 EndDraw(Graphics* g);
    /*inline*/ void                 MakeParentGraphicsFrame(Graphics* g);

    // Mod API: 全局唯一的实例 ID，用于 Lua mod 跨帧保存 per-instance 状态。
    // DataArray 回收死亡对象内存后地址会复用，用原始指针做 table key 会导致
    // 新对象继承旧对象的状态。mInstanceId 在构造时从全局计数器分配，永不复用。
    uint32_t                        mInstanceId = 0;
};
