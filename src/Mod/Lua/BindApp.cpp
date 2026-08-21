#include "LuaBindUtil.h"
#include "../../LawnApp.h"
#include "../../Sexy.TodLib/Reanimator.h"  // Reanimation / ReanimLoopType
#include "../../ConstEnums.h"              // CrazyDaveState / ReanimLoopType 枚举
#include "../../SexyAppFramework/SexyAppBase.h"  // gSexyAppBase / mStringProperties
#include "../../SexyAppFramework/graphics/Image.h"
#include "../../SexyAppFramework/graphics/MemoryImage.h"
#include "../../SexyAppFramework/imagelib/ImageLib.h"
#include "../../Lawn/System/PlayerInfo.h"  // PlayerInfo / ResetChallengeRecord
#include "LuaRuntime.h"
#include <cstdio>
#include <cstring>

namespace ModLua {

// 在 BindReanimation.cpp 中定义，这里前向声明
void PushReanimation(lua_State* L, Reanimation* r);

namespace {

// l_get_app: pvz.get_app() -> App userdata（无 app 时返回 nil）
int l_get_app(lua_State* L) {
    if (!gLawnApp) { lua_pushnil(L); return 1; }
    NewUserdata(L, gLawnApp, MT_APP);
    return 1;
}

// pvz.set_string(key, value) —— 覆盖游戏字符串表中的值
// 用于修改 UI 显示文本，如关卡名称、按钮文本等
// key 不含方括号，如 "POGO_PARTY"（对应原版 "[POGO_PARTY]"）
// value 为新文本，如 "跳跳舞会"
// 设置后所有 TodStringTranslate("[POGO_PARTY]") 调用都会返回新值
// 例：pvz.set_string("POGO_PARTY", "跳跳舞会")
int l_pvz_set_string(lua_State* L) {
    const char* key = luaL_checkstring(L, 1);
    const char* value = luaL_checkstring(L, 2);
    if (!gSexyAppBase) return 0;
    gSexyAppBase->mStringProperties[std::string(key)] = std::string(value);
    return 0;
}

// pvz.set_challenge_icon(game_mode, image_path) —— 为指定关卡设置自定义封面图标
// game_mode: pvz.GameMode 枚举值（如 pvz.GameMode.CHALLENGE_POGO_PARTY）
// image_path: 图片文件路径（相对于 mod 目录或资源根目录，如 "images/my_pogo_icon.png"）
// 设置后选关界面中该关卡的封面图会替换为自定义图片
// 例：pvz.set_challenge_icon(pvz.GameMode.CHALLENGE_POGO_PARTY, "images/my_pogo_icon.png")
int l_pvz_set_challenge_icon(lua_State* L) {
    int gameMode = static_cast<int>(luaL_checkinteger(L, 1));
    const char* path = luaL_checkstring(L, 2);

    // 用 ImageLib 加载图片文件（支持 PNG/JPG/GIF/TGA，通过 PakInterface 支持 mod overlay）
    ImageLib::Image* srcImg = ImageLib::GetImage(path, false);
    if (!srcImg) {
        std::fprintf(stderr, "[ModAPI] set_challenge_icon: failed to load '%s'\n", path);
        return 0;
    }

    // 创建 MemoryImage 并拷贝像素数据（与 set_background_image 相同的模式）
    MemoryImage* memImg = new MemoryImage(gLawnApp);
    memImg->Create(srcImg->mWidth, srcImg->mHeight);
    uint32_t* bits = memImg->GetBits();
    if (srcImg->mBits) {
        std::memcpy(bits, srcImg->mBits, sizeof(uint32_t) * srcImg->mWidth * srcImg->mHeight);
    }
    memImg->BitsChanged();
    delete srcImg;

    // 通过函数接口设置（SetCustomChallengeIcon 会自动释放旧图标）
    SetCustomChallengeIcon(gameMode, memImg);
    return 0;
}

// pvz.set_mod_button_image(path_normal [, path_over, path_down])
// 为主菜单 Mod 关卡按钮设置自定义图片（三态：常态/悬停/按下）
// path_over/path_down 缺省时复用 path_normal
// mod 加载时调用一次即可，GameSelector 每次构造都会读取应用
// 例：pvz.set_mod_button_image("images/mod_button.png", "images/mod_button_over.png")
int l_pvz_set_mod_button_image(lua_State* L) {
    const char* pathNormal = luaL_checkstring(L, 1);
    const char* pathOver   = luaL_optstring(L, 2, pathNormal);
    const char* pathDown   = luaL_optstring(L, 3, pathOver);

    // 复用 set_challenge_icon 的图片加载样板：ImageLib → MemoryImage
    auto loadImg = [](const char* path) -> MemoryImage* {
        ImageLib::Image* src = ImageLib::GetImage(path, false);
        if (!src) {
            std::fprintf(stderr, "[ModAPI] set_mod_button_image: failed to load '%s'\n", path);
            return nullptr;
        }
        MemoryImage* mem = new MemoryImage(gLawnApp);
        mem->Create(src->mWidth, src->mHeight);
        if (src->mBits) {
            std::memcpy(mem->GetBits(), src->mBits, sizeof(uint32_t) * src->mWidth * src->mHeight);
        }
        mem->BitsChanged();
        delete src;
        return mem;
    };

    MemoryImage* normal = loadImg(pathNormal);
    if (!normal) return 0;  // 常态图加载失败，放弃
    MemoryImage* over = loadImg(pathOver);
    if (!over) over = normal;  // 退化：over 复用 normal
    MemoryImage* down = loadImg(pathDown);
    if (!down) down = over;    // 退化：down 复用 over

    SetModButtonImages(normal, over, down);
    return 0;
}

// pvz.set_challenge_name(game_mode, name) —— 为指定关卡设置自定义显示名称
// game_mode: pvz.GameMode 枚举值
// name: 显示名称（明文字符串，不经过翻译表）
// 设置后选关界面中该关卡按钮下方显示此名称
// 例：pvz.set_challenge_name(pvz.GameMode.MOD_CUSTOM_1, "草地入侵")
int l_pvz_set_challenge_name(lua_State* L) {
    int gameMode = static_cast<int>(luaL_checkinteger(L, 1));
    const char* name = luaL_checkstring(L, 2);
    SetCustomChallengeName(gameMode, std::string(name));
    return 0;
}

// pvz.set_hud_custom(game_mode, enabled) —— 标记指定关卡由 mod 自定义绘制 HUD
// game_mode: pvz.GameMode 枚举值
// enabled: true 表示该关卡跳过默认 DrawProgressMeter，由 mod 通过 on_board_draw_hud 自行绘制
// 例：pvz.set_hud_custom(pvz.GameMode.CHALLENGE_POGO_PARTY, true)
int l_pvz_set_hud_custom(lua_State* L) {
    int gameMode = static_cast<int>(luaL_checkinteger(L, 1));
    bool enabled = lua_toboolean(L, 2);
    SetCustomHudMode(gameMode, enabled);
    return 0;
}

// pvz.reset_challenge_record(game_mode) —— 重置指定关卡的通关记录
// game_mode: pvz.GameMode 枚举值（如 pvz.GameMode.MOD_CUSTOM_1）
// 清零 mChallengeRecords 中该关卡的记录并立即保存到用户档案，使该关卡回到未通关状态
// 例：pvz.reset_challenge_record(pvz.GameMode.MOD_CUSTOM_1)
int l_pvz_reset_challenge_record(lua_State* L) {
    int gameMode = static_cast<int>(luaL_checkinteger(L, 1));
    if (!gLawnApp || !gLawnApp->mPlayerInfo) {
        std::fprintf(stderr, "[ModAPI] reset_challenge_record: app or playerInfo is null\n");
        return 0;
    }
    gLawnApp->mPlayerInfo->ResetChallengeRecord(static_cast<GameMode>(gameMode));
    gLawnApp->mPlayerInfo->SaveDetails();
    return 0;
}

// pvz.load_image(path) -> Image or nil —— 从文件加载图片，返回 Image userdata
// 用于 set_image_override 等需要 Image* 的 API
// path 相对于资源目录或 mod overlay 目录（如 "images/my_weapon.png"）
// 例：local img = pvz.load_image("images/gargantuar_weapon.png")
int l_pvz_load_image(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    ImageLib::Image* srcImg = ImageLib::GetImage(path, false);
    if (!srcImg) {
        std::fprintf(stderr, "[ModAPI] load_image: failed to load '%s'\n", path);
        lua_pushnil(L);
        return 1;
    }
    MemoryImage* memImg = new MemoryImage(gLawnApp);
    memImg->Create(srcImg->mWidth, srcImg->mHeight);
    uint32_t* bits = memImg->GetBits();
    if (srcImg->mBits) {
        std::memcpy(bits, srcImg->mBits, sizeof(uint32_t) * srcImg->mWidth * srcImg->mHeight);
    }
    memImg->BitsChanged();
    delete srcImg;
    NewUserdata(L, memImg, MT_IMAGE);
    return 1;
}

// app:crazy_dave_enter() —— 戴夫进场
int l_app_dave_enter(lua_State* L) {
    LawnApp* a = CheckUserdata<LawnApp>(L, 1, MT_APP);
    if (!a) return 0;
    a->CrazyDaveEnter();
    return 0;
}

// app:crazy_dave_talk(text) —— 戴夫说话（自定义文本）
int l_app_dave_talk(lua_State* L) {
    LawnApp* a = CheckUserdata<LawnApp>(L, 1, MT_APP);
    if (!a) return 0;
    const char* text = luaL_checkstring(L, 2);
    a->CrazyDaveTalkMessage(std::string(text));
    return 0;
}

// app:crazy_dave_leave() —— 戴夫离场
int l_app_dave_leave(lua_State* L) {
    LawnApp* a = CheckUserdata<LawnApp>(L, 1, MT_APP);
    if (!a) return 0;
    a->CrazyDaveLeave();
    return 0;
}

// app:crazy_dave_stop_talking() —— 停止说话
int l_app_dave_stop_talking(lua_State* L) {
    LawnApp* a = CheckUserdata<LawnApp>(L, 1, MT_APP);
    if (!a) return 0;
    a->CrazyDaveStopTalking();
    return 0;
}

// app.crazy_dave_state（只读）—— 返回 CrazyDaveState 枚举值
int l_app_get_dave_state(lua_State* L) {
    LawnApp* a = CheckUserdata<LawnApp>(L, 1, MT_APP);
    if (!a) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, static_cast<lua_Integer>(a->mCrazyDaveState));
    return 1;
}

// app.crazy_dave_message_index（读写）—— 当前消息索引 getter
int l_app_get_dave_msg_index(lua_State* L) {
    LawnApp* a = CheckUserdata<LawnApp>(L, 1, MT_APP);
    if (!a) { lua_pushinteger(L, -1); return 1; }
    lua_pushinteger(L, a->mCrazyDaveMessageIndex);
    return 1;
}
// app.crazy_dave_message_index setter
int l_app_set_dave_msg_index(lua_State* L) {
    LawnApp* a = CheckUserdata<LawnApp>(L, 1, MT_APP);
    if (!a) return 0;
    a->mCrazyDaveMessageIndex = static_cast<int>(luaL_checkinteger(L, 2));
    return 0;
}

// app.crazy_dave_reanim（只读）—— 返回戴夫的 Reanimation userdata，可能为 nil
int l_app_get_dave_reanim(lua_State* L) {
    LawnApp* a = CheckUserdata<LawnApp>(L, 1, MT_APP);
    if (!a) { lua_pushnil(L); return 1; }
    Reanimation* r = a->ReanimationTryToGet(a->mCrazyDaveReanimID);
    if (!r) { lua_pushnil(L); return 1; }
    PushReanimation(L, r);
    return 1;
}

// app:play_dave_anim(track_name, loop_type, blend_time, anim_rate) —— 播放戴夫动画轨道
int l_app_play_dave_anim(lua_State* L) {
    LawnApp* a = CheckUserdata<LawnApp>(L, 1, MT_APP);
    if (!a) return 0;
    Reanimation* r = a->ReanimationTryToGet(a->mCrazyDaveReanimID);
    if (!r) return 0;
    const char* track = luaL_checkstring(L, 2);
    int loopType = static_cast<int>(luaL_optinteger(L, 3, static_cast<lua_Integer>(REANIM_LOOP)));
    int blendTime = static_cast<int>(luaL_optinteger(L, 4, 0));
    float rate = static_cast<float>(luaL_optnumber(L, 5, 12.0f));
    r->PlayReanim(track, static_cast<ReanimLoopType>(loopType), blendTime, rate);
    return 0;
}

// app:__index —— 分发属性与方法
int l_app_index(lua_State* L) {
    LawnApp* a = CheckUserdata<LawnApp>(L, 1, MT_APP);
    if (!a) { lua_pushnil(L); return 1; }
    const char* key = luaL_checkstring(L, 2);

    // 属性
    if (strcmp(key, "crazy_dave_state") == 0)         return l_app_get_dave_state(L);
    if (strcmp(key, "crazy_dave_message_index") == 0) return l_app_get_dave_msg_index(L);
    if (strcmp(key, "crazy_dave_reanim") == 0)        return l_app_get_dave_reanim(L);

    // 方法（push C 闭包）
    struct { const char* name; lua_CFunction fn; } methods[] = {
        {"crazy_dave_enter",        l_app_dave_enter},
        {"crazy_dave_talk",         l_app_dave_talk},
        {"crazy_dave_leave",        l_app_dave_leave},
        {"crazy_dave_stop_talking", l_app_dave_stop_talking},
        {"play_dave_anim",          l_app_play_dave_anim},
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

// app:__newindex —— 仅允许写入 crazy_dave_message_index
int l_app_newindex(lua_State* L) {
    LawnApp* a = CheckUserdata<LawnApp>(L, 1, MT_APP);
    if (!a) return 0;
    const char* key = luaL_checkstring(L, 2);
    if (strcmp(key, "crazy_dave_message_index") == 0) {
        a->mCrazyDaveMessageIndex = static_cast<int>(luaL_checkinteger(L, 3));
        return 0;
    }
    return 0;
}

} // namespace

void BindApp(lua_State* L) {
    CreateMetatable(L, MT_APP);
    SetFuncField(L, "__index",    l_app_index);
    SetFuncField(L, "__newindex", l_app_newindex);
    lua_pop(L, 1); // 弹出 metatable

    // pvz.get_app() —— 返回当前 LawnApp userdata（无 app 时返回 nil）
    // pvz 全局表由 BindEnums 创建，这里追加 get_app 字段
    lua_getglobal(L, "pvz");
    if (lua_istable(L, -1)) {
        lua_pushcfunction(L, l_get_app);
        lua_setfield(L, -2, "get_app");

        // pvz.set_string(key, value) —— 覆盖字符串表（改关卡名等）
        lua_pushcfunction(L, l_pvz_set_string);
        lua_setfield(L, -2, "set_string");

        // pvz.set_challenge_icon(game_mode, image_path) —— 设置自定义关卡封面
        lua_pushcfunction(L, l_pvz_set_challenge_icon);
        lua_setfield(L, -2, "set_challenge_icon");

        // pvz.set_mod_button_image(path_normal [, path_over, path_down]) —— 设置主菜单 Mod 按钮图片
        lua_pushcfunction(L, l_pvz_set_mod_button_image);
        lua_setfield(L, -2, "set_mod_button_image");

        // pvz.set_challenge_name(game_mode, name) —— 设置自定义关卡显示名称
        lua_pushcfunction(L, l_pvz_set_challenge_name);
        lua_setfield(L, -2, "set_challenge_name");

        // pvz.set_hud_custom(game_mode, enabled) —— 标记关卡由 mod 自定义绘制 HUD
        lua_pushcfunction(L, l_pvz_set_hud_custom);
        lua_setfield(L, -2, "set_hud_custom");

        // pvz.reset_challenge_record(game_mode) —— 重置指定关卡通关记录并立即保存
        lua_pushcfunction(L, l_pvz_reset_challenge_record);
        lua_setfield(L, -2, "reset_challenge_record");

        // pvz.load_image(path) —— 从文件加载图片，返回 Image userdata
        lua_pushcfunction(L, l_pvz_load_image);
        lua_setfield(L, -2, "load_image");
    }
    lua_pop(L, 1);
}

} // namespace ModLua
