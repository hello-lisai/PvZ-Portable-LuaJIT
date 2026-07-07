// BindGraphics.cpp - Graphics/Image/Font 的 Lua 绑定
// 让 mod 能在 ON_BOARD_DRAW_HUD 事件中绘制自定义内容
//
// 设计：
//   - Graphics 用 light userdata（指针仅在绘制回调期间有效，不 GC）
//   - Image/Font 同样是 light userdata，指向全局 IMAGE_*/FONT_* 指针
//   - 颜色用 0xAARRGGBB 整数表示（与游戏 Color 类兼容）

#include "LuaBindUtil.h"
#include "../../SexyAppFramework/graphics/Graphics.h"
#include "../../SexyAppFramework/graphics/Image.h"
#include "../../SexyAppFramework/graphics/Font.h"
#include "../../Sexy.TodLib/TodCommon.h"  // TodDrawString
#include "../../Resources.h"              // FONT_* / IMAGE_* 全局指针

#include <cstring>

// Graphics/Image/_Font/Color 均在 Sexy 命名空间内
using namespace Sexy;

namespace ModLua {

namespace {

// ====== Graphics 绑定 ======

Graphics* CheckGfx(lua_State* L, int idx) {
    Graphics** pp = static_cast<Graphics**>(luaL_checkudata(L, idx, MT_GRAPHICS));
    return pp ? *pp : nullptr;
}

// g:draw_string(text, x, y)
int l_gfx_draw_string(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    const char* s = luaL_checkstring(L, 2);
    int x = static_cast<int>(luaL_checkinteger(L, 3));
    int y = static_cast<int>(luaL_checkinteger(L, 4));
    g->DrawString(s, x, y);
    return 0;
}

// g:draw_string_color(text, x, y) —— 支持 ^ff0000^ 颜色标签
int l_gfx_draw_string_color(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    const char* s = luaL_checkstring(L, 2);
    int x = static_cast<int>(luaL_checkinteger(L, 3));
    int y = static_cast<int>(luaL_checkinteger(L, 4));
    g->DrawStringColor(s, x, y);
    return 0;
}

// g:string_width(text) -> int
int l_gfx_string_width(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) { lua_pushinteger(L, 0); return 1; }
    const char* s = luaL_checkstring(L, 2);
    lua_pushinteger(L, g->StringWidth(s));
    return 1;
}

// g:fill_rect(x, y, w, h)
int l_gfx_fill_rect(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    int x = static_cast<int>(luaL_checkinteger(L, 2));
    int y = static_cast<int>(luaL_checkinteger(L, 3));
    int w = static_cast<int>(luaL_checkinteger(L, 4));
    int h = static_cast<int>(luaL_checkinteger(L, 5));
    g->FillRect(x, y, w, h);
    return 0;
}

// g:draw_rect(x, y, w, h)
int l_gfx_draw_rect(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    int x = static_cast<int>(luaL_checkinteger(L, 2));
    int y = static_cast<int>(luaL_checkinteger(L, 3));
    int w = static_cast<int>(luaL_checkinteger(L, 4));
    int h = static_cast<int>(luaL_checkinteger(L, 5));
    g->DrawRect(x, y, w, h);
    return 0;
}

// g:draw_line(x1, y1, x2, y2)
int l_gfx_draw_line(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    int x1 = static_cast<int>(luaL_checkinteger(L, 2));
    int y1 = static_cast<int>(luaL_checkinteger(L, 3));
    int x2 = static_cast<int>(luaL_checkinteger(L, 4));
    int y2 = static_cast<int>(luaL_checkinteger(L, 5));
    g->DrawLine(x1, y1, x2, y2);
    return 0;
}

// g:draw_line_aa(x1, y1, x2, y2) —— 抗锯齿线
int l_gfx_draw_line_aa(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    int x1 = static_cast<int>(luaL_checkinteger(L, 2));
    int y1 = static_cast<int>(luaL_checkinteger(L, 3));
    int x2 = static_cast<int>(luaL_checkinteger(L, 4));
    int y2 = static_cast<int>(luaL_checkinteger(L, 5));
    g->DrawLineAA(x1, y1, x2, y2);
    return 0;
}

// g:draw_image(image, x, y)
int l_gfx_draw_image(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    Image** pp = static_cast<Image**>(luaL_checkudata(L, 2, MT_IMAGE));
    if (!pp || !*pp) return 0;
    int x = static_cast<int>(luaL_checkinteger(L, 3));
    int y = static_cast<int>(luaL_checkinteger(L, 4));
    g->DrawImage(*pp, x, y);
    return 0;
}

// g:draw_image_cel(image_strip, x, y, cel)
int l_gfx_draw_image_cel(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    Image** pp = static_cast<Image**>(luaL_checkudata(L, 2, MT_IMAGE));
    if (!pp || !*pp) return 0;
    int x = static_cast<int>(luaL_checkinteger(L, 3));
    int y = static_cast<int>(luaL_checkinteger(L, 4));
    int cel = static_cast<int>(luaL_checkinteger(L, 5));
    g->DrawImageCel(*pp, x, y, cel);
    return 0;
}

// g:draw_image_stretched(image, x, y, w, h)
int l_gfx_draw_image_stretched(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    Image** pp = static_cast<Image**>(luaL_checkudata(L, 2, MT_IMAGE));
    if (!pp || !*pp) return 0;
    int x = static_cast<int>(luaL_checkinteger(L, 3));
    int y = static_cast<int>(luaL_checkinteger(L, 4));
    int w = static_cast<int>(luaL_checkinteger(L, 5));
    int h = static_cast<int>(luaL_checkinteger(L, 6));
    g->DrawImage(*pp, x, y, w, h);
    return 0;
}

// g:draw_image_rotated(image, x, y, rot)
int l_gfx_draw_image_rotated(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    Image** pp = static_cast<Image**>(luaL_checkudata(L, 2, MT_IMAGE));
    if (!pp || !*pp) return 0;
    int x = static_cast<int>(luaL_checkinteger(L, 3));
    int y = static_cast<int>(luaL_checkinteger(L, 4));
    double rot = luaL_checknumber(L, 5);
    g->DrawImageRotated(*pp, x, y, rot);
    return 0;
}

// g:set_color(0xAARRGGBB)
int l_gfx_set_color(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    uint32_t c = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    g->SetColor(Color(static_cast<int32_t>(c)));
    return 0;
}

// g:set_font(font)
int l_gfx_set_font(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    _Font** pp = static_cast<_Font**>(luaL_checkudata(L, 2, MT_FONT));
    if (!pp || !*pp) return 0;
    g->SetFont(*pp);
    return 0;
}

// g:push_state() / g:pop_state()
int l_gfx_push_state(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    g->PushState();
    return 0;
}
int l_gfx_pop_state(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    g->PopState();
    return 0;
}

// g:translate(x, y)
int l_gfx_translate(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    int x = static_cast<int>(luaL_checkinteger(L, 2));
    int y = static_cast<int>(luaL_checkinteger(L, 3));
    g->Translate(x, y);
    return 0;
}

// g:set_clip_rect(x, y, w, h)
int l_gfx_set_clip_rect(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    int x = static_cast<int>(luaL_checkinteger(L, 2));
    int y = static_cast<int>(luaL_checkinteger(L, 3));
    int w = static_cast<int>(luaL_checkinteger(L, 4));
    int h = static_cast<int>(luaL_checkinteger(L, 5));
    g->SetClipRect(x, y, w, h);
    return 0;
}

// g:clear_clip_rect()
int l_gfx_clear_clip_rect(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    g->ClearClipRect();
    return 0;
}

// g:set_draw_mode(mode) —— 0=normal, 1=additive
int l_gfx_set_draw_mode(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    int mode = static_cast<int>(luaL_checkinteger(L, 2));
    g->SetDrawMode(mode);
    return 0;
}

// g:set_colorize_images(bool)
int l_gfx_set_colorize_images(lua_State* L) {
    Graphics* g = CheckGfx(L, 1); if (!g) return 0;
    g->SetColorizeImages(lua_toboolean(L, 2) != 0);
    return 0;
}

// ====== Image 绑定 ======

Image* CheckImg(lua_State* L, int idx) {
    Image** pp = static_cast<Image**>(luaL_checkudata(L, idx, MT_IMAGE));
    return pp ? *pp : nullptr;
}

// image:get_width() -> int
int l_img_get_width(lua_State* L) {
    Image* img = CheckImg(L, 1); if (!img) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, img->mWidth);
    return 1;
}

