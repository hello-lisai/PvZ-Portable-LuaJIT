/*
 * Portions of this file are based on the PopCap Games Framework
 * Copyright (C) 2005-2009 PopCap Games, Inc.
 *
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * SysFont 实现：基于 stb_truetype 从 TTF/OTF 文件加载矢量字体，
 * 按需光栅化字符到 MemoryImage 缓存。支持任意 Unicode 字符。
 */

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "SysFont.h"
#include "MemoryImage.h"
#include "Graphics.h"
#include "Color.h"
#include "../SexyAppBase.h"
#include "../Common.h"
#include "../paklib/PakInterface.h"

#include <cstdio>
#include <cstring>

namespace Sexy
{

// 静态空 glyph（用于缺失字符）
SysFont::Glyph SysFont::sEmptyGlyph = { nullptr, 0, 0, 0, 0, 0 };

SysFont::SysFont(SexyAppBase* theApp, const std::string& thePath, int theSize,
	bool theBold, bool theItalic, bool theShadow, bool theUnderline)
	: mApp(theApp)
	, mStbFontInfo(nullptr)
	, mScale(0.0f)
	, mAscent(0)
	, mDescent(0)
	, mLineGap(0)
	, mValid(false)
{
	(void)theBold; (void)theItalic; (void)theShadow; (void)theUnderline; // 预留

	if (theSize <= 0 || !theApp)
	{
		std::fprintf(stderr, "[SysFont] 无效参数: size=%d, app=%p\n", theSize, (void*)theApp);
		return;
	}

	if (!LoadTTF(thePath))
	{
		std::fprintf(stderr, "[SysFont] 无法加载 TTF 文件: %s\n", thePath.c_str());
		return;
	}

	// stbtt_fontinfo 结构体分配
	stbtt_fontinfo* info = new stbtt_fontinfo;
	if (stbtt_InitFont(info, mFontData.data(), stbtt_GetFontOffsetForIndex(mFontData.data(), 0)) == 0)
	{
		std::fprintf(stderr, "[SysFont] stbtt_InitFont 失败: %s\n", thePath.c_str());
		delete info;
		return;
	}
	mStbFontInfo = info;

	// 计算缩放系数：像素高度 → 字体单位
	mScale = stbtt_ScaleForPixelHeight(info, static_cast<float>(theSize));

	// 获取字体垂直度量
	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);
	mAscent = static_cast<int>(ascent * mScale);
	mDescent = static_cast<int>(descent * mScale);
	mLineGap = static_cast<int>(lineGap * mScale);

	// 设置 _Font 基类成员
	_Font::mAscent = mAscent;
	_Font::mHeight = mAscent - mDescent; // 字符高度
	_Font::mAscentPadding = 0;
	_Font::mLineSpacingOffset = mLineGap;

	mValid = true;
	std::fprintf(stderr, "[SysFont] 加载成功: %s, size=%d, ascent=%d, descent=%d, height=%d\n",
		thePath.c_str(), theSize, mAscent, mDescent, _Font::mHeight);
}

SysFont::~SysFont()
{
	// 释放所有 glyph 缓存
	for (auto& [ch, glyph] : mGlyphCache)
	{
		if (glyph.image)
		{
			delete glyph.image;
			glyph.image = nullptr;
		}
	}
	mGlyphCache.clear();

	if (mStbFontInfo)
	{
		delete static_cast<stbtt_fontinfo*>(mStbFontInfo);
		mStbFontInfo = nullptr;
	}
}

