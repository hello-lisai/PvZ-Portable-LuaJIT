// BindSol2.cpp — sol2 批量绑定入口
//
// 此文件包含自动生成的 sol2 绑定代码（BindSol2Generated.inl），
// 并提供 BindSol2() 函数供 LuaRuntime::Initialize() 调用。
//
// sol2 usertype 替换原有的手动 metatable 绑定：
//   - 原有 Bind*.cpp 中的 metatable 先创建，sol2 随后覆盖 registry 中的同名 metatable
//   - 原有 Push* 函数（NewUserdata + luaL_setmetatable）创建的 userdata 自动获得 sol2 metatable
//   - userdata 内部格式一致（均存储 T* 指针），sol2 的 __index/__newindex 可正确工作
//
// 生成方式: python tools/gen_sol2_bindings.py
// 绑定规模: 327 成员 + 513 方法（7 个类）

#include "LuaBindUtil.h"
#include "BindSol2Generated.inl"

namespace ModLua {

namespace {

// 将 sol2 usertype 的 metatable 设置到 registry 中，覆盖原有 Bind*.cpp 创建的 metatable。
// 这样原有 Push* 函数（使用 luaL_setmetatable）创建的 userdata 也能使用 sol2 绑定的所有方法。
//
// sol2 new_usertype<T>("PvZ.Zombie", ...) 会在全局表创建 _G.PvZ.Zombie（usertype table），
// 该 table 的 metatable 就是包含所有绑定方法的 usertype metatable。
// 我们把它取出并写入 registry["PvZ.Zombie"]，覆盖原有 metatable。
void OverwriteRegistryMetatable(lua_State* L, const char* globalTable, const char* typeName, const char* mtName) {
    // _G[globalTable][typeName] → usertype table
    lua_getglobal(L, globalTable);       // push PvZ
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return;
    }
    lua_getfield(L, -1, typeName);       // push PvZ.Zombie
    if (!lua_istable(L, -1)) {
        lua_pop(L, 2);
        return;
    }
    // 获取 usertype table 的 metatable
    if (lua_getmetatable(L, -1)) {
        // registry[mtName] = metatable（弹出 metatable）
        lua_setfield(L, LUA_REGISTRYINDEX, mtName);
    }
    lua_pop(L, 2);  // 弹出 PvZ.Zombie 和 PvZ
}

} // namespace

// 注册所有 sol2 usertype，并覆盖原有 registry metatable
// 必须在所有 Bind*() 之后调用，以便覆盖旧 metatable
void BindSol2(lua_State* L) {
    sol::state_view lua(L);
    BindAllSol2(lua);

    // 覆盖 registry 中的 7 个 metatable，使原有 Push* 函数创建的 userdata
    // 也能使用 sol2 绑定的所有方法和属性
    OverwriteRegistryMetatable(L, "PvZ", "Zombie",     MT_ZOMBIE);
    OverwriteRegistryMetatable(L, "PvZ", "Plant",      MT_PLANT);
    OverwriteRegistryMetatable(L, "PvZ", "Board",      MT_BOARD);
    OverwriteRegistryMetatable(L, "PvZ", "Projectile", MT_PROJECTILE);
    OverwriteRegistryMetatable(L, "PvZ", "Coin",       MT_COIN);
    OverwriteRegistryMetatable(L, "PvZ", "GridItem",   MT_GRIDITEM);
    OverwriteRegistryMetatable(L, "PvZ", "LawnMower",  MT_LAWNMOWER);
}

} // namespace ModLua
