-- resource_test/main.lua
-- 资源覆盖 + 自定义动画测试 mod
--
-- 测试项：
--   1. 验证 pvz.resources.mount/unmount API 可用
--   2. 验证 mod 目录已自动挂载为覆盖目录
--   3. 注册一个自定义 .reanim 动画文件（复用原版文件路径测试覆盖机制）
--   4. 注册使用该动画的自定义植物，在图鉴中查看
--
-- 验证方法：
--   - 启动游戏，查看 log.txt 中 [resource_test] 日志
--   - 打开图鉴，查看自定义植物条目
--   - 如果 mod 目录下放了自定义 .reanim 文件，应能看到动画变化

local log = function(msg)
    print("[resource_test] " .. tostring(msg))
end

return {
    on_app_init = function()
        log("resource_test mod loaded")

        -- ====== 1. 验证 pvz.resources API ======
        if not pvz.resources then
            log("ERROR: pvz.resources not available")
            return
        end
        log("pvz.resources.mount = " .. tostring(pvz.resources.mount))
        log("pvz.resources.unmount = " .. tostring(pvz.resources.unmount))

        -- ====== 2. 验证 mod 目录已自动挂载 ======
        -- mod 目录在 LoadMod 时已自动挂载，这里手动再挂载一次测试 API
        local mod_dir = __MOD_DIR__ or ""
        log("mod dir = " .. tostring(mod_dir))
        if mod_dir ~= "" then
            local ok = pvz.resources.mount(mod_dir)
            log("manual re-mount result = " .. tostring(ok))
        end

        -- ====== 3. 注册自定义 .reanim 动画 ======
        -- 尝试注册 mod 目录下的 reanim/custom_plant.reanim
        -- 如果文件不存在，会注册成功但加载时失败（图鉴中显示空白）
        -- 如果文件存在，会正常加载并显示
        local reanim_path = "reanim/custom_plant.reanim"
        log("registering reanim: " .. reanim_path)
        local custom_reanim = pvz.reanim.register(reanim_path)
        log("registered reanim type = " .. tostring(custom_reanim))

        -- ====== 4. 注册使用自定义动画的植物 ======
        local custom_plant = pvz.plants.register({
            name         = "ResourceTestPlant",
            cost         = 300,
            refresh      = 10000,
            reanim_type  = custom_reanim,  -- 使用自定义动画
            packet_index = 0,
            subclass     = 0,
            launch_rate  = 2000,
            almanac_name        = "资源测试植物",
            almanac_description = "这个植物使用自定义 .reanim 动画文件。\n" ..
                                  "如果 mod 目录下有 reanim/custom_plant.reanim，\n" ..
                                  "图鉴中会显示该动画。\n\n" ..
                                  "{KEYWORD}成本:{STAT} 300 阳光\n" ..
                                  "{KEYWORD}冷却:{STAT} 长\n\n" ..
                                  "提示：如果动画加载失败，图鉴会显示空白",
        })
        log("registered custom plant, SeedType = " .. tostring(custom_plant))

        -- ====== 5. 同时注册一个复用原版动画的植物（作为对照）======
        local fallback_plant = pvz.plants.register({
            name         = "FallbackPlant",
            cost         = 100,
            refresh      = 5000,
            reanim_type  = pvz.reanim.types.SUNFLOWER,  -- 复用原版向日葵动画
            packet_index = 0,
            subclass     = 0,
            launch_rate  = 1000,
            almanac_name        = "对照植物（原版动画）",
            almanac_description = "这个植物复用原版向日葵动画作为对照。\n" ..
                                  "如果上面的资源测试植物显示空白，\n" ..
                                  "但这个正常显示，说明 .reanim 文件加载有问题。",
        })
        log("registered fallback plant, SeedType = " .. tostring(fallback_plant))

        log("now open Almanac to verify:")
        log("  - '资源测试植物' should use custom reanim (may be blank if file missing)")
        log("  - '对照植物（原版动画）' should show sunflower animation")
    end,
}
