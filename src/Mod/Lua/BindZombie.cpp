#include "LuaBindUtil.h"
#include "../../Lawn/Zombie.h"
#include "../../Sexy.TodLib/Reanimator.h"
#include "../../LawnApp.h"

namespace ModLua {

// 前向声明：PushReanimation 在 BindReanimation.cpp 中实现
void PushReanimation(lua_State* L, Reanimation* r);

namespace {

// === Zombie 属性 getter ===

int l_zombie_get_type(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(z->mZombieType));
    return 1;
}

int l_zombie_get_health(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, z->mBodyHealth);
    return 1;
}

int l_zombie_get_max_health(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, z->mBodyMaxHealth);
    return 1;
}

int l_zombie_get_helm_health(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, z->mHelmHealth);
    return 1;
}

int l_zombie_get_shield_health(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, z->mShieldHealth);
    return 1;
}

int l_zombie_get_pos_x(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushnumber(L, z->mPosX);
    return 1;
}

int l_zombie_get_pos_y(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushnumber(L, z->mPosY);
    return 1;
}

int l_zombie_get_row(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, z->mRow);
    return 1;
}

int l_zombie_get_vel_x(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushnumber(L, z->mVelX);
    return 1;
}

int l_zombie_get_chilled(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, z->mChilledCounter);
    return 1;
}

int l_zombie_get_buttered(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, z->mButteredCounter);
    return 1;
}

int l_zombie_get_icetrap(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, z->mIceTrapCounter);
    return 1;
}

int l_zombie_get_mind_controlled(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushboolean(L, z->mMindControlled);
    return 1;
}

int l_zombie_get_dead(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushboolean(L, z->mDead);
    return 1;
}

int l_zombie_get_phase(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(z->mZombiePhase));
    return 1;
}

int l_zombie_get_from_wave(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, z->mFromWave);
    return 1;
}

int l_zombie_get_abilities(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(z->mAbilities));
    return 1;
}

int l_zombie_get_is_eating(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    lua_pushboolean(L, z->mIsEating);
    return 1;
}

// zombie.body_reanim —— 获取身体动画对象（Reanimation userdata），无动画时返回 nil
// mod 可通过此对象读取当前轨道名/时间轴/速率并修改：
//   local r = zombie.body_reanim
//   if r then
//       print(r.current_track_name, r.anim_time, r.anim_rate)
//       r.anim_rate = 2.0  -- 加速播放
//       r:play_reanim("anim_walk", pvz.ReanimLoop.LOOP, 0, 0)  -- 切换轨道
//   end
int l_zombie_get_body_reanim(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) { lua_pushnil(L); return 1; }
    if (z->mBodyReanimID == ReanimationID::REANIMATIONID_NULL || !gLawnApp) {
        lua_pushnil(L);
        return 1;
    }
    Reanimation* r = gLawnApp->ReanimationTryToGet(z->mBodyReanimID);
    if (!r) { lua_pushnil(L); return 1; }
    PushReanimation(L, r);
    return 1;
}

// zombie.body_reanim_id —— 动画对象的 ID（uint32），可用于 pvz.get_reanimation(id)
int l_zombie_get_body_reanim_id(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, static_cast<lua_Integer>(z->mBodyReanimID));
    return 1;
}

int l_zombie_set_abilities(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    z->mAbilities = static_cast<ZombieAbility>(luaL_checkinteger(L, 3));
    return 0;
}

// === Zombie 方法 ===

// zombie:take_damage(damage, flags)
int l_zombie_take_damage(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    int dmg = static_cast<int>(luaL_checkinteger(L, 2));
    unsigned int flags = static_cast<unsigned int>(luaL_optinteger(L, 3, 0));
    z->TakeDamage(dmg, flags);
    return 0;
}

// zombie:apply_chill(is_ice_trap)
int l_zombie_apply_chill(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    z->ApplyChill(lua_toboolean(L, 2) != 0);
    return 0;
}

// zombie:die()  -- 无掉落死亡
int l_zombie_die(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    z->DieNoLoot();
    return 0;
}

// zombie:die_with_loot()  -- 带掉落死亡
int l_zombie_die_with_loot(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    z->DieWithLoot();
    return 0;
}

// zombie:get_ptr() — 返回原始指针（light userdata），供 LuaJIT FFI 使用
// 例：local ptr = zombie:get_ptr()
//     ffi.C.pvz_zombie_take_damage(ptr, 9999, 0)
int l_zombie_get_ptr(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) { lua_pushnil(L); return 1; }
    lua_pushlightuserdata(L, z);
    return 1;
}

// zombie:set_phase(phase) — 设置僵尸阶段（ZombiePhase 枚举值）
// 仅修改 mZombiePhase 字段，不自动播放动画。如需播放对应动画，mod 应自行调用
// zombie:play_zombie_reanim(track_name, loop_type, blend_time, anim_rate)
// 例：zombie:set_phase(PHASE_ZOMBIE_NORMAL)
int l_zombie_set_phase(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    int phase = static_cast<int>(luaL_checkinteger(L, 2));
    z->mZombiePhase = static_cast<ZombiePhase>(phase);
    return 0;
}

