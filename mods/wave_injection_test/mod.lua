-- wave_injection_test/mod.lua
-- 验证 ON_PICK_ZOMBIE_WAVES_POST 事件：把自定义僵尸追加到任意波次
-- 引擎在解析时会自动在本 table 前补 'return '，所以这里只写纯 table

{
    name        = "波次注入测试",
    version     = "1.0.0",
    author      = "test",
    description = "注册自定义僵尸并注入到指定波次（不影响原版出怪，仅追加）",
    enabled     = true,
}
