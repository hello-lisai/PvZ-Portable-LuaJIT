/*
 * PvzExport.cpp - 游戏核心函数的 C ABI 导出实现
 *
 * 对应 PvzExport.h 中声明的所有 extern "C" 函数。
 * 每个函数都是简单的 C++ 成员调用包装。
 */

// 定义此宏让 PVZ_API 展开为 __declspec(dllexport)（而非 dllimport）
#define PVZ_EXPORTS 1
#include "PvzExport.h"

#include "../../Lawn/Zombie.h"
#include "../../Lawn/Plant.h"
#include "../../Lawn/Board.h"
#include <cstddef>      // offsetof
#include <cstring>      // strcmp

// ====== Zombie 导出实现 ======

PVZ_API void pvz_zombie_take_damage(void* z, int dmg, unsigned int flags) {
    if (!z) return;
    static_cast<Zombie*>(z)->TakeDamage(dmg, flags);
}

PVZ_API void pvz_zombie_die_no_loot(void* z) {
    if (!z) return;
    static_cast<Zombie*>(z)->DieNoLoot();
}

PVZ_API void pvz_zombie_die_with_loot(void* z) {
    if (!z) return;
    static_cast<Zombie*>(z)->DieWithLoot();
}

PVZ_API void pvz_zombie_apply_chill(void* z, int is_ice_trap) {
    if (!z) return;
    static_cast<Zombie*>(z)->ApplyChill(is_ice_trap != 0);
}

PVZ_API void pvz_zombie_set_row(void* z, int row) {
    if (!z) return;
    static_cast<Zombie*>(z)->SetRow(row);
}

PVZ_API void pvz_zombie_pick_random_speed(void* z) {
    if (!z) return;
    static_cast<Zombie*>(z)->PickRandomSpeed();
}

PVZ_API void pvz_zombie_update_anim_speed(void* z) {
    if (!z) return;
    static_cast<Zombie*>(z)->UpdateAnimSpeed();
}

PVZ_API void pvz_zombie_start_eating(void* z) {
    if (!z) return;
    static_cast<Zombie*>(z)->StartEating();
}

PVZ_API void pvz_zombie_stop_eating(void* z) {
    if (!z) return;
    static_cast<Zombie*>(z)->StopEating();
}

PVZ_API float pvz_zombie_get_pos_y_based_on_row(void* z, int row) {
    if (!z) return 0.0f;
    return static_cast<Zombie*>(z)->GetPosYBasedOnRow(row);
}

PVZ_API int pvz_zombie_get_type(void* z) {
    if (!z) return -1;
    return static_cast<int>(static_cast<Zombie*>(z)->mZombieType);
}

PVZ_API int pvz_zombie_get_health(void* z) {
    if (!z) return 0;
    return static_cast<Zombie*>(z)->mBodyHealth;
}

PVZ_API void pvz_zombie_set_health(void* z, int v) {
    if (!z) return;
    static_cast<Zombie*>(z)->mBodyHealth = v;
}

PVZ_API int pvz_zombie_get_max_health(void* z) {
    if (!z) return 0;
    return static_cast<Zombie*>(z)->mBodyMaxHealth;
}

PVZ_API void pvz_zombie_set_max_health(void* z, int v) {
    if (!z) return;
    static_cast<Zombie*>(z)->mBodyMaxHealth = v;
}

PVZ_API float pvz_zombie_get_pos_x(void* z) {
    if (!z) return 0.0f;
    return static_cast<Zombie*>(z)->mPosX;
}

PVZ_API void pvz_zombie_set_pos_x(void* z, float v) {
    if (!z) return;
    static_cast<Zombie*>(z)->mPosX = v;
}

PVZ_API float pvz_zombie_get_pos_y(void* z) {
    if (!z) return 0.0f;
    return static_cast<Zombie*>(z)->mPosY;
}

PVZ_API void pvz_zombie_set_pos_y(void* z, float v) {
    if (!z) return;
    static_cast<Zombie*>(z)->mPosY = v;
}

PVZ_API float pvz_zombie_get_vel_x(void* z) {
    if (!z) return 0.0f;
    return static_cast<Zombie*>(z)->mVelX;
}

PVZ_API void pvz_zombie_set_vel_x(void* z, float v) {
    if (!z) return;
    Zombie* z_ = static_cast<Zombie*>(z);
    z_->mVelX = v;
    // 关键：mVelX 是动画速率输入参数，必须重新调用 UpdateAnimSpeed
    // 才能让实际移动速度（_ground track velocity）生效
    z_->UpdateAnimSpeed();
}

PVZ_API int pvz_zombie_get_row(void* z) {
    if (!z) return -1;
    return static_cast<Zombie*>(z)->mRow;
}

