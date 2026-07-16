-- quick_sunflower_test/main.lua
-- 简化测试 mod：仅快速向日葵（最小化测试，避免自定义僵尸崩溃）
--
-- 植物：快速向日葵
--   - 100 阳光（原版向日葵 50）
--   - 原版向日葵动画（REANIM_SUNFLOWER）
--   - 产阳光速度更快（launch_rate=1000，原版 2500）
--
-- 验证项：
--   1. 图鉴中植物页第二页首位显示快速向日葵（有头像，可点击查看）
--   2. 选种界面第二页首位显示快速向日葵卡片
--   3. 关卡中种植快速向日葵后能产出阳光

local M = {}

local g_plant_type = nil

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
end

print("[quick_sunflower_test] 已加载：快速向日葵")
return M
