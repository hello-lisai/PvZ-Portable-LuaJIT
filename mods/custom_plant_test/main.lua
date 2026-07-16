-- custom_plant_test/main.lua
-- 验证自定义植物注册 + on_update 行为回调
--
-- 测试项：
--   1. 注册"火焰豌豆射手"（复用 PEASHOOTER 的 reanim，sub class=SHOOTER）
--   2. on_level_start 时在场上放置 2 株验证植物
--   3. on_update 回调中每 90 帧（~1.5s）发射一颗豌豆，验证行为回调能正常触发
--   4. on_plant_created 时打印属性，确认 Plant::Initialize 正确读取了 def 字段
--
-- 验证方法：
--   - 启动游戏，进入任意关卡
--   - 关卡开始时第 2、3 列各出现一株豌豆射手
--   - 控制台输出 [custom_plant_test] registered / spawned / created / fire 日志
--   - 植物应能正常显示动画、每 ~1.5 秒发射一颗豌豆
--
-- 注意：
--   - 不依赖 pvz.SeedType 枚举（运行时可能被覆盖为 nil），直接用数值常量
--   - 种子卡片系统尚未暴露给 mod，所以本 mod 通过 board:add_plant 直接放置植物
--   - on_update 每帧调用，用 mod-level table 维护每株植物的发射计数器

local M = {}

-- 常量（见 src/ConstEnums.h ProjectileType / PlantSubClass）
local PROJECTILE_PEA      = 0   -- 普通豌豆
local PLANT_SUBCLASS_SHOOTER = 1 -- 射手类

-- 发射间隔（帧数，约 1.5 秒 @ 60fps）
local FIRE_INTERVAL = 90

-- 注册得到的 SeedType（on_app_init 后填充）
local g_custom_type = nil

-- 每株植物的发射计数器，key = "x_row"，value = 剩余帧数
local g_fire_counters = {}

function M.on_app_init()
    if not pvz or not pvz.plants or not pvz.plants.register then
        print("[custom_plant_test] ERROR: pvz.plants.register 不可用")
        return
    end
    if not pvz.reanim or not pvz.reanim.types or not pvz.reanim.types.PEASHOOTER then
        print("[custom_plant_test] ERROR: pvz.reanim.types.PEASHOOTER 不可用")
        return
    end

    g_custom_type = pvz.plants.register({
        name               = "FirePeashooter",
        cost               = 200,
        refresh            = 7500,
        reanim_type        = pvz.reanim.types.PEASHOOTER,
        packet_index       = 0,
        subclass           = PLANT_SUBCLASS_SHOOTER,
        launch_rate        = 1500,
        almanac_name        = "火焰豌豆射手",
        almanac_description = "验证用自定义植物。\n每 1.5 秒发射一颗豌豆。\n\n{KEYWORD}伤害:{STAT} 中等\n{KEYWORD}范围:{STAT} 单行",
        on_update          = function(plant)
            M._on_update(plant)
        end,
    })
    print(string.format("[custom_plant_test] registered FirePeashooter, SeedType=%d", g_custom_type))
end

function M.on_level_start(board)
    if g_custom_type == nil then
        print("[custom_plant_test] skip spawn: custom type not registered")
        return
    end
    if board == nil then
        print("[custom_plant_test] skip spawn: board is nil")
        return
    end
    -- 在第 0 行的第 2、3 列各放置一株验证植物
    for _, col in ipairs({2, 3}) do
        local p = board:add_plant(col, 0, g_custom_type)
        if p == nil then
            print(string.format("[custom_plant_test] ERROR: add_plant returned nil at col=%d", col))
        else
            print(string.format("[custom_plant_test] spawned FirePeashooter at col=%d row=0", col))
        end
    end
end

function M.on_plant_created(plant)
    if plant == nil then return end
    if g_custom_type == nil or plant.type ~= g_custom_type then return end
    -- 打印属性，确认 Plant::Initialize 正确设置了字段
    print(string.format(
        "[custom_plant_test] created: type=%d health=%d max_health=%d col=%d row=%d x=%d y=%d",
        plant.type, plant.health, plant.max_health, plant.col, plant.row, plant.x, plant.y
    ))
end

-- on_update 回调：每帧调用，累计帧数到 FIRE_INTERVAL 时发射一颗豌豆
function M._on_update(plant)
    if plant.dead then return end
    local board = get_board()
    if board == nil then return end

    -- 用 plant 的 x+row 作为唯一 key（同一位置只能有一株植物）
    local key = tostring(plant.x) .. "_" .. tostring(plant.row)
    local counter = g_fire_counters[key] or FIRE_INTERVAL
    counter = counter - 1

    if counter <= 0 then
        counter = FIRE_INTERVAL
        -- 发射位置：植物右侧偏上（豌豆射手发射口大致位置）
        local px = plant.x + 40
        local py = plant.y - 20
        board:add_projectile(px, py, plant.row, PROJECTILE_PEA)
        print(string.format("[custom_plant_test] fire pea from col=%d row=%d", plant.col, plant.row))
    end

    g_fire_counters[key] = counter
end

print("[custom_plant_test] 已加载：自定义植物验证")
return M
