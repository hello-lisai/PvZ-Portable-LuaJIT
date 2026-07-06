#include "LuaBindUtil.h"
#include "../../Lawn/Coin.h"

namespace ModLua {

namespace {

int l_coin_get_type(lua_State* L) {
    Coin* c = CheckUserdata<Coin>(L, 1, MT_COIN);
    if (!c) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(c->mType));
    return 1;
}
int l_coin_get_pos_x(lua_State* L) {
    Coin* c = CheckUserdata<Coin>(L, 1, MT_COIN);
    if (!c) return 0;
    lua_pushnumber(L, c->mPosX);
    return 1;
}
int l_coin_get_pos_y(lua_State* L) {
    Coin* c = CheckUserdata<Coin>(L, 1, MT_COIN);
    if (!c) return 0;
    lua_pushnumber(L, c->mPosY);
    return 1;
}
int l_coin_get_vel_x(lua_State* L) {
    Coin* c = CheckUserdata<Coin>(L, 1, MT_COIN);
    if (!c) return 0;
    lua_pushnumber(L, c->mVelX);
    return 1;
}
int l_coin_get_vel_y(lua_State* L) {
    Coin* c = CheckUserdata<Coin>(L, 1, MT_COIN);
    if (!c) return 0;
    lua_pushnumber(L, c->mVelY);
    return 1;
}
int l_coin_get_dead(lua_State* L) {
    Coin* c = CheckUserdata<Coin>(L, 1, MT_COIN);
    if (!c) return 0;
    lua_pushboolean(L, c->mDead);
    return 1;
}
int l_coin_get_age(lua_State* L) {
    Coin* c = CheckUserdata<Coin>(L, 1, MT_COIN);
    if (!c) return 0;
    lua_pushinteger(L, c->mCoinAge);
    return 1;
}

int l_coin_die(lua_State* L) {
    Coin* c = CheckUserdata<Coin>(L, 1, MT_COIN);
    if (!c) return 0;
    c->Die();
    return 0;
}
int l_coin_collect(lua_State* L) {
    Coin* c = CheckUserdata<Coin>(L, 1, MT_COIN);
    if (!c) return 0;
    c->Collect();
    return 0;
}

int l_coin_index(lua_State* L) {
    Coin* c = CheckUserdata<Coin>(L, 1, MT_COIN);
    if (!c) { lua_pushnil(L); return 1; }
    const char* key = luaL_checkstring(L, 2);

    struct { const char* name; lua_CFunction fn; } props[] = {
        {"type",   l_coin_get_type},
        {"pos_x",  l_coin_get_pos_x},
        {"pos_y",  l_coin_get_pos_y},
        {"vel_x",  l_coin_get_vel_x},
        {"vel_y",  l_coin_get_vel_y},
        {"dead",   l_coin_get_dead},
        {"age",    l_coin_get_age},
    };
    for (auto& pr : props) {
        if (strcmp(key, pr.name) == 0) return pr.fn(L);
    }

    if (strcmp(key, "die") == 0) {
        lua_pushcfunction(L, l_coin_die);
        return 1;
    }
    if (strcmp(key, "collect") == 0) {
        lua_pushcfunction(L, l_coin_collect);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

int l_coin_newindex(lua_State* L) {
    Coin* c = CheckUserdata<Coin>(L, 1, MT_COIN);
    if (!c) return 0;
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "pos_x") == 0) {
        c->mPosX = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "pos_y") == 0) {
        c->mPosY = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "vel_x") == 0) {
        c->mVelX = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "vel_y") == 0) {
        c->mVelY = static_cast<float>(luaL_checknumber(L, 3));
    }
    return 0;
}

} // namespace

void BindCoin(lua_State* L) {
    CreateMetatable(L, MT_COIN);
    SetFuncField(L, "__index",    l_coin_index);
    SetFuncField(L, "__newindex", l_coin_newindex);
    lua_pop(L, 1);
}

void PushCoin(lua_State* L, Coin* c) {
    NewUserdata(L, c, MT_COIN);
}

} // namespace ModLua
