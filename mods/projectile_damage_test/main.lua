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

function M.on_plant_take_damage(plant, zombie, damage)
    if plant == nil then return end

    -- zombie == nil 表示抛射物撞击（区别于被僵尸吃，那里 zombie 非 nil）
    if zombie ~= nil then return end

    local ptype = plant.type
    local phealth = plant.health
    print(string.format(
        "[projectile_damage_test] projectile hit: plant_type=%d plant_health=%d damage=%d",
        ptype, phealth, damage
    ))

    -- 向日葵：免疫抛射物伤害
    if ptype == SEED_SUNFLOWER then
        print("[projectile_damage_test] sunflower immune to projectile (cancel)")
        return { cancel = true }
    end

    -- 其他植物：抛射物受伤减半
    print(string.format("[projectile_damage_test] halved projectile damage: %d -> %d", damage, damage // 2))
    return { damage = damage // 2 }
end

print("[projectile_damage_test] 已加载：抛射物伤害事件（向日葵免疫/其他减半）")
return M
