/*
 * PvzExport.h - 游戏核心函数的 C ABI 导出层
 *
 * 目的：让 LuaJIT FFI 能通过 ffi.C 直接调用游戏函数，
 *      绕过 metatable 绑定，实现"极高自由度"的 mod 开发。
 *
 * 设计原则：
 *   1. 所有导出函数都是 extern "C"，避免 C++ name mangling
 *   2. 使用 PVZ_API 宏统一处理平台符号导出
 *   3. 参数类型只用 C 基本类型（void* / int / float / 等），
 *      不暴露 C++ 类引用，保证 ABI 稳定
 *   4. 所有函数都做空指针检查，避免 mod 传 nil 导致崩溃
 *   5. 对象指针由 Lua 这边的 get_ptr() 方法提供
 *
 * 使用示例（Lua 端）：
 *   local ffi = require("ffi")
 *   ffi.cdef[[
 *       void pvz_zombie_take_damage(void* z, int dmg, unsigned int flags);
 *       int  pvz_zombie_get_health(void* z);
 *   ]]
 *   local ptr = zombie:get_ptr()
 *   ffi.C.pvz_zombie_take_damage(ptr, 9999, 0)  -- 秒杀
 */

#pragma once

// 跨平台符号导出宏
#if defined(_WIN32) || defined(__CYGWIN__)
    #if defined(PVZ_EXPORTS)
        #define PVZ_API __declspec(dllexport)
    #else
        #define PVZ_API __declspec(dllimport)
    #endif
#else
    #define PVZ_API __attribute__((visibility("default")))
#endif

// 所有导出函数使用 C 链接，避免 name mangling
// 参数中 void* 代表游戏对象指针（Zombie*/Plant*/Board*），
// 由 Lua 端的 get_ptr() 方法提供

