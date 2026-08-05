/*
 * Portions of this file are based on the PopCap Games Framework
 * Copyright (C) 2005-2009 PopCap Games, Inc.
 *
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later AND LicenseRef-PopCap
 *
 * SysFont: 基于 stb_truetype 的矢量字体实现，从 TTF/OTF 文件加载，
 * 按需光栅化字符到 MemoryImage 缓存。支持任意 Unicode 字符（含中文）。
 * 与 ImageFont（位图字体）并存，由 mod/API 决定使用哪种字体。
 */

#ifndef __SYSFONT_H__
#define __SYSFONT_H__

#include "Font.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace Sexy
{

class SexyAppBase;
class MemoryImage;
class Graphics;

class SysFont : public _Font
{
public:
	// 从 TTF/OTF 文件加载系统字体
	// thePath: 相对资源目录的 TTF 文件路径（如 "fonts/SourceHanSansCN.ttf"）
	// theSize: 字号（像素高度）
	// theBold/theItalic/theShadow/theUnderline: 样式标志（暂未实现，预留）
	SysFont(SexyAppBase* theApp, const std::string& thePath, int theSize,
		bool theBold = false, bool theItalic = false,
		bool theShadow = false, bool theUnderline = false);
	~SysFont() override;

	// _Font 接口实现
	int			StringWidth(std::string_view theString) override;
	int			CharWidth(char32_t theChar) override;
	int			CharWidthKern(char32_t theChar, char32_t thePrevChar) override;
	void		DrawString(Graphics* g, int theX, int theY, std::string_view theString,
				const Color& theColor, const Rect& theClipRect) override;
	_Font*		Duplicate() override;

	// 是否成功加载 TTF 文件
	bool		IsValid() const { return mValid; }

private:
	// 字符 glyph 缓存
	struct Glyph
	{
		MemoryImage*	image;			// 光栅化后的字符位图（白色 + alpha）
		int				width;			// 字符前进宽度（advance width）
		int				leftBearing;	// 左侧 bearing
		int				topOffset;		// 相对于基线的垂直偏移（向上为负）
		int				bitmapW;
		int				bitmapH;
	};

	// 获取或光栅化一个字符的 glyph
	const Glyph&	GetGlyph(char32_t theChar);

	// 加载 TTF 文件
	bool			LoadTTF(const std::string& thePath);

	SexyAppBase*				mApp;
	std::vector<uint8_t>		mFontData;		// TTF 文件原始字节
	void*						mStbFontInfo;	// stbtt_fontinfo*（用 void* 避免头文件泄漏）
	float						mScale;			// 像素高度对应的缩放系数
	int							mAscent;		// 字体 ascent（像素）
	int							mDescent;		// 字体 descent（像素）
	int							mLineGap;		// 行间距（像素）
	bool						mValid;

	std::unordered_map<char32_t, Glyph>	mGlyphCache;

	// 空字符（缺失字符）的默认 glyph
	static Glyph					sEmptyGlyph;
};

}

#endif //__SYSFONT_H__
