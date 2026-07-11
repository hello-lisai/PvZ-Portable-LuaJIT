// ===== sol2 自动绑定代码 =====
// 此文件由 tools/gen_sol2_bindings.py 自动生成
// 请勿手动编辑，修改请运行: python tools/gen_sol2_bindings.py
//
// 绑定策略:
//   - 成员变量: &Class::member (sol2 自动生成 getter/setter)
//   - 简单方法: &Class::method (sol2 自动处理类型转换)
//   - 复杂方法(引用参数/默认值): 跳过，需手动绑定
//
// 绑定类数: 7

#pragma once

#include <sol/sol.hpp>

// 项目头文件
#include "../../Lawn/Zombie.h"
#include "../../Lawn/Plant.h"
#include "../../Lawn/Board.h"
#include "../../Lawn/Projectile.h"
#include "../../Lawn/Coin.h"
#include "../../Lawn/GridItem.h"
#include "../../Lawn/LawnMower.h"

// ===== 自动生成的 Zombie sol2 绑定 =====
// 由 tools/gen_sol2_bindings.py 生成，请勿手动编辑
static void BindSol2Zombie(sol::state_view& lua) {
    auto ut = lua.new_usertype<Zombie>("PvZ.Zombie",
        sol::no_constructor,
        "mZombieType", &Zombie::mZombieType,
        "mZombiePhase", &Zombie::mZombiePhase,
        "mPosX", &Zombie::mPosX,
        "mPosY", &Zombie::mPosY,
        "mVelX", &Zombie::mVelX,
        "mAnimCounter", &Zombie::mAnimCounter,
        "mGroanCounter", &Zombie::mGroanCounter,
        "mAnimTicksPerFrame", &Zombie::mAnimTicksPerFrame,
        "mAnimFrames", &Zombie::mAnimFrames,
        "mFrame", &Zombie::mFrame,
        "mPrevFrame", &Zombie::mPrevFrame,
        "mVariant", &Zombie::mVariant,
        "mIsEating", &Zombie::mIsEating,
        "mJustGotShotCounter", &Zombie::mJustGotShotCounter,
        "mShieldJustGotShotCounter", &Zombie::mShieldJustGotShotCounter,
        "mShieldRecoilCounter", &Zombie::mShieldRecoilCounter,
        "mZombieAge", &Zombie::mZombieAge,
        "mZombieHeight", &Zombie::mZombieHeight,
        "mPhaseCounter", &Zombie::mPhaseCounter,
        "mFromWave", &Zombie::mFromWave,
        "mDroppedLoot", &Zombie::mDroppedLoot,
        "mZombieFade", &Zombie::mZombieFade,
        "mFlatTires", &Zombie::mFlatTires,
        "mUseLadderCol", &Zombie::mUseLadderCol,
        "mTargetCol", &Zombie::mTargetCol,
        "mAltitude", &Zombie::mAltitude,
        "mHitUmbrella", &Zombie::mHitUmbrella,
        // mZombieRect: 跳过 (unbindable value type (Rect))
        // mZombieAttackRect: 跳过 (unbindable value type (Rect))
        "mChilledCounter", &Zombie::mChilledCounter,
        "mButteredCounter", &Zombie::mButteredCounter,
        "mIceTrapCounter", &Zombie::mIceTrapCounter,
        "mMindControlled", &Zombie::mMindControlled,
        "mBlowingAway", &Zombie::mBlowingAway,
        "mHasHead", &Zombie::mHasHead,
        "mHasArm", &Zombie::mHasArm,
        "mHasObject", &Zombie::mHasObject,
        "mInPool", &Zombie::mInPool,
        "mOnHighGround", &Zombie::mOnHighGround,
        "mYuckyFace", &Zombie::mYuckyFace,
        "mYuckyFaceCounter", &Zombie::mYuckyFaceCounter,
        "mHelmType", &Zombie::mHelmType,
        "mBodyHealth", &Zombie::mBodyHealth,
        "mBodyMaxHealth", &Zombie::mBodyMaxHealth,
        "mHelmHealth", &Zombie::mHelmHealth,
        "mHelmMaxHealth", &Zombie::mHelmMaxHealth,
        "mShieldType", &Zombie::mShieldType,
        "mShieldHealth", &Zombie::mShieldHealth,
        "mShieldMaxHealth", &Zombie::mShieldMaxHealth,
        "mFlyingHealth", &Zombie::mFlyingHealth,
        "mFlyingMaxHealth", &Zombie::mFlyingMaxHealth,
        "mDead", &Zombie::mDead,
        "mRelatedZombieID", &Zombie::mRelatedZombieID,
        // mFollowerZombieID: 跳过 (array member)
        "mPlayingSong", &Zombie::mPlayingSong,
        "mParticleOffsetX", &Zombie::mParticleOffsetX,
        "mParticleOffsetY", &Zombie::mParticleOffsetY,
        "mAttachmentID", &Zombie::mAttachmentID,
        "mSummonCounter", &Zombie::mSummonCounter,
        "mBodyReanimID", &Zombie::mBodyReanimID,
        "mScaleZombie", &Zombie::mScaleZombie,
        "mVelZ", &Zombie::mVelZ,
        "mOriginalAnimRate", &Zombie::mOriginalAnimRate,
        "mTargetPlantID", &Zombie::mTargetPlantID,
        "mBossMode", &Zombie::mBossMode,
        "mTargetRow", &Zombie::mTargetRow,
        "mBossBungeeCounter", &Zombie::mBossBungeeCounter,
        "mBossStompCounter", &Zombie::mBossStompCounter,
        "mBossHeadCounter", &Zombie::mBossHeadCounter,
        "mBossFireBallReanimID", &Zombie::mBossFireBallReanimID,
        "mSpecialHeadReanimID", &Zombie::mSpecialHeadReanimID,
        "mFireballRow", &Zombie::mFireballRow,
        "mIsFireBall", &Zombie::mIsFireBall,
        "mMoweredReanimID", &Zombie::mMoweredReanimID,
        "mLastPortalX", &Zombie::mLastPortalX,
        "zombie_initialize", &Zombie::ZombieInitialize,
        "animate", &Zombie::Animate,
        "check_if_prey_caught", &Zombie::CheckIfPreyCaught,
        "eat_zombie", &Zombie::EatZombie,
        "eat_plant", &Zombie::EatPlant,
        "is_plant_inedible_at_eating", &Zombie::IsPlantInedibleAtEating,
        "try_trigger_plant_on_eating", &Zombie::TryTriggerPlantOnEating,
        "handle_i_zombie_sunflower_drop", &Zombie::HandleIZombieSunflowerDrop,
        "handle_plant_eaten_death", &Zombie::HandlePlantEatenDeath,
        "update", &Zombie::Update,
        "die_no_loot", &Zombie::DieNoLoot,
        // draw: 跳过 (has unregistered pointer param (Graphics*)) — void                            Draw(Graphics* g);
        // draw_zombie_part: 跳过 (has reference param) — void                            DrawZombiePart(Graphics* g, Image* theImage, int theFrame, int theRow, const ZombieDrawPosition& theDrawPos);
        // draw_bungee_cord: 跳过 (has unregistered pointer param (Graphics*)) — void                            DrawBungeeCord(Graphics* g, int theOffsetX);
        "take_damage", &Zombie::TakeDamage,
        "get_pos_y_based_on_row", &Zombie::GetPosYBasedOnRow,
        "apply_chill", &Zombie::ApplyChill,
        "update_zombie_bungee", &Zombie::UpdateZombieBungee,
        "bungee_landing", &Zombie::BungeeLanding,
        "effected_by_damage", &Zombie::EffectedByDamage,
        "pick_random_speed", &Zombie::PickRandomSpeed,
        "update_zombie_polevaulter", &Zombie::UpdateZombiePolevaulter,
        "update_zombie_dolphin_rider", &Zombie::UpdateZombieDolphinRider,
        "pick_bungee_zombie_target", &Zombie::PickBungeeZombieTarget,
        "count_bungees_targeting_sun_flowers", &Zombie::CountBungeesTargetingSunFlowers,
        "find_plant_target", &Zombie::FindPlantTarget,
        "check_squish", &Zombie::CheckSquish,
        "rise_from_grave", &Zombie::RiseFromGrave,
        "update_zombie_rise_from_grave", &Zombie::UpdateZombieRiseFromGrave,
        "update_damage_states", &Zombie::UpdateDamageStates,
        "update_zombie_pool", &Zombie::UpdateZombiePool,
        "check_for_pool", &Zombie::CheckForPool,
        // get_draw_pos: 跳过 (has reference param) — void                            GetDrawPos(ZombieDrawPosition& theDrawPos);
        "update_zombie_high_ground", &Zombie::UpdateZombieHighGround,
        "check_for_high_ground", &Zombie::CheckForHighGround,
        "is_on_high_ground", &Zombie::IsOnHighGround,
        "drop_loot", &Zombie::DropLoot,
        "try_spawn_level_award", &Zombie::TrySpawnLevelAward,
        "stop_zombie_sound", &Zombie::StopZombieSound,
        "update_zombie_jack_in_the_box", &Zombie::UpdateZombieJackInTheBox,
        // draw_zombie_head: 跳过 (has reference param) — void                            DrawZombieHead(Graphics* g, const ZombieDrawPosition& theDrawPos, int theFrame);
        "update_zombie_position", &Zombie::UpdateZombiePosition,
        // get_zombie_rect: 跳过 (returns unbindable type (Rect)) — Rect                            GetZombieRect();
        // get_zombie_attack_rect: 跳过 (returns unbindable type (Rect)) — Rect                            GetZombieAttackRect();
        "update_zombie_walking", &Zombie::UpdateZombieWalking,
        "update_zombie_bobsled", &Zombie::UpdateZombieBobsled,
        "bobsled_crash", &Zombie::BobsledCrash,
        "is_standing_on_spikeweed", &Zombie::IsStandingOnSpikeweed,
        "check_for_zombie_step", &Zombie::CheckForZombieStep,
        "pool_splash", &Zombie::PoolSplash,
        "update_zombie_flyer", &Zombie::UpdateZombieFlyer,
        "update_zombie_pogo", &Zombie::UpdateZombiePogo,
        "update_zombie_newspaper", &Zombie::UpdateZombieNewspaper,
        "land_flyer", &Zombie::LandFlyer,
        "update_zombie_digger", &Zombie::UpdateZombieDigger,
        "is_walking_backwards", &Zombie::IsWalkingBackwards,
        // add_attached_particle: 跳过 (returns unbindable type (TodParticle)) — TodParticleSystem*              AddAttachedParticle(int thePosX, int thePosY, ParticleEffect theEffect);
        "pogo_break", &Zombie::PogoBreak,
        "update_zombie_falling", &Zombie::UpdateZombieFalling,
        "update_zombie_dancer", &Zombie::UpdateZombieDancer,
        "summon_backup_dancer", &Zombie::SummonBackupDancer,
        "summon_backup_dancers", &Zombie::SummonBackupDancers,
        "get_dancer_frame", &Zombie::GetDancerFrame,
        "bungee_steal_target", &Zombie::BungeeStealTarget,
        "bungee_lift_target", &Zombie::BungeeLiftTarget,
        "update_yucky_face", &Zombie::UpdateYuckyFace,
        // draw_ice_trap: 跳过 (has reference param) — void                            DrawIceTrap(Graphics* g, const ZombieDrawPosition& theDrawPos, bool theFront);
        "hit_ice_trap", &Zombie::HitIceTrap,
        "get_helm_damage_index", &Zombie::GetHelmDamageIndex,
        "get_shield_damage_index", &Zombie::GetShieldDamageIndex,
        // draw_reanim: 跳过 (has reference param) — void                            DrawReanim(Graphics* g, const ZombieDrawPosition& theDrawPos, int theBaseRenderGroup);
        "update_playing", &Zombie::UpdatePlaying,
        "needs_more_backup_dancers", &Zombie::NeedsMoreBackupDancers,
        "convert_to_normal_zombie", &Zombie::ConvertToNormalZombie,
        "start_eating", &Zombie::StartEating,
        "stop_eating", &Zombie::StopEating,
        "update_anim_speed", &Zombie::UpdateAnimSpeed,
        "play_death_anim", &Zombie::PlayDeathAnim,
        "update_death", &Zombie::UpdateDeath,
        // draw_shadow: 跳过 (has unregistered pointer param (Graphics*)) — void                            DrawShadow(Graphics* g);
        "has_shadow", &Zombie::HasShadow,
        // load_reanim: 跳过 (returns unregistered pointer type (Reanimation*)) — Reanimation*                    LoadReanim(ReanimationType theReanimationType);
        "take_shield_damage", &Zombie::TakeShieldDamage,
        "take_helm_damage", &Zombie::TakeHelmDamage,
        "take_body_damage", &Zombie::TakeBodyDamage,
        "attach_shield", &Zombie::AttachShield,
        "detach_shield", &Zombie::DetachShield,
        "update_reanim", &Zombie::UpdateReanim,
        // get_track_position: 跳过 (has reference param) — void                            GetTrackPosition(const char* theTrackName, float& thePosX, float& thePosY);
        "load_plain_zombie_reanim", &Zombie::LoadPlainZombieReanim,
        "show_door_arms", &Zombie::ShowDoorArms,
        "start_mind_controlled", &Zombie::StartMindControlled,
        // get_mind_control_color: 跳过 (has default param) — Color                           GetMindControlColor(int alpha = 255);
        // apply_mind_control_image_tint: 跳过 (has reference param) — void                            ApplyMindControlImageTint(Graphics* g, Image* theImage, const Rect& aDestRect, const Rect& aSrcRect, bool aMirror, int anAlpha);
        // get_mind_control_reanim_color: 跳过 (has reference param) — void                            GetMindControlReanimColor(Color& aColorOverride, Color& aExtraAdditiveColor, bool& aEnableExtraAdditiveDraw, int aFadeAlpha);
        // apply_mind_control_reanim_mirror: 跳过 (has reference param) — void                            ApplyMindControlReanimMirror(bool& anOpposite);
        "check_mind_control_edge_death", &Zombie::CheckMindControlEdgeDeath,
        "is_mind_control_attack_target", &Zombie::IsMindControlAttackTarget,
        "try_mind_control_attack", &Zombie::TryMindControlAttack,
        // apply_mind_control_particle_tint: 跳过 (has unregistered pointer param (TodParticleSystem*)) — void                            ApplyMindControlParticleTint(TodParticleSystem* aParticle);
        "is_flying", &Zombie::IsFlying,
        "drop_head", &Zombie::DropHead,
        "can_target_plant", &Zombie::CanTargetPlant,
        "update_zombie_catapult", &Zombie::UpdateZombieCatapult,
        "find_catapult_target", &Zombie::FindCatapultTarget,
        "zombie_catapult_fire", &Zombie::ZombieCatapultFire,
        "update_climbing_ladder", &Zombie::UpdateClimbingLadder,
        "update_zombie_gargantuar", &Zombie::UpdateZombieGargantuar,
        "gargantuar_smash_attack", &Zombie::GargantuarSmashAttack,
        "gargantuar_throw_imp", &Zombie::GargantuarThrowImp,
        "gargantuar_should_smash", &Zombie::GargantuarShouldSmash,
        "init_zombie_type_normal", &Zombie::InitZombieTypeNormal,
        "init_zombie_type_ducky_tube", &Zombie::InitZombieTypeDuckyTube,
        "init_zombie_type_conehead", &Zombie::InitZombieTypeConehead,
        "init_zombie_type_buckethead", &Zombie::InitZombieTypeBuckethead,
        "init_zombie_type_door", &Zombie::InitZombieTypeDoor,
        "init_zombie_type_ladder", &Zombie::InitZombieTypeLadder,
        // init_zombie_type_bungee: 跳过 (has reference param) — void                            InitZombieTypeBungee(RenderLayer& aRenderLayer, int& aRenderOffset);  // 蹦极僵尸初始化
        "init_zombie_type_football", &Zombie::InitZombieTypeFootball,
        // init_zombie_type_digger: 跳过 (has reference param) — void                            InitZombieTypeDigger(int& aRenderOffset);  // 矿工僵尸初始化
        "init_zombie_type_polevaulter", &Zombie::InitZombieTypePolevaulter,
        "init_zombie_type_dolphin_rider", &Zombie::InitZombieTypeDolphinRider,
        // init_zombie_type_gargantuar: 跳过 (has reference param) — void                            InitZombieTypeGargantuar(int& aRenderOffset);  // 伽刚特尔初始化
        // init_zombie_type_zamboni: 跳过 (has reference param) — void                            InitZombieTypeZamboni(int& aRenderOffset);  // 雪犁僵尸初始化
        "init_zombie_type_catapult", &Zombie::InitZombieTypeCatapult,
        "init_zombie_type_snorkel", &Zombie::InitZombieTypeSnorkel,
        "init_zombie_type_jack_in_the_box", &Zombie::InitZombieTypeJackInTheBox,
        // init_zombie_type_bobsled: 跳过 (has reference param) — void                            InitZombieTypeBobsled(Zombie* theParentZombie, int& aRenderOffset);  // 雪橇僵尸初始化
        "init_zombie_type_pogo", &Zombie::InitZombieTypePogo,
        "init_zombie_type_newspaper", &Zombie::InitZombieTypeNewspaper,
        "init_zombie_type_balloon", &Zombie::InitZombieTypeBalloon,
        "init_zombie_type_dancing", &Zombie::InitZombieTypeDancing,
        "init_zombie_type_backup_dancer", &Zombie::InitZombieTypeBackupDancer,
        "init_zombie_type_imp", &Zombie::InitZombieTypeImp,
        // init_zombie_type_boss: 跳过 (has reference param) — void                            InitZombieTypeBoss(RenderLayer& aRenderLayer);  // Boss僵尸初始化
        "update_boss_idle", &Zombie::UpdateBossIdle,
        "update_boss_spawning", &Zombie::UpdateBossSpawning,
        "update_boss_bungees_enter", &Zombie::UpdateBossBungeesEnter,
        "update_boss_stomping", &Zombie::UpdateBossStomping,
        "update_boss_bungee_exit", &Zombie::UpdateBossBungeeExit,
        "update_boss_drop_rv", &Zombie::UpdateBossDropRV,
        "update_boss_head_enter", &Zombie::UpdateBossHeadEnter,
        "update_boss_head_spit", &Zombie::UpdateBossHeadSpit,
        "update_boss_head_leave", &Zombie::UpdateBossHeadLeave,
        // get_zombie_fall_time: 跳过 (has unregistered pointer param (Reanimation*)) — float                           GetZombieFallTime(Reanimation* aBodyReanim);  // 获取僵尸倒地时间
        // update_boss_death_explosions: 跳过 (has unregistered pointer param (Reanimation*)) — void                            UpdateBossDeathExplosions(Reanimation* aBodyReanim);  // Boss死亡爆炸序列
        "update_zamboni_catapult_death", &Zombie::UpdateZamboniCatapultDeath,
        "take_body_damage_zamboni", &Zombie::TakeBodyDamageZamboni,
        "take_body_damage_catapult", &Zombie::TakeBodyDamageCatapult,
        "take_body_damage_gargantuar", &Zombie::TakeBodyDamageGargantuar,
        "take_body_damage_boss", &Zombie::TakeBodyDamageBoss,
        "get_body_damage_index", &Zombie::GetBodyDamageIndex,
        "apply_burn", &Zombie::ApplyBurn,
        "update_burn", &Zombie::UpdateBurn,
        "zombie_not_walking", &Zombie::ZombieNotWalking,
        "find_zombie_target", &Zombie::FindZombieTarget,
        "update_zombie_backup_dancer", &Zombie::UpdateZombieBackupDancer,
        "get_dancer_phase", &Zombie::GetDancerPhase,
        "is_moving_at_chilled_speed", &Zombie::IsMovingAtChilledSpeed,
        "start_walk_anim", &Zombie::StartWalkAnim,
        // add_attached_reanim: 跳过 (returns unregistered pointer type (Reanimation*)) — Reanimation*                    AddAttachedReanim(int thePosX, int thePosY, ReanimationType theReanimType);
        "drag_under", &Zombie::DragUnder,
        // draw_butter: 跳过 (has reference param) — void                            DrawButter(Graphics* g, const ZombieDrawPosition& theDrawPos);
        "is_immobilizied", &Zombie::IsImmobilizied,
        "apply_butter", &Zombie::ApplyButter,
        "zombie_target_lead_x", &Zombie::ZombieTargetLeadX,
        "update_zombie_imp", &Zombie::UpdateZombieImp,
        "squish_all_in_square", &Zombie::SquishAllInSquare,
        "remove_ice_trap", &Zombie::RemoveIceTrap,
        "is_bouncing_pogo", &Zombie::IsBouncingPogo,
        "get_bobsled_position", &Zombie::GetBobsledPosition,
        // draw_bobsled_reanim: 跳过 (has reference param) — void                            DrawBobsledReanim(Graphics* g, const ZombieDrawPosition& theDrawPos, bool theBeforeZombie);
        "bobsled_die", &Zombie::BobsledDie,
        "bobsled_burn", &Zombie::BobsledBurn,
        "is_bobsled_team_with_sled", &Zombie::IsBobsledTeamWithSled,
        "can_be_frozen", &Zombie::CanBeFrozen,
        "can_be_chilled", &Zombie::CanBeChilled,
        "update_zombie_snorkel", &Zombie::UpdateZombieSnorkel,
        "reanim_ignore_clip_rect", &Zombie::ReanimIgnoreClipRect,
        "set_anim_rate", &Zombie::SetAnimRate,
        "apply_anim_rate", &Zombie::ApplyAnimRate,
        // draw_dancer_reanim: 跳过 (has unregistered pointer param (Graphics*)) — void                            DrawDancerReanim(Graphics* g);
        // draw_bungee_reanim: 跳过 (has unregistered pointer param (Graphics*)) — void                            DrawBungeeReanim(Graphics* g);
        // draw_bungee_target: 跳过 (has unregistered pointer param (Graphics*)) — void                            DrawBungeeTarget(Graphics* g);
        "bungee_die", &Zombie::BungeeDie,
        "zamboni_death", &Zombie::ZamboniDeath,
        "catapult_death", &Zombie::CatapultDeath,
        // setup_draw_zombie_won: 跳过 (has unregistered pointer param (Graphics*)) — bool                            SetupDrawZombieWon(Graphics* g);
        "walk_into_house", &Zombie::WalkIntoHouse,
        "update_zamboni", &Zombie::UpdateZamboni,
        "update_zombie_chimney", &Zombie::UpdateZombieChimney,
        "update_ladder", &Zombie::UpdateLadder,
        "drop_arm", &Zombie::DropArm,
        "can_lose_body_parts", &Zombie::CanLoseBodyParts,
        "drop_helm", &Zombie::DropHelm,
        "drop_shield", &Zombie::DropShield,
        "reanim_reenable_clipping", &Zombie::ReanimReenableClipping,
        "update_boss", &Zombie::UpdateBoss,
        "boss_play_idle", &Zombie::BossPlayIdle,
        "boss_rv_landing", &Zombie::BossRVLanding,
        "boss_stomp_contact", &Zombie::BossStompContact,
        "boss_are_bungees_done", &Zombie::BossAreBungeesDone,
        "boss_bungee_spawn", &Zombie::BossBungeeSpawn,
        "boss_spawn_attack", &Zombie::BossSpawnAttack,
        "boss_bungee_attack", &Zombie::BossBungeeAttack,
        "boss_rv_attack", &Zombie::BossRVAttack,
        "boss_spawn_contact", &Zombie::BossSpawnContact,
        "boss_bungee_leave", &Zombie::BossBungeeLeave,
        "boss_stomp_attack", &Zombie::BossStompAttack,
        "boss_can_stomp_row", &Zombie::BossCanStompRow,
        "boss_die", &Zombie::BossDie,
        "boss_head_attack", &Zombie::BossHeadAttack,
        "boss_head_spit_contact", &Zombie::BossHeadSpitContact,
        "boss_head_spit", &Zombie::BossHeadSpit,
        "update_boss_fireball", &Zombie::UpdateBossFireball,
        "boss_destroy_fireball", &Zombie::BossDestroyFireball,
        "boss_destroy_iceball_in_row", &Zombie::BossDestroyIceballInRow,
        "digger_lose_axe", &Zombie::DiggerLoseAxe,
        "bungee_drop_zombie", &Zombie::BungeeDropZombie,
        "show_yucky_face", &Zombie::ShowYuckyFace,
        "animate_chew_sound", &Zombie::AnimateChewSound,
        "animate_chew_effect", &Zombie::AnimateChewEffect,
        "update_actions", &Zombie::UpdateActions,
        "check_for_board_edge", &Zombie::CheckForBoardEdge,
        "update_yeti", &Zombie::UpdateYeti,
        // draw_boss_part: 跳过 (has unregistered pointer param (Graphics*)) — void                            DrawBossPart(Graphics* g, BossPart theBossPart);
        "boss_setup_reanim", &Zombie::BossSetupReanim,
        "mow_down", &Zombie::MowDown,
        "update_mowered", &Zombie::UpdateMowered,
        "drop_flag", &Zombie::DropFlag,
        "drop_pole", &Zombie::DropPole,
        // draw_boss_back_arm: 跳过 (has reference param) — void                            DrawBossBackArm(Graphics* g, const ZombieDrawPosition& theDrawPos);
        "boss_start_death", &Zombie::BossStartDeath,
        "remove_cold_effects", &Zombie::RemoveColdEffects,
        "boss_head_spit_effect", &Zombie::BossHeadSpitEffect,
        // draw_boss_fire_ball: 跳过 (has unregistered pointer param (Graphics*)) — void                            DrawBossFireBall(Graphics* g);
        "update_zombie_pea_head", &Zombie::UpdateZombiePeaHead,
        "update_zombie_jalapeno_head", &Zombie::UpdateZombieJalapenoHead,
        "apply_boss_smoke_particles", &Zombie::ApplyBossSmokeParticles,
        "update_zombiquarium", &Zombie::UpdateZombiquarium,
        "zombiquarium_find_closest_brain", &Zombie::ZombiquariumFindClosestBrain,
        "update_zombie_gatling_head", &Zombie::UpdateZombieGatlingHead,
        "update_zombie_squash_head", &Zombie::UpdateZombieSquashHead,
        "pea_head_shoot_projectile", &Zombie::PeaHeadShootProjectile,
        "jalapeno_head_ignite", &Zombie::JalapenoHeadIgnite,
        "squash_head_get_dest_x", &Zombie::SquashHeadGetDestX,
        "squash_head_smash_attack", &Zombie::SquashHeadSmashAttack,
        "is_tanglekelp_target", &Zombie::IsTanglekelpTarget,
        "has_yucky_face_image", &Zombie::HasYuckyFaceImage,
        "is_tangle_kelp_target", &Zombie::IsTangleKelpTarget,
        "is_fire_resistant", &Zombie::IsFireResistant,
        "bungee_drop_plant", &Zombie::BungeeDropPlant,
        "remove_butter", &Zombie::RemoveButter,
        "balloon_propeller_hat_spin", &Zombie::BalloonPropellerHatSpin,
        "do_daisies", &Zombie::DoDaisies,
        "setup_water_track", &Zombie::SetupWaterTrack,
        "burn_row", &Zombie::BurnRow,
        "setup_reanim_for_lost_head", &Zombie::SetupReanimForLostHead,
        "setup_reanim_for_lost_arm", &Zombie::SetupReanimForLostArm,
        "is_squash_target", &Zombie::IsSquashTarget
    );
}

