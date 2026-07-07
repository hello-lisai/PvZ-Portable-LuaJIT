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
#include "../../Lawn/Projectile.h"
#include "../../Lawn/Coin.h"
#include "../../Lawn/LawnMower.h"
#include "../../Lawn/GridItem.h"
#include "../../LawnApp.h"
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

PVZ_API int pvz_board_get_current_wave(void* b) {
    if (!b) return -1;
    return static_cast<Board*>(b)->mCurrentWave;
}

PVZ_API int pvz_board_get_num_waves(void* b) {
    if (!b) return 0;
    return static_cast<Board*>(b)->mNumWaves;
}

PVZ_API int pvz_board_get_main_counter(void* b) {
    if (!b) return 0;
    return static_cast<int>(static_cast<Board*>(b)->mMainCounter);
}

PVZ_API int pvz_board_get_paused(void* b) {
    if (!b) return 0;
    return static_cast<Board*>(b)->mPaused ? 1 : 0;
}

// ====== Projectile 导出实现 ======

PVZ_API void pvz_projectile_die(void* p) {
    if (!p) return;
    static_cast<Projectile*>(p)->Die();
}

PVZ_API void pvz_projectile_do_impact(void* p, void* zombie) {
    if (!p || !zombie) return;
    static_cast<Projectile*>(p)->DoImpact(static_cast<Zombie*>(zombie));
}

PVZ_API void pvz_projectile_do_splash_damage(void* p, void* zombie) {
    if (!p) return;
    static_cast<Projectile*>(p)->DoSplashDamage(static_cast<Zombie*>(zombie));
}

PVZ_API void pvz_projectile_update_motion(void* p) {
    if (!p) return;
    static_cast<Projectile*>(p)->UpdateMotion();
}

PVZ_API void pvz_projectile_convert_to_fireball(void* p, int grid_x) {
    if (!p) return;
    static_cast<Projectile*>(p)->ConvertToFireball(grid_x);
}

PVZ_API void pvz_projectile_convert_to_pea(void* p, int grid_x) {
    if (!p) return;
    static_cast<Projectile*>(p)->ConvertToPea(grid_x);
}

PVZ_API int pvz_projectile_get_type(void* p) {
    if (!p) return -1;
    return static_cast<int>(static_cast<Projectile*>(p)->mProjectileType);
}

PVZ_API int pvz_projectile_get_motion_type(void* p) {
    if (!p) return -1;
    return static_cast<int>(static_cast<Projectile*>(p)->mMotionType);
}

PVZ_API int pvz_projectile_get_row(void* p) {
    if (!p) return -1;
    return static_cast<Projectile*>(p)->mRow;
}

PVZ_API int pvz_projectile_get_x(void* p) {
    if (!p) return 0;
    return static_cast<Projectile*>(p)->mX;
}

PVZ_API int pvz_projectile_get_y(void* p) {
    if (!p) return 0;
    return static_cast<Projectile*>(p)->mY;
}

PVZ_API float pvz_projectile_get_pos_x(void* p) {
    if (!p) return 0.0f;
    return static_cast<Projectile*>(p)->mPosX;
}

PVZ_API void pvz_projectile_set_pos_x(void* p, float v) {
    if (!p) return;
    static_cast<Projectile*>(p)->mPosX = v;
}

PVZ_API float pvz_projectile_get_pos_y(void* p) {
    if (!p) return 0.0f;
    return static_cast<Projectile*>(p)->mPosY;
}

PVZ_API void pvz_projectile_set_pos_y(void* p, float v) {
    if (!p) return;
    static_cast<Projectile*>(p)->mPosY = v;
}

PVZ_API float pvz_projectile_get_vel_x(void* p) {
    if (!p) return 0.0f;
    return static_cast<Projectile*>(p)->mVelX;
}

PVZ_API void pvz_projectile_set_vel_x(void* p, float v) {
    if (!p) return;
    static_cast<Projectile*>(p)->mVelX = v;
}

PVZ_API float pvz_projectile_get_vel_y(void* p) {
    if (!p) return 0.0f;
    return static_cast<Projectile*>(p)->mVelY;
}