PVZ_API int pvz_zombie_get_from_wave(void* z) {
    if (!z) return -1;
    return static_cast<Zombie*>(z)->mFromWave;
}

PVZ_API int pvz_zombie_get_chilled(void* z) {
    if (!z) return 0;
    return static_cast<Zombie*>(z)->mChilledCounter;
}

PVZ_API int pvz_zombie_get_buttered(void* z) {
    if (!z) return 0;
    return static_cast<Zombie*>(z)->mButteredCounter;
}

PVZ_API int pvz_zombie_get_ice_trap(void* z) {
    if (!z) return 0;
    return static_cast<Zombie*>(z)->mIceTrapCounter;
}

PVZ_API int pvz_zombie_get_mind_controlled(void* z) {
    if (!z) return 0;
    return static_cast<Zombie*>(z)->mMindControlled ? 1 : 0;
}

PVZ_API int pvz_zombie_get_dead(void* z) {
    if (!z) return 1;
    return static_cast<Zombie*>(z)->mDead ? 1 : 0;
}

PVZ_API int pvz_zombie_get_helm_health(void* z) {
    if (!z) return 0;
    return static_cast<Zombie*>(z)->mHelmHealth;
}

PVZ_API void pvz_zombie_set_helm_health(void* z, int v) {
    if (!z) return;
    static_cast<Zombie*>(z)->mHelmHealth = v;
}

PVZ_API int pvz_zombie_get_shield_health(void* z) {
    if (!z) return 0;
    return static_cast<Zombie*>(z)->mShieldHealth;
}

PVZ_API void pvz_zombie_set_shield_health(void* z, int v) {
    if (!z) return;
    static_cast<Zombie*>(z)->mShieldHealth = v;
}

// ====== Plant 导出实现 ======

PVZ_API void pvz_plant_do_special(void* p) {
    if (!p) return;
    static_cast<Plant*>(p)->DoSpecial();
}

PVZ_API void pvz_plant_die(void* p) {
    if (!p) return;
    static_cast<Plant*>(p)->Die();
}

PVZ_API void pvz_plant_squish(void* p) {
    if (!p) return;
    static_cast<Plant*>(p)->Squish();
}

PVZ_API void pvz_plant_update_abilities(void* p) {
    if (!p) return;
    static_cast<Plant*>(p)->UpdateAbilities();
}

PVZ_API void pvz_plant_do_row_area_damage(void* p, int dmg, unsigned int flags) {
    if (!p) return;
    static_cast<Plant*>(p)->DoRowAreaDamage(dmg, flags);
}

PVZ_API int pvz_plant_get_seed_type(void* p) {
    if (!p) return -1;
    return static_cast<int>(static_cast<Plant*>(p)->mSeedType);
}

PVZ_API int pvz_plant_get_col(void* p) {
    if (!p) return -1;
    return static_cast<Plant*>(p)->mPlantCol;
}

PVZ_API int pvz_plant_get_row(void* p) {
    if (!p) return -1;
    return static_cast<Plant*>(p)->mRow;
}

PVZ_API int pvz_plant_get_health(void* p) {
    if (!p) return 0;
    return static_cast<Plant*>(p)->mPlantHealth;
}

PVZ_API void pvz_plant_set_health(void* p, int v) {
    if (!p) return;
    static_cast<Plant*>(p)->mPlantHealth = v;
}

PVZ_API int pvz_plant_get_max_health(void* p) {
    if (!p) return 0;
    return static_cast<Plant*>(p)->mPlantMaxHealth;
}

PVZ_API void pvz_plant_set_max_health(void* p, int v) {
    if (!p) return;
    static_cast<Plant*>(p)->mPlantMaxHealth = v;
}

PVZ_API int pvz_plant_get_state(void* p) {
    if (!p) return -1;
    return static_cast<int>(static_cast<Plant*>(p)->mState);
}

// 注意：mX/mY 继承自 GameObject，类型为 int32_t
PVZ_API int pvz_plant_get_x(void* p) {
    if (!p) return 0;
    return static_cast<Plant*>(p)->mX;
}

PVZ_API int pvz_plant_get_y(void* p) {
    if (!p) return 0;
    return static_cast<Plant*>(p)->mY;
}

// ====== Board 导出实现 ======

PVZ_API void pvz_board_add_sun_money(void* b, int amount) {
    if (!b) return;
    static_cast<Board*>(b)->AddSunMoney(amount);
}

PVZ_API void pvz_board_pause(void* b, int pause) {
    if (!b) return;
    static_cast<Board*>(b)->Pause(pause != 0);
}

PVZ_API void pvz_board_remove_all_zombies(void* b) {
    if (!b) return;
    static_cast<Board*>(b)->RemoveAllZombies();
}

