-- almanac_test/main.lua
-- 图鉴自定义植物/僵尸测试 mod
--
-- 测试项：
--   1. 注册一个自定义植物（复用 PEASHOOTER 的 reanim，用绿色着色区分）
--   2. 注册一个自定义僵尸（复用 ZOMBIE 的 reanim）
--   3. 打开图鉴查看是否显示
--
-- 验证方法：
--   - 启动游戏，主菜单点"图鉴"（Suburban Almanac）
--   - 植物页应多出"自定义豌豆"卡片，僵尸页应多出"自定义僵尸"卡片
--   - 点击卡片应显示名称和描述

local log = function(msg)
    print("[almanac_test] " .. tostring(msg))
end

return {
    on_app_init = function()
        log("almanac_test mod loaded")

        -- 检查 pvz.reanim.types 是否可用
        if not pvz.reanim or not pvz.reanim.types then
            log("ERROR: pvz.reanim.types not available")
            return
        end
        log("pvz.reanim.types available:")
        for k, v in pairs(pvz.reanim.types) do
            log("  " .. tostring(k) .. " = " .. tostring(v))
        end

        -- ====== 1. 注册自定义植物 ======
        -- 复用 PEASHOOTER 的 reanim，这样图鉴里能看到一个绿色的豌豆射手
        local custom_plant = pvz.plants.register({
            name         = "CustomPeashooter",
            cost         = 200,
            refresh      = 7500,
            reanim_type  = pvz.reanim.types.PEASHOOTER,
            packet_index = 0,
            subclass     = 0,  -- SUBCLASS_NORMAL
            launch_rate  = 1500,
            almanac_name        = "自定义豌豆射手",
            almanac_description = "这是一个由 mod 注册的测试植物。\n它复用了原版豌豆射手的动画，\n但在图鉴中显示为独立条目。\n\n{KEYWORD}成本:{STAT} 200 阳光\n{KEYWORD}冷却:{STAT} 短",
        })
        log("registered custom plant, SeedType=" .. tostring(custom_plant))

        -- ====== 2. 注册自定义僵尸 ======
        -- 复用 ZOMBIE 的 reanim
        local custom_zombie = pvz.zombies.register({
            name               = "CustomZombie",
            reanim_type        = pvz.reanim.types.ZOMBIE,
            value              = 100,
            starting_level     = 1,
            first_allowed_wave = 1,
            pick_weight        = 10,
            almanac_name        = "自定义僵尸",
            almanac_description = "这是一个由 mod 注册的测试僵尸。\n它复用了原版普通僵尸的动画，\n但在图鉴中显示为独立条目。\n\n{KEYWORD} toughness:{STAT} low\n{KEYWORD} speed:{STAT} normal",
        })
        log("registered custom zombie, ZombieType=" .. tostring(custom_zombie))

        log("now open the Almanac to verify custom entries appear")
    end,
}
