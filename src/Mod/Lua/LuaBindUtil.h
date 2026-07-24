#pragma once

// Lua 绑定公共工具
// 提供 userdata 的创建/检查、字段读写等模板化辅助
//
// LuaJIT 兼容层：LuaJIT 2.1 基于 Lua 5.1 API，但兼容部分 5.2/5.3 API。
// 缺失的 5.4 API（lua_newuserdatauv、lua_absindex、lua_isinteger）通过宏补丁实现。
// 此头文件同时是项目内 Lua C API 的统一入口——所有 Bind*.cpp 与 LuaRuntime.cpp
// 都通过 #include 此文件获得兼容宏，避免散落定义。

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string>

// ====== LuaJIT 兼容性补丁 ======
// LuaJIT 2.1 兼容 Lua 5.1 API，缺少 Lua 5.2/5.3/5.4 的部分函数。
// 通过条件宏提供等价实现，对原生 5.3/5.4 无副作用。

// LUA_OK：Lua 5.3+ 引入，LuaJIT 2.1 已补丁定义，但旧版可能没有
#ifndef LUA_OK
#define LUA_OK 0
#endif

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
// lua_absindex：Lua 5.2+ 引入，LuaJIT 缺失
// 伪索引（<= LUA_REGISTRYINDEX）与正索引（>0）原样返回；负索引转绝对索引。
static inline int _lua_absindex_compat(lua_State* L, int idx) {
    return (idx > 0 || idx <= LUA_REGISTRYINDEX) ? idx : (int)lua_gettop(L) + idx + 1;
}
#define lua_absindex(L, idx) _lua_absindex_compat((L), (idx))
#endif

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 503
// lua_isinteger：Lua 5.3+ 引入，LuaJIT 缺失
// 降级实现：判断数值是否等于其整数转换值。用于 JSON 序列化区分 Int/Double。
// 注：LuaJIT DUALNUM 模式下内部可区分整数/浮点，但 C API 未直接暴露。
static inline int _lua_isinteger_compat(lua_State* L, int idx) {
    if (lua_type(L, idx) != LUA_TNUMBER) return 0;
    lua_Number n = lua_tonumber(L, idx);
    lua_Integer i = lua_tointeger(L, idx);
    return n == (lua_Number)i;
}
#define lua_isinteger(L, idx) _lua_isinteger_compat((L), (idx))
#endif

namespace ModLua {

// 为类型 T 创建一个 userdata，存储 T* 指针
// metatableName 用于 luaL_checkudata 类型检查
template <typename T>
T* NewUserdata(lua_State* L, T* ptr, const char* metatableName) {
    if (!ptr) {
        lua_pushnil(L);
        return nullptr;
    }
    // 轻量 userdata：直接存指针，无需 GC（对象生命周期由游戏的 DataArray 管理）
    // LuaJIT 用 lua_newuserdata（5.4 的 lua_newuserdatauv 等价于 uv=0 的情形）
    T** pp = static_cast<T**>(lua_newuserdata(L, sizeof(T*)));
    *pp = ptr;
    luaL_setmetatable(L, metatableName);  // LuaJIT 2.1 兼容此 5.2 API
    return ptr;
}

// 从 userdata 取出 T* 指针，失败返回 nullptr
template <typename T>
T* CheckUserdata(lua_State* L, int idx, const char* metatableName) {
    T** pp = static_cast<T**>(luaL_checkudata(L, idx, metatableName));
    return pp ? *pp : nullptr;
}

// 注册一个 metatable，名字为 name，绑定 __index 到自身
// 调用后栈顶是该 metatable（供调用方继续填充字段）
inline void CreateMetatable(lua_State* L, const char* name) {
    luaL_newmetatable(L, name);  // 创建并压栈
    lua_pushvalue(L, -1);         // 复制
    lua_setfield(L, -2, "__index"); // mt.__index = mt
}

// 注册整数常量到栈顶 table
inline void SetIntField(lua_State* L, const char* key, lua_Integer v) {
    lua_pushinteger(L, v);
    lua_setfield(L, -2, key);
}

inline void SetStringField(lua_State* L, const char* key, const char* v) {
    lua_pushstring(L, v);
    lua_setfield(L, -2, key);
}

inline void SetFuncField(lua_State* L, const char* key, lua_CFunction fn) {
    lua_pushcfunction(L, fn);
    lua_setfield(L, -2, key);
}

// 把一个 C 枚举的若干键值对注册到栈顶 table（用于构造 ZombieType 等常量表）
struct EnumEntry { const char* name; lua_Integer value; };
inline void RegisterEnum(lua_State* L, const EnumEntry* entries, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        SetIntField(L, entries[i].name, entries[i].value);
    }
}

} // namespace ModLua

// 各对象类型的 metatable 名（用于 luaL_checkudata）
#define MT_BOARD       "PvZ.Board"
#define MT_ZOMBIE      "PvZ.Zombie"
#define MT_PLANT       "PvZ.Plant"
#define MT_PROJECTILE  "PvZ.Projectile"
#define MT_COIN        "PvZ.Coin"
#define MT_GRIDITEM    "PvZ.GridItem"
#define MT_LAWNMOWER   "PvZ.LawnMower"
#define MT_GRAPHICS    "PvZ.Graphics"
#define MT_IMAGE       "PvZ.Image"
#define MT_FONT        "PvZ.Font"
#define MT_REANIMATION "PvZ.Reanimation"