// ===== 自动生成的 Plant sol2 绑定 =====
// 由 tools/gen_sol2_bindings.py 生成，请勿手动编辑
static void BindSol2Plant(sol::state_view& lua) {
    auto ut = lua.new_usertype<Plant>("PvZ.Plant",
        sol::no_constructor,
        "mSeedType", &Plant::mSeedType,
        "mPlantCol", &Plant::mPlantCol,
        "mAnimCounter", &Plant::mAnimCounter,
        "mFrame", &Plant::mFrame,
        "mFrameLength", &Plant::mFrameLength,
        "mNumFrames", &Plant::mNumFrames,
        "mState", &Plant::mState,
        "mPlantHealth", &Plant::mPlantHealth,
        "mPlantMaxHealth", &Plant::mPlantMaxHealth,
        "mSubclass", &Plant::mSubclass,
        "mDisappearCountdown", &Plant::mDisappearCountdown,
        "mDoSpecialCountdown", &Plant::mDoSpecialCountdown,
        "mStateCountdown", &Plant::mStateCountdown,
        "mLaunchCounter", &Plant::mLaunchCounter,
        "mLaunchRate", &Plant::mLaunchRate,
        // mPlantRect: 跳过 (unbindable value type (Rect))
        // mPlantAttackRect: 跳过 (unbindable value type (Rect))
        "mTargetX", &Plant::mTargetX,
        "mTargetY", &Plant::mTargetY,
        "mStartRow", &Plant::mStartRow,
        "mParticleID", &Plant::mParticleID,
        "mShootingCounter", &Plant::mShootingCounter,
        "mBodyReanimID", &Plant::mBodyReanimID,
        "mHeadReanimID", &Plant::mHeadReanimID,
        "mHeadReanimID2", &Plant::mHeadReanimID2,
        "mHeadReanimID3", &Plant::mHeadReanimID3,
        "mBlinkReanimID", &Plant::mBlinkReanimID,
        "mLightReanimID", &Plant::mLightReanimID,
        "mSleepingReanimID", &Plant::mSleepingReanimID,
        "mBlinkCountdown", &Plant::mBlinkCountdown,
        "mRecentlyEatenCountdown", &Plant::mRecentlyEatenCountdown,
        "mEatenFlashCountdown", &Plant::mEatenFlashCountdown,
        "mBeghouledFlashCountdown", &Plant::mBeghouledFlashCountdown,
        "mShakeOffsetX", &Plant::mShakeOffsetX,
        "mShakeOffsetY", &Plant::mShakeOffsetY,
        // mMagnetItems: 跳过 (array member)
        "mTargetZombieID", &Plant::mTargetZombieID,
        "mWakeUpCounter", &Plant::mWakeUpCounter,
        "mOnBungeeState", &Plant::mOnBungeeState,
        "mImitaterType", &Plant::mImitaterType,
        "mPottedPlantIndex", &Plant::mPottedPlantIndex,
        "mAnimPing", &Plant::mAnimPing,
        "mDead", &Plant::mDead,
        "mSquished", &Plant::mSquished,
        "mIsAsleep", &Plant::mIsAsleep,
        "mIsOnBoard", &Plant::mIsOnBoard,
        "mHighlighted", &Plant::mHighlighted,
        "plant_initialize", &Plant::PlantInitialize,
        // init_plant_body_reanim: 跳过 (has reference param) — void                    InitPlantBodyReanim(const PlantDefinition& aPlantDef, Reanimation*& aBodyReanim);  // 创建 Body Reanimation 并基础设置
        // init_plant_peashooter_family: 跳过 (has reference param) — void                    InitPlantPeashooterFamily(const PlantDefinition& aPlantDef);   // 豌豆射手家族（含 repeater/leftpeater/gatlingpea）
        // init_plant_split_pea: 跳过 (has reference param) — void                    InitPlantSplitPea(const PlantDefinition& aPlantDef);           // 双发射手
        // init_plant_threepeater: 跳过 (has reference param) — void                    InitPlantThreepeater(const PlantDefinition& aPlantDef);        // 三线射手
        // init_plant_wallnut_family: 跳过 (has unregistered pointer param (Reanimation*)) — void                    InitPlantWallnutFamily(Reanimation* aBodyReanim);              // 坚果家族（坚果/爆炸坚果/巨型坚果/高坚果）
        // init_plant_cherry_jalapeno: 跳过 (has unregistered pointer param (Reanimation*)) — void                    InitPlantCherryJalapeno(Reanimation* aBodyReanim);             // 樱桃/辣椒
        // init_plant_potato_mine: 跳过 (has unregistered pointer param (Reanimation*)) — void                    InitPlantPotatoMine(Reanimation* aBodyReanim);                 // 土豆雷
        // init_plant_grave_buster: 跳过 (has unregistered pointer param (Reanimation*)) — void                    InitPlantGraveBuster(Reanimation* aBodyReanim);                // 墓碑吞噬者
        // init_plant_sun_shroom: 跳过 (has unregistered pointer param (Reanimation*)) — void                    InitPlantSunShroom(Reanimation* aBodyReanim);                  // 阳光菇
        // init_plant_pumpkin_shell: 跳过 (has unregistered pointer param (Reanimation*)) — void                    InitPlantPumpkinShell(Reanimation* aBodyReanim);               // 南瓜头
        // init_plant_cob_cannon: 跳过 (has unregistered pointer param (Reanimation*)) — void                    InitPlantCobCannon(Reanimation* aBodyReanim);                  // 玉米加农炮
        "init_plant_flower_pot_or_lily_pad", &Plant::InitPlantFlowerPotOrLilyPad,
        // init_plant_by_type: 跳过 (has reference param) — void                    InitPlantByType(SeedType theSeedType, const PlantDefinition& aPlantDef, Reanimation* aBodyReanim);  // switch 分发
        "update", &Plant::Update,
        "animate", &Plant::Animate,
        // draw: 跳过 (has unregistered pointer param (Graphics*)) — void                    Draw(Graphics* g);
        "mouse_down", &Plant::MouseDown,
        "do_special", &Plant::DoSpecial,
        "do_special_blover", &Plant::DoSpecialBlover,
        "do_special_cherry_bomb", &Plant::DoSpecialCherryBomb,
        "do_special_doom_shroom", &Plant::DoSpecialDoomShroom,
        "do_special_jalapeno", &Plant::DoSpecialJalapeno,
        "do_special_umbrella", &Plant::DoSpecialUmbrella,
        "do_special_ice_shroom", &Plant::DoSpecialIceShroom,
        "do_special_potato_mine", &Plant::DoSpecialPotatoMine,
        "do_special_instant_coffee", &Plant::DoSpecialInstantCoffee,
        // fire: 跳过 (has default param) — void                    Fire(Zombie* theTargetZombie, int theRow, PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
        "get_fire_projectile_type", &Plant::GetFireProjectileType,
        // get_fire_origin: 跳过 (has reference param) — void                    GetFireOrigin(PlantWeapon thePlantWeapon, int& aOriginX, int& aOriginY);  // 计算发射点坐标
        "play_fire_sound", &Plant::PlayFireSound,
        "play_fire_muzzle_particle", &Plant::PlayFireMuzzleParticle,
        "setup_projectile_motion", &Plant::SetupProjectileMotion,
        // find_target_zombie: 跳过 (has default param) — Zombie*                 FindTargetZombie(int theRow, PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
        "die", &Plant::Die,
        "update_production_plant", &Plant::UpdateProductionPlant,
        "update_shooter", &Plant::UpdateShooter,
        // find_target_and_fire: 跳过 (has default param) — bool                    FindTargetAndFire(int theRow, PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
        "launch_threepeater", &Plant::LaunchThreepeater,
        "update_abilities", &Plant::UpdateAbilities,
        "squish", &Plant::Squish,
        "do_row_area_damage", &Plant::DoRowAreaDamage,
        // get_damage_range_flags: 跳过 (has default param) — int                     GetDamageRangeFlags(PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
        // get_plant_rect: 跳过 (returns unbindable type (Rect)) — Rect                    GetPlantRect();
        // get_plant_attack_rect: 跳过 (has default param) — Rect                    GetPlantAttackRect(PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
        "find_squash_target", &Plant::FindSquashTarget,
        "update_squash", &Plant::UpdateSquash,
        "do_squash_damage", &Plant::DoSquashDamage,
        "burn_row", &Plant::BurnRow,
        "ice_zombies", &Plant::IceZombies,
        "blow_away_fliers", &Plant::BlowAwayFliers,
        "update_grave_buster", &Plant::UpdateGraveBuster,
        // add_attached_particle: 跳过 (returns unbindable type (TodParticle)) — TodParticleSystem*      AddAttachedParticle(int thePosX, int thePosY, int theRenderPosition, ParticleEffect theEffect);
        // get_pea_head_offset: 跳过 (has reference param) — void                    GetPeaHeadOffset(int& theOffsetX, int& theOffsetY);
        "kill_all_plants_near_doom", &Plant::KillAllPlantsNearDoom,
        "is_on_high_ground", &Plant::IsOnHighGround,
        "update_torchwood", &Plant::UpdateTorchwood,
        "launch_star_fruit", &Plant::LaunchStarFruit,
        "find_star_fruit_target", &Plant::FindStarFruitTarget,
        "update_chomper", &Plant::UpdateChomper,
        "do_blink", &Plant::DoBlink,
        "update_blink", &Plant::UpdateBlink,
        "play_body_reanim", &Plant::PlayBodyReanim,
        "update_magnet_shroom", &Plant::UpdateMagnetShroom,
        // get_free_magnet_item: 跳过 (returns unregistered pointer type (MagnetItem*)) — MagnetItem*             GetFreeMagnetItem();
        // draw_magnet_items: 跳过 (has unregistered pointer param (Graphics*)) — void                    DrawMagnetItems(Graphics* g);
        "update_doom_shroom", &Plant::UpdateDoomShroom,
        "update_ice_shroom", &Plant::UpdateIceShroom,
        "update_potato", &Plant::UpdatePotato,
        "calc_render_order", &Plant::CalcRenderOrder,
        "animate_nuts", &Plant::AnimateNuts,
        "set_sleeping", &Plant::SetSleeping,
        "update_shooting", &Plant::UpdateShooting,
        "update_fume_shroom_shooting", &Plant::UpdateFumeShroomShooting,
        "update_gloom_shroom_shooting", &Plant::UpdateGloomShroomShooting,
        "update_gatling_pea_shooting", &Plant::UpdateGatlingPeaShooting,
        "update_cattail_shooting", &Plant::UpdateCattailShooting,
        "fire_threepeater_at_counter_one", &Plant::FireThreepeaterAtCounterOne,
        "fire_split_pea_at_counter_one", &Plant::FireSplitPeaAtCounterOne,
        "fire_catapult_at_counter_one", &Plant::FireCatapultAtCounterOne,
        "fire_at_counter_one", &Plant::FireAtCounterOne,
        "reset_threepeater_shooting_anim", &Plant::ResetThreepeaterShootingAnim,
        "reset_split_pea_shooting_anim", &Plant::ResetSplitPeaShootingAnim,
        "reset_shooting_anim", &Plant::ResetShootingAnim,
        // draw_shadow: 跳过 (has unregistered pointer param (Graphics*)) — void                    DrawShadow(Graphics* g, float theOffsetX, float theOffsetY);
        "update_scaredy_shroom", &Plant::UpdateScaredyShroom,
        "distance_to_closest_zombie", &Plant::DistanceToClosestZombie,
        "update_spikeweed", &Plant::UpdateSpikeweed,
        "magnet_shroom_attact_item", &Plant::MagnetShroomAttactItem,
        "update_sun_shroom", &Plant::UpdateSunShroom,
        "update_bowling", &Plant::UpdateBowling,
        "animate_pumpkin", &Plant::AnimatePumpkin,
        "update_blover", &Plant::UpdateBlover,
        "update_cactus", &Plant::UpdateCactus,
        "star_fruit_fire", &Plant::StarFruitFire,
        "update_tanglekelp", &Plant::UpdateTanglekelp,
        // attach_blink_anim: 跳过 (returns unregistered pointer type (Reanimation*)) — Reanimation*            AttachBlinkAnim(Reanimation* theReanimBody);
        "update_reanim_color", &Plant::UpdateReanimColor,
        "is_upgradable_to", &Plant::IsUpgradableTo,
        "is_part_of_upgradable_to", &Plant::IsPartOfUpgradableTo,
        "update_cob_cannon", &Plant::UpdateCobCannon,
        "cob_cannon_fire", &Plant::CobCannonFire,
        "update_gold_magnet_shroom", &Plant::UpdateGoldMagnetShroom,
        "remove_effects", &Plant::RemoveEffects,
        "update_coffee_bean", &Plant::UpdateCoffeeBean,
        "update_umbrella", &Plant::UpdateUmbrella,
        "end_blink", &Plant::EndBlink,
        "animate_garlic", &Plant::AnimateGarlic,
        "find_gold_magnet_target", &Plant::FindGoldMagnetTarget,
        "spikeweed_attack", &Plant::SpikeweedAttack,
        "imitater_morph", &Plant::ImitaterMorph,
        "update_imitater", &Plant::UpdateImitater,
        "update_reanim", &Plant::UpdateReanim,
        "spike_rock_take_damage", &Plant::SpikeRockTakeDamage,
        "is_spiky", &Plant::IsSpiky,
        "play_idle_anim", &Plant::PlayIdleAnim,
        "update_flower_pot", &Plant::UpdateFlowerPot,
        "update_lilypad", &Plant::UpdateLilypad,
        "gold_magnet_find_targets", &Plant::GoldMagnetFindTargets,
        "is_a_gold_magnet_about_to_suck", &Plant::IsAGoldMagnetAboutToSuck,
        "draw_magnet_items_on_top", &Plant::DrawMagnetItemsOnTop
    );
}

