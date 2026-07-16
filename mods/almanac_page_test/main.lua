-- almanac_page_test/main.lua
-- 验证图鉴翻页功能
--
-- 原版 49 个植物（NUM_ALMANAC_SEEDS=49）占第 1 页
-- 本 mod 注册 5 个自定义植物，它们出现在第 2 页
--
-- 验证项：
--   1. 打开图鉴（主菜单 -> Almanac -> View Plants）
--   2. 第 1 页显示原版 49 个植物（布局不变）
--   3. 底部出现 Prev/Next 翻页按钮 + 页码 "1/2"
--   4. 点 Next 翻到第 2 页，显示 5 个自定义植物
--   5. 页码显示 "2/2"
--   6. 点自定义植物卡片，右侧显示植物预览和描述
--   7. 点 Prev 回到第 1 页

local M = {}

local PLANT_SUBCLASS_SHOOTER = 1
local registered = {}

function M.on_app_init()
    if not pvz or not pvz.plants or not pvz.plants.register then
        print("[almanac_page_test] ERROR: pvz.plants.register 不可用")
        return
    end

    local names = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon"}
    for i, name in ipairs(names) do
        local st = pvz.plants.register({
            name               = "PageTest" .. name,
            cost               = 50 + i * 25,
            refresh            = 5000,
            reanim_type        = pvz.reanim.types.PEASHOOTER,
            packet_index       = 0,
            subclass           = PLANT_SUBCLASS_SHOOTER,
            launch_rate        = 1500,
            almanac_name        = "测试植物 " .. name,
            almanac_description = "图鉴翻页测试用自定义植物 #" .. i .. "\n名称: " .. name .. "\n\n{KEYWORD}伤害:{STAT} 中等\n{KEYWORD}范围:{STAT} 单行",
        })
        table.insert(registered, st)
        print(string.format("[almanac_page_test] registered %s, SeedType=%d", name, st))
    end

    print(string.format("[almanac_page_test] 共注册 %d 个自定义植物，应触发图鉴第 2 页", #registered))
end

print("[almanac_page_test] 已加载：图鉴翻页验证（5 个自定义植物）")
return M