// image:get_height() -> int
int l_img_get_height(lua_State* L) {
    Image* img = CheckImg(L, 1); if (!img) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, img->mHeight);
    return 1;
}

// image:get_cel_width() -> int
int l_img_get_cel_width(lua_State* L) {
    Image* img = CheckImg(L, 1); if (!img) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, img->GetCelWidth());
    return 1;
}

// image:get_cel_height() -> int
int l_img_get_cel_height(lua_State* L) {
    Image* img = CheckImg(L, 1); if (!img) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, img->GetCelHeight());
    return 1;
}

// ====== Font 绑定 ======

_Font* CheckFont(lua_State* L, int idx) {
    _Font** pp = static_cast<_Font**>(luaL_checkudata(L, idx, MT_FONT));
    return pp ? *pp : nullptr;
}

// font:string_width(text) -> int
int l_font_string_width(lua_State* L) {
    _Font* f = CheckFont(L, 1); if (!f) { lua_pushinteger(L, 0); return 1; }
    const char* s = luaL_checkstring(L, 2);
    lua_pushinteger(L, f->StringWidth(s));
    return 1;
}

// font:get_height() -> int
int l_font_get_height(lua_State* L) {
    _Font* f = CheckFont(L, 1); if (!f) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, f->GetHeight());
    return 1;
}

