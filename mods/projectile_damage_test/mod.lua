-- projectile_damage_test/mod.lua
-- 验证抛射物撞击植物触发 ON_PLANT_TAKE_DAMAGE_PRE 事件

{
    name        = "抛射物伤害事件验证",
    version     = "1.0.0",
    author      = "test",
    description = "验证僵尸豌豆/投掷物撞击植物时触发 on_plant_take_damage（zombie=nil 区分来源），向日葵免疫抛射物，其他植物受伤减半",
    enabled     = true,
}
