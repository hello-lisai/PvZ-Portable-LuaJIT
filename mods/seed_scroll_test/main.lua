-- seed_scroll_test/main.lua
-- 验证选种界面滚动条功能
--
-- 原版 53 个内置植物占 7 行（7×8=56 槽位）
-- 本 mod 注册 8 个自定义植物，总计 61 个，占 8 行
-- 第 8 行（row 7, y=613）超出可视区域，需要滚动条才能看到
--
-- 验证项：
--   1. 进入需要选种的关卡（如冒险模式第 1 关）
--   2. 选种界面右侧应出现滚动条（灰色滑块+上下箭头）
--   3. 滚动条不应遮挡返回菜单按钮（右上角）
--   4. 点击下箭头或拖动滑块向下滚动
--   5. 滚动后应看到第 8 行的自定义植物卡片
--   6. 点击自定义植物卡片可正常选入种子栏
--   7. 鼠标滚轮应能滚动选种界面
--   8. 无自定义植物时（禁用此 mod）不应出现滚动条

local M = {}

local PLANT_SUBCLASS_SHOOTER = 1
local registered = {}

function M.on_app_init()
    if not pvz or not pvz.plants or not pvz.plants.register then
        print("[seed_scroll_test] ERROR: pvz.plants.register 不可用")
        return
    end
    if not pvz.reanim or not pvz.reanim.types or not pvz.reanim.types.PEASHOOTER then
        print("[seed_scroll_test] ERROR: pvz.reanim.types.PEASHOOTER 不可用")
        return
    end

    local names = {"Aqua", "Bolt", "Coral", "Dawn", "Echo", "Flame", "Gale", "Halo"}
    for i, name in ipairs(names) do
        local st = pvz.plants.register({
            name               = "ScrollTest" .. name,
            cost               = 75 + i * 25,
            refresh            = 5000,
            reanim_type        = pvz.reanim.types.PEASHOOTER,
            packet_index       = 0,
            subclass           = PLANT_SUBCLASS_SHOOTER,
            launch_rate        = 1500,
            almanac_name        = "滚动测试植物 " .. name,
            almanac_description = "选种滚动条测试用自定义植物 #" .. i .. "\n名称: " .. name .. "\n\n{KEYWORD}伤害:{STAT} 中等\n{KEYWORD}范围:{STAT} 单行",
        })
        table.insert(registered, st)
        print(string.format("[seed_scroll_test] registered %s, SeedType=%d", name, st))
    end

    print(string.format("[seed_scroll_test] 共注册 %d 个自定义植物，总计 %d 个植物（8 行），应触发滚动条",
        #registered, 53 + #registered))
end

function M.on_plant_created(plant)
    if plant == nil or #registered == 0 then return end
    for _, st in ipairs(registered) do
        if plant.type == st then
            print(string.format(
                "[seed_scroll_test] created: type=%d health=%d col=%d row=%d",
                plant.type, plant.health, plant.col, plant.row
            ))
            break
        end
    end
end

print("[seed_scroll_test] 已加载：选种滚动条验证（8 个自定义植物）")
return M
