-- seed_packet_test/main.lua
-- 验证 board:set_seed_packet API
--
-- 测试项：
--   1. 注册一个自定义植物"快速豌豆射手"（复用 PEASHOOTER reanim，冷却短）
--   2. on_level_start 时 board:set_seed_packet 把它添加到种子栏最后一个槽位
--   3. 验证种子卡片能正常显示（图标 + 费用）
--   4. 验证玩家能点击卡片，光标变成植物，在草坪上种植
--   5. 验证种植后植物能正常显示动画并触发 on_update 回调
--
-- 验证方法：
--   - 启动游戏，进入冒险模式任意关卡
--   - 种子栏最后应多出一个豌豆射手卡片（费用 100）
--   - 有 100 阳光时点击卡片，光标变成植物
--   - 点击草坪种植，植物出现并开始发射豌豆
--   - 控制台输出 [seed_packet_test] set_seed_packet / planted / fire 日志

local M = {}

-- 常量
local PROJECTILE_PEA = 0
local FIRE_INTERVAL = 90

local g_custom_type = nil
local g_fire_counters = {}

function M.on_app_init()
    if not pvz or not pvz.plants or not pvz.plants.register then
        print("[seed_packet_test] ERROR: pvz.plants.register 不可用")
        return
    end

    g_custom_type = pvz.plants.register({
        name               = "RapidPeashooter",
        cost               = 100,              -- 便宜一点便于测试
        refresh            = 5000,             -- 短冷却
        reanim_type        = pvz.reanim.types.PEASHOOTER,
        packet_index       = 0,
        subclass           = 1,                -- SHOOTER
        launch_rate        = 750,             -- 快速发射
        almanac_name        = "快速豌豆射手",
        almanac_description = "验证用：种子卡片系统。\n冷却短，发射快。\n\n{KEYWORD}伤害:{STAT} 中等\n{KEYWORD}冷却:{STAT} 短",
        on_update          = function(plant)
            M._on_update(plant)
        end,
    })
    print(string.format("[seed_packet_test] registered RapidPeashooter, SeedType=%d", g_custom_type))
end

function M.on_level_start(board)
    if g_custom_type == nil then
        print("[seed_packet_test] skip: custom type not registered")
        return
    end
    if board == nil then return end

    -- 获取当前种子栏槽位数，把自定义植物放到最后一个槽位
    -- SEEDBANK_MAX = 10，但实际槽位由关卡决定（冒险模式 6-10）
    -- 用 slot 9（第 10 个槽位）确保不覆盖原版种子
    -- 但如果 mNumPackets < 10，set_seed_packet 会自动扩展
    local slot = 9
    local ok = board:set_seed_packet(slot, g_custom_type)
    if ok then
        print(string.format("[seed_packet_test] set_seed_packet(slot=%d, type=%d) -> true", slot, g_custom_type))
    else
        print(string.format("[seed_packet_test] set_seed_packet failed at slot=%d", slot))
    end
end

function M.on_plant_created(plant)
    if plant == nil then return end
    if g_custom_type == nil or plant.type ~= g_custom_type then return end
    print(string.format(
        "[seed_packet_test] planted: type=%d health=%d col=%d row=%d",
        plant.type, plant.health, plant.col, plant.row
    ))
end

function M._on_update(plant)
    if plant.dead then return end
    local board = get_board()
    if board == nil then return end

    local key = tostring(plant.x) .. "_" .. tostring(plant.row)
    local counter = g_fire_counters[key] or FIRE_INTERVAL
    counter = counter - 1

    if counter <= 0 then
        counter = FIRE_INTERVAL
        local px = plant.x + 40
        local py = plant.y - 20
        board:add_projectile(px, py, plant.row, PROJECTILE_PEA)
        print(string.format("[seed_packet_test] fire pea from col=%d row=%d", plant.col, plant.row))
    end

    g_fire_counters[key] = counter
end

print("[seed_packet_test] 已加载：种子卡片系统验证")
return M
