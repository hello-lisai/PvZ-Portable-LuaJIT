#include "LuaBindUtil.h"
#include "../../Lawn/Projectile.h"

namespace ModLua {

namespace {

int l_proj_get_type(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(p->mProjectileType));
    return 1;
}
int l_proj_get_pos_x(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) return 0;
    lua_pushnumber(L, p->mPosX);
    return 1;
}
int l_proj_get_pos_y(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) return 0;
    lua_pushnumber(L, p->mPosY);
    return 1;
}
int l_proj_get_vel_x(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) return 0;
    lua_pushnumber(L, p->mVelX);
    return 1;
}
int l_proj_get_vel_y(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) return 0;
    lua_pushnumber(L, p->mVelY);
    return 1;
}
int l_proj_get_row(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) return 0;
    lua_pushinteger(L, p->mRow);
    return 1;
}
int l_proj_get_dead(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) return 0;
    lua_pushboolean(L, p->mDead);
    return 1;
}
int l_proj_get_age(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) return 0;
    lua_pushinteger(L, p->mProjectileAge);
    return 1;
}

int l_proj_die(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) return 0;
    p->Die();
    return 0;
}

// projectile:get_ptr() — 返回原始指针（light userdata），供 LuaJIT FFI 使用
int l_proj_get_ptr(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) { lua_pushnil(L); return 1; }
    lua_pushlightuserdata(L, p);
    return 1;
}

int l_proj_index(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) { lua_pushnil(L); return 1; }
    const char* key = luaL_checkstring(L, 2);

    struct { const char* name; lua_CFunction fn; } props[] = {
        {"type",    l_proj_get_type},
        {"pos_x",   l_proj_get_pos_x},
        {"pos_y",   l_proj_get_pos_y},
        {"vel_x",   l_proj_get_vel_x},
        {"vel_y",   l_proj_get_vel_y},
        {"row",     l_proj_get_row},
        {"dead",    l_proj_get_dead},
        {"age",     l_proj_get_age},
    };
    for (auto& pr : props) {
        if (strcmp(key, pr.name) == 0) return pr.fn(L);
    }

    if (strcmp(key, "die") == 0) {
        lua_pushcfunction(L, l_proj_die);
        return 1;
    }
    if (strcmp(key, "get_ptr") == 0) {
        lua_pushcfunction(L, l_proj_get_ptr);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

int l_proj_newindex(lua_State* L) {
    Projectile* p = CheckUserdata<Projectile>(L, 1, MT_PROJECTILE);
    if (!p) return 0;
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "pos_x") == 0) {
        p->mPosX = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "pos_y") == 0) {
        p->mPosY = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "vel_x") == 0) {
        p->mVelX = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "vel_y") == 0) {
        p->mVelY = static_cast<float>(luaL_checknumber(L, 3));
    }
    return 0;
}

} // namespace

void BindProjectile(lua_State* L) {
    CreateMetatable(L, MT_PROJECTILE);
    SetFuncField(L, "__index",    l_proj_index);
    SetFuncField(L, "__newindex", l_proj_newindex);
    lua_pop(L, 1);
}

void PushProjectile(lua_State* L, Projectile* p) {
    NewUserdata(L, p, MT_PROJECTILE);
}

} // namespace ModLua
