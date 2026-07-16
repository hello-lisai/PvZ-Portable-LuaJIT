-- custom_reanim_test/main.lua
-- 验证 pvz.reanim.register API：动态注册新 ReanimationType
--
-- 测试项：
--   1. 调用 pvz.reanim.register 注册原版 reanim/zombie.reanim，得到新的 ReanimationType
--   2. 用这个新 ReanimationType 注册一个自定义僵尸（视觉同原版，但是是独立的 ReanimationType）
--   3. on_level_start 时召唤验证：动画应能正常加载并显示
--
-- 验证方法：
--   - 启动游戏，进入任意关卡
--   - 关卡开始时屏幕右侧出现一只普通外观的僵尸
--   - 控制台输出 registered_reanim / registered_zombie / spawned / created 四条日志
--   - 新 ReanimationType 应 >= NUM_REANIMS（原版动画数）
--   - 僵尸动画应正常播放（走动/受伤/死亡）
--
-- 注意：
--   - 本 mod 复用原版 reanim/zombie.reanim，所以视觉上和普通僵尸无区别
--   - 但 ReanimationType 是动态分配的新值，验证了 API 能正常工作
--   - 后续 mod 可以用自己 mod 目录下的 .reanim 文件实现真正独特的动画

local M = {}

-- 注册得到的 ReanimationType 和 ZombieType
local g_custom_reanim = nil
local g_custom_type = nil

function M.on_app_init()
    if not pvz or not pvz.reanim or not pvz.reanim.register then
        print("[custom_reanim_test] ERROR: pvz.reanim.register 不可用")
        return
    end

    -- 1. 注册原版 zombie.reanim 文件，得到新的 ReanimationType
    --    路径相对资源目录，原版 .reanim 在 reanim/ 下
    g_custom_reanim = pvz.reanim.register("reanim/Zombie.reanim")
    print(string.format("[custom_reanim_test] registered reanim: type=%d", g_custom_reanim))
    -- 新 ReanimationType 应 >= 原版 NUM_REANIMS（具体值取决于原版动画数量）
    if g_custom_reanim <= 0 then
        print("[custom_reanim_test] ERROR: reanim.register 返回无效值")
        return
    end

    -- 2. 用新 ReanimationType 注册自定义僵尸
    if not pvz.zombies or not pvz.zombies.register then
        print("[custom_reanim_test] ERROR: pvz.zombies.register 不可用")
        return
    end

    g_custom_type = pvz.zombies.register({
        name               = "CustomReanimZombie",
        reanim_type        = g_custom_reanim,  -- 用动态注册的 ReanimationType
        value              = 100,
        starting_level     = 1,
        first_allowed_wave = 1,
        pick_weight        = 0,  -- 不参与随机生成
        -- 用 custom_zombie_test 已验证过的初始化字段
        body_health    = 270,
        abilities      = 1,  -- ABILITY_WALK
        helm_type      = 0,  -- HELMTYPE_NONE
        helm_health    = 0,
        shield_type    = 0,  -- SHIELDTYPE_NONE
        shield_health  = 0,
        almanac_name        = "自定义动画僵尸",
        almanac_description = "验证用：使用动态注册的 ReanimationType。\n外观同普通僵尸，但 ReanimationType 是独立分配的。\n\n{KEYWORD} toughness:{STAT} low\n{KEYWORD} speed:{STAT} normal",
    })
    print(string.format("[custom_reanim_test] registered zombie: type=%d reanim=%d", g_custom_type, g_custom_reanim))
end

function M.on_level_start(board)
    if g_custom_type == nil then
        print("[custom_reanim_test] skip spawn: custom type not registered")
        return
    end
    if board == nil then return end

    local z = board:add_zombie(g_custom_type, 0)
    if z == nil then
        print("[custom_reanim_test] ERROR: add_zombie returned nil")
    else
        print(string.format("[custom_reanim_test] spawned zombie at row 0 (type=%d)", g_custom_type))
    end
end

function M.on_zombie_created(zombie)
    if zombie == nil then return end
    if g_custom_type == nil or zombie.type ~= g_custom_type then return end
    print(string.format(
        "[custom_reanim_test] created: type=%d health=%d abilities=%d",
        zombie.type, zombie.health, zombie.abilities
    ))
end

print("[custom_reanim_test] 已加载：自定义动画注册验证")
return M
