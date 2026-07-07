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

// ====== 内存偏移查询（第二步） ======
// 返回关键字段在对象内的字节偏移，供 mod 用 ffi 直接读写内存
// 例：local off = pvz.offset_of("zombie", "health")
//     local ptr = ffi.cast("char*", zombie:get_ptr())
//     local health = ffi.cast("int*", ptr + off)[0]

PVZ_API int pvz_offset_of_zombie(const char* field_name);
PVZ_API int pvz_offset_of_plant(const char* field_name);
PVZ_API int pvz_offset_of_board(const char* field_name);

#ifdef __cplusplus
} // extern "C"
#endif
