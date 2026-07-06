#include "LuaBindUtil.h"
#include "../../Lawn/GridItem.h"

#include <cstring>

namespace ModLua {

namespace {

int l_griditem_get_type(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(g->mGridItemType));
    return 1;
}
int l_griditem_get_state(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(g->mGridItemState));
    return 1;
}
int l_griditem_get_grid_x(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    lua_pushinteger(L, g->mGridX);
    return 1;
}
int l_griditem_get_grid_y(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    lua_pushinteger(L, g->mGridY);
    return 1;
}
int l_griditem_get_pos_x(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    lua_pushnumber(L, g->mPosX);
    return 1;
}
int l_griditem_get_pos_y(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    lua_pushnumber(L, g->mPosY);
    return 1;
}
int l_griditem_get_counter(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    lua_pushinteger(L, g->mGridItemCounter);
    return 1;
}
int l_griditem_get_dead(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    lua_pushboolean(L, g->mDead);
    return 1;
}
int l_griditem_get_sun_count(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    lua_pushinteger(L, g->mSunCount);
    return 1;
}
int l_griditem_get_highlighted(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    lua_pushboolean(L, g->mHighlighted);
    return 1;
}

int l_griditem_die(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    g->GridItemDie();
    return 0;
}

int l_griditem_index(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) { lua_pushnil(L); return 1; }
    const char* key = luaL_checkstring(L, 2);

    struct { const char* name; lua_CFunction fn; } props[] = {
        {"type",        l_griditem_get_type},
        {"state",       l_griditem_get_state},
        {"grid_x",      l_griditem_get_grid_x},
        {"grid_y",      l_griditem_get_grid_y},
        {"pos_x",       l_griditem_get_pos_x},
        {"pos_y",       l_griditem_get_pos_y},
        {"counter",     l_griditem_get_counter},
        {"dead",        l_griditem_get_dead},
        {"sun_count",   l_griditem_get_sun_count},
        {"highlighted", l_griditem_get_highlighted},
    };
    for (auto& pr : props) {
        if (strcmp(key, pr.name) == 0) return pr.fn(L);
    }

    if (strcmp(key, "die") == 0) {
        lua_pushcfunction(L, l_griditem_die);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

int l_griditem_newindex(lua_State* L) {
    GridItem* g = CheckUserdata<GridItem>(L, 1, MT_GRIDITEM);
    if (!g) return 0;
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "pos_x") == 0) {
        g->mPosX = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "pos_y") == 0) {
        g->mPosY = static_cast<float>(luaL_checknumber(L, 3));
    } else if (strcmp(key, "counter") == 0) {
        g->mGridItemCounter = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "sun_count") == 0) {
        g->mSunCount = static_cast<int32_t>(luaL_checkinteger(L, 3));
    } else if (strcmp(key, "highlighted") == 0) {
        g->mHighlighted = lua_toboolean(L, 3) != 0;
    }
    return 0;
}

} // namespace

void BindGridItem(lua_State* L) {
    CreateMetatable(L, MT_GRIDITEM);
    SetFuncField(L, "__index",    l_griditem_index);
    SetFuncField(L, "__newindex", l_griditem_newindex);
    lua_pop(L, 1);
}

void PushGridItem(lua_State* L, GridItem* g) {
    NewUserdata(L, g, MT_GRIDITEM);
}

} // namespace ModLua
