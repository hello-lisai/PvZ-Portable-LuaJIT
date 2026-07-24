#include "LuaBindUtil.h"
#include "../../Lawn/Plant.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../LawnApp.h"

namespace ModLua {

// 前向声明：PushReanimation 在 BindReanimation.cpp 中实现
void PushReanimation(lua_State* L, Reanimation* r);

namespace {

// === Plant 属性 getter ===
int l_plant_get_type(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(p->mSeedType));
    return 1;
}
int l_plant_get_instance_id(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) { lua_pushnil(L); return 1; }
    lua_pushinteger(L, p->mInstanceId);
    return 1;
}
int l_plant_get_health(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    lua_pushinteger(L, p->mPlantHealth);
    return 1;
}
int l_plant_get_max_health(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    lua_pushinteger(L, p->mPlantMaxHealth);
    return 1;
}
int l_plant_get_col(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    lua_pushinteger(L, p->mPlantCol);
    return 1;
}
int l_plant_get_row(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    lua_pushinteger(L, p->mRow);
    return 1;
}
int l_plant_get_state(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(p->mState));
    return 1;
}
int l_plant_get_asleep(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    lua_pushboolean(L, p->mIsAsleep);
    return 1;
}
int l_plant_get_dead(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    lua_pushboolean(L, p->mDead);
    return 1;
}
int l_plant_get_x(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    lua_pushinteger(L, p->mX);
    return 1;
}
int l_plant_get_y(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    lua_pushinteger(L, p->mY);
    return 1;
}

// plant.body_reanim —— 获取身体动画对象（Reanimation userdata），无动画时返回 nil
// mod 可通过此对象读取/修改植物动画：
//   local r = plant.body_reanim
//   if r then
//       print(r.current_track_name, r.anim_time)
//       r.anim_rate = 1.5  -- 加速播放
//       r:play_reanim("anim_shooting", pvz.ReanimLoop.PLAY_ONCE, 0, 0)
//   end
int l_plant_get_body_reanim(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) { lua_pushnil(L); return 1; }
    if (p->mBodyReanimID == ReanimationID::REANIMATIONID_NULL || !gLawnApp) {
        lua_pushnil(L);
        return 1;
    }
    Reanimation* r = gLawnApp->ReanimationTryToGet(p->mBodyReanimID);
    if (!r) { lua_pushnil(L); return 1; }
    PushReanimation(L, r);
    return 1;
}

// plant.body_reanim_id —— 动画对象的 ID（uint32），可用于 pvz.get_reanimation(id)
int l_plant_get_body_reanim_id(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, static_cast<lua_Integer>(p->mBodyReanimID));
    return 1;
}

// === Plant 方法 ===
int l_plant_die(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    p->Die();
    return 0;
}
int l_plant_do_special(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    p->DoSpecial();
    return 0;
}
int l_plant_squish(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    p->Squish();
    return 0;
}

// plant:get_ptr() — 返回原始指针（light userdata），供 LuaJIT FFI 使用
int l_plant_get_ptr(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) { lua_pushnil(L); return 1; }
    lua_pushlightuserdata(L, p);
    return 1;
}

// plant:set_state(state) — 设置植物状态（PlantState 枚举值）
// 仅修改 mState 字段，不自动播放动画。如需播放对应动画，mod 应自行调用
// plant:play_body_reanim(track_name, loop_type, blend_time, anim_rate)
// 例：plant:set_state(STATE_POTATO_ARMED)
int l_plant_set_state(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    int state = static_cast<int>(luaL_checkinteger(L, 2));
    p->mState = static_cast<PlantState>(state);
    return 0;
}

// plant:play_body_reanim(track_name, loop_type, blend_time, anim_rate)
// 切换身体动画到指定轨道（封装 Plant::PlayBodyReanim）
//   track_name  : 轨道名（如 "anim_idle"、"anim_shooting"、"anim_sun" 等）
//   loop_type   : ReanimLoopType 枚举值（0=LOOP, 2=PLAY_ONCE, 3=PLAY_ONCE_AND_HOLD ...）
//   blend_time  : 混合过渡时长（0=立即切换）
//   anim_rate   : 播放速率（0=保持原速率；负值=倒放）
// 例：plant:play_body_reanim("anim_shooting", pvz.ReanimLoop.PLAY_ONCE, 0, 0)
int l_plant_play_body_reanim(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    const char* track = luaL_checkstring(L, 2);
    int loopType = static_cast<int>(luaL_optinteger(L, 3, 0));
    int blendTime = static_cast<int>(luaL_optinteger(L, 4, 0));
    float rate = static_cast<float>(luaL_optnumber(L, 5, 0.0f));
    p->PlayBodyReanim(track, static_cast<ReanimLoopType>(loopType), blendTime, rate);
    return 0;
}

int l_plant_index(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) { lua_pushnil(L); return 1; }
    const char* key = luaL_checkstring(L, 2);

    struct { const char* name; lua_CFunction fn; } props[] = {
        {"instance_id",    l_plant_get_instance_id},
        {"type",           l_plant_get_type},
        {"health",         l_plant_get_health},
        {"max_health",     l_plant_get_max_health},
        {"col",            l_plant_get_col},
        {"row",            l_plant_get_row},
        {"state",          l_plant_get_state},
        {"asleep",         l_plant_get_asleep},
        {"dead",           l_plant_get_dead},
        {"x",              l_plant_get_x},
        {"y",              l_plant_get_y},
        {"body_reanim",    l_plant_get_body_reanim},
        {"body_reanim_id", l_plant_get_body_reanim_id},
    };
    for (auto& pr : props) {
        if (strcmp(key, pr.name) == 0) return pr.fn(L);
    }

    struct { const char* name; lua_CFunction fn; } methods[] = {
        {"die",              l_plant_die},
        {"do_special",       l_plant_do_special},
        {"squish",           l_plant_squish},
        {"get_ptr",          l_plant_get_ptr},
        {"set_state",        l_plant_set_state},
        {"play_body_reanim", l_plant_play_body_reanim},
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

int l_plant_newindex(lua_State* L) {
    Plant* p = CheckUserdata<Plant>(L, 1, MT_PLANT);
    if (!p) return 0;
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "health") == 0) {
        p->mPlantHealth = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "max_health") == 0) {
        p->mPlantMaxHealth = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "asleep") == 0) {
        p->mIsAsleep = lua_toboolean(L, 3) != 0;
    } else if (strcmp(key, "x") == 0) {
        // mX/mY 来自 GameObject 基类，直接修改渲染位置
        p->mX = static_cast<int>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "y") == 0) {
        p->mY = static_cast<int>(luaL_checkinteger(L, 3));
    }
    return 0;
}

} // namespace

void BindPlant(lua_State* L) {
    CreateMetatable(L, MT_PLANT);
    SetFuncField(L, "__index",    l_plant_index);
    SetFuncField(L, "__newindex", l_plant_newindex);
    lua_pop(L, 1);
}

void PushPlant(lua_State* L, Plant* p) {
    NewUserdata(L, p, MT_PLANT);
}

} // namespace ModLua