PVZ_API void pvz_projectile_set_vel_y(void* p, float v) {
    if (!p) return;
    static_cast<Projectile*>(p)->mVelY = v;
}

PVZ_API int pvz_projectile_get_age(void* p) {
    if (!p) return 0;
    return static_cast<Projectile*>(p)->mProjectileAge;
}

PVZ_API int pvz_projectile_get_dead(void* p) {
    if (!p) return 1;
    return static_cast<Projectile*>(p)->mDead ? 1 : 0;
}

PVZ_API int pvz_projectile_get_damage_range_flags(void* p) {
    if (!p) return 0;
    return static_cast<Projectile*>(p)->mDamageRangeFlags;
}

PVZ_API int pvz_projectile_get_target_zombie_id(void* p) {
    if (!p) return 0;
    return static_cast<int>(static_cast<Projectile*>(p)->mTargetZombieID);
}

// ====== Coin 导出实现 ======

PVZ_API void pvz_coin_die(void* c) {
    if (!c) return;
    static_cast<Coin*>(c)->Die();
}

PVZ_API void pvz_coin_collect(void* c) {
    if (!c) return;
    static_cast<Coin*>(c)->Collect();
}

PVZ_API void pvz_coin_start_fade(void* c) {
    if (!c) return;
    static_cast<Coin*>(c)->StartFade();
}

PVZ_API int pvz_coin_get_sun_value(void* c) {
    if (!c) return 0;
    return static_cast<Coin*>(c)->GetSunValue();
}

PVZ_API int pvz_coin_is_money(void* c) {
    if (!c) return 0;
    return static_cast<Coin*>(c)->IsMoney() ? 1 : 0;
}

PVZ_API int pvz_coin_is_sun(void* c) {
    if (!c) return 0;
    return static_cast<Coin*>(c)->IsSun() ? 1 : 0;
}

PVZ_API int pvz_coin_is_level_award(void* c) {
    if (!c) return 0;
    return static_cast<Coin*>(c)->IsLevelAward() ? 1 : 0;
}

PVZ_API int pvz_coin_get_type(void* c) {
    if (!c) return -1;
    return static_cast<int>(static_cast<Coin*>(c)->mType);
}

PVZ_API int pvz_coin_get_motion(void* c) {
    if (!c) return -1;
    return static_cast<int>(static_cast<Coin*>(c)->mCoinMotion);
}

PVZ_API int pvz_coin_get_row(void* c) {
    if (!c) return -1;
    return static_cast<Coin*>(c)->mRow;
}

PVZ_API int pvz_coin_get_x(void* c) {
    if (!c) return 0;
    return static_cast<Coin*>(c)->mX;
}

PVZ_API int pvz_coin_get_y(void* c) {
    if (!c) return 0;
    return static_cast<Coin*>(c)->mY;
}

PVZ_API float pvz_coin_get_pos_x(void* c) {
    if (!c) return 0.0f;
    return static_cast<Coin*>(c)->mPosX;
}

PVZ_API void pvz_coin_set_pos_x(void* c, float v) {
    if (!c) return;
    static_cast<Coin*>(c)->mPosX = v;
}

PVZ_API float pvz_coin_get_pos_y(void* c) {
    if (!c) return 0.0f;
    return static_cast<Coin*>(c)->mPosY;
}

PVZ_API void pvz_coin_set_pos_y(void* c, float v) {
    if (!c) return;
    static_cast<Coin*>(c)->mPosY = v;
}

PVZ_API float pvz_coin_get_vel_x(void* c) {
    if (!c) return 0.0f;
    return static_cast<Coin*>(c)->mVelX;
}

PVZ_API void pvz_coin_set_vel_x(void* c, float v) {
    if (!c) return;
    static_cast<Coin*>(c)->mVelX = v;
}

PVZ_API float pvz_coin_get_vel_y(void* c) {
    if (!c) return 0.0f;
    return static_cast<Coin*>(c)->mVelY;
}

