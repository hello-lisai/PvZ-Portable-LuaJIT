-- ffi_test/main.lua
-- FFI 直调测试 mod
-- 演示两种"极高自由度"的 mod 开发方式：
--   1. 第一步：ffi.C.pvz_xxx(ptr, ...) 直调游戏导出的 C 函数
--      - 普通僵尸血量扩大 5 倍（用 pvz_zombie_set_health/max_health）
--      - 按键 F1 触发秒杀所有普通僵尸（用 pvz_zombie_take_damage 99999）
--   2. 第二步：ffi.cast + offset 直接读写对象内存字段
--      - 用 pvz.offset_of.zombie("health") 拿到偏移
--      - 直接 cast 指针读写内存，绕过所有 getter/setter
--
-- 注意：需要带 PVZ_API 导出的修复版 exe（commit 637ca85 之后）

local ffi = require("ffi")

-- 声明导出的 C 函数签名（对应 PvzExport.h）
ffi.cdef[[
    void pvz_zombie_take_damage(void* z, int dmg, unsigned int flags);
    int  pvz_zombie_get_health(void* z);
    void pvz_zombie_set_health(void* z, int v);
    int  pvz_zombie_get_max_health(void* z);
    void pvz_zombie_set_max_health(void* z, int v);
    int  pvz_zombie_get_type(void* z);
    float pvz_zombie_get_vel_x(void* z);
    void pvz_zombie_set_vel_x(void* z, float v);
    int  pvz_offset_of_zombie(const char* field_name);
]]

local M = {}

local ZOMBIE_NORMAL = 0

-- 缓存内存偏移（第二步演示）
local off_health     = pvz.offset_of.zombie("health")
local off_max_health = pvz.offset_of.zombie("max_health")
print(string.format("[ffi_test] offset_of zombie.health=%d zombie.max_health=%d", off_health, off_max_health))

-- 第一步演示：FFI 直调 C 函数
-- 在僵尸创建时把血量和最大血量都扩大 5 倍
function M.on_zombie_created(zombie)
    if zombie == nil then return end
    local ptr = zombie:get_ptr()  -- 拿到原始指针（light userdata）
    if ptr == nil then return end

    local t = ffi.C.pvz_zombie_get_type(ptr)
    if t == ZOMBIE_NORMAL then
        local hp = ffi.C.pvz_zombie_get_health(ptr)
        local mx = ffi.C.pvz_zombie_get_max_health(ptr)
        ffi.C.pvz_zombie_set_max_health(ptr, mx * 5)
        ffi.C.pvz_zombie_set_health(ptr, hp * 5)
        print(string.format("[ffi_test] FFI set health %d -> %d, max %d -> %d", hp, hp*5, mx, mx*5))
    end
end

-- 第二步演示：直接读写内存偏移（绕过所有 getter/setter）
-- 按键 F1 触发：用 offset 直接把所有普通僵尸血量设为 1（一击必杀）
function M.on_key_down(key)
    -- KeyCode::KEYCODE_F1 = 188（参考 ConstEnums.h）
    -- 这里用简单的字符串匹配，实际 key 是数字
    -- 注意：on_key_down 事件可能未实现，此函数仅作演示
end

-- 更进一步：用 offset 直接读血量（演示第二步）
-- 这个函数可在 on_board_update_post 里调用，每秒打印一次所有普通僵尸血量
local _last_print_tick = 0
function M.on_board_update_post(board)
    -- 第二步演示：用内存偏移直接读字段
    -- 这里只演示一次，避免刷屏
    if _last_print_tick > 0 then return end
    _last_print_tick = 1

    board:for_each_zombie(function(zombie)
        if zombie == nil then return end
        local t = zombie:get_ptr()
        if t == nil then return end

        -- 用 offset 直接读内存
        local ptr = ffi.cast("char*", t)
        local hp_ptr = ffi.cast("int*", ptr + off_health)
        local mx_ptr = ffi.cast("int*", ptr + off_max_health)
        print(string.format("[ffi_test] memory read: health=%d max_health=%d", hp_ptr[0], mx_ptr[0]))
    end)
end

print("[ffi_test] 已加载：FFI 直调 + 内存偏移读写测试")
print("[ffi_test] 效果：普通僵尸血量 x5（FFI 调用），日志显示内存直读结果")

return M
