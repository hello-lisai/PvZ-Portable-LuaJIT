-- quick_sunflower_test/main.lua
-- 简化测试 mod：快速向日葵 + 跑步僵尸
--
-- 植物：快速向日葵
--   - 100 阳光（原版向日葵 50）
--   - 原版向日葵动画（REANIM_SUNFLOWER）
--   - 产阳光速度更快（launch_rate=1000，原版 2500）
--
-- 僵尸：跑步僵尸
--   - 使用原版普通僵尸动画
--   - 速度更快（通过 on_zombie_created 修改 vel_x）
--
-- 验证项：
--   1. 图鉴中植物页显示快速向日葵（有头像，可点击查看）
--   2. 图鉴中僵尸页显示跑步僵尸（有头像，可点击查看）
--   3. 选种界面显示快速向日葵卡片
--   4. 关卡中召唤的跑步僵尸速度更快

local M = {}

local g_plant_type = nil
local g_zombie_type = nil

function M.on_app_init()
    -- 注册快速向日葵
    if pvz and pvz.plants and pvz.plants.register and pvz.reanim and pvz.reanim.types then
        g_plant_type = pvz.plants.register({
            name               = "QuickSunflower",
            cost               = 100,
            refresh            = 750,
            reanim_type        = pvz.reanim.types.SUNFLOWER,
            packet_index       = 1,
            subclass           = 0,
            launch_rate        = 1000,
            makes_sun          = true,
            seed_sort_order    = 49,  -- 排在第二页首位（原版 49 个植物在第一页）
            almanac_name        = "快速向日葵",
            almanac_description = "产阳光速度比普通向日葵快一倍。\n\n{KEYWORD}阳光产出:{STAT} 每 10 秒\n{KEYWORD}成本:{STAT} 100 阳光\n{KEYWORD}冷却:{STAT} 短",
        })
        print(string.format("[quick_sunflower_test] registered QuickSunflower, SeedType=%d", g_plant_type))
    end

    -- 注册跑步僵尸
    if pvz and pvz.zombies and pvz.zombies.register then
        g_zombie_type = pvz.zombies.register({
            name               = "RunnerZombie",
            reanim_type        = pvz.reanim.types.ZOMBIE,
            value              = 100,
            starting_level     = 1,
            first_allowed_wave = 1,
            pick_weight        = 0,
            body_health    = 270,
            abilities      = 1,
            helm_type      = 0,
            helm_health    = 0,
            shield_type    = 0,
            shield_health  = 0,
            almanac_name        = "跑步僵尸",
            almanac_description = "移动速度比普通僵尸快的变体。\n\n{KEYWORD} toughness:{STAT} low\n{KEYWORD} speed:{STAT} fast",
        })
        print(string.format("[quick_sunflower_test] registered RunnerZombie, ZombieType=%d", g_zombie_type))
    end
end

-- 关卡开始时召唤一只跑步僵尸
function M.on_level_start(board)
    if g_zombie_type == nil or board == nil then return end
    local z = board:add_zombie(g_zombie_type, 0)
    if z then
        print("[quick_sunflower_test] spawned RunnerZombie at row 0")
    end
end

-- 僵尸创建时加速
function M.on_zombie_created(zombie)
    if zombie == nil or g_zombie_type == nil then return end
    if zombie.type ~= g_zombie_type then return end
    -- 速度设为 2 倍（原版普通僵尸约 0.15-0.3，这里直接乘 2）
    if zombie.vel_x ~= nil then
        zombie.vel_x = zombie.vel_x * 2
    end
    print(string.format("[quick_sunflower_test] RunnerZombie created: health=%d vel_x=%g", zombie.health, zombie.vel_x or 0))
end

print("[quick_sunflower_test] 已加载：快速向日葵 + 跑步僵尸")
return M
