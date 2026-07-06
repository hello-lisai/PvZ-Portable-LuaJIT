#include "LuaBindUtil.h"
#include "../../Lawn/LawnMower.h"

#include <cstring>

namespace ModLua {

namespace {

int l_mower_get_pos_x(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    lua_pushnumber(L, m->mPosX);
    return 1;
}
int l_mower_get_pos_y(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    lua_pushnumber(L, m->mPosY);
    return 1;
}
int l_mower_get_row(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    lua_pushinteger(L, m->mRow);
    return 1;
}
int l_mower_get_state(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(m->mMowerState));
    return 1;
}
int l_mower_get_type(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(m->mMowerType));
    return 1;
}
int l_mower_get_dead(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    lua_pushboolean(L, m->mDead);
    return 1;
}
int l_mower_get_visible(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    lua_pushboolean(L, m->mVisible);
    return 1;
}

int l_mower_start(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    m->StartMower();
    return 0;
}
int l_mower_die(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    m->Die();
    return 0;
}
int l_mower_squish(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    m->SquishMower();
    return 0;
}
int l_mower_enable_super(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    bool enable = lua_toboolean(L, 2) != 0;
    m->EnableSuperMower(enable);
    return 0;
}

int l_mower_index(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) { lua_pushnil(L); return 1; }
    const char* key = luaL_checkstring(L, 2);

    struct { const char* name; lua_CFunction fn; } props[] = {
        {"pos_x",   l_mower_get_pos_x},
        {"pos_y",   l_mower_get_pos_y},
        {"row",     l_mower_get_row},
        {"state",   l_mower_get_state},
        {"type",    l_mower_get_type},
        {"dead",    l_mower_get_dead},
        {"visible", l_mower_get_visible},
    };
    for (auto& pr : props) {
        if (strcmp(key, pr.name) == 0) return pr.fn(L);
    }

    if (strcmp(key, "start") == 0) {
        lua_pushcfunction(L, l_mower_start);
        return 1;
    }
    if (strcmp(key, "die") == 0) {
        lua_pushcfunction(L, l_mower_die);
        return 1;
    }
    if (strcmp(key, "squish") == 0) {
        lua_pushcfunction(L, l_mower_squish);
        return 1;
    }
    if (strcmp(key, "enable_super") == 0) {
        lua_pushcfunction(L, l_mower_enable_super);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

int l_mower_newindex(lua_State* L) {
    LawnMower* m = CheckUserdata<LawnMower>(L, 1, MT_LAWNMOWER);
    if (!m) return 0;
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "pos_x") == 0) {
        m->mPosX = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "pos_y") == 0) {
        m->mPosY = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "visible") == 0) {
        m->mVisible = lua_toboolean(L, 3) != 0;
    }
    return 0;
}

} // namespace

void BindLawnMower(lua_State* L) {
    CreateMetatable(L, MT_LAWNMOWER);
    SetFuncField(L, "__index",    l_mower_index);
    SetFuncField(L, "__newindex", l_mower_newindex);
    lua_pop(L, 1);
}

void PushLawnMower(lua_State* L, LawnMower* m) {
    NewUserdata(L, m, MT_LAWNMOWER);
}

} // namespace ModLua
