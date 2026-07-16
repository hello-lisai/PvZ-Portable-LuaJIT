-- custom_reanim_test/mod.lua
-- 验证 pvz.reanim.register API：动态注册新 ReanimationType
-- 引擎在解析时会自动在本 table 前补 'return '

{
    name        = "自定义动画注册验证",
    version     = "1.0.0",
    author      = "test",
    description = "用 pvz.reanim.register 注册原版 zombie.reanim 得到新 ReanimationType，注册自定义僵尸使用它，验证动态动画注册流程",
    enabled     = true,
}