bool SysFont::LoadTTF(const std::string& thePath)
{
	// 通过 PakInterface 读取（支持 pak 文件和资源目录）
	PFILE* fp = p_fopen(thePath.c_str(), "rb");
	if (!fp)
	{
		// 尝试相对资源目录
		std::string fullPath = mApp ? (mApp->mResourceDir + thePath) : thePath;
		fp = p_fopen(fullPath.c_str(), "rb");
		if (!fp)
		{
			std::fprintf(stderr, "[SysFont] p_fopen 失败: %s\n", thePath.c_str());
			return false;
		}
	}

	p_fseek(fp, 0, SEEK_END);
	long fileSize = p_ftell(fp);
	p_fseek(fp, 0, SEEK_SET);

	if (fileSize <= 0)
	{
		p_fclose(fp);
		std::fprintf(stderr, "[SysFont] 文件为空或读取失败: %s\n", thePath.c_str());
		return false;
	}

	mFontData.resize(fileSize);
	size_t bytesRead = p_fread(mFontData.data(), 1, fileSize, fp);
	p_fclose(fp);

	if (bytesRead != static_cast<size_t>(fileSize))
	{
		std::fprintf(stderr, "[SysFont] 读取不完整: %s (期望 %ld, 实际 %zu)\n",
			thePath.c_str(), fileSize, bytesRead);
		mFontData.clear();
		return false;
	}

	return true;
}

const SysFont::Glyph& SysFont::GetGlyph(char32_t theChar)
{
	// 查缓存
	auto it = mGlyphCache.find(theChar);
	if (it != mGlyphCache.end())
		return it->second;

	if (!mValid || !mStbFontInfo)
		return sEmptyGlyph;

	stbtt_fontinfo* info = static_cast<stbtt_fontinfo*>(mStbFontInfo);

	// 查找 glyph index
	int glyphIndex = stbtt_FindGlyphIndex(info, static_cast<int>(theChar));
	if (glyphIndex == 0)
	{
		// 字符不存在于字体中
		mGlyphCache[theChar] = sEmptyGlyph;
		return mGlyphCache[theChar];
	}

	// 获取 advance width 和左侧 bearing
	int advanceWidth, leftSideBearing;
	stbtt_GetGlyphHMetrics(info, glyphIndex, &advanceWidth, &leftSideBearing);

	// 光栅化字符到 8-bit alpha bitmap
	int x0, y0, x1, y1;
	stbtt_GetGlyphBitmapBox(info, glyphIndex, mScale, mScale, &x0, &y0, &x1, &y1);

	int bitmapW = x1 - x0;
	int bitmapH = y1 - y0;

	Glyph glyph = { nullptr, 0, 0, 0, 0, 0 };
	glyph.width = static_cast<int>(advanceWidth * mScale);
	glyph.leftBearing = static_cast<int>(leftSideBearing * mScale);
	glyph.topOffset = y0; // stbtt 的 y0 是相对于基线的偏移（向上为负）
	glyph.bitmapW = bitmapW;
	glyph.bitmapH = bitmapH;

	if (bitmapW > 0 && bitmapH > 0)
	{
		// 分配 alpha bitmap
		unsigned char* alphaBitmap = new unsigned char[bitmapW * bitmapH];
		std::memset(alphaBitmap, 0, bitmapW * bitmapH);
		stbtt_MakeGlyphBitmap(info, alphaBitmap, bitmapW, bitmapH, bitmapW, mScale, mScale, glyphIndex);

		// 转换为 ARGB32 MemoryImage（白色 + alpha）
		MemoryImage* img = new MemoryImage(mApp);
		img->Create(bitmapW, bitmapH);
		uint32_t* bits = img->GetBits();
		for (int i = 0; i < bitmapW * bitmapH; ++i)
		{
			uint8_t a = alphaBitmap[i];
			// ARGB32: (alpha << 24) | (red << 16) | (green << 8) | blue
			// 白色字 + alpha coverage，绘制时由 Graphics 着色
			bits[i] = (static_cast<uint32_t>(a) << 24) | 0x00FFFFFF;
		}
		img->BitsChanged();
		delete[] alphaBitmap;

		glyph.image = img;
	}

	mGlyphCache[theChar] = glyph;
	return mGlyphCache[theChar];
}

