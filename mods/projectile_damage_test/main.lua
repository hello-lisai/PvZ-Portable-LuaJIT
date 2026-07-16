-- projectile_damage_test/main.lua
-- 验证抛射物撞击植物触发 ON_PLANT_TAKE_DAMAGE_PRE 事件
--
-- 测试项：
--   1. 僵尸豌豆/投掷物撞击植物时触发 on_plant_take_damage(plant, zombie, damage)
--   2. 抛射物撞击时 zombie 为 nil（区别于被僵尸吃，zombie 非 nil）
--   3. 返回 {cancel = true} 让向日葵免疫抛射物伤害（植物不掉血，抛射物仍消失）
--   4. 返回 {damage = N} 让其他植物抛射物受伤减半
--
-- 验证方法：
--   - 进入有僵尸豌豆/投掷物的关卡（如 I-Zombie 或含投掷类僵尸的关）
--   - 种向日葵：被抛射物击中时不掉血（cancel=true）
--   - 种其他植物：被抛射物击中时受伤减半
--   - 控制台输出 [projectile_damage_test] 日志，zombie=nil 表示抛射物来源

local M = {}

-- SeedType 常量（见 BindEnums.cpp）
local SEED_SUNFLOWER = 1
local SEED_SPIKEROCK = 46

function M.on_plant_take_damage(plant, zombie, damage)
    if plant == nil then return end

    -- zombie == nil 表示非僵尸直接造成的伤害，包含两种来源：
    --   1. 抛射物撞击植物（Projectile.cpp 行314/615，ctx.projectile 非空，Lua 侧暂未暴露 projectile）
    --   2. Spikerock 被碾压车碾时自伤 50（Plant.cpp SpikeRockTakeDamage，ctx.projectile 为空）
    -- 区别于被僵尸吃（Zombie::EatPlant，zombie 非 nil）
    if zombie ~= nil then return end

    local ptype = plant.type
    local phealth = plant.health
    print(string.format(
        "[projectile_damage_test] non-zombie hit: plant_type=%d plant_health=%d damage=%d",
        ptype, phealth, damage
    ))

    -- Spikerock：免疫非僵尸伤害（碾压车自伤 + 抛射物击中）
    if ptype == SEED_SPIKEROCK then
        print("[projectile_damage_test] spikerock immune to non-zombie damage (cancel)")
        return { cancel = true }
    end

    -- 向日葵：免疫抛射物伤害
    if ptype == SEED_SUNFLOWER then
        print("[projectile_damage_test] sunflower immune to projectile (cancel)")
        return { cancel = true }
    end

    -- 其他植物：非僵尸伤害减半
    print(string.format("[projectile_damage_test] halved damage: %d -> %d", damage, damage // 2))
    return { damage = damage // 2 }
end

print("[projectile_damage_test] 已加载：非僵尸伤害事件（Spikerock/向日葵免疫，其他减半）")
return M