// font:get_line_spacing() -> int
int l_font_get_line_spacing(lua_State* L) {
    _Font* f = CheckFont(L, 1); if (!f) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, f->GetLineSpacing());
    return 1;
}

// ====== Graphics __index 分发 ======

int l_gfx_index(lua_State* L) {
    const char* key = luaL_checkstring(L, 2);

    struct { const char* name; lua_CFunction fn; } methods[] = {
        {"draw_string",         l_gfx_draw_string},
        {"draw_string_color",   l_gfx_draw_string_color},
        {"string_width",        l_gfx_string_width},
        {"fill_rect",           l_gfx_fill_rect},
        {"draw_rect",           l_gfx_draw_rect},
        {"draw_line",           l_gfx_draw_line},
        {"draw_line_aa",        l_gfx_draw_line_aa},
        {"draw_image",          l_gfx_draw_image},
        {"draw_image_cel",      l_gfx_draw_image_cel},
        {"draw_image_stretched",l_gfx_draw_image_stretched},
        {"draw_image_rotated",  l_gfx_draw_image_rotated},
        {"set_color",           l_gfx_set_color},
        {"set_font",            l_gfx_set_font},
        {"push_state",          l_gfx_push_state},
        {"pop_state",           l_gfx_pop_state},
        {"translate",           l_gfx_translate},
        {"set_clip_rect",       l_gfx_set_clip_rect},
        {"clear_clip_rect",     l_gfx_clear_clip_rect},
        {"set_draw_mode",       l_gfx_set_draw_mode},
        {"set_colorize_images", l_gfx_set_colorize_images},
    };
    for (auto& m : methods) {
        if (strcmp(key, m.name) == 0) {
            lua_pushcfunction(L, m.fn);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

int l_img_index(lua_State* L) {
    const char* key = luaL_checkstring(L, 2);
    if (strcmp(key, "width") == 0)      return l_img_get_width(L);
    if (strcmp(key, "height") == 0)     return l_img_get_height(L);
    if (strcmp(key, "cel_width") == 0)  return l_img_get_cel_width(L);
    if (strcmp(key, "cel_height") == 0) return l_img_get_cel_height(L);
    lua_pushnil(L);
    return 1;
}

int l_font_index(lua_State* L) {
    const char* key = luaL_checkstring(L, 2);
    if (strcmp(key, "string_width") == 0)  return l_font_string_width(L);
    if (strcmp(key, "height") == 0)         return l_font_get_height(L);
    if (strcmp(key, "line_spacing") == 0)   return l_font_get_line_spacing(L);
    lua_pushnil(L);
    return 1;
}

} // namespace

// ====== 注册入口（由 LuaRuntime.cpp 调用）======

void BindGraphics(lua_State* L) {
    CreateMetatable(L, MT_GRAPHICS);
    SetFuncField(L, "__index", l_gfx_index);
    lua_pop(L, 1);

    CreateMetatable(L, MT_IMAGE);
    SetFuncField(L, "__index", l_img_index);
    lua_pop(L, 1);

    CreateMetatable(L, MT_FONT);
    SetFuncField(L, "__index", l_font_index);
    lua_pop(L, 1);
}

// 把 Graphics* push 成 Lua userdata（供事件回调使用）
void PushGraphics(lua_State* L, Graphics* g) {
    NewUserdata(L, g, MT_GRAPHICS);
}

// 把 Image* push 成 Lua userdata
void PushImage(lua_State* L, Image* img) {
    NewUserdata(L, img, MT_IMAGE);
}

// 把 _Font* push 成 Lua userdata
void PushFont(lua_State* L, _Font* f) {
    NewUserdata(L, f, MT_FONT);
}

} // namespace ModLua