// zombie:play_zombie_reanim(track_name, loop_type, blend_time, anim_rate)
// 切换身体动画到指定轨道（封装 Zombie::PlayZombieReanim）
//   track_name  : 轨道名（如 "anim_walk"、"anim_idle"、"anim_armraise" 等）
//   loop_type   : ReanimLoopType 枚举值（0=LOOP, 2=PLAY_ONCE, 3=PLAY_ONCE_AND_HOLD ...）
//   blend_time  : 混合过渡时长（0=立即切换）
//   anim_rate   : 播放速率（0=保持原速率；负值=倒放）
// 例：zombie:play_zombie_reanim("anim_armraise", pvz.ReanimLoop.PLAY_ONCE_AND_HOLD, 0, 24)
int l_zombie_play_zombie_reanim(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    const char* track = luaL_checkstring(L, 2);
    int loopType = static_cast<int>(luaL_optinteger(L, 3, 0));
    int blendTime = static_cast<int>(luaL_optinteger(L, 4, 0));
    float rate = static_cast<float>(luaL_optnumber(L, 5, 0.0f));
    z->PlayZombieReanim(track, static_cast<ReanimLoopType>(loopType), blendTime, rate);
    return 0;
}

// zombie:__index
int l_zombie_index(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) { lua_pushnil(L); return 1; }
    const char* key = luaL_checkstring(L, 2);

    // 属性（getter）
    struct { const char* name; lua_CFunction fn; } props[] = {
        {"type",             l_zombie_get_type},
        {"health",           l_zombie_get_health},
        {"max_health",       l_zombie_get_max_health},
        {"helm_health",      l_zombie_get_helm_health},
        {"shield_health",    l_zombie_get_shield_health},
        {"pos_x",            l_zombie_get_pos_x},
        {"pos_y",            l_zombie_get_pos_y},
        {"row",              l_zombie_get_row},
        {"vel_x",            l_zombie_get_vel_x},
        {"chilled",          l_zombie_get_chilled},
        {"buttered",         l_zombie_get_buttered},
        {"ice_trap",         l_zombie_get_icetrap},
        {"mind_controlled",  l_zombie_get_mind_controlled},
        {"dead",             l_zombie_get_dead},
        {"phase",            l_zombie_get_phase},
        {"from_wave",        l_zombie_get_from_wave},
        {"abilities",        l_zombie_get_abilities},
        {"is_eating",        l_zombie_get_is_eating},
        {"body_reanim",      l_zombie_get_body_reanim},
        {"body_reanim_id",   l_zombie_get_body_reanim_id},
    };
    for (auto& p : props) {
        if (strcmp(key, p.name) == 0) return p.fn(L);
    }

    // 方法
    struct { const char* name; lua_CFunction fn; } methods[] = {
        {"take_damage",         l_zombie_take_damage},
        {"apply_chill",         l_zombie_apply_chill},
        {"die",                 l_zombie_die},
        {"die_with_loot",       l_zombie_die_with_loot},
        {"get_ptr",             l_zombie_get_ptr},
        {"set_phase",           l_zombie_set_phase},
        {"play_zombie_reanim",  l_zombie_play_zombie_reanim},
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

// zombie:__newindex（可写字段）
int l_zombie_newindex(lua_State* L) {
    Zombie* z = CheckUserdata<Zombie>(L, 1, MT_ZOMBIE);
    if (!z) return 0;
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "health") == 0) {
        z->mBodyHealth = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "max_health") == 0) {
        z->mBodyMaxHealth = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "helm_health") == 0) {
        z->mHelmHealth = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "shield_health") == 0) {
        z->mShieldHealth = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "pos_x") == 0) {
        z->mPosX = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "pos_y") == 0) {
        z->mPosY = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "vel_x") == 0) {
        z->mVelX = static_cast<float>(luaL_checknumber(L, 3));
        // mVelX 是动画速率的输入参数，修改后必须重新调用 UpdateAnimSpeed()
        // 才能更新动画播放速率，进而更新 _ground track velocity（实际移动速度）
        z->UpdateAnimSpeed();
    } else if (strcmp(key, "row") == 0) {
        z->mRow = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "chilled") == 0) {
        z->mChilledCounter = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "buttered") == 0) {
        z->mButteredCounter = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "ice_trap") == 0) {
        z->mIceTrapCounter = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "mind_controlled") == 0) {
        z->mMindControlled = lua_toboolean(L, 3) != 0;
    } else if (strcmp(key, "abilities") == 0) {
        z->mAbilities = static_cast<ZombieAbility>(luaL_checkinteger(L, 3));
    }
    return 0;
}

} // namespace

void BindZombie(lua_State* L) {
    CreateMetatable(L, MT_ZOMBIE);
    SetFuncField(L, "__index",    l_zombie_index);
    SetFuncField(L, "__newindex", l_zombie_newindex);
    lua_pop(L, 1);
}

void PushZombie(lua_State* L, Zombie* z) {
    NewUserdata(L, z, MT_ZOMBIE);
}

} // namespace ModLua