// ===== 自动生成的 Board sol2 绑定 =====
// 由 tools/gen_sol2_bindings.py 生成，请勿手动编辑
static void BindSol2Board(sol::state_view& lua) {
    auto ut = lua.new_usertype<Board>("PvZ.Board",
        sol::no_constructor,
        // mApp: 跳过 (pointer to unregistered type (LawnApp*))
        // mZombies: 跳过 (complex type (DataArray<Zombie>))
        // mPlants: 跳过 (complex type (DataArray<Plant>))
        // mProjectiles: 跳过 (complex type (DataArray<Projectile>))
        // mCoins: 跳过 (complex type (DataArray<Coin>))
        // mLawnMowers: 跳过 (complex type (DataArray<LawnMower>))
        // mGridItems: 跳过 (complex type (DataArray<GridItem>))
        // mCursorObject: 跳过 (pointer to unregistered type (CursorObject*))
        // mCursorPreview: 跳过 (pointer to unregistered type (CursorPreview*))
        // mAdvice: 跳过 (pointer to unregistered type (MessageWidget*))
        // mSeedBank: 跳过 (pointer to unregistered type (SeedBank*))
        // mMenuButton: 跳过 (pointer to unregistered type (GameButton*))
        // mStoreButton: 跳过 (pointer to unregistered type (GameButton*))
        "mIgnoreMouseUp", &Board::mIgnoreMouseUp,
        // mToolTip: 跳过 (pointer to unregistered type (ToolTipWidget*))
        // mCutScene: 跳过 (pointer to unregistered type (CutScene*))
        // mChallenge: 跳过 (pointer to unregistered type (Challenge*))
        "mPaused", &Board::mPaused,
        // mGridSquareType: 跳过 (array member)
        // mGridCelLook: 跳过 (array member)
        // mGridCelOffset: 跳过 (array member)
        // mGridCelFog: 跳过 (array member)
        "mEnableGraveStones", &Board::mEnableGraveStones,
        "mSpecialGraveStoneX", &Board::mSpecialGraveStoneX,
        "mSpecialGraveStoneY", &Board::mSpecialGraveStoneY,
        "mFogOffset", &Board::mFogOffset,
        "mFogBlownCountDown", &Board::mFogBlownCountDown,
        // mPlantRow: 跳过 (array member)
        // mWaveRowGotLawnMowered: 跳过 (array member)
        "mBonusLawnMowersRemaining", &Board::mBonusLawnMowersRemaining,
        // mIceMinX: 跳过 (array member)
        // mIceTimer: 跳过 (array member)
        // mIceParticleID: 跳过 (array member)
        // mRowPickingArray: 跳过 (array member)
        // mZombiesInWave: 跳过 (array member)
        // mZombieAllowed: 跳过 (array member)
        "mSunCountDown", &Board::mSunCountDown,
        "mNumSunsFallen", &Board::mNumSunsFallen,
        "mShakeCounter", &Board::mShakeCounter,
        "mShakeAmountX", &Board::mShakeAmountX,
        "mShakeAmountY", &Board::mShakeAmountY,
        "mBackground", &Board::mBackground,
        "mLevel", &Board::mLevel,
        "mSodPosition", &Board::mSodPosition,
        "mPrevMouseX", &Board::mPrevMouseX,
        "mPrevMouseY", &Board::mPrevMouseY,
        "mSunMoney", &Board::mSunMoney,
        "mNumWaves", &Board::mNumWaves,
        "mMainCounter", &Board::mMainCounter,
        "mEffectCounter", &Board::mEffectCounter,
        "mDrawCount", &Board::mDrawCount,
        "mRiseFromGraveCounter", &Board::mRiseFromGraveCounter,
        "mOutOfMoneyCounter", &Board::mOutOfMoneyCounter,
        "mCurrentWave", &Board::mCurrentWave,
        "mTotalSpawnedWaves", &Board::mTotalSpawnedWaves,
        "mTutorialState", &Board::mTutorialState,
        "mTutorialParticleID", &Board::mTutorialParticleID,
        "mTutorialTimer", &Board::mTutorialTimer,
        "mLastBungeeWave", &Board::mLastBungeeWave,
        "mZombieHealthToNextWave", &Board::mZombieHealthToNextWave,
        "mZombieHealthWaveStart", &Board::mZombieHealthWaveStart,
        "mZombieCountDown", &Board::mZombieCountDown,
        "mZombieCountDownStart", &Board::mZombieCountDownStart,
        "mHugeWaveCountDown", &Board::mHugeWaveCountDown,
        // mHelpDisplayed: 跳过 (array member)
        "mHelpIndex", &Board::mHelpIndex,
        "mFinalBossKilled", &Board::mFinalBossKilled,
        "mShowShovel", &Board::mShowShovel,
        "mCoinBankFadeCount", &Board::mCoinBankFadeCount,
        "mDebugTextMode", &Board::mDebugTextMode,
        "mLevelComplete", &Board::mLevelComplete,
        "mBoardFadeOutCounter", &Board::mBoardFadeOutCounter,
        "mNextSurvivalStageCounter", &Board::mNextSurvivalStageCounter,
        "mScoreNextMowerCounter", &Board::mScoreNextMowerCounter,
        "mLevelAwardSpawned", &Board::mLevelAwardSpawned,
        "mProgressMeterWidth", &Board::mProgressMeterWidth,
        "mFlagRaiseCounter", &Board::mFlagRaiseCounter,
        "mIceTrapCounter", &Board::mIceTrapCounter,
        "mBoardRandSeed", &Board::mBoardRandSeed,
        "mPoolSparklyParticleID", &Board::mPoolSparklyParticleID,
        // mFwooshID: 跳过 (array member)
        "mFwooshCountDown", &Board::mFwooshCountDown,
        "mTimeStopCounter", &Board::mTimeStopCounter,
        "mDroppedFirstCoin", &Board::mDroppedFirstCoin,
        "mFinalWaveSoundCounter", &Board::mFinalWaveSoundCounter,
        "mCobCannonCursorDelayCounter", &Board::mCobCannonCursorDelayCounter,
        "mCobCannonMouseX", &Board::mCobCannonMouseX,
        "mCobCannonMouseY", &Board::mCobCannonMouseY,
        "mKilledYeti", &Board::mKilledYeti,
        "mMustacheMode", &Board::mMustacheMode,
        "mSuperMowerMode", &Board::mSuperMowerMode,
        "mFutureMode", &Board::mFutureMode,
        "mPinataMode", &Board::mPinataMode,
        "mDanceMode", &Board::mDanceMode,
        "mDaisyMode", &Board::mDaisyMode,
        "mSukhbirMode", &Board::mSukhbirMode,
        "mPrevBoardResult", &Board::mPrevBoardResult,
        "mTriggeredLawnMowers", &Board::mTriggeredLawnMowers,
        "mPlayTimeActiveLevel", &Board::mPlayTimeActiveLevel,
        "mPlayTimeInactiveLevel", &Board::mPlayTimeInactiveLevel,
        "mMaxSunPlants", &Board::mMaxSunPlants,
        "mStartDrawTime", &Board::mStartDrawTime,
        "mIntervalDrawTime", &Board::mIntervalDrawTime,
        "mIntervalDrawCountStart", &Board::mIntervalDrawCountStart,
        "mMinFPS", &Board::mMinFPS,
        "mPreloadTime", &Board::mPreloadTime,
        "mGameID", &Board::mGameID,
        "mGravesCleared", &Board::mGravesCleared,
        "mPlantsEaten", &Board::mPlantsEaten,
        "mPlantsShoveled", &Board::mPlantsShoveled,
        "mPeaShooterUsed", &Board::mPeaShooterUsed,
        "mCatapultPlantsUsed", &Board::mCatapultPlantsUsed,
        "mMushroomAndCoffeeBeansOnly", &Board::mMushroomAndCoffeeBeansOnly,
        "mMushroomsUsed", &Board::mMushroomsUsed,
        "mLevelCoinsCollected", &Board::mLevelCoinsCollected,
        "mGargantuarsKillsByCornCob", &Board::mGargantuarsKillsByCornCob,
        "mCoinsCollected", &Board::mCoinsCollected,
        "mDiamondsCollected", &Board::mDiamondsCollected,
        "mPottedPlantsCollected", &Board::mPottedPlantsCollected,
        "mChocolateCollected", &Board::mChocolateCollected,
        "dispose_board", &Board::DisposeBoard,
        "count_sun_being_collected", &Board::CountSunBeingCollected,
        // draw_game_objects: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawGameObjects(Graphics* g);
        "clear_cursor", &Board::ClearCursor,
        "find_lawn_mower_in_row", &Board::FindLawnMowerInRow,
        // load_game: 跳过 (has reference param) — bool							LoadGame(const std::string& theFileName);
        "init_level", &Board::InitLevel,
        "display_advice", &Board::DisplayAdvice,
        "start_level", &Board::StartLevel,
        // add_plant: 跳过 (has default param) — Plant*							AddPlant(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType = SeedType::SEED_NONE);
        "add_projectile", &Board::AddProjectile,
        "add_coin", &Board::AddCoin,
        "refresh_seed_packet_from_cursor", &Board::RefreshSeedPacketFromCursor,
        "pick_grave_rising_zombie_type", &Board::PickGraveRisingZombieType,
        // pick_zombie_type: 跳过 (has unregistered pointer param (ZombiePicker*)) — ZombieType						PickZombieType(int theZombiePoints, int theWaveIndex, ZombiePicker* theZombiePicker);
        "pick_row_for_new_zombie", &Board::PickRowForNewZombie,
        "spawn_zombie_wave", &Board::SpawnZombieWave,
        "remove_all_zombies", &Board::RemoveAllZombies,
        "remove_cutscene_zombies", &Board::RemoveCutsceneZombies,
        "spawn_zombies_from_graves", &Board::SpawnZombiesFromGraves,
        "can_plant_at", &Board::CanPlantAt,
        "update_layers", &Board::UpdateLayers,
        // draw_backdrop: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawBackdrop(Graphics* g);
        "take_sun_money", &Board::TakeSunMoney,
        "try_to_save_game", &Board::TryToSaveGame,
        "process_delete_queue", &Board::ProcessDeleteQueue,
        "choose_seeds_on_current_level", &Board::ChooseSeedsOnCurrentLevel,
        "get_num_seeds_in_bank", &Board::GetNumSeedsInBank,
        "stage_has_grave_stones", &Board::StageHasGraveStones,
        "pixel_to_grid_x", &Board::PixelToGridX,
        "pixel_to_grid_y", &Board::PixelToGridY,
        "grid_to_pixel_y", &Board::GridToPixelY,
        "update_game_objects", &Board::UpdateGameObjects,
        // mouse_hit_test: 跳过 (has unregistered pointer param (HitResult*)) — bool							MouseHitTest(int x, int y, HitResult* theHitResult);
        "mouse_down_with_plant", &Board::MouseDownWithPlant,
        "mouse_down_with_tool", &Board::MouseDownWithTool,
        "can_interact_with_board_buttons", &Board::CanInteractWithBoardButtons,
        // draw_progress_meter: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawProgressMeter(Graphics* g);
        "update_tool_tip", &Board::UpdateToolTip,
        "get_top_plant_at", &Board::GetTopPlantAt,
        // get_plants_on_lawn: 跳过 (has unregistered pointer param (PlantsOnLawn*)) — void							GetPlantsOnLawn(int theGridX, int theGridY, PlantsOnLawn* thePlantOnLawn);
        "get_seed_packet_position_x", &Board::GetSeedPacketPositionX,
        // add_grave_stones: 跳过 (has reference param) — void							AddGraveStones(int theGridX, int theCount, MTRand& theLevelRNG);
        "get_grave_stone_count", &Board::GetGraveStoneCount,
        // zombies_won: 跳过 (has default param) — void							ZombiesWon(Zombie* theZombie = nullptr);
        // draw_level: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawLevel(Graphics* g);
        // draw_shovel: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawShovel(Graphics* g);
        "update_zombie_spawning", &Board::UpdateZombieSpawning,
        "update_sun_spawning", &Board::UpdateSunSpawning,
        "row_can_have_zombie_type", &Board::RowCanHaveZombieType,
        "total_zombies_health_in_wave", &Board::TotalZombiesHealthInWave,
        // draw_debug_text: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawDebugText(Graphics* g);
        // draw_ui_coin_bank: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawUICoinBank(Graphics* g);
        "fade_out_level", &Board::FadeOutLevel,
        // draw_fade_out: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawFadeOut(Graphics* g);
        // draw_ice: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawIce(Graphics* g, int theGridY);
        "is_ice_at", &Board::IsIceAt,
        // draw_debug_object_rects: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawDebugObjectRects(Graphics* g);
        "update_ice", &Board::UpdateIce,
        "count_untrigger_lawn_mowers", &Board::CountUntriggerLawnMowers,
        // iterate_zombies: 跳过 (has reference param) — bool							IterateZombies(Zombie*& theZombie);
        // iterate_plants: 跳过 (has reference param) — bool							IteratePlants(Plant*& thePlant);
        // iterate_projectiles: 跳过 (has reference param) — bool							IterateProjectiles(Projectile*& theProjectile);
        // iterate_coins: 跳过 (has reference param) — bool							IterateCoins(Coin*& theCoin);
        // iterate_lawn_mowers: 跳过 (has reference param) — bool							IterateLawnMowers(LawnMower*& theLawnMower);
        // iterate_particles: 跳过 (has reference param) — bool							IterateParticles(TodParticleSystem*& theParticle);
        // iterate_reanimations: 跳过 (has reference param) — bool							IterateReanimations(Reanimation*& theReanimation);
        // iterate_grid_items: 跳过 (has reference param) — bool							IterateGridItems(GridItem*& theGridItem);
        "pick_zombie_waves", &Board::PickZombieWaves,
        "stop_all_zombie_sounds", &Board::StopAllZombieSounds,
        "update_progress_meter", &Board::UpdateProgressMeter,
        // draw_ui_bottom: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawUIBottom(Graphics* g);
        // draw_ui_top: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawUITop(Graphics* g);
        "zombie_hit_test", &Board::ZombieHitTest,
        "kill_all_plants_in_radius", &Board::KillAllPlantsInRadius,
        "get_pumpkin_at", &Board::GetPumpkinAt,
        "get_flower_pot_at", &Board::GetFlowerPotAt,
        "is_zombie_wave_distribution_ok", &Board::IsZombieWaveDistributionOk,
        "pick_background", &Board::PickBackground,
        "init_zombie_waves", &Board::InitZombieWaves,
        "init_survival_stage", &Board::InitSurvivalStage,
        "update_game", &Board::UpdateGame,
        "init_zombie_waves_for_level", &Board::InitZombieWavesForLevel,
        "seed_not_recommended_for_level", &Board::SeedNotRecommendedForLevel,
        // draw_top_right_ui: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawTopRightUI(Graphics* g);
        // draw_fog: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawFog(Graphics* g);
        "update_fog", &Board::UpdateFog,
        "drop_loot_piece", &Board::DropLootPiece,
        "update_level_end_sequence", &Board::UpdateLevelEndSequence,
        "get_bottom_lawn_mower", &Board::GetBottomLawnMower,
        "can_drop_loot", &Board::CanDropLoot,
        "get_introduced_zombie_type", &Board::GetIntroducedZombieType,
        "pick_special_grave_stone", &Board::PickSpecialGraveStone,
        "get_pos_y_based_on_row", &Board::GetPosYBasedOnRow,
        "next_wave_coming", &Board::NextWaveComing,
        "bungee_is_targeting_cell", &Board::BungeeIsTargetingCell,
        "find_umbrella_plant", &Board::FindUmbrellaPlant,
        "set_tutorial_state", &Board::SetTutorialState,
        "do_fwoosh", &Board::DoFwoosh,
        "update_fwoosh", &Board::UpdateFwoosh,
        "special_plant_hit_test", &Board::SpecialPlantHitTest,
        "update_mouse_position", &Board::UpdateMousePosition,
        "can_add_grave_stone_at", &Board::CanAddGraveStoneAt,
        "update_grid_items", &Board::UpdateGridItems,
        "add_grid_item", &Board::AddGridItem,
        "add_lawn_mower", &Board::AddLawnMower,
        "get_survival_flags_completed", &Board::GetSurvivalFlagsCompleted,
        "has_progress_meter", &Board::HasProgressMeter,
        "update_cursor", &Board::UpdateCursor,
        "update_tutorial", &Board::UpdateTutorial,
        "get_seed_type_in_cursor", &Board::GetSeedTypeInCursor,
        "planting_requirements_met", &Board::PlantingRequirementsMet,
        "has_valid_cob_cannon_spot", &Board::HasValidCobCannonSpot,
        "is_valid_cob_cannon_spot", &Board::IsValidCobCannonSpot,
        "is_valid_cob_cannon_spot_helper", &Board::IsValidCobCannonSpotHelper,
        "mouse_down_cobcannon_fire", &Board::MouseDownCobcannonFire,
        "kill_all_zombies_in_radius", &Board::KillAllZombiesInRadius,
        "is_flag_wave", &Board::IsFlagWave,
        // draw_house_door_top: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawHouseDoorTop(Graphics* g);
        // draw_house_door_bottom: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawHouseDoorBottom(Graphics* g);
        "get_boss_zombie", &Board::GetBossZombie,
        "has_conveyor_belt_seed_bank", &Board::HasConveyorBeltSeedBank,
        "spawn_zombies_from_pool", &Board::SpawnZombiesFromPool,
        "spawn_zombies_from_sky", &Board::SpawnZombiesFromSky,
        "pick_up_tool", &Board::PickUpTool,
        "tutorial_arrow_show", &Board::TutorialArrowShow,
        "tutorial_arrow_remove", &Board::TutorialArrowRemove,
        "count_coins_being_collected", &Board::CountCoinsBeingCollected,
        // bungee_drop_zombie: 跳过 (has unregistered pointer param (BungeeDropGrid*)) — void							BungeeDropZombie(BungeeDropGrid* theBungeeDropGrid, ZombieType theZombieType);
        // setup_bungee_drop: 跳过 (has unregistered pointer param (BungeeDropGrid*)) — void							SetupBungeeDrop(BungeeDropGrid* theBungeeDropGrid);
        // get_shovel_button_rect: 跳过 (returns unbindable type (Rect)) — Rect							GetShovelButtonRect();
        // get_zen_button_rect: 跳过 (has reference param) — void							GetZenButtonRect(GameObjectType theObjectType, Rect& theRect);
        // new_plant: 跳过 (has default param) — Plant*							NewPlant(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType = SeedType::SEED_NONE);
        "do_planting_effects", &Board::DoPlantingEffects,
        "is_final_survival_stage", &Board::IsFinalSurvivalStage,
        "survival_save_score", &Board::SurvivalSaveScore,
        "count_zombies_on_screen", &Board::CountZombiesOnScreen,
        "get_live_gargantuar_count", &Board::GetLiveGargantuarCount,
        "get_level_rand_seed", &Board::GetLevelRandSeed,
        // add_boss_render_item: 跳过 (has reference param) — void							AddBossRenderItem(RenderItem* theRenderList, int& theCurRenderItem, Zombie* theBossZombie);
        "init_lawn_mowers", &Board::InitLawnMowers,
        "highlight_plants_for_mouse", &Board::HighlightPlantsForMouse,
        "clear_fog_around_plant", &Board::ClearFogAroundPlant,
        "puzzle_save_streak", &Board::PuzzleSaveStreak,
        "get_squirrel_at", &Board::GetSquirrelAt,
        "get_zen_tool_at", &Board::GetZenToolAt,
        "is_plant_in_gold_watering_can_range", &Board::IsPlantInGoldWateringCanRange,
        "stage_has_zombie_walk_in_from_right", &Board::StageHasZombieWalkInFromRight,
        "place_rake", &Board::PlaceRake,
        "get_rake", &Board::GetRake,
        "count_empty_pots_or_lilies", &Board::CountEmptyPotsOrLilies,
        "get_grid_item_at", &Board::GetGridItemAt,
        "progress_meter_has_flags", &Board::ProgressMeterHasFlags,
        "get_current_plant_cost", &Board::GetCurrentPlantCost,
        "freeze_effects_for_cutscene", &Board::FreezeEffectsForCutscene,
        "load_background_images", &Board::LoadBackgroundImages,
        "can_use_game_object", &Board::CanUseGameObject,
        "set_mustache_mode", &Board::SetMustacheMode,
        "count_coin_by_type", &Board::CountCoinByType,
        "set_super_mower_mode", &Board::SetSuperMowerMode,
        // draw_zen_wheel_barrow_button: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawZenWheelBarrowButton(Graphics* g, int theOffsetY);
        // draw_zen_buttons: 跳过 (has unregistered pointer param (Graphics*)) — void							DrawZenButtons(Graphics* g);
        "set_dance_mode", &Board::SetDanceMode,
        "set_future_mode", &Board::SetFutureMode,
        "set_pinata_mode", &Board::SetPinataMode,
        "set_daisy_mode", &Board::SetDaisyMode,
        "set_sukhbir_mode", &Board::SetSukhbirMode,
        // mouse_hit_test_plant: 跳过 (has unregistered pointer param (HitResult*)) — bool							MouseHitTestPlant(int x, int y, HitResult* theHitResult);
        "complete_end_level_sequence_for_saving", &Board::CompleteEndLevelSequenceForSaving,
        "remove_zombies_for_repick", &Board::RemoveZombiesForRepick,
        "get_grave_stones_count", &Board::GetGraveStonesCount,
        "do_typing_check", &Board::DoTypingCheck,
        "count_zombie_by_type", &Board::CountZombieByType
    );
}

