-- example_mod/mod.lua
-- Mod 清单文件：声明 mod 的元信息和入口
return {
    name        = "示例 Mod",
    version     = "1.0.0",
    author      = "PvZ-Portable Mod API",
    description = "演示 Mod API：事件回调、对象操作、可拦截事件",
    api_version = 0,    -- 对应 pvz.api_version_major（当前 0.x 开发阶段）
    priority    = 100,  -- 加载优先级（数字越小越先加载，默认 100）
    main        = "main.lua",
    enabled     = true,
}