#ifdef __cplusplus
extern "C" {
#endif

// ====== Zombie 导出 API ======
// 对应 Zombie 类的公开方法（src/Lawn/Zombie.h）

PVZ_API void pvz_zombie_take_damage(void* z, int dmg, unsigned int flags);
PVZ_API void pvz_zombie_die_no_loot(void* z);
PVZ_API void pvz_zombie_die_with_loot(void* z);
PVZ_API void pvz_zombie_apply_chill(void* z, int is_ice_trap);
PVZ_API void pvz_zombie_set_row(void* z, int row);
PVZ_API void pvz_zombie_pick_random_speed(void* z);
PVZ_API void pvz_zombie_update_anim_speed(void* z);
PVZ_API void pvz_zombie_start_eating(void* z);
PVZ_API void pvz_zombie_stop_eating(void* z);
PVZ_API float pvz_zombie_get_pos_y_based_on_row(void* z, int row);

// Zombie 字段 getter/setter
PVZ_API int   pvz_zombie_get_type(void* z);
PVZ_API int   pvz_zombie_get_health(void* z);
PVZ_API void  pvz_zombie_set_health(void* z, int v);
PVZ_API int   pvz_zombie_get_max_health(void* z);
PVZ_API void  pvz_zombie_set_max_health(void* z, int v);
PVZ_API float pvz_zombie_get_pos_x(void* z);
PVZ_API void  pvz_zombie_set_pos_x(void* z, float v);
PVZ_API float pvz_zombie_get_pos_y(void* z);
PVZ_API void  pvz_zombie_set_pos_y(void* z, float v);
PVZ_API float pvz_zombie_get_vel_x(void* z);
PVZ_API void  pvz_zombie_set_vel_x(void* z, float v);  // 内部自动调用 UpdateAnimSpeed
PVZ_API int   pvz_zombie_get_row(void* z);
PVZ_API int   pvz_zombie_get_from_wave(void* z);
PVZ_API int   pvz_zombie_get_chilled(void* z);
PVZ_API int   pvz_zombie_get_buttered(void* z);
PVZ_API int   pvz_zombie_get_ice_trap(void* z);
PVZ_API int   pvz_zombie_get_mind_controlled(void* z);
PVZ_API int   pvz_zombie_get_dead(void* z);
PVZ_API int   pvz_zombie_get_helm_health(void* z);
PVZ_API void  pvz_zombie_set_helm_health(void* z, int v);
PVZ_API int   pvz_zombie_get_shield_health(void* z);
PVZ_API void  pvz_zombie_set_shield_health(void* z, int v);

// ====== Plant 导出 API ======
// 对应 Plant 类的公开方法（src/Lawn/Plant.h）

PVZ_API void  pvz_plant_do_special(void* p);
PVZ_API void  pvz_plant_die(void* p);
PVZ_API void  pvz_plant_squish(void* p);
PVZ_API void  pvz_plant_update_abilities(void* p);
PVZ_API void  pvz_plant_do_row_area_damage(void* p, int dmg, unsigned int flags);

// Plant 字段 getter/setter
PVZ_API int   pvz_plant_get_seed_type(void* p);
PVZ_API int   pvz_plant_get_col(void* p);
PVZ_API int   pvz_plant_get_row(void* p);
PVZ_API int   pvz_plant_get_health(void* p);
PVZ_API void  pvz_plant_set_health(void* p, int v);
PVZ_API int   pvz_plant_get_max_health(void* p);
PVZ_API void  pvz_plant_set_max_health(void* p, int v);
PVZ_API int   pvz_plant_get_state(void* p);
// 注意：Plant 的 mX/mY 继承自 GameObject，类型为 int32_t（非 float）
PVZ_API int   pvz_plant_get_x(void* p);
PVZ_API int   pvz_plant_get_y(void* p);

// ====== Board 导出 API ======
// 对应 Board 类的公开方法（src/Lawn/Board.h）

PVZ_API void  pvz_board_add_sun_money(void* b, int amount);
PVZ_API void  pvz_board_pause(void* b, int pause);
PVZ_API void  pvz_board_remove_all_zombies(void* b);
PVZ_API void  pvz_board_spawn_zombie_wave(void* b);
PVZ_API void  pvz_board_start_level(void* b);
PVZ_API void  pvz_board_init_level(void* b);

// Board 字段 getter/setter
PVZ_API int   pvz_board_get_sun_money(void* b);
PVZ_API void  pvz_board_set_sun_money(void* b, int v);
PVZ_API int   pvz_board_get_level(void* b);
PVZ_API int   pvz_board_get_current_wave(void* b);
PVZ_API int   pvz_board_get_num_waves(void* b);
PVZ_API int   pvz_board_get_main_counter(void* b);
PVZ_API int   pvz_board_get_paused(void* b);

// ====== Projectile 导出 API ======
// 对应 Projectile 类（src/Lawn/Projectile.h），继承自 GameObject

PVZ_API void  pvz_projectile_die(void* p);
PVZ_API void  pvz_projectile_do_impact(void* p, void* zombie);
PVZ_API void  pvz_projectile_do_splash_damage(void* p, void* zombie);
PVZ_API void  pvz_projectile_update_motion(void* p);
PVZ_API void  pvz_projectile_convert_to_fireball(void* p, int grid_x);
PVZ_API void  pvz_projectile_convert_to_pea(void* p, int grid_x);

// Projectile 字段 getter/setter
PVZ_API int   pvz_projectile_get_type(void* p);
PVZ_API int   pvz_projectile_get_motion_type(void* p);
PVZ_API int   pvz_projectile_get_row(void* p);          // 继承自 GameObject
PVZ_API int   pvz_projectile_get_x(void* p);            // 继承自 GameObject (int32)
PVZ_API int   pvz_projectile_get_y(void* p);            // 继承自 GameObject (int32)
PVZ_API float pvz_projectile_get_pos_x(void* p);
PVZ_API void  pvz_projectile_set_pos_x(void* p, float v);
PVZ_API float pvz_projectile_get_pos_y(void* p);
PVZ_API void  pvz_projectile_set_pos_y(void* p, float v);
PVZ_API float pvz_projectile_get_vel_x(void* p);
PVZ_API void  pvz_projectile_set_vel_x(void* p, float v);
PVZ_API float pvz_projectile_get_vel_y(void* p);
PVZ_API void  pvz_projectile_set_vel_y(void* p, float v);
PVZ_API int   pvz_projectile_get_age(void* p);
PVZ_API int   pvz_projectile_get_dead(void* p);
PVZ_API int   pvz_projectile_get_damage_range_flags(void* p);
PVZ_API int   pvz_projectile_get_target_zombie_id(void* p);

// ====== Coin 导出 API ======
// 对应 Coin 类（src/Lawn/Coin.h），继承自 GameObject

PVZ_API void  pvz_coin_die(void* c);
PVZ_API void  pvz_coin_collect(void* c);
PVZ_API void  pvz_coin_start_fade(void* c);
PVZ_API int   pvz_coin_get_sun_value(void* c);
PVZ_API int   pvz_coin_is_money(void* c);
PVZ_API int   pvz_coin_is_sun(void* c);
PVZ_API int   pvz_coin_is_level_award(void* c);

// Coin 字段 getter/setter
PVZ_API int   pvz_coin_get_type(void* c);
PVZ_API int   pvz_coin_get_motion(void* c);
PVZ_API int   pvz_coin_get_row(void* c);                // 继承自 GameObject
PVZ_API int   pvz_coin_get_x(void* c);                  // 继承自 GameObject (int32)
PVZ_API int   pvz_coin_get_y(void* c);                  // 继承自 GameObject (int32)
PVZ_API float pvz_coin_get_pos_x(void* c);
PVZ_API void  pvz_coin_set_pos_x(void* c, float v);
PVZ_API float pvz_coin_get_pos_y(void* c);
PVZ_API void  pvz_coin_set_pos_y(void* c, float v);
PVZ_API float pvz_coin_get_vel_x(void* c);
PVZ_API void  pvz_coin_set_vel_x(void* c, float v);
PVZ_API float pvz_coin_get_vel_y(void* c);
PVZ_API void  pvz_coin_set_vel_y(void* c, float v);
PVZ_API int   pvz_coin_get_age(void* c);
PVZ_API int   pvz_coin_get_dead(void* c);
PVZ_API int   pvz_coin_get_being_collected(void* c);

// ====== LawnMower 导出 API ======
// 对应 LawnMower 类（src/Lawn/LawnMower.h），不继承 GameObject

PVZ_API void  pvz_mower_start(void* m);
PVZ_API void  pvz_mower_die(void* m);
PVZ_API void  pvz_mower_squish(void* m);
PVZ_API void  pvz_mower_enable_super(void* m, int enable);

// LawnMower 字段 getter/setter
PVZ_API int   pvz_mower_get_state(void* m);
PVZ_API int   pvz_mower_get_type(void* m);
PVZ_API int   pvz_mower_get_row(void* m);
PVZ_API float pvz_mower_get_pos_x(void* m);
PVZ_API void  pvz_mower_set_pos_x(void* m, float v);
PVZ_API float pvz_mower_get_pos_y(void* m);
PVZ_API int   pvz_mower_get_dead(void* m);
PVZ_API int   pvz_mower_get_visible(void* m);

// ====== GridItem 导出 API ======
// 对应 GridItem 类（src/Lawn/GridItem.h），不继承 GameObject

PVZ_API void  pvz_griditem_die(void* g);
PVZ_API void  pvz_griditem_open_portal(void* g);
PVZ_API void  pvz_griditem_close_portal(void* g);
PVZ_API int   pvz_griditem_is_open_portal(void* g);

// GridItem 字段 getter/setter
PVZ_API int   pvz_griditem_get_type(void* g);
PVZ_API int   pvz_griditem_get_state(void* g);
PVZ_API int   pvz_griditem_get_grid_x(void* g);
PVZ_API int   pvz_griditem_get_grid_y(void* g);
PVZ_API float pvz_griditem_get_pos_x(void* g);
PVZ_API void  pvz_griditem_set_pos_x(void* g, float v);
PVZ_API float pvz_griditem_get_pos_y(void* g);
PVZ_API void  pvz_griditem_set_pos_y(void* g, float v);
PVZ_API int   pvz_griditem_get_dead(void* g);
PVZ_API int   pvz_griditem_get_zombie_type(void* g);
PVZ_API int   pvz_griditem_get_seed_type(void* g);

// ====== LawnApp 全局 API ======
// 对应 LawnApp 类（src/LawnApp.h），全局单例 gLawnApp
// mod 无需传指针，直接调用即可

PVZ_API void* pvz_app_get();                    // 返回 gLawnApp 指针
PVZ_API void* pvz_app_get_board();              // 返回当前 Board 指针
PVZ_API int   pvz_app_get_level();
PVZ_API int   pvz_app_get_game_mode();
PVZ_API int   pvz_app_get_game_scene();
PVZ_API int   pvz_app_get_board_result();
PVZ_API int   pvz_app_is_adventure_mode();
PVZ_API int   pvz_app_is_survival_mode();
PVZ_API int   pvz_app_is_challenge_mode();
PVZ_API int   pvz_app_is_puzzle_mode();
PVZ_API int   pvz_app_is_night();
PVZ_API int   pvz_app_is_final_boss_level();
PVZ_API int   pvz_app_has_finished_adventure();
PVZ_API void  pvz_app_end_level();
PVZ_API void  pvz_app_kill_board();
PVZ_API void  pvz_app_show_game_selector();
PVZ_API int   pvz_app_save_file_exists();
PVZ_API void  pvz_app_play_foley(int foley_type);
PVZ_API void  pvz_app_toggle_slow_mo();
PVZ_API void  pvz_app_toggle_fast_mo();

// ====== 内存偏移查询（第二步） ======
// 返回关键字段在对象内的字节偏移，供 mod 用 ffi 直接读写内存
// 例：local off = pvz.offset_of.zombie("health")
//     local ptr = ffi.cast("char*", zombie:get_ptr())
//     local health = ffi.cast("int*", ptr + off)[0]

PVZ_API int pvz_offset_of_zombie(const char* field_name);
PVZ_API int pvz_offset_of_plant(const char* field_name);
PVZ_API int pvz_offset_of_board(const char* field_name);
PVZ_API int pvz_offset_of_projectile(const char* field_name);
PVZ_API int pvz_offset_of_coin(const char* field_name);
PVZ_API int pvz_offset_of_mower(const char* field_name);
PVZ_API int pvz_offset_of_griditem(const char* field_name);

// ====== 字段类型查询（增强：让 mod 知道字段类型以正确 cast 指针） ======
// 返回字段类型代号：
//   1 = int32, 2 = int64, 3 = float, 4 = double, 5 = bool, 6 = pointer, 0 = 未知
PVZ_API int pvz_field_type_of(int object_kind, const char* field_name);
// object_kind: 0=Zombie, 1=Plant, 2=Board, 3=Projectile, 4=Coin, 5=Mower, 6=GridItem

#ifdef __cplusplus
} // extern "C"
#endif
