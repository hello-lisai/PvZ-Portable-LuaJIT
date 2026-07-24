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

#include "GameObject.h"
#include "../LawnApp.h"
#include <atomic>

// Mod API: 全局实例 ID 计数器。从 1 开始，0 保留给未初始化的对象。
// DataArrayAlloc 用 placement new 调用构造函数，所以每次分配都会递增。
static std::atomic<uint32_t> g_instanceIdCounter{1};

GameObject::GameObject()
{
	mApp = gLawnApp;
	mBoard = gLawnApp->mBoard;
	mX = 0;
	mY = 0;
	mWidth = 0;
	mHeight = 0;
	mVisible = true;
	mRow = -1;
	mRenderOrder = RenderLayer::RENDER_LAYER_TOP;
	mInstanceId = g_instanceIdCounter.fetch_add(1, std::memory_order_relaxed);
}

bool GameObject::BeginDraw(Graphics* g)
{
	if (!mVisible)
		return false;

	g->Translate(mX, mY);
	return true;
}

void GameObject::EndDraw(Graphics* g)
{
	g->Translate(-mX, -mY);
}

void GameObject::MakeParentGraphicsFrame(Graphics* g)
{
	g->Translate(-mX, -mY);
}
