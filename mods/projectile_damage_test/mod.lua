-- projectile_damage_test/mod.lua
-- 验证抛射物撞击植物触发 ON_PLANT_TAKE_DAMAGE_PRE 事件

{
    name        = "抛射物伤害事件验证",
    version     = "1.0.0",
    author      = "test",
    description = "验证非僵尸伤害（抛射物撞击 + Spikerock 碾压自伤）触发 on_plant_take_damage（zombie=nil），向日葵/Spikerock 免疫，其他植物受伤减半",
    enabled     = true,
}
