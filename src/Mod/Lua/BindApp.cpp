#include "LuaBindUtil.h"
#include "../../LawnApp.h"
#include "../../Sexy.TodLib/Reanimator.h"  // Reanimation / ReanimLoopType
#include "../../ConstEnums.h"              // CrazyDaveState / ReanimLoopType 枚举
#include "../../SexyAppFramework/SexyAppBase.h"  // gSexyAppBase / mStringProperties
#include "../../SexyAppFramework/graphics/Image.h"
#include "../../SexyAppFramework/graphics/MemoryImage.h"
#include "../../SexyAppFramework/imagelib/ImageLib.h"
#include "LuaRuntime.h"
#include <cstdio>
#include <cstring>
#include <unordered_map>

namespace ModLua {

// 在 BindReanimation.cpp 中定义，这里前向声明
void PushReanimation(lua_State* L, Reanimation* r);

// g_customChallengeIcons 在 BindBoard.cpp 中定义
extern std::unordered_map<int, Sexy::Image*> g_customChallengeIcons;

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

    // 释放旧图标
    auto it = g_customChallengeIcons.find(gameMode);
    if (it != g_customChallengeIcons.end() && it->second) {
        delete it->second;
    }

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

    g_customChallengeIcons[gameMode] = memImg;
    return 0;
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
    }
    lua_pop(L, 1);
}

} // namespace ModLua
