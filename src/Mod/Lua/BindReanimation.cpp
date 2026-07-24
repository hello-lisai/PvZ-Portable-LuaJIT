#include "LuaBindUtil.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../LawnApp.h"

namespace ModLua {

namespace {

// === Reanimation 属性 getter ===

// reanim.anim_time —— 当前动画时间轴位置（归一化到 [0,1] 的循环率）
int l_reanim_get_anim_time(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    lua_pushnumber(L, r->mAnimTime);
    return 1;
}

// reanim.anim_rate —— 动画速率（每秒推进多少个循环；负值=倒放）
int l_reanim_get_anim_rate(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    lua_pushnumber(L, r->mAnimRate);
    return 1;
}

// reanim.loop_type —— 循环类型（ReanimLoopType 枚举值）
int l_reanim_get_loop_type(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(r->mLoopType));
    return 1;
}

// reanim.loop_count —— 已循环次数（每次完成一个循环 +1）
int l_reanim_get_loop_count(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    lua_pushinteger(L, r->mLoopCount);
    return 1;
}

// reanim.dead —— 是否已标记死亡（等清理）
int l_reanim_get_dead(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    lua_pushboolean(L, r->mDead);
    return 1;
}

// reanim.frame_start —— 当前轨道在 mDefinition->mTracks 中的起始帧
int l_reanim_get_frame_start(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    lua_pushinteger(L, r->mFrameStart);
    return 1;
}

// reanim.frame_count —— 当前轨道的帧数
int l_reanim_get_frame_count(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    lua_pushinteger(L, r->mFrameCount);
    return 1;
}

// reanim.reanim_type —— 动画类型（ReanimationType 枚举值）
int l_reanim_get_reanim_type(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(r->mReanimationType));
    return 1;
}

// reanim.current_track_name —— 当前正在播放的轨道名（遍历所有轨道匹配 mFrameStart/mFrameCount）
// 返回 string 或 nil（无法确定时）
int l_reanim_get_current_track_name(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) { lua_pushnil(L); return 1; }
    if (!r->mDefinition || r->mDefinition->mTracks.count == 0) {
        lua_pushnil(L);
        return 1;
    }
    // 遍历所有轨道，找出第一个其 frameStart/frameCount 与当前匹配的轨道
    // 与 IsAnimPlaying 内部逻辑一致
    for (int i = 0; i < r->mDefinition->mTracks.count; ++i) {
        const char* name = r->mDefinition->mTracks.tracks[i].mName;
        if (!name || !*name) continue;
        int aFrameStart = 0, aFrameCount = 0;
        r->GetFramesForLayer(name, aFrameStart, aFrameCount);
        if (aFrameStart == r->mFrameStart && aFrameCount == r->mFrameCount) {
            lua_pushstring(L, name);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

// === Reanimation 方法 ===

// reanim:play_reanim(track_name, loop_type, blend_time, anim_rate)
// 切换到指定轨道并重新设置时间轴
//   track_name  : 轨道名（如 "anim_walk"、"anim_idle"）
//   loop_type   : ReanimLoopType 枚举值（0=LOOP, 1=LOOP_FULL_LAST_FRAME, 2=PLAY_ONCE, ...）
//   blend_time  : 混合过渡时长（0=立即切换）
//   anim_rate   : 播放速率（0=保持原速率；负值=倒放）
int l_reanim_play_reanim(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    const char* track = luaL_checkstring(L, 2);
    int loopType = static_cast<int>(luaL_optinteger(L, 3, 0));
    int blendTime = static_cast<int>(luaL_optinteger(L, 4, 0));
    float rate = static_cast<float>(luaL_optnumber(L, 5, 0.0f));
    r->PlayReanim(track, static_cast<ReanimLoopType>(loopType), blendTime, rate);
    return 0;
}

// reanim:is_anim_playing(track_name) → bool
// 判断指定轨道是否正在播放（通过比较 frameStart/frameCount）
int l_reanim_is_anim_playing(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) { lua_pushboolean(L, 0); return 1; }
    const char* track = luaL_checkstring(L, 2);
    lua_pushboolean(L, r->IsAnimPlaying(track) ? 1 : 0);
    return 1;
}

// reanim:track_exists(track_name) → bool
int l_reanim_track_exists(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) { lua_pushboolean(L, 0); return 1; }
    const char* track = luaL_checkstring(L, 2);
    lua_pushboolean(L, r->TrackExists(track) ? 1 : 0);
    return 1;
}

