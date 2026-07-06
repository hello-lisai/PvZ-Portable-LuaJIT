-- zombie_3x_hp/mod.lua
-- Mod 清单：普通僵尸血量扩大三倍
-- 注意：引擎的 ParseManifest 会在内容前拼 "return "，
-- 所以 mod.lua 内容本身不能写 return 关键字，必须是纯 table 表达式。
{
    name        = "普通僵尸血量x3",
    version     = "1.0.0",
    author      = "PvZ-Portable Mod API",
    description = "所有 ZOMBIE_NORMAL 类型僵尸的血量和最大血量扩大到 3 倍",
    api_version = 0,
    priority    = 100,
    main        = "main.lua",
    enabled     = true,
}