// ===== 自动生成的 Projectile sol2 绑定 =====
// 由 tools/gen_sol2_bindings.py 生成，请勿手动编辑
static void BindSol2Projectile(sol::state_view& lua) {
    auto ut = lua.new_usertype<Projectile>("PvZ.Projectile",
        sol::no_constructor,
        "mFrame", &Projectile::mFrame,
        "mNumFrames", &Projectile::mNumFrames,
        "mAnimCounter", &Projectile::mAnimCounter,
        "mPosX", &Projectile::mPosX,
        "mPosY", &Projectile::mPosY,
        "mPosZ", &Projectile::mPosZ,
        "mVelX", &Projectile::mVelX,
        "mVelY", &Projectile::mVelY,
        "mVelZ", &Projectile::mVelZ,
        "mAccZ", &Projectile::mAccZ,
        "mShadowY", &Projectile::mShadowY,
        "mDead", &Projectile::mDead,
        "mAnimTicksPerFrame", &Projectile::mAnimTicksPerFrame,
        "mMotionType", &Projectile::mMotionType,
        "mProjectileType", &Projectile::mProjectileType,
        "mProjectileAge", &Projectile::mProjectileAge,
        "mClickBackoffCounter", &Projectile::mClickBackoffCounter,
        "mRotation", &Projectile::mRotation,
        "mRotationSpeed", &Projectile::mRotationSpeed,
        "mOnHighGround", &Projectile::mOnHighGround,
        "mDamageRangeFlags", &Projectile::mDamageRangeFlags,
        "mHitTorchwoodGridX", &Projectile::mHitTorchwoodGridX,
        "mAttachmentID", &Projectile::mAttachmentID,
        "mCobTargetX", &Projectile::mCobTargetX,
        "mCobTargetRow", &Projectile::mCobTargetRow,
        "mTargetZombieID", &Projectile::mTargetZombieID,
        "mLastPortalX", &Projectile::mLastPortalX,
        "projectile_initialize", &Projectile::ProjectileInitialize,
        "update", &Projectile::Update,
        // draw: 跳过 (has unregistered pointer param (Graphics*)) — void                    Draw(Graphics* g);
        // draw_shadow: 跳过 (has unregistered pointer param (Graphics*)) — void                    DrawShadow(Graphics* g);
        "die", &Projectile::Die,
        "do_impact", &Projectile::DoImpact,
        "update_motion", &Projectile::UpdateMotion,
        "check_for_collision", &Projectile::CheckForCollision,
        "find_collision_target", &Projectile::FindCollisionTarget,
        "update_lob_motion", &Projectile::UpdateLobMotion,
        "check_for_high_ground", &Projectile::CheckForHighGround,
        "cant_hit_high_ground", &Projectile::CantHitHighGround,
        "do_splash_damage", &Projectile::DoSplashDamage,
        // get_projectile_def: 跳过 (returns reference) — const ProjectileDefinition&   GetProjectileDef();
        // get_damage_flags: 跳过 (has default param) — unsigned int            GetDamageFlags(Zombie* theZombie/* = nullptr*/);
        // get_projectile_rect: 跳过 (returns unbindable type (Rect)) — Rect                    GetProjectileRect();
        "update_normal_motion", &Projectile::UpdateNormalMotion,
        "find_collision_target_plant", &Projectile::FindCollisionTargetPlant,
        "convert_to_fireball", &Projectile::ConvertToFireball,
        "convert_to_pea", &Projectile::ConvertToPea,
        // is_splash_damage: 跳过 (has default param) — bool                    IsSplashDamage(Zombie* theZombie/* = nullptr*/);
        "play_impact_sound", &Projectile::PlayImpactSound,
        "is_zombie_hit_by_splash", &Projectile::IsZombieHitBySplash,
        "pea_about_to_hit_torchwood", &Projectile::PeaAboutToHitTorchwood
    );
}

