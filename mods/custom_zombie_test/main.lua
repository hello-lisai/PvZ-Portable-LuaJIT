-- custom_zombie_test/main.lua
-- 验证自定义僵尸初始化缺口修复：
--   1. 注册一只"坦克僵尸"，覆盖所有新增字段（body_health/abilities/helm/shield）
--   2. on_level_start 时在场上召唤一只，验证它能正常出现/移动/受伤/显示动画
--   3. on_zombie_created 时打印属性，确认 InitZombieTypeCustom 正确读取了 def 字段
--
-- 验证方法：
--   - 启动游戏，进入任意关卡
--   - 关卡开始时屏幕右侧出现一只带桶头盔 + 门护甲的"坦克僵尸"
--   - 控制台应输出 [custom_zombie_test] registered / spawned / created 三条日志
--   - 僵尸应能正常行走、被豌豆逐步打掉头盔血量、最终死亡
--
-- 注意：
--   - 不依赖 pvz.ZombieType 枚举（运行时可能被覆盖为 nil），直接用数值常量
--   - pick_weight=0，不参与随机波次生成，只由本 mod 主动召唤，避免污染正常关卡

local M = {}

-- 常量（见 src/Lawn/Zombie.h ABILITY_* / src/ConstEnums.h HelmType / ShieldType）
local ABILITY_WALK      = 1      -- 1 << 0
local HELMTYPE_PAIL     = 2      -- 桶头盔（Buckethead 同款）
local SHIELDTYPE_DOOR   = 1      -- 门护甲

-- 注册得到的 ZombieType（on_app_init 后填充，nil 表示尚未注册）
local g_custom_type = nil

function M.on_app_init()
    if not pvz or not pvz.zombies or not pvz.zombies.register then
        print("[custom_zombie_test] ERROR: pvz.zombies.register 不可用")
        return
    end
    if not pvz.reanim or not pvz.reanim.types or not pvz.reanim.types.ZOMBIE then
        print("[custom_zombie_test] ERROR: pvz.reanim.types.ZOMBIE 不可用")
        return
    end

    g_custom_type = pvz.zombies.register({
        name               = "TankZombie",
        reanim_type        = pvz.reanim.types.ZOMBIE,
        value              = 200,
        starting_level     = 1,
        first_allowed_wave = 1,
        pick_weight        = 0,            -- 不参与随机生成，仅由本 mod 主动召唤
        -- 新增初始化字段（ZombieInitialize default 分支经 InitZombieTypeCustom 读取）
        body_health    = 500,             -- 比普通僵尸 270 更硬
        abilities      = ABILITY_WALK,    -- 基本行走
        helm_type      = HELMTYPE_PAIL,   -- 桶头盔
        helm_health    = 1100,            -- 与 Buckethead 同款
        shield_type    = SHIELDTYPE_DOOR, -- 门护甲
        shield_health  = 400,
        -- 图鉴字段（便于同时验证图鉴条目）
        almanac_name        = "坦克僵尸",
        almanac_description = "验证用自定义僵尸。\n带桶头盔和门护甲，本体血量 500。\n\n{KEYWORD} toughness:{STAT} high\n{KEYWORD} speed:{STAT} normal",
    })
    print(string.format("[custom_zombie_test] registered TankZombie, ZombieType=%d", g_custom_type))
end

function M.on_level_start(board)
    if g_custom_type == nil then
        print("[custom_zombie_test] skip spawn: custom type not registered")
        return
    end
    if board == nil then
        print("[custom_zombie_test] skip spawn: board is nil")
        return
    end
    -- 在第 0 行生成一只验证僵尸（from_wave 省略，默认当前波）
    local z = board:add_zombie(g_custom_type, 0)
    if z == nil then
        print("[custom_zombie_test] ERROR: add_zombie returned nil")
    else
        print(string.format("[custom_zombie_test] spawned TankZombie at row 0 (userdata=%s)", tostring(z)))
    end
end

function M.on_zombie_created(zombie)
    if zombie == nil then return end
    -- 只关心本 mod 注册的自定义类型
    if g_custom_type == nil or zombie.type ~= g_custom_type then return end
    -- 打印属性，确认 InitZombieTypeCustom 把 def 字段正确写入对象
    print(string.format(
        "[custom_zombie_test] created: type=%d health=%d max_health=%d helm_health=%d shield_health=%d abilities=%d",
        zombie.type, zombie.health, zombie.max_health, zombie.helm_health, zombie.shield_health, zombie.abilities
    ))
end

print("[custom_zombie_test] 已加载：自定义僵尸缺口验证")
return M
