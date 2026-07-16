-- plant_damage_event_test/main.lua
-- 验证 ON_PLANT_TAKE_DAMAGE_PRE 事件
--
-- 测试项：
--   1. 僵尸吃植物时触发 on_plant_take_damage(plant, zombie, damage) 回调
--   2. 回调能正确读取 plant 和 zombie 的属性
--   3. 返回 {damage = N} 修改伤害值（让坚果墙受伤减半）
--   4. 返回 {cancel = true} 完全取消伤害（让豌豆射手免伤）
--
-- 验证方法：
--   - 启动游戏，进入任意关卡
--   - 种一颗坚果墙，让僵尸吃它：受伤减半（DAMAGE_PER_EAT 20 -> 10）
--   - 种一颗豌豆射手，让僵尸吃它：完全免伤（cancel=true）
--   - 控制台输出 [plant_damage_event_test] hit/cancel 日志

local M = {}

-- SeedType 常量（见 BindEnums.cpp）
local SEED_WALLNUT     = 3
local SEED_PEASHOOTER  = 0

function M.on_plant_take_damage(plant, zombie, damage)
    if plant == nil or zombie == nil then return end

    local ptype = plant.type
    local phealth = plant.health
    local ztype = zombie.type
    print(string.format(
        "[plant_damage_event_test] hit: plant_type=%d plant_health=%d zombie_type=%d damage=%d",
        ptype, phealth, ztype, damage
    ))

    -- 坚果墙：受伤减半
    if ptype == SEED_WALLNUT then
        print(string.format("[plant_damage_event_test] walnut halved damage: %d -> %d", damage, damage // 2))
        return { damage = damage // 2 }
    end

    -- 豌豆射手：完全免伤
    if ptype == SEED_PEASHOOTER then
        print("[plant_damage_event_test] peashooter immune (cancel)")
        return { cancel = true }
    end

    -- 其他植物：正常受伤（不返回 = 不修改）
end

print("[plant_damage_event_test] 已加载：植物受伤事件（坚果减半/豌豆免伤）")
return M