// ===== 自动生成的 Coin sol2 绑定 =====
// 由 tools/gen_sol2_bindings.py 生成，请勿手动编辑
static void BindSol2Coin(sol::state_view& lua) {
    auto ut = lua.new_usertype<Coin>("PvZ.Coin",
        sol::no_constructor,
        "mPosX", &Coin::mPosX,
        "mPosY", &Coin::mPosY,
        "mVelX", &Coin::mVelX,
        "mVelY", &Coin::mVelY,
        "mScale", &Coin::mScale,
        "mDead", &Coin::mDead,
        "mFadeCount", &Coin::mFadeCount,
        "mCollectX", &Coin::mCollectX,
        "mCollectY", &Coin::mCollectY,
        "mGroundY", &Coin::mGroundY,
        "mCoinAge", &Coin::mCoinAge,
        "mIsBeingCollected", &Coin::mIsBeingCollected,
        "mDisappearCounter", &Coin::mDisappearCounter,
        "mType", &Coin::mType,
        "mCoinMotion", &Coin::mCoinMotion,
        "mAttachmentID", &Coin::mAttachmentID,
        "mCollectionDistance", &Coin::mCollectionDistance,
        "mUsableSeedType", &Coin::mUsableSeedType,
        "mPottedPlantSpec", &Coin::mPottedPlantSpec,
        "mNeedsBouncyArrow", &Coin::mNeedsBouncyArrow,
        "mHasBouncyArrow", &Coin::mHasBouncyArrow,
        "mHitGround", &Coin::mHitGround,
        "mTimesDropped", &Coin::mTimesDropped,
        "coin_initialize", &Coin::CoinInitialize,
        "mouse_down", &Coin::MouseDown,
        // mouse_hit_test: 跳过 (has unregistered pointer param (HitResult*)) — bool                    MouseHitTest(int theX, int theY, HitResult* theHitResult);
        "die", &Coin::Die,
        "start_fade", &Coin::StartFade,
        "update", &Coin::Update,
        // draw: 跳过 (has unregistered pointer param (Graphics*)) — void                    Draw(Graphics* g);
        "collect", &Coin::Collect,
        "update_fade", &Coin::UpdateFade,
        "update_fall", &Coin::UpdateFall,
        "score_coin", &Coin::ScoreCoin,
        "update_collected", &Coin::UpdateCollected,
        // get_color: 跳过 (returns unbindable type (Color)) — Color                   GetColor();
        "get_sun_scale", &Coin::GetSunScale,
        "get_final_seed_packet_type", &Coin::GetFinalSeedPacketType,
        "is_level_award", &Coin::IsLevelAward,
        "coin_gets_bouncy_arrow", &Coin::CoinGetsBouncyArrow,
        "fan_out_coins", &Coin::FanOutCoins,
        "get_disappear_time", &Coin::GetDisappearTime,
        "dropped_usable_seed", &Coin::DroppedUsableSeed,
        "play_collect_sound", &Coin::PlayCollectSound,
        "try_auto_collect_after_level_award", &Coin::TryAutoCollectAfterLevelAward,
        "is_present_with_advice", &Coin::IsPresentWithAdvice,
        "play_launch_sound", &Coin::PlayLaunchSound,
        "play_ground_sound", &Coin::PlayGroundSound
    );
}