// reanim:set_position(x, y) —— 设置动画渲染位置
int l_reanim_set_position(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));
    r->SetPosition(x, y);
    return 0;
}

// reanim:override_scale(scale_x, scale_y) —— 覆盖动画整体缩放
int l_reanim_override_scale(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    float sx = static_cast<float>(luaL_checknumber(L, 2));
    float sy = static_cast<float>(luaL_checknumber(L, 3));
    r->OverrideScale(sx, sy);
    return 0;
}

// reanim:find_sub_reanim(reanim_type) → Reanimation or nil
// 查找附加的子动画（如僵尸的头/帽子等）
int l_reanim_find_sub_reanim(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) { lua_pushnil(L); return 1; }
    int t = static_cast<int>(luaL_checkinteger(L, 2));
    Reanimation* sub = r->FindSubReanim(static_cast<ReanimationType>(t));
    if (!sub) { lua_pushnil(L); return 1; }
    NewUserdata(L, sub, MT_REANIMATION);
    return 1;
}

// reanim:get_ptr() — 返回原始指针（light userdata），供 LuaJIT FFI 使用
int l_reanim_get_ptr(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) { lua_pushnil(L); return 1; }
    lua_pushlightuserdata(L, r);
    return 1;
}

// reanim:__index
int l_reanim_index(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) { lua_pushnil(L); return 1; }
    const char* key = luaL_checkstring(L, 2);

    struct { const char* name; lua_CFunction fn; } props[] = {
        {"anim_time",          l_reanim_get_anim_time},
        {"anim_rate",          l_reanim_get_anim_rate},
        {"loop_type",          l_reanim_get_loop_type},
        {"loop_count",         l_reanim_get_loop_count},
        {"dead",               l_reanim_get_dead},
        {"frame_start",        l_reanim_get_frame_start},
        {"frame_count",        l_reanim_get_frame_count},
        {"reanim_type",        l_reanim_get_reanim_type},
        {"current_track_name", l_reanim_get_current_track_name},
    };
    for (auto& p : props) {
        if (strcmp(key, p.name) == 0) return p.fn(L);
    }

    struct { const char* name; lua_CFunction fn; } methods[] = {
        {"play_reanim",      l_reanim_play_reanim},
        {"is_anim_playing",  l_reanim_is_anim_playing},
        {"track_exists",     l_reanim_track_exists},
        {"set_position",     l_reanim_set_position},
        {"override_scale",   l_reanim_override_scale},
        {"find_sub_reanim",  l_reanim_find_sub_reanim},
        {"get_ptr",          l_reanim_get_ptr},
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

// reanim:__newindex（可写字段）
int l_reanim_newindex(lua_State* L) {
    Reanimation* r = CheckUserdata<Reanimation>(L, 1, MT_REANIMATION);
    if (!r) return 0;
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "anim_time") == 0) {
        // 直接修改时间轴位置；归一化到 [0,1] 避免越界
        float t = static_cast<float>(luaL_checknumber(L, 3));
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        r->mAnimTime = t;
        r->mLastFrameTime = -1.0f;  // 强制下一帧重新计算
    } else if (strcmp(key, "anim_rate") == 0) {
        r->mAnimRate = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "loop_type") == 0) {
        r->mLoopType = static_cast<ReanimLoopType>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "loop_count") == 0) {
        r->mLoopCount = static_cast<int32_t>(luaL_checkinteger(L, 3));
    }
    return 0;
}

} // namespace

void BindReanimation(lua_State* L) {
    CreateMetatable(L, MT_REANIMATION);
    SetFuncField(L, "__index",    l_reanim_index);
    SetFuncField(L, "__newindex", l_reanim_newindex);
    lua_pop(L, 1);
}

void PushReanimation(lua_State* L, Reanimation* r) {
    NewUserdata(L, r, MT_REANIMATION);
}

} // namespace ModLua
