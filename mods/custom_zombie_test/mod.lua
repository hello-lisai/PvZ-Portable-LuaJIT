-- custom_zombie_test/mod.lua
-- 验证自定义僵尸完整支持：
--   1. ID 冲突修复（自定义僵尸从 NUM_CACHED_ZOMBIE_TYPES 开始，避让 ZOMBIE_CACHED_POLEVAULTER_WITH_POLE）
--   2. SetupReanimLayers 根据 helm_type/shield_type 显示装饰层
--   3. GetZombieFallTime 自定义僵尸正常倒地
--   4. DrawCachedZombie 图鉴网格正确显示自定义僵尸（非 polevaulter 带杆姿态）
--   5. 图鉴预览与关卡内僵尸形象一致（两者都走 ZombieInitialize 路径）
-- 引擎在解析时会自动在本 table 前补 'return '，所以这里只写纯 table

{
    name        = "自定义僵尸完整验证",
    version     = "1.1.0",
    author      = "test",
    description = "注册带桶头盔+门护甲的坦克僵尸，验证图鉴与关卡内形象一致、装饰层正确显示、死亡正常倒地",
    enabled     = true,
}