// ===== 自动生成的 GridItem sol2 绑定 =====
// 由 tools/gen_sol2_bindings.py 生成，请勿手动编辑
static void BindSol2GridItem(sol::state_view& lua) {
    auto ut = lua.new_usertype<GridItem>("PvZ.GridItem",
        sol::no_constructor,
        // mApp: 跳过 (pointer to unregistered type (LawnApp*))
        "mBoard", &GridItem::mBoard,
        "mGridItemType", &GridItem::mGridItemType,
        "mGridItemState", &GridItem::mGridItemState,
        "mGridX", &GridItem::mGridX,
        "mGridY", &GridItem::mGridY,
        "mGridItemCounter", &GridItem::mGridItemCounter,
        "mRenderOrder", &GridItem::mRenderOrder,
        "mDead", &GridItem::mDead,
        "mPosX", &GridItem::mPosX,
        "mPosY", &GridItem::mPosY,
        "mGoalX", &GridItem::mGoalX,
        "mGoalY", &GridItem::mGoalY,
        "mGridItemReanimID", &GridItem::mGridItemReanimID,
        "mGridItemParticleID", &GridItem::mGridItemParticleID,
        "mZombieType", &GridItem::mZombieType,
        "mSeedType", &GridItem::mSeedType,
        "mScaryPotType", &GridItem::mScaryPotType,
        "mHighlighted", &GridItem::mHighlighted,
        "mTransparentCounter", &GridItem::mTransparentCounter,
        "mSunCount", &GridItem::mSunCount,
        // mMotionTrailFrames: 跳过 (array member)
        "mMotionTrailCount", &GridItem::mMotionTrailCount,
        // draw_ladder: 跳过 (has unregistered pointer param (Sexy::Graphics*)) — void					DrawLadder(Sexy::Graphics* g);
        // draw_crater: 跳过 (has unregistered pointer param (Sexy::Graphics*)) — void					DrawCrater(Sexy::Graphics* g);
        // draw_grave_stone: 跳过 (has unregistered pointer param (Sexy::Graphics*)) — void					DrawGraveStone(Sexy::Graphics* g);
        "grid_item_die", &GridItem::GridItemDie,
        "add_grave_stone_particles", &GridItem::AddGraveStoneParticles,
        // draw_grid_item: 跳过 (has unregistered pointer param (Sexy::Graphics*)) — void					DrawGridItem(Sexy::Graphics* g);
        // draw_grid_item_overlay: 跳过 (has unregistered pointer param (Sexy::Graphics*)) — void					DrawGridItemOverlay(Sexy::Graphics* g);
        "open_portal", &GridItem::OpenPortal,
        "update", &GridItem::Update,
        "close_portal", &GridItem::ClosePortal,
        // draw_scary_pot: 跳过 (has unregistered pointer param (Sexy::Graphics*)) — void					DrawScaryPot(Sexy::Graphics* g);
        "update_scary_pot", &GridItem::UpdateScaryPot,
        "update_portal", &GridItem::UpdatePortal,
        // draw_squirrel: 跳过 (has unregistered pointer param (Sexy::Graphics*)) — void					DrawSquirrel(Sexy::Graphics* g);
        "update_rake", &GridItem::UpdateRake,
        "rake_find_zombie", &GridItem::RakeFindZombie,
        // draw_i_zombie_brain: 跳过 (has unregistered pointer param (Sexy::Graphics*)) — void					DrawIZombieBrain(Sexy::Graphics* g);
        "update_brain", &GridItem::UpdateBrain
        // draw_stinky: 跳过 (has unregistered pointer param (Sexy::Graphics*)) — void					DrawStinky(Sexy::Graphics* g);
    );
}

