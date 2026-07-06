-- zombie_3x_hp/main.lua
-- 监听 on_zombie_created 事件，把普通僵尸的血量和最大血量都扩大到 3 倍
-- 普通僵尸原始血量在 ZombieDefinition 里定义（默认 270），这里直接乘 3
--
-- 注意 1：引擎要求 main.lua 必须返回一个 table，回调作为 table 的字段提供
-- 注意 2：当前 exe 的 BindEnums 有 bug，pvz.ZombieType 会被覆盖为 nil，
--         所以这里直接用 ZombieType::ZOMBIE_NORMAL 的数值（0）代替枚举
-- 注意 3：BindZombie 只把 get_type/get_health 等注册为"属性"而非"方法"，
--         必须用 zombie.type / zombie.health 访问，不能用 zombie:get_type()

local M = {}

-- ZombieType::ZOMBIE_NORMAL = 0（见 ConstEnums.h）
local ZOMBIE_NORMAL = 0

-- 事件回调：僵尸创建时触发
function M.on_zombie_created(zombie)
    if zombie == nil then return end
    -- 只对普通僵尸生效（用属性访问，不是方法调用）
    if zombie.type == ZOMBIE_NORMAL then
        local cur = zombie.health
        local mx  = zombie.max_health
        zombie.max_health = mx * 3
        zombie.health     = cur * 3
    end
end

-- Mod 加载时打印一条日志
print("[zombie_3x_hp] 已加载：普通僵尸血量扩大 3 倍")

return M
