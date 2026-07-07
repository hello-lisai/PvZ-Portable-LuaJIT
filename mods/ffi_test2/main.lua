-- ffi_test2/main.lua
-- 扩展 API 测试 mod：验证 Projectile / Coin / LawnMower / GridItem / LawnApp 的 FFI 导出
--
-- 测试项：
--   1. pvz_app_get_level / pvz_app_get_game_mode / pvz_app_is_night 等 LawnApp 全局 API
--   2. pvz.offset_of.projectile/coin/mower/griditem 内存偏移查询
--   3. pvz.field_type_of 字段类型查询
--   4. board:for_each_projectile/coin 内用 get_ptr() + ffi.C 直调
--   5. 用 offset + ffi.cast 直接读内存字段

local ffi = require("ffi")

ffi.cdef[[
    -- LawnApp 全局 API
    int   pvz_app_get_level();
    int   pvz_app_get_game_mode();
    int   pvz_app_get_game_scene();
    int   pvz_app_is_adventure_mode();
    int   pvz_app_is_survival_mode();
    int   pvz_app_is_challenge_mode();
    int   pvz_app_is_night();
    int   pvz_app_is_final_boss_level();
    int   pvz_app_has_finished_adventure();
    void* pvz_app_get_board();

    -- Board
    int   pvz_board_get_current_wave(void* b);
    int   pvz_board_get_num_waves(void* b);
    int   pvz_board_get_main_counter(void* b);
    int   pvz_board_get_paused(void* b);

    -- Projectile
    int   pvz_projectile_get_type(void* p);
    float pvz_projectile_get_pos_x(void* p);
    float pvz_projectile_get_vel_x(void* p);
    int   pvz_projectile_get_age(void* p);
    int   pvz_projectile_get_row(void* p);

    -- Coin
    int   pvz_coin_get_type(void* c);
    int   pvz_coin_get_sun_value(void* c);
    int   pvz_coin_is_sun(void* c);
    int   pvz_coin_is_money(void* c);
    float pvz_coin_get_pos_x(void* c);

    -- LawnMower
    int   pvz_mower_get_state(void* m);
    int   pvz_mower_get_row(void* m);
    float pvz_mower_get_pos_x(void* m);

    -- GridItem
    int   pvz_griditem_get_type(void* g);
    int   pvz_griditem_get_grid_x(void* g);
    int   pvz_griditem_get_grid_y(void* g);

    -- offset_of 新对象
    int pvz_offset_of_projectile(const char* field_name);
    int pvz_offset_of_coin(const char* field_name);
    int pvz_offset_of_mower(const char* field_name);
    int pvz_offset_of_griditem(const char* field_name);

    -- field_type_of
    int pvz_field_type_of(int object_kind, const char* field_name);
]]

local M = {}

-- 缓存偏移
local off_proj_pos_x = pvz.offset_of.projectile("pos_x")
local off_proj_vel_x = pvz.offset_of.projectile("vel_x")
local off_coin_pos_x = pvz.offset_of.coin("pos_x")
local off_mower_pos_x = pvz.offset_of.mower("pos_x")
local off_griditem_grid_x = pvz.offset_of.griditem("grid_x")

-- field_type_of 类型代号
local TYPE_INT32 = 1
local TYPE_FLOAT  = 3
local TYPE_BOOL  = 5

print(string.format("[ffi_test2] offsets: proj.pos_x=%d proj.vel_x=%d coin.pos_x=%d mower.pos_x=%d griditem.grid_x=%d",
    off_proj_pos_x, off_proj_vel_x, off_coin_pos_x, off_mower_pos_x, off_griditem_grid_x))

-- 验证 field_type_of
local t_proj_pos_x = pvz.field_type_of(3, "pos_x")  -- Projectile=3
local t_proj_row   = pvz.field_type_of(3, "row")
local t_proj_dead  = pvz.field_type_of(3, "dead")
print(string.format("[ffi_test2] field_type_of Projectile: pos_x=%d (expect %d/float), row=%d (expect %d/int32), dead=%d (expect %d/bool)",
    t_proj_pos_x, TYPE_FLOAT, t_proj_row, TYPE_INT32, t_proj_dead, TYPE_BOOL))