// ===== 自动生成的 LawnMower sol2 绑定 =====
// 由 tools/gen_sol2_bindings.py 生成，请勿手动编辑
static void BindSol2LawnMower(sol::state_view& lua) {
    auto ut = lua.new_usertype<LawnMower>("PvZ.LawnMower",
        sol::no_constructor,
        // mApp: 跳过 (pointer to unregistered type (LawnApp*))
        "mBoard", &LawnMower::mBoard,
        "mPosX", &LawnMower::mPosX,
        "mPosY", &LawnMower::mPosY,
        "mRenderOrder", &LawnMower::mRenderOrder,
        "mRow", &LawnMower::mRow,
        "mAnimTicksPerFrame", &LawnMower::mAnimTicksPerFrame,
        "mReanimID", &LawnMower::mReanimID,
        "mChompCounter", &LawnMower::mChompCounter,
        "mRollingInCounter", &LawnMower::mRollingInCounter,
        "mSquishedCounter", &LawnMower::mSquishedCounter,
        "mMowerState", &LawnMower::mMowerState,
        "mDead", &LawnMower::mDead,
        "mVisible", &LawnMower::mVisible,
        "mMowerType", &LawnMower::mMowerType,
        "mAltitude", &LawnMower::mAltitude,
        "mMowerHeight", &LawnMower::mMowerHeight,
        "mLastPortalX", &LawnMower::mLastPortalX,
        "lawn_mower_initialize", &LawnMower::LawnMowerInitialize,
        "start_mower", &LawnMower::StartMower,
        "update", &LawnMower::Update,
        // draw: 跳过 (has unregistered pointer param (Graphics*)) — void                Draw(Graphics* g);
        "die", &LawnMower::Die,
        // get_lawn_mower_attack_rect: 跳过 (returns unbindable type (Rect)) — Rect                GetLawnMowerAttackRect();
        "update_pool", &LawnMower::UpdatePool,
        "mow_zombie", &LawnMower::MowZombie,
        "squish_mower", &LawnMower::SquishMower
    );
}

// ===== 绑定注册入口 =====
static void BindAllSol2(sol::state_view& lua) {
    BindSol2Zombie(lua);
    BindSol2Plant(lua);
    BindSol2Board(lua);
    BindSol2Projectile(lua);
    BindSol2Coin(lua);
    BindSol2GridItem(lua);
    BindSol2LawnMower(lua);
}
