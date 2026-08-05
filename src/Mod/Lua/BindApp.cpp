#include "LuaBindUtil.h"
#include "../../LawnApp.h"
#include "../../Sexy.TodLib/Reanimator.h"  // Reanimation / ReanimLoopType
#include "../../ConstEnums.h"              // CrazyDaveState / ReanimLoopType 枚举

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
    }
    lua_pop(L, 1);
}

} // namespace ModLua
