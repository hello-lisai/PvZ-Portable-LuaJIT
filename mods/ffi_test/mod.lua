-- ffi_test/mod.lua
-- Mod 清单：FFI 直调测试
-- 演示两种"极高自由度"的 mod 开发方式：
--   1. ffi.C.pvz_xxx(ptr, ...) 直调游戏导出的 C 函数
--   2. ffi.cast + offset 直接读写对象内存字段（绕过所有 getter/setter）

{
    name        = "FFI 直调测试",
    version     = "1.0.0",
    author      = "PvZ-Portable Mod API",
    description = "演示 LuaJIT FFI 直调游戏函数 + 内存偏移读写（普通僵尸秒杀 + 血量翻倍）",
    api_version = 0,
    priority    = 100,
    main        = "main.lua",
    enabled     = true,
}
