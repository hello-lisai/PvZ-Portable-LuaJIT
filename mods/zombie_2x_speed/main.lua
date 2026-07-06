-- zombie_2x_speed/main.lua
-- 监听 on_zombie_created 事件，把普通僵尸的移动速度扩大 2 倍
-- 普通僵尸的实际移动速度 = 动画 _ground track velocity，由 mVelX 经 UpdateAnimSpeed 计算
-- 修改 vel_x 后引擎会自动调用 UpdateAnimSpeed() 更新动画速率（需要修复版 exe）

local M = {}

local ZOMBIE_NORMAL = 0

function M.on_zombie_created(zombie)
    if zombie == nil then return end
    if zombie.type == ZOMBIE_NORMAL then
        local v = zombie.vel_x
        zombie.vel_x = v * 2
        print("[zombie_2x_speed] applied: vel_x " .. tostring(v) .. " -> " .. tostring(v * 2))
    end
end

print("[zombie_2x_speed] 已加载")

return M