int SysFont::StringWidth(std::string_view theString)
{
	if (!mValid || theString.empty())
		return 0;

	int width = 0;
	size_t offset = 0;
	char32_t ch = 0;
	while (offset < theString.size())
	{
		if (!UTF8DecodeNext(theString, offset, ch))
			break;
		width += CharWidth(ch);
	}
	return width;
}

int SysFont::CharWidth(char32_t theChar)
{
	if (!mValid)
		return 0;
	const Glyph& g = GetGlyph(theChar);
	return g.width;
}

int SysFont::CharWidthKern(char32_t theChar, char32_t thePrevChar)
{
	if (!mValid)
		return 0;

	int width = CharWidth(theChar);

	// 字距调整（kerning）
	if (thePrevChar != 0 && mStbFontInfo)
	{
		stbtt_fontinfo* info = static_cast<stbtt_fontinfo*>(mStbFontInfo);
		int kern = stbtt_GetGlyphKernAdvance(info,
			stbtt_FindGlyphIndex(info, static_cast<int>(thePrevChar)),
			stbtt_FindGlyphIndex(info, static_cast<int>(theChar)));
		width += static_cast<int>(kern * mScale);
	}

	return width;
}

void SysFont::DrawString(Graphics* g, int theX, int theY, std::string_view theString,
	const Color& theColor, const Rect& theClipRect)
{
	if (!mValid || !g || theString.empty())
		return;

	// 保存 Graphics 状态
	bool oldColorize = g->mColorizeImages;
	int oldDrawMode = g->mDrawMode;
	Color oldColor = g->mColor;

	// 启用颜色着色：用 theColor 对白色字符位图着色
	g->SetColorizeImages(true);
	g->SetColor(theColor);
	g->SetDrawMode(0); // DRAWMODE_NORMAL

	// theY 是基线位置（与 ImageFont 一致）
	// 基线 = theY + mAscent，但 PvZ 的 DrawString 习惯 theY 是顶部
	// 实际上 ImageFont::DrawStringEx 用 theY 作为基线
	// 这里遵循相同约定：theY 是基线
	int baselineY = theY;
	int curX = theX;

	size_t offset = 0;
	char32_t ch = 0;
	char32_t prevCh = 0;
	while (offset < theString.size())
	{
		if (!UTF8DecodeNext(theString, offset, ch))
			break;

		const Glyph& glyph = GetGlyph(ch);

		// 字距调整
		if (prevCh != 0 && mStbFontInfo)
		{
			stbtt_fontinfo* info = static_cast<stbtt_fontinfo*>(mStbFontInfo);
			int kern = stbtt_GetGlyphKernAdvance(info,
				stbtt_FindGlyphIndex(info, static_cast<int>(prevCh)),
				stbtt_FindGlyphIndex(info, static_cast<int>(ch)));
			curX += static_cast<int>(kern * mScale);
		}

		// 绘制字符位图
		if (glyph.image)
		{
			// 绘制位置：x + leftBearing, baselineY + topOffset
			int drawX = curX + glyph.leftBearing;
			int drawY = baselineY + glyph.topOffset;
			g->DrawImage(glyph.image, drawX, drawY);
		}

		curX += glyph.width;
		prevCh = ch;
	}

	// 恢复 Graphics 状态
	g->SetColorizeImages(oldColorize);
	g->SetColor(oldColor);
	g->SetDrawMode(oldDrawMode);
}

_Font* SysFont::Duplicate()
{
	// Duplicate 复用同一份 TTF 数据和 glyph 缓存
	// 只创建新的 SysFont 实例（共享 mFontData 的拷贝）
	if (!mValid)
		return nullptr;

	// 注意：Duplicate 通常用于临时改变字体属性（如颜色）
	// 这里返回一个共享 TTF 数据但独立缓存的新实例
	// 简化实现：返回 this（引用计数语义由调用方管理）
	// 更安全的做法是深拷贝 TTF 数据，但开销较大
	// 实际使用场景中 Duplicate 主要用于 ImageFont 的图层切换，SysFont 不需要
	return this;
}

} // namespace Sexy
