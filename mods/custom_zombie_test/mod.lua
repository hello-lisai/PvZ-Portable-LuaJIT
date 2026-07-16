-- custom_zombie_test/mod.lua
-- 验证自定义僵尸初始化缺口修复（InitZombieTypeCustom default 分支）
-- 引擎在解析时会自动在本 table 前补 'return '，所以这里只写纯 table

{
    name        = "自定义僵尸缺口验证",
    version     = "1.0.0",
    author      = "test",
    description = "注册带血量/护甲/能力位字段的坦克僵尸，关卡开始时召唤验证其能正常初始化",
    enabled     = true,
}