PVZ_API void pvz_coin_set_vel_y(void* c, float v) {
    if (!c) return;
    static_cast<Coin*>(c)->mVelY = v;
}

PVZ_API int pvz_coin_get_age(void* c) {
    if (!c) return 0;
    return static_cast<Coin*>(c)->mCoinAge;
}

PVZ_API int pvz_coin_get_dead(void* c) {
    if (!c) return 1;
    return static_cast<Coin*>(c)->mDead ? 1 : 0;
}

PVZ_API int pvz_coin_get_being_collected(void* c) {
    if (!c) return 0;
    return static_cast<Coin*>(c)->mIsBeingCollected ? 1 : 0;
}

// ====== LawnMower 导出实现 ======

PVZ_API void pvz_mower_start(void* m) {
    if (!m) return;
    static_cast<LawnMower*>(m)->StartMower();
}

PVZ_API void pvz_mower_die(void* m) {
    if (!m) return;
    static_cast<LawnMower*>(m)->Die();
}

PVZ_API void pvz_mower_squish(void* m) {
    if (!m) return;
    static_cast<LawnMower*>(m)->SquishMower();
}

PVZ_API void pvz_mower_enable_super(void* m, int enable) {
    if (!m) return;
    static_cast<LawnMower*>(m)->EnableSuperMower(enable != 0);
}

PVZ_API int pvz_mower_get_state(void* m) {
    if (!m) return -1;
    return static_cast<int>(static_cast<LawnMower*>(m)->mMowerState);
}

PVZ_API int pvz_mower_get_type(void* m) {
    if (!m) return -1;
    return static_cast<int>(static_cast<LawnMower*>(m)->mMowerType);
}

PVZ_API int pvz_mower_get_row(void* m) {
    if (!m) return -1;
    return static_cast<LawnMower*>(m)->mRow;
}

PVZ_API float pvz_mower_get_pos_x(void* m) {
    if (!m) return 0.0f;
    return static_cast<LawnMower*>(m)->mPosX;
}

PVZ_API void pvz_mower_set_pos_x(void* m, float v) {
    if (!m) return;
    static_cast<LawnMower*>(m)->mPosX = v;
}

PVZ_API float pvz_mower_get_pos_y(void* m) {
    if (!m) return 0.0f;
    return static_cast<LawnMower*>(m)->mPosY;
}

PVZ_API int pvz_mower_get_dead(void* m) {
    if (!m) return 1;
    return static_cast<LawnMower*>(m)->mDead ? 1 : 0;
}

PVZ_API int pvz_mower_get_visible(void* m) {
    if (!m) return 0;
    return static_cast<LawnMower*>(m)->mVisible ? 1 : 0;
}

// ====== GridItem 导出实现 ======

PVZ_API void pvz_griditem_die(void* g) {
    if (!g) return;
    static_cast<GridItem*>(g)->GridItemDie();
}

PVZ_API void pvz_griditem_open_portal(void* g) {
    if (!g) return;
    static_cast<GridItem*>(g)->OpenPortal();
}

PVZ_API void pvz_griditem_close_portal(void* g) {
    if (!g) return;
    static_cast<GridItem*>(g)->ClosePortal();
}

PVZ_API int pvz_griditem_is_open_portal(void* g) {
    if (!g) return 0;
    return static_cast<GridItem*>(g)->IsOpenPortal() ? 1 : 0;
}

PVZ_API int pvz_griditem_get_type(void* g) {
    if (!g) return -1;
    return static_cast<int>(static_cast<GridItem*>(g)->mGridItemType);
}

PVZ_API int pvz_griditem_get_state(void* g) {
    if (!g) return -1;
    return static_cast<int>(static_cast<GridItem*>(g)->mGridItemState);
}

PVZ_API int pvz_griditem_get_grid_x(void* g) {
    if (!g) return -1;
    return static_cast<GridItem*>(g)->mGridX;
}

PVZ_API int pvz_griditem_get_grid_y(void* g) {
    if (!g) return -1;
    return static_cast<GridItem*>(g)->mGridY;
}

