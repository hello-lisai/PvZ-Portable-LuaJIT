-- wave_injection_test/main.lua
-- 演示 ON_PICK_ZOMBIE_WAVES_POST 事件：把自定义僵尸追加到任意波次
--
-- 功能：
--   1. 注册一只"快速僵尸"（套用橄榄球僵尸 reanim，速度更快、血量更低）
--   2. on_pick_zombie_waves_post 时，把自定义僵尸追加到指定波次：
--      - 第 1 波（wave=0）：追加 1 只
--      - 最后一波（wave=num_waves-1）：追加 2 只
--   3. 原版出怪不受影响，自定义僵尸作为额外威胁出现
--   4. 演示 hide_from_preview：让最后一波突袭的快速僵尸不出现在右侧预览
--
-- 验证方法：
--   - 启动游戏，进入任意关卡
--   - 关卡开始前右侧预览应出现 1 只快速僵尸（来自第 1 波）
--   - 最后一波突袭的 2 只快速僵尸不会出现在预览中（制造突袭感）
--   - 第 1 波开始时，场上应有原版僵尸 + 1 只快速僵尸
--   - 最后一波（旗波）应有原版僵尸 + 2 只快速僵尸（突袭出现）
--   - 图鉴中应有"快速僵尸"条目

local M = {}

-- 常量（见 src/Lawn/Zombie.h ABILITY_* / src/ConstEnums.h HelmType / ShieldType）
local ABILITY_WALK      = 1      -- 1 << 0

-- 注册得到的 ZombieType（on_app_init 后填充）
local g_fast_zombie_type = nil

function M.on_app_init()
    if not pvz or not pvz.zombies or not pvz.zombies.register then
        print("[wave_injection_test] ERROR: pvz.zombies.register 不可用")
        return
    end
    if not pvz.reanim or not pvz.reanim.types or not pvz.reanim.types.ZOMBIE_FOOTBALL then
        print("[wave_injection_test] ERROR: pvz.reanim.types.ZOMBIE_FOOTBALL 不可用")
        return
    end

    g_fast_zombie_type = pvz.zombies.register({
        name               = "FastZombie",
        reanim_type        = pvz.reanim.types.ZOMBIE_FOOTBALL,
        value              = 50,
        starting_level     = 1,
        first_allowed_wave = 1,
        pick_weight        = 0,            -- 不参与随机生成，仅通过波次注入出现
        body_health        = 100,          -- 比普通僵尸 270 更脆
        abilities          = ABILITY_WALK,
        helm_type          = 0,            -- 无头盔
        helm_health        = 0,
        shield_type        = 0,            -- 无护甲
        shield_health      = 0,
        almanac_name        = "快速僵尸",
        almanac_description = "移动速度极快的脆弱僵尸。\n\n{KEYWORD} toughness:{STAT} low\n{KEYWORD} speed:{STAT} very fast",
    })
    print(string.format("[wave_injection_test] registered FastZombie, ZombieType=%d", g_fast_zombie_type))
end

-- ON_PICK_ZOMBIE_WAVES_POST: board, level, num_waves
-- 原版波次已生成，此处往指定波次追加自定义僵尸
function M.on_pick_zombie_waves_post(board, level, num_waves)
    if g_fast_zombie_type == nil then
        print("[wave_injection_test] skip injection: custom type not registered")
        return
    end
    if num_waves == nil or num_waves <= 0 then
        return
    end

    -- 构建追加计划：wave 索引 0-based
    -- 注意：Lua 表用 1-based 索引时引擎会自动转 0-based，也可以直接用 0-based
    local append_plan = {}
    -- 第 1 波追加 1 只快速僵尸（会出现在右侧预览中）
    append_plan[0] = { g_fast_zombie_type }
    -- 最后一波（旗波）追加 2 只快速僵尸（通过 hide_from_preview 隐藏预览，制造突袭感）
    local last_wave = num_waves - 1
    if last_wave > 0 then
        append_plan[last_wave] = { g_fast_zombie_type, g_fast_zombie_type }
    end

    print(string.format("[wave_injection_test] injecting FastZombie: wave 0 (%d), wave %d (%d)",
        1, last_wave, append_plan[last_wave] and #append_plan[last_wave] or 0))

    -- hide_from_preview: 标记快速僵尸不参与最后一波的预览
    -- 但因为第 1 波也有该类型，所以预览中仍会出现 1 只（来自第 1 波）
    -- 要完全隐藏某类型，不要在任何"会预览"的波次中加入它
    -- 这里演示如何隐藏整个类型：取消下面一行的注释即可让快速僵尸完全不出现在预览中
    -- return { append = append_plan, hide_from_preview = { g_fast_zombie_type } }

    return { append = append_plan }
end

-- 打印验证日志
function M.on_zombie_created(zombie)
    if zombie == nil or g_fast_zombie_type == nil then return end
    if zombie.type ~= g_fast_zombie_type then return end
    print(string.format("[wave_injection_test] FastZombie created: health=%d max_health=%d",
        zombie.health, zombie.max_health))
end

print("[wave_injection_test] 已加载：波次注入测试")
return M

