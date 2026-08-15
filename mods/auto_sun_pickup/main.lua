-- auto_sun_pickup/main.lua
-- 自动拾取阳光
--
-- 功能：阳光落地 2 秒后，如果玩家没有手动点击拾取，则自动拾取
-- 实现：在 on_board_update_post 中遍历所有 Coin，
--       检查已落地阳光的 disappear_counter（落地后开始递增），
--       超过 120 帧（2 秒 @60fps）则自动调用 coin:collect()

local M = {}

-- 阳光类型集合
local SUN_TYPES = {
    [pvz.CoinType.SUN]       = true,
    [pvz.CoinType.SMALL_SUN] = true,
    [pvz.CoinType.BIG_SUN]   = true,
}

-- 2 秒对应的帧数（游戏逻辑 60 FPS → 120 帧）
local PICKUP_DELAY_FRAMES = 120

function M.on_board_update_post(board)
    board:for_each_coin(function(coin)
        -- 只处理阳光类型
        if not SUN_TYPES[coin.type] then return end
        -- 跳过已死亡或正在被收集的
        if coin.dead or coin.is_being_collected then return end
        -- 只处理已落地的阳光
        if not coin.is_on_ground then return end
        -- 落地后超过 2 秒未被拾取，自动收集
        if coin.disappear_counter >= PICKUP_DELAY_FRAMES then
            coin:collect()
        end
    end)
end

print("[auto_sun_pickup] 已加载：阳光落地 2 秒后自动拾取")
return M