PVZ_API float pvz_griditem_get_pos_x(void* g) {
    if (!g) return 0.0f;
    return static_cast<GridItem*>(g)->mPosX;
}

PVZ_API void pvz_griditem_set_pos_x(void* g, float v) {
    if (!g) return;
    static_cast<GridItem*>(g)->mPosX = v;
}

PVZ_API float pvz_griditem_get_pos_y(void* g) {
    if (!g) return 0.0f;
    return static_cast<GridItem*>(g)->mPosY;
}

PVZ_API void pvz_griditem_set_pos_y(void* g, float v) {
    if (!g) return;
    static_cast<GridItem*>(g)->mPosY = v;
}

PVZ_API int pvz_griditem_get_dead(void* g) {
    if (!g) return 1;
    return static_cast<GridItem*>(g)->mDead ? 1 : 0;
}

PVZ_API int pvz_griditem_get_zombie_type(void* g) {
    if (!g) return -1;
    return static_cast<int>(static_cast<GridItem*>(g)->mZombieType);
}

PVZ_API int pvz_griditem_get_seed_type(void* g) {
    if (!g) return -1;
    return static_cast<int>(static_cast<GridItem*>(g)->mSeedType);
}

// ====== LawnApp 全局 API 导出实现 ======

PVZ_API void* pvz_app_get() {
    return gLawnApp;
}

PVZ_API void* pvz_app_get_board() {
    return gLawnApp ? gLawnApp->mBoard : nullptr;
}

PVZ_API int pvz_app_get_level() {
    if (!gLawnApp || !gLawnApp->mBoard) return -1;
    return gLawnApp->mBoard->mLevel;
}

PVZ_API int pvz_app_get_game_mode() {
    if (!gLawnApp) return -1;
    return static_cast<int>(gLawnApp->mGameMode);
}

PVZ_API int pvz_app_get_game_scene() {
    if (!gLawnApp) return -1;
    return static_cast<int>(gLawnApp->mGameScene);
}

PVZ_API int pvz_app_get_board_result() {
    if (!gLawnApp) return -1;
    return static_cast<int>(gLawnApp->mBoardResult);
}

PVZ_API int pvz_app_is_adventure_mode() {
    if (!gLawnApp) return 0;
    return gLawnApp->IsAdventureMode() ? 1 : 0;
}

PVZ_API int pvz_app_is_survival_mode() {
    if (!gLawnApp) return 0;
    return gLawnApp->IsSurvivalMode() ? 1 : 0;
}

PVZ_API int pvz_app_is_challenge_mode() {
    if (!gLawnApp) return 0;
    return gLawnApp->IsChallengeMode() ? 1 : 0;
}

PVZ_API int pvz_app_is_puzzle_mode() {
    if (!gLawnApp) return 0;
    return gLawnApp->IsPuzzleMode() ? 1 : 0;
}

PVZ_API int pvz_app_is_night() {
    if (!gLawnApp) return 0;
    return gLawnApp->IsNight() ? 1 : 0;
}

PVZ_API int pvz_app_is_final_boss_level() {
    if (!gLawnApp) return 0;
    return gLawnApp->IsFinalBossLevel() ? 1 : 0;
}

PVZ_API int pvz_app_has_finished_adventure() {
    if (!gLawnApp) return 0;
    return gLawnApp->HasFinishedAdventure() ? 1 : 0;
}

PVZ_API void pvz_app_end_level() {
    if (!gLawnApp) return;
    gLawnApp->EndLevel();
}

PVZ_API void pvz_app_kill_board() {
    if (!gLawnApp) return;
    gLawnApp->KillBoard();
}

PVZ_API void pvz_app_show_game_selector() {
    if (!gLawnApp) return;
    gLawnApp->ShowGameSelector();
}

PVZ_API int pvz_app_save_file_exists() {
    if (!gLawnApp) return 0;
    return gLawnApp->SaveFileExists() ? 1 : 0;
}

PVZ_API void pvz_app_play_foley(int foley_type) {
    if (!gLawnApp) return;
    gLawnApp->PlayFoley(static_cast<FoleyType>(foley_type));
}

