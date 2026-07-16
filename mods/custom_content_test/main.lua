-- custom_content_test/main.lua
-- 综合测试 mod：自定义植物 + 自定义僵尸
--
-- 验证项：
--   1. 图鉴中自定义植物有头像（DrawCachedPlant 修复）
--   2. 图鉴中自定义僵尸有头像（DrawCachedZombie 修复）
--   3. 图鉴翻页正常（49 个原版植物占第 1 页，自定义植物在第 2 页）
--   4. 选种界面翻页正常（53 个原版植物 + 5 个自定义 = 58 > 56，触发翻页）
--   5. 选种界面翻页按钮在开始按钮旁边，不遮挡其他按钮
--   6. 自定义僵尸在关卡中正常出现
--
-- 使用方法：
--   - 启用本 mod，进入主菜单 -> Almanac 查看图鉴
--   - 进入任意关卡查看选种界面翻页
--   - 关卡开始时会出现一只坦克僵尸

local M = {}

-- 常量
local PLANT_SUBCLASS_SHOOTER = 1
local ABILITY_WALK      = 1
local HELMTYPE_PAIL     = 2
local SHIELDTYPE_DOOR   = 1

local g_custom_zombie_type = nil

-- 注册 5 个自定义植物
function M.on_app_init()
    -- 注册植物
    if pvz and pvz.plants and pvz.plants.register then
        local plant_names = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon"}
        for i, name in ipairs(plant_names) do
            local st = pvz.plants.register({
                name               = "TestPlant" .. name,
                cost               = 50 + i * 25,
                refresh            = 5000,
                reanim_type        = pvz.reanim.types.PEASHOOTER,
                packet_index       = 0,
                subclass           = PLANT_SUBCLASS_SHOOTER,
                launch_rate        = 1500,
                almanac_name        = "测试植物 " .. name,
                almanac_description = "自定义植物 #" .. i .. "\n名称: " .. name .. "\n\n{KEYWORD}伤害:{STAT} 中等\n{KEYWORD}范围:{STAT} 单行",
            })
            print(string.format("[custom_content_test] registered plant %s, SeedType=%d", name, st))
        end
        print(string.format("[custom_content_test] 共注册 %d 个自定义植物", #plant_names))
    else
        print("[custom_content_test] WARNING: pvz.plants.register 不可用")
    end

    -- 注册僵尸
    if pvz and pvz.zombies and pvz.zombies.register and pvz.reanim and pvz.reanim.types then
        g_custom_zombie_type = pvz.zombies.register({
            name               = "TankZombie",
            reanim_type        = pvz.reanim.types.ZOMBIE,
            value              = 200,
            starting_level     = 1,
            first_allowed_wave = 1,
            pick_weight        = 0,
            body_health    = 500,
            abilities      = ABILITY_WALK,
            helm_type      = HELMTYPE_PAIL,
            helm_health    = 1100,
            shield_type    = SHIELDTYPE_DOOR,
            shield_health  = 400,
            almanac_name        = "坦克僵尸",
            almanac_description = "验证用自定义僵尸。\n带桶头盔和门护甲，本体血量 500。\n\n{KEYWORD} toughness:{STAT} high\n{KEYWORD} speed:{STAT} normal",
        })
        print(string.format("[custom_content_test] registered zombie TankZombie, ZombieType=%d", g_custom_zombie_type))
    else
        print("[custom_content_test] WARNING: pvz.zombies.register 不可用")
    end
end

-- 关卡开始时召唤一只自定义僵尸
function M.on_level_start(board)
    if g_custom_zombie_type == nil or board == nil then return end
    local z = board:add_zombie(g_custom_zombie_type, 0)
    if z then
        print(string.format("[custom_content_test] spawned TankZombie at row 0"))
    end
end

-- 验证僵尸属性
function M.on_zombie_created(zombie)
    if zombie == nil or g_custom_zombie_type == nil then return end
    if zombie.type ~= g_custom_zombie_type then return end
    print(string.format(
        "[custom_content_test] zombie created: type=%d health=%d max_health=%d helm=%d shield=%d",
        zombie.type, zombie.health, zombie.max_health, zombie.helm_health, zombie.shield_health
    ))
end

print("[custom_content_test] 已加载：自定义植物 + 僵尸综合测试")
return M
