-- plant_die_event_test/main.lua
-- 验证 ON_PLANT_DIE_PRE 事件
--
-- 测试项：
--   1. 任意植物死亡时（被僵尸吃掉/铲除/爆炸等）触发 on_plant_die 回调
--   2. 回调中能正确读取 plant 对象属性（type/health/col/row）
--   3. 在回调中调用 board:add_coin 掉落一颗小阳光（25）作为"死亡遗物"
--
-- 验证方法：
--   - 启动游戏，进入任意关卡
--   - 种一颗植物，让僵尸吃掉它
--   - 控制台输出 [plant_die_event_test] died: type=X health=0 col=X row=X
--   - 植物死亡位置应出现一颗 25 阳光
--
-- 注意：
--   - 事件在 Plant::Die 入口触发，此时 plant.dead 还未设为 true
--   - plant.health 可能已经是 0（被吃光）或负数（被爆炸瞬杀）
--   - 用 plant.col/plant.row 拿到植物所在格子，转换为像素坐标后掉落阳光

local M = {}

-- CoinType（见 src/ConstEnums.h）
local COIN_SUN = 6        -- 阳光（小）
-- CoinMotion
local COIN_MOTION_FALL = 2 -- 从天上掉下来

function M.on_plant_die(plant)
    if plant == nil then return end

    -- 读取植物属性，确认回调能拿到完整对象
    local ptype  = plant.type
    local phealth = plant.health
    local pcol   = plant.col
    local prow   = plant.row
    print(string.format(
        "[plant_die_event_test] died: type=%d health=%d col=%d row=%d",
        ptype, phealth, pcol, prow
    ))

    -- 在植物位置掉落一颗 25 阳光
    local board = get_board()
    if board == nil then
        print("[plant_die_event_test] WARN: board is nil, skip sun drop")
        return
    end

    -- 把格子坐标转成像素坐标（GridToPixelX/Y 需要 col/row）
    -- 注意 board:grid_to_pixel 返回 x, y 两个值
    local px, py = board:grid_to_pixel(prow, pcol)
    if px == nil then
        print("[plant_die_event_test] WARN: grid_to_pixel failed, skip sun drop")
        return
    end

    board:add_coin(px, py, COIN_SUN, COIN_MOTION_FALL)
    print(string.format("[plant_die_event_test] dropped sun at (%d,%d)", px, py))
end

print("[plant_die_event_test] 已加载：植物死亡掉落阳光")
return M
