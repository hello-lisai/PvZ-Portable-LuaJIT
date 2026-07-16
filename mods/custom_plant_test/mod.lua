-- custom_plant_test/mod.lua
-- 验证自定义植物注册 + on_update 行为回调
-- 引擎在解析时会自动在本 table 前补 'return '，所以这里只写纯 table

{
    name        = "自定义植物验证",
    version     = "1.0.0",
    author      = "test",
    description = "注册火焰豌豆射手，on_update 中发射豌豆，验证自定义植物能正常显示动画/运行行为回调",
    enabled     = true,
}