PVZ_API void pvz_app_toggle_slow_mo() {
    if (!gLawnApp) return;
    gLawnApp->ToggleSlowMo();
}

PVZ_API void pvz_app_toggle_fast_mo() {
    if (!gLawnApp) return;
    gLawnApp->ToggleFastMo();
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

PVZ_API int pvz_offset_of_projectile(const char* field_name) {
    if (!field_name) return -1;
    using P = Projectile;
    if (strcmp(field_name, "row")                == 0) return offsetof(P, mRow);
    if (strcmp(field_name, "x")                  == 0) return offsetof(P, mX);
    if (strcmp(field_name, "y")                  == 0) return offsetof(P, mY);
    if (strcmp(field_name, "pos_x")              == 0) return offsetof(P, mPosX);
    if (strcmp(field_name, "pos_y")              == 0) return offsetof(P, mPosY);
    if (strcmp(field_name, "vel_x")              == 0) return offsetof(P, mVelX);
    if (strcmp(field_name, "vel_y")              == 0) return offsetof(P, mVelY);
    if (strcmp(field_name, "type")               == 0) return offsetof(P, mProjectileType);
    if (strcmp(field_name, "motion_type")        == 0) return offsetof(P, mMotionType);
    if (strcmp(field_name, "age")                == 0) return offsetof(P, mProjectileAge);
    if (strcmp(field_name, "dead")               == 0) return offsetof(P, mDead);
    if (strcmp(field_name, "damage_range_flags") == 0) return offsetof(P, mDamageRangeFlags);
    return -1;
}

PVZ_API int pvz_offset_of_coin(const char* field_name) {
    if (!field_name) return -1;
    using C = Coin;
    if (strcmp(field_name, "row")             == 0) return offsetof(C, mRow);
    if (strcmp(field_name, "x")               == 0) return offsetof(C, mX);
    if (strcmp(field_name, "y")               == 0) return offsetof(C, mY);
    if (strcmp(field_name, "pos_x")           == 0) return offsetof(C, mPosX);
    if (strcmp(field_name, "pos_y")           == 0) return offsetof(C, mPosY);
    if (strcmp(field_name, "vel_x")           == 0) return offsetof(C, mVelX);
    if (strcmp(field_name, "vel_y")           == 0) return offsetof(C, mVelY);
    if (strcmp(field_name, "type")            == 0) return offsetof(C, mType);
    if (strcmp(field_name, "motion")          == 0) return offsetof(C, mCoinMotion);
    if (strcmp(field_name, "age")             == 0) return offsetof(C, mCoinAge);
    if (strcmp(field_name, "dead")            == 0) return offsetof(C, mDead);
    if (strcmp(field_name, "being_collected") == 0) return offsetof(C, mIsBeingCollected);
    return -1;
}

PVZ_API int pvz_offset_of_mower(const char* field_name) {
    if (!field_name) return -1;
    using M = LawnMower;
    if (strcmp(field_name, "row")       == 0) return offsetof(M, mRow);
    if (strcmp(field_name, "pos_x")     == 0) return offsetof(M, mPosX);
    if (strcmp(field_name, "pos_y")     == 0) return offsetof(M, mPosY);
    if (strcmp(field_name, "state")     == 0) return offsetof(M, mMowerState);
    if (strcmp(field_name, "type")      == 0) return offsetof(M, mMowerType);
    if (strcmp(field_name, "dead")      == 0) return offsetof(M, mDead);
    if (strcmp(field_name, "visible")   == 0) return offsetof(M, mVisible);
    return -1;
}

PVZ_API int pvz_offset_of_griditem(const char* field_name) {
    if (!field_name) return -1;
    using G = GridItem;
    if (strcmp(field_name, "grid_x")      == 0) return offsetof(G, mGridX);
    if (strcmp(field_name, "grid_y")      == 0) return offsetof(G, mGridY);
    if (strcmp(field_name, "pos_x")       == 0) return offsetof(G, mPosX);
    if (strcmp(field_name, "pos_y")       == 0) return offsetof(G, mPosY);
    if (strcmp(field_name, "type")        == 0) return offsetof(G, mGridItemType);
    if (strcmp(field_name, "state")       == 0) return offsetof(G, mGridItemState);
    if (strcmp(field_name, "dead")        == 0) return offsetof(G, mDead);
    if (strcmp(field_name, "zombie_type") == 0) return offsetof(G, mZombieType);
    if (strcmp(field_name, "seed_type")   == 0) return offsetof(G, mSeedType);
    return -1;
}

// ====== 字段类型查询 ======
// 返回类型代号：1=int32, 2=int64, 3=float, 4=double, 5=bool, 6=pointer, 0=未知
// mod 用此 API 确定如何 cast 指针读取字段

PVZ_API int pvz_field_type_of(int object_kind, const char* field_name) {
    if (!field_name) return 0;
    // 内联辅助：根据字段名后缀判断类型
    // 命名约定：以 _x/_y 结尾的浮点字段用 mPosX/mPosY（float）
    //          整数字段如 row/grid_x/grid_y/age/type 等为 int32
    //          dead/visible/mind_controlled 等为 bool
    auto ends_with = [](const char* s, const char* suffix) -> bool {
        size_t ls = strlen(s), lf = strlen(suffix);
        if (ls < lf) return false;
        return strcmp(s + ls - lf, suffix) == 0;
    };

    // 浮点字段：pos_x, pos_y, vel_x, vel_y, scale 等
    if (ends_with(field_name, "_x") || ends_with(field_name, "_y")) {
        // x/y 单独（GameObject 基类 int32）需要特殊处理
        if (strcmp(field_name, "x") == 0 || strcmp(field_name, "y") == 0) {
            return 1;  // int32 (GameObject::mX/mY)
        }
        return 3;  // float (mPosX/mPosY/mVelX/mVelY)
    }

    // bool 字段
    if (strcmp(field_name, "dead") == 0 || strcmp(field_name, "visible") == 0 ||
        strcmp(field_name, "mind_controlled") == 0 || strcmp(field_name, "being_collected") == 0) {
        return 5;  // bool
    }

    // 其余已知字段都是 int32（row, type, state, age, level, sun_money, wave, counter, health, etc.）
    // 对于未识别字段，返回 0 让 mod 自行判断
    if (strcmp(field_name, "row") == 0 || strcmp(field_name, "grid_x") == 0 ||
        strcmp(field_name, "grid_y") == 0 || strcmp(field_name, "age") == 0 ||
        strcmp(field_name, "type") == 0 || strcmp(field_name, "state") == 0 ||
        strcmp(field_name, "motion") == 0 || strcmp(field_name, "motion_type") == 0 ||
        strcmp(field_name, "level") == 0 || strcmp(field_name, "sun_money") == 0 ||
        strcmp(field_name, "current_wave") == 0 || strcmp(field_name, "num_waves") == 0 ||
        strcmp(field_name, "main_counter") == 0 || strcmp(field_name, "health") == 0 ||
        strcmp(field_name, "max_health") == 0 || strcmp(field_name, "helm_health") == 0 ||
        strcmp(field_name, "helm_max_health") == 0 || strcmp(field_name, "shield_health") == 0 ||
        strcmp(field_name, "shield_max_health") == 0 || strcmp(field_name, "from_wave") == 0 ||
        strcmp(field_name, "chilled") == 0 || strcmp(field_name, "buttered") == 0 ||
        strcmp(field_name, "ice_trap") == 0 || strcmp(field_name, "phase") == 0 ||
        strcmp(field_name, "seed_type") == 0 || strcmp(field_name, "col") == 0 ||
        strcmp(field_name, "subclass") == 0 || strcmp(field_name, "zombie_type") == 0 ||
        strcmp(field_name, "damage_range_flags") == 0 || strcmp(field_name, "level_complete") == 0) {
        return 1;  // int32
    }

    // object_kind 未使用（字段名已足够区分），保留参数以便未来扩展
    (void)object_kind;
    return 0;  // 未知
}
