-- seed_chooser_test/main.lua
-- 验证 SeedChooserScreen 集成：自定义植物出现在选种界面并可被选择
--
-- 测试项：
--   1. 注册一个自定义植物"选种测试射手"（复用 PEASHOOTER reanim）
--   2. 进入需要选种的关卡（如冒险模式第 1 关）
--   3. 选种界面应显示自定义植物卡片（在内置植物下方第 8 行）
--   4. 点击卡片应能选入种子栏
--   5. 开始关卡后选中该卡片可正常种植
--
-- 验证方法：
--   - 启动游戏，进入冒险模式任意需要选种的关卡
--   - 选种界面底部（第 8 行起）应出现自定义植物卡片
--   - 点击卡片选入种子栏，点 Lets Rock 开始
--   - 选中卡片种植，控制台输出 [seed_chooser_test] created 日志

local M = {}

local PLANT_SUBCLASS_SHOOTER = 1
local g_custom_type = nil

function M.on_app_init()
    if not pvz or not pvz.plants or not pvz.plants.register then
        print("[seed_chooser_test] ERROR: pvz.plants.register 不可用")
        return
    end
    if not pvz.reanim or not pvz.reanim.types or not pvz.reanim.types.PEASHOOTER then
        print("[seed_chooser_test] ERROR: pvz.reanim.types.PEASHOOTER 不可用")
        return
    end

    g_custom_type = pvz.plants.register({
        name               = "ChooserTestShooter",
        cost               = 100,
        refresh            = 5000,
        reanim_type        = pvz.reanim.types.PEASHOOTER,
        packet_index       = 0,
        subclass           = PLANT_SUBCLASS_SHOOTER,
        launch_rate        = 1500,
        almanac_name        = "选种测试射手",
        almanac_description = "验证用自定义植物。\n用于测试选种界面集成。\n\n{KEYWORD}伤害:{STAT} 中等\n{KEYWORD}范围:{STAT} 单行",
    })
    print(string.format("[seed_chooser_test] registered ChooserTestShooter, SeedType=%d", g_custom_type))
    print(string.format("[seed_chooser_test] 选种界面应在此植物类型=%d 的位置显示卡片", g_custom_type))
end

function M.on_plant_created(plant)
    if plant == nil or g_custom_type == nil then return end
    if plant.type ~= g_custom_type then return end
    print(string.format(
        "[seed_chooser_test] created: type=%d health=%d col=%d row=%d",
        plant.type, plant.health, plant.col, plant.row
    ))
end

print("[seed_chooser_test] 已加载：选种界面集成验证")
return M