-- 关卡开始时打印 LawnApp 全局信息
function M.on_level_start(board)
    local level = ffi.C.pvz_app_get_level()
    local mode = ffi.C.pvz_app_get_game_mode()
    local scene = ffi.C.pvz_app_get_game_scene()
    local is_night = ffi.C.pvz_app_is_night()
    local is_adv = ffi.C.pvz_app_is_adventure_mode()
    local is_surv = ffi.C.pvz_app_is_survival_mode()
    local is_chal = ffi.C.pvz_app_is_challenge_mode()
    print(string.format("[ffi_test2] on_level_start: level=%d mode=%d scene=%d night=%d adv=%d surv=%d chal=%d",
        level, mode, scene, is_night, is_adv, is_surv, is_chal))

    -- 用 Board offset_of 直接读 current_wave / num_waves
    local bptr = board:get_ptr()
    if bptr then
        local wave = ffi.C.pvz_board_get_current_wave(bptr)
        local num_waves = ffi.C.pvz_board_get_num_waves(bptr)
        local counter = ffi.C.pvz_board_get_main_counter(bptr)
        local paused = ffi.C.pvz_board_get_paused(bptr)
        print(string.format("[ffi_test2] board: current_wave=%d num_waves=%d main_counter=%d paused=%d",
            wave, num_waves, counter, paused))
    end
end

-- 每帧后：扫描场上投射物/金币/割草机/网格物品（仅打印一次避免刷屏）
local _scanned = false
function M.on_board_update_post(board)
    if _scanned then return end
    _scanned = true

    -- 扫描投射物
    board:for_each_projectile(function(proj)
        local p = proj:get_ptr()
        if not p then return end
        local t = ffi.C.pvz_projectile_get_type(p)
        local px = ffi.C.pvz_projectile_get_pos_x(p)
        local vx = ffi.C.pvz_projectile_get_vel_x(p)
        local age = ffi.C.pvz_projectile_get_age(p)
        local row = ffi.C.pvz_projectile_get_row(p)

        -- 用 offset 直读内存（演示第二步）
        local char_ptr = ffi.cast("char*", p)
        local px_mem = ffi.cast("float*", char_ptr + off_proj_pos_x)[0]
        local vx_mem = ffi.cast("float*", char_ptr + off_proj_vel_x)[0]

        print(string.format("[ffi_test2] projectile: type=%d row=%d pos_x=%.1f(ffi)/%.1f(mem) vel_x=%.3f(ffi)/%.3f(mem) age=%d",
            t, row, px, px_mem, vx, vx_mem, age))
    end)

    -- 扫描金币
    board:for_each_coin(function(coin)
        local c = coin:get_ptr()
        if not c then return end
        local t = ffi.C.pvz_coin_get_type(c)
        local is_sun = ffi.C.pvz_coin_is_sun(c)
        local is_money = ffi.C.pvz_coin_is_money(c)
        local val = ffi.C.pvz_coin_get_sun_value(c)
        local px = ffi.C.pvz_coin_get_pos_x(c)

        -- 用 offset 直读内存
        local char_ptr = ffi.cast("char*", c)
        local px_mem = ffi.cast("float*", char_ptr + off_coin_pos_x)[0]

        print(string.format("[ffi_test2] coin: type=%d is_sun=%d is_money=%d sun_value=%d pos_x=%.1f(ffi)/%.1f(mem)",
            t, is_sun, is_money, val, px, px_mem))
    end)

    -- 扫描割草机（如果有 for_each_lawnmower）
    if board.for_each_lawnmower then
        board:for_each_lawnmower(function(mower)
            local m = mower:get_ptr()
            if not m then return end
            local state = ffi.C.pvz_mower_get_state(m)
            local row = ffi.C.pvz_mower_get_row(m)
            local px = ffi.C.pvz_mower_get_pos_x(m)

            local char_ptr = ffi.cast("char*", m)
            local px_mem = ffi.cast("float*", char_ptr + off_mower_pos_x)[0]

            print(string.format("[ffi_test2] mower: state=%d row=%d pos_x=%.1f(ffi)/%.1f(mem)",
                state, row, px, px_mem))
        end)
    end

    -- 扫描网格物品（如果有 for_each_griditem）
    if board.for_each_griditem then
        board:for_each_griditem(function(griditem)
            local g = griditem:get_ptr()
            if not g then return end
            local t = ffi.C.pvz_griditem_get_type(g)
            local gx = ffi.C.pvz_griditem_get_grid_x(g)
            local gy = ffi.C.pvz_griditem_get_grid_y(g)

            local char_ptr = ffi.cast("char*", g)
            local gx_mem = ffi.cast("int*", char_ptr + off_griditem_grid_x)[0]

            print(string.format("[ffi_test2] griditem: type=%d grid_x=%d(ffi)/%d(mem) grid_y=%d",
                t, gx, gx_mem, gy))
        end)
    end

    print("[ffi_test2] 扫描完成（仅打印一次，重启关卡可重新扫描）")
end

print("[ffi_test2] 已加载：扩展 FFI API 测试（Projectile/Coin/Mower/GridItem/App）")

return M