PVZ_API void pvz_board_spawn_zombie_wave(void* b) {
    if (!b) return;
    static_cast<Board*>(b)->SpawnZombieWave();
}

PVZ_API void pvz_board_start_level(void* b) {
    if (!b) return;
    static_cast<Board*>(b)->StartLevel();
}

PVZ_API void pvz_board_init_level(void* b) {
    if (!b) return;
    static_cast<Board*>(b)->InitLevel();
}

PVZ_API int pvz_board_get_sun_money(void* b) {
    if (!b) return 0;
    return static_cast<Board*>(b)->mSunMoney;
}

PVZ_API void pvz_board_set_sun_money(void* b, int v) {
    if (!b) return;
    static_cast<Board*>(b)->mSunMoney = v;
}

PVZ_API int pvz_board_get_level(void* b) {
    if (!b) return -1;
    return static_cast<Board*>(b)->mLevel;
}

// ====== 内存偏移查询（第二步） ======
// 返回字段在对象内的字节偏移，-1 表示字段名未知
// mod 用 ffi.cast + offset 直接读写内存，绕过所有 getter/setter

PVZ_API int pvz_offset_of_zombie(const char* field_name) {
    if (!field_name) return -1;
    using Z = Zombie;
    if (strcmp(field_name, "type")            == 0) return offsetof(Z, mZombieType);
    if (strcmp(field_name, "phase")           == 0) return offsetof(Z, mZombiePhase);
    if (strcmp(field_name, "pos_x")           == 0) return offsetof(Z, mPosX);
    if (strcmp(field_name, "pos_y")           == 0) return offsetof(Z, mPosY);
    if (strcmp(field_name, "vel_x")           == 0) return offsetof(Z, mVelX);
    if (strcmp(field_name, "row")             == 0) return offsetof(Z, mRow);
    if (strcmp(field_name, "from_wave")       == 0) return offsetof(Z, mFromWave);
    if (strcmp(field_name, "chilled")         == 0) return offsetof(Z, mChilledCounter);
    if (strcmp(field_name, "buttered")        == 0) return offsetof(Z, mButteredCounter);
    if (strcmp(field_name, "ice_trap")        == 0) return offsetof(Z, mIceTrapCounter);
    if (strcmp(field_name, "mind_controlled") == 0) return offsetof(Z, mMindControlled);
    if (strcmp(field_name, "dead")            == 0) return offsetof(Z, mDead);
    if (strcmp(field_name, "health")          == 0) return offsetof(Z, mBodyHealth);
    if (strcmp(field_name, "max_health")      == 0) return offsetof(Z, mBodyMaxHealth);
    if (strcmp(field_name, "helm_health")     == 0) return offsetof(Z, mHelmHealth);
    if (strcmp(field_name, "helm_max_health") == 0) return offsetof(Z, mHelmMaxHealth);
    if (strcmp(field_name, "shield_health")   == 0) return offsetof(Z, mShieldHealth);
    if (strcmp(field_name, "shield_max_health")== 0) return offsetof(Z, mShieldMaxHealth);
    return -1;
}

PVZ_API int pvz_offset_of_plant(const char* field_name) {
    if (!field_name) return -1;
    using P = Plant;
    if (strcmp(field_name, "seed_type")       == 0) return offsetof(P, mSeedType);
    if (strcmp(field_name, "col")             == 0) return offsetof(P, mPlantCol);
    if (strcmp(field_name, "row")             == 0) return offsetof(P, mRow);
    if (strcmp(field_name, "state")           == 0) return offsetof(P, mState);
    if (strcmp(field_name, "health")          == 0) return offsetof(P, mPlantHealth);
    if (strcmp(field_name, "max_health")      == 0) return offsetof(P, mPlantMaxHealth);
    if (strcmp(field_name, "subclass")        == 0) return offsetof(P, mSubclass);
    if (strcmp(field_name, "x")               == 0) return offsetof(P, mX);
    if (strcmp(field_name, "y")               == 0) return offsetof(P, mY);
    return -1;
}

PVZ_API int pvz_offset_of_board(const char* field_name) {
    if (!field_name) return -1;
    using B = Board;
    if (strcmp(field_name, "sun_money")       == 0) return offsetof(B, mSunMoney);
    if (strcmp(field_name, "level")           == 0) return offsetof(B, mLevel);
    if (strcmp(field_name, "current_wave")    == 0) return offsetof(B, mCurrentWave);
    if (strcmp(field_name, "num_waves")       == 0) return offsetof(B, mNumWaves);
    if (strcmp(field_name, "main_counter")    == 0) return offsetof(B, mMainCounter);
    if (strcmp(field_name, "level_complete")  == 0) return offsetof(B, mLevelComplete);
    return -1;
}
