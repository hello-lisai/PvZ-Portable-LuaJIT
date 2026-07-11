/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * This file is part of PvZ-Portable.
 *
 * PvZ-Portable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PvZ-Portable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PvZ-Portable. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __ZOMBIE_H__
#define __ZOMBIE_H__

#include <cstdint>
#include <string>
#include <vector>
#include "GameObject.h"
#include "../GameConstants.h"

#define MAX_ZOMBIE_FOLLOWERS 4
#define NUM_BOBSLED_FOLLOWERS 3
#define NUM_BACKUP_DANCERS 4
#define NUM_BOSS_BUNGEES 3

constexpr const int ZOMBIE_START_RANDOM_OFFSET = 40;
constexpr const int BUNGEE_ZOMBIE_HEIGHT = 3000;
constexpr const int RENDER_GROUP_SHIELD = 1;
constexpr const int RENDER_GROUP_ARMS = 2;
constexpr const int RENDER_GROUP_OVER_SHIELD = 3;
constexpr const int RENDER_GROUP_BOSS_BACK_LEG = 4;
constexpr const int RENDER_GROUP_BOSS_FRONT_LEG = 5;
constexpr const int RENDER_GROUP_BOSS_BACK_ARM = 6;
constexpr const int RENDER_GROUP_BOSS_FIREBALL_ADDITIVE = 7;
constexpr const int RENDER_GROUP_BOSS_FIREBALL_TOP = 8;
constexpr const int ZOMBIE_LIMP_SPEED_FACTOR = 2;
constexpr const int POGO_BOUNCE_TIME = 80;
constexpr const int DOLPHIN_JUMP_TIME = 120;
constexpr const int JackInTheBoxZombieRadius = 115;
constexpr const int JackInTheBoxPlantRadius = 90;
constexpr const int BOBSLED_CRASH_TIME = 150;
constexpr const int ZOMBIE_BACKUP_DANCER_RISE_HEIGHT = -200;
constexpr const int BOSS_FLASH_HEALTH_FRACTION = 10;
constexpr const int TICKS_BETWEEN_EATS = 4;
constexpr const int DAMAGE_PER_EAT = TICKS_BETWEEN_EATS;
constexpr const float THOWN_ZOMBIE_GRAVITY = 0.05f;
constexpr const float CHILLED_SPEED_FACTOR = 0.4f;
constexpr const float CLIP_HEIGHT_LIMIT = -100.0f;
constexpr const float CLIP_HEIGHT_OFF = -200.0f;
const Color ZOMBIE_MINDCONTROLLED_COLOR = Color(128, 0, 192, 255);

enum ZombieAttackType : int32_t
{
    ATTACKTYPE_CHEW,
    ATTACKTYPE_DRIVE_OVER,
    ATTACKTYPE_VAULT,
    ATTACKTYPE_LADDER
};

enum ZombieParts : int32_t
{
    PART_BODY,
    PART_HEAD,
    PART_HEAD_EATING,
    PART_TONGUE,
    PART_ARM,
    PART_HAIR,
    PART_HEAD_YUCKY,
    PART_ARM_PICKAXE,
    PART_ARM_POLEVAULT,
    PART_ARM_LEASH,
    PART_ARM_FLAG,
    PART_POGO,
    PART_DIGGER
};

class ZombieDrawPosition
{
public:
    int                             mHeadX;
    int                             mHeadY;
    int                             mArmY;
    float                           mBodyY;
    float                           mImageOffsetX;
    float                           mImageOffsetY;
    float                           mClipHeight;
};

class Plant;
class Reanimation;
class TodParticleSystem;
class Zombie : public GameObject
{
public:
    enum
    {
        ZOMBIE_WAVE_DEBUG = -1,
        ZOMBIE_WAVE_CUTSCENE = -2,
        ZOMBIE_WAVE_UI = -3,
        ZOMBIE_WAVE_WINNER = -4
    };

public:
	ZombieType			            mZombieType;
	ZombiePhase			            mZombiePhase;
	float				            mPosX;
	float				            mPosY;
	float				            mVelX;
    int32_t                         mAnimCounter;
    int32_t                         mGroanCounter;
    int32_t                         mAnimTicksPerFrame;
    int32_t                         mAnimFrames;
    int32_t                         mFrame;
    int32_t                         mPrevFrame;
    bool                            mVariant;
    bool                            mIsEating;
    int32_t                         mJustGotShotCounter;
    int32_t                         mShieldJustGotShotCounter;
    int32_t                         mShieldRecoilCounter;
    int32_t                         mZombieAge;
    ZombieHeight                    mZombieHeight;
    int32_t                         mPhaseCounter;
    int32_t                         mFromWave;
    bool                            mDroppedLoot;
    int32_t                         mZombieFade;
    bool                            mFlatTires;
    int32_t                         mUseLadderCol;
    int32_t                         mTargetCol;
    float                           mAltitude;
    bool                            mHitUmbrella;
    Rect                            mZombieRect;
    Rect                            mZombieAttackRect;
    int32_t                         mChilledCounter;
    int32_t                         mButteredCounter;
    int32_t                         mIceTrapCounter;
    bool                            mMindControlled;
    bool                            mBlowingAway;
    bool                            mHasHead;
    bool                            mHasArm;
    bool                            mHasObject;
    bool                            mInPool;
    bool                            mOnHighGround;
    bool                            mYuckyFace;
    int32_t                         mYuckyFaceCounter;
    HelmType                        mHelmType;
    int32_t                         mBodyHealth;
    int32_t                         mBodyMaxHealth;
    int32_t                         mHelmHealth;
    int32_t                         mHelmMaxHealth;
    ShieldType                      mShieldType;
    int32_t                         mShieldHealth;
    int32_t                         mShieldMaxHealth;
    int32_t                         mFlyingHealth;
    int32_t                         mFlyingMaxHealth;
    bool                            mDead;
    ZombieID                        mRelatedZombieID;
    ZombieID                        mFollowerZombieID[MAX_ZOMBIE_FOLLOWERS];
    bool                            mPlayingSong;
    int32_t                         mParticleOffsetX;
    int32_t                         mParticleOffsetY;
    AttachmentID                    mAttachmentID;
    int32_t                         mSummonCounter;
    ReanimationID                   mBodyReanimID;
    float                           mScaleZombie;
    float                           mVelZ;
    float                           mOriginalAnimRate;
    PlantID                         mTargetPlantID;
    int32_t                         mBossMode;
    int32_t                         mTargetRow;
    int32_t                         mBossBungeeCounter;
    int32_t                         mBossStompCounter;
    int32_t                         mBossHeadCounter;
    ReanimationID                   mBossFireBallReanimID;
    ReanimationID                   mSpecialHeadReanimID;
    int32_t                         mFireballRow;
    bool                            mIsFireBall;
    ReanimationID                   mMoweredReanimID;
    int32_t                         mLastPortalX;

public:
    Zombie();
    ~Zombie();

    void                            ZombieInitialize(int theRow, ZombieType theType, bool theVariant, Zombie* theParentZombie, int theFromWave);
    void                            Animate();
    // ===== 僵尸动画小函数 =====
    void                            GetChewSoundTriggerFrame(float& theLeftHandTime, float& theRightHandTime);  // 按 mZombieType 获取咀嚼音效触发时间点
    // ===== 僵尸动画小函数结束 =====
    void                            CheckIfPreyCaught();
    void                            EatZombie(Zombie* theZombie);
    void                            EatPlant(Plant* thePlant);
    // ===== 吃植物相关小函数（提取自 EatPlant）=====
    bool                            IsPlantInedibleAtEating(Plant* thePlant);       // 判定植物是否不可被啃食（辣椒/樱桃/毁灭菇/冰菇/催眠菇等）
    bool                            TryTriggerPlantOnEating(Plant* thePlant);       // 尝试触发植物（三叶草/冰菇），返回 true 表示已触发应中止啃食
    void                            HandleIZombieSunflowerDrop(Plant* thePlant);    // IZombie 模式向日葵被啃食时掉落阳光
    void                            HandlePlantEatenDeath(Plant* thePlant);         // 植物被吃完时的死亡处理
    // ===== 吃植物相关小函数结束 =====
    void                            Update();
    void                            DieNoLoot();
    /*inline*/ void                 DieWithLoot();
    void                            Draw(Graphics* g);
//  void                            DrawZombie(Graphics* g, const ZombieDrawPosition& theDrawPos);
//  void                            DrawZombieWithParts(Graphics* g, const ZombieDrawPosition& theDrawPos);
    void                            DrawZombiePart(Graphics* g, Image* theImage, int theFrame, int theRow, const ZombieDrawPosition& theDrawPos);
    void                            DrawBungeeCord(Graphics* g, int theOffsetX);
    void                            TakeDamage(int theDamage, unsigned int theDamageFlags);
    /*inline*/ void                 SetRow(int theRow);
    float                           GetPosYBasedOnRow(int theRow);
    void                            ApplyChill(bool theIsIceTrap);
    void                            UpdateZombieBungee();
    void                            BungeeLanding();
    bool                            EffectedByDamage(unsigned int theDamageRangeFlags);
    // ===== 受伤判定小函数（提取自 EffectedByDamage）=====
    bool                            IsBossDamageable();             // 僵王博士低头状态判定
    bool                            IsInOffGroundPhase();           // 是否处于离地阶段（撑杆跳/潜水/升起等）
    // ===== 受伤判定小函数结束 =====
    void                            PickRandomSpeed();
    void                            UpdateZombiePolevaulter();
    // ===== 撑杆跳僵尸更新小函数 =====
    void                            UpdatePolevaulterPreVault();
    void                            UpdatePolevaulterInVault();
    void                            UpdateZombieDolphinRider();
    // ===== 海豚骑士僵尸更新小函数 =====
    void                            UpdateDolphinWalking();
    void                            UpdateDolphinIntoPool();
    void                            UpdateDolphinRiding();
    void                            UpdateDolphinInJump();
    void                            UpdateDolphinWalkingInPool(bool aBackwards);
    void                            PickBungeeZombieTarget(int theColumn);
    int                             CountBungeesTargetingSunFlowers();
    Plant*                          FindPlantTarget(ZombieAttackType theAttackType);
    void                            CheckSquish(ZombieAttackType theAttackType);
    void                            RiseFromGrave(int theCol, int theRow);
    // ===== 墓地升起小函数（提取自 RiseFromGrave）=====
    void                            RiseFromGraveInPool(int theCol, int theRow);   // 水池场景升起（鸭子泳圈/海藻粒子）
    void                            RiseFromGraveOnLand(int theCol, int theRow);   // 陆地场景升起（墓碑粒子）
    // ===== 墓地升起小函数结束 =====
    void                            UpdateZombieRiseFromGrave();
    void                            UpdateDamageStates(unsigned int theDamageFlags);
    void                            UpdateZombiePool();
    void                            CheckForPool();
    void                            GetDrawPos(ZombieDrawPosition& theDrawPos);
    void                            UpdateZombieHighGround();
    void                            CheckForHighGround();
    bool                            IsOnHighGround();
    void                            DropLoot();
    bool                            TrySpawnLevelAward();
    // ===== 关卡奖励小函数（提取自 TrySpawnLevelAward）=====
    bool                            CanSpawnLevelAward();            // 检查前置条件（关卡类型/波次/僵尸状态）
    CoinType                        PickLevelAwardCoinType(int aCenterX, int aCenterY);  // 按关卡类型/编号选择奖励硬币类型
    // ===== 关卡奖励小函数结束 =====
    /*inline*/ void                 StartZombieSound();
    void                            StopZombieSound();
    void                            UpdateZombieJackInTheBox();
    void                            DrawZombieHead(Graphics* g, const ZombieDrawPosition& theDrawPos, int theFrame);
    void                            UpdateZombiePosition();
    Rect                            GetZombieRect();
    Rect                            GetZombieAttackRect();
    void                            UpdateZombieWalking();
    // ===== 僵尸行走更新小函数 =====
    float                           CalcWalkingSpeed(Reanimation* aBodyReanim);   // 按 mZombiePhase/mZombieType 分支计算行走速度
    void                            UpdateWalkingDustParticles(Reanimation* aBodyReanim); // 扬尘粒子生成（Football/Polevaulter）
    void                            UpdateWalkingFrameFallback();                  // 无动画时的帧回退逻辑
    // ===== 僵尸行走更新小函数结束 =====
    void                            UpdateZombieBobsled();
    void                            BobsledCrash();
    Plant*                          IsStandingOnSpikeweed();
    void                            CheckForZombieStep();
    void                            CountExpectedMowers() { ; }
    /*inline*/ void                 OverrideParticleColor(TodParticleSystem* aParticle);
    /*inline*/ void                 OverrideParticleScale(TodParticleSystem* aParticle);
    void                            PoolSplash(bool theInToPoolSound);
    void                            UpdateZombieFlyer();
    void                            UpdateZombiePogo();

    // ===== 弹跳僵尸更新小函数 =====
    float                           GetPogoBounceHeight();
    void                            UpdatePogoBouncePhase();

    void                            UpdateZombieNewspaper();
    void                            LandFlyer(unsigned int theDamageFlags);
    void                            UpdateZombieDigger();

    // ===== 矿工僵尸更新小函数 =====
    void                            UpdateDiggerTunneling();
    void                            UpdateDiggerRising();
    void                            UpdateDiggerTunnelingPauseWithoutAxe();
    void                            UpdateDiggerRiseWithoutAxe();
    void                            UpdateDiggerStunned();

    bool                            IsWalkingBackwards();
    TodParticleSystem*              AddAttachedParticle(int thePosX, int thePosY, ParticleEffect theEffect);
    void                            PogoBreak(unsigned int theDamageFlags);
    void                            UpdateZombieFalling();
    void                            UpdateZombieDancer();
    // ===== 舞王僵尸更新小函数（提取自 UpdateZombieDancer）=====
    void                            UpdateDancerSummonCounter();    // 召唤计数器倒计时
    void                            UpdateDancerSnappingFingers();  // 响指阶段处理（召唤伴舞）
    void                            TransitionDancerPhase(ZombiePhase aDancerPhase);  // 舞者阶段切换
    // ===== 舞王僵尸更新小函数结束 =====
    ZombieID                        SummonBackupDancer(int theRow, int thePosX);
    void                            SummonBackupDancers();
    int                             GetDancerFrame();
    void                            BungeeStealTarget();
    void                            BungeeLiftTarget();
    void                            UpdateYuckyFace();
    // ===== 难看脸小函数（提取自 UpdateYuckyFace）=====
    void                            PlayYuckySound();               // 播放难看脸音效（按屏幕僵尸数量）
    void                            CalcYuckyFaceRowChange();       // 计算难看脸换行目标
    // ===== 难看脸小函数结束 =====
    void                            DrawIceTrap(Graphics* g, const ZombieDrawPosition& theDrawPos, bool theFront);
    void                            HitIceTrap();
    int                             GetHelmDamageIndex();
    int                             GetShieldDamageIndex();
    void                            DrawReanim(Graphics* g, const ZombieDrawPosition& theDrawPos, int theBaseRenderGroup);
    void                            UpdatePlaying();
    // ===== 僵尸游戏更新小函数（提取自 UpdatePlaying）=====
    void                            UpdatePlayingGroanSound();        // 呻吟声播放（按 mZombieType 分支）
    void                            UpdatePlayingStatusCounters();    // 状态计数器更新（冰冻/黄油等）
    void                            UpdatePlayingContinuousDamage();  // 持续掉血逻辑（无头/低血量）
    // ===== 僵尸游戏更新小函数结束 =====
    bool                            NeedsMoreBackupDancers();
    void                            ConvertToNormalZombie();
    void                            UpdateDancerWalking() { ; }
    void                            StartEating();
    void                            StopEating();
    void                            UpdateAnimSpeed();
    /*inline*/ void                 ReanimShowPrefix(const char* theTrackPrefix, int theRenderGroup);
    void                            PlayDeathAnim(unsigned int theDamageFlags);
    // ===== 死亡动画小函数（提取自 PlayDeathAnim）=====
    bool                            CanPlayDeathAnim();                                      // 前置检查，返回 false 表示已处理（DieNoLoot 已调用）
    void                            ClearDeathStatusEffects();                               // 清理冰冻/黄油/恶心脸状态
    float                           GetDeathAnimRate(Reanimation* aBodyReanim);              // 按僵尸类型获取死亡动画速率
    const char*                     GetDeathTrackName(Reanimation* aBodyReanim, float& aDeathAnimRate);  // 选择死亡轨道名（可能修改速率）
    // ===== 死亡动画小函数结束 =====
    void                            UpdateDeath();
    void                            DrawShadow(Graphics* g);
    bool                            HasShadow();
    Reanimation*                    LoadReanim(ReanimationType theReanimationType);
    /*inline*/ int                  TakeFlyingDamage(int theDamage, unsigned int theDamageFlags);
    int                             TakeShieldDamage(int theDamage, unsigned int theDamageFlags);
    int                             TakeHelmDamage(int theDamage, unsigned int theDamageFlags);
    // ===== 头盔受伤小函数（提取自 TakeHelmDamage）=====
    void                            UpdateHelmDamageImageOverride(int aDamageIndex);  // 按头盔类型/损伤等级切换贴图
    // ===== 头盔受伤小函数结束 =====
    void                            TakeBodyDamage(int theDamage, unsigned int theDamageFlags);
    void                            AttachShield();
    void                            DetachShield();
    void                            UpdateReanim();
    // ===== 重新动画更新小函数（提取自 UpdateReanim）=====
    void                            UpdateReanimShakeAndScale(Reanimation* aBodyReanim, float& anOffsetX, float& anOffsetY);  // 抖动/偏移/缩放（Catapult/Zamboni/Football）
    void                            UpdateReanimMirror(bool& anOpposite);  // 镜像处理（Dancer/BackupDancer）
    // ===== 重新动画更新小函数结束 =====
    void                            GetTrackPosition(const char* theTrackName, float& thePosX, float& thePosY);
    void                            LoadPlainZombieReanim();
    void                            ShowDoorArms(bool theShow);
    /*inline*/ void                 ReanimShowTrack(const char* theTrackName, int theRenderGroup);
    /*inline*/ void                 PlayZombieAppearSound();
    void                            StartMindControlled();
    // ===== 魅惑相关小函数（提取自巨形函数，方便 Mod 调用）=====
    // 获取魅惑紫色（带 alpha 修正）
    Color                           GetMindControlColor(int alpha = 255);
    // Image 部件绘制时的紫色着色 + 叠加发光（提取自 DrawZombiePart）
    void                            ApplyMindControlImageTint(Graphics* g, Image* theImage, const Rect& aDestRect, const Rect& aSrcRect, bool aMirror, int anAlpha);
    // 获取 Reanimation 魅惑颜色参数（提取自 DrawReanim）
    void                            GetMindControlReanimColor(Color& aColorOverride, Color& aExtraAdditiveColor, bool& aEnableExtraAdditiveDraw, int aFadeAlpha);
    // 翻转 Reanimation 镜像方向（提取自 UpdateReanim）
    void                            ApplyMindControlReanimMirror(bool& anOpposite);
    // 检查魅惑僵尸越右界死亡（提取自 CheckForBoardEdge）
    bool                            CheckMindControlEdgeDeath();
    // 判断目标僵尸是否为魅惑攻击目标（提取自 FindZombieTarget）
    bool                            IsMindControlAttackTarget(Zombie* other);
    // 执行魅惑攻击逻辑（提取自 CheckIfPreyCaught）
    bool                            TryMindControlAttack();
    // 为粒子应用魅惑紫色（提取自 OverrideParticleColor）
    void                            ApplyMindControlParticleTint(TodParticleSystem* aParticle);
    // ===== 魅惑相关小函数结束 =====
    bool                            IsFlying();
    void                            DropHead(unsigned int theDamageFlags);
    // ===== 掉头小函数（提取自 DropHead）=====
    void                            GetHeadDropPos(float& aPosX, float& aPosY, int& aRenderOrder);  // 计算头部掉落位置
    ParticleEffect                  GetHeadDropParticleEffect();                                   // 按状态选择粒子效果
    void                            HandleHeadDropTypeSpecifics(unsigned int theDamageFlags);      // 类型特定处理（Pogo/Balloon 等）
    void                            OverrideHeadParticleImage(TodParticleSystem* aParticle);       // 按类型设置头部粒子图片
    void                            DropHeadMustache(float aPosX, float aPosY, int aRenderOrder);  // 胡子模式掉落
    void                            DropHeadFuture(float aPosX, float aPosY, int aRenderOrder);    // 未来模式掉落
    void                            DropHeadPinata(float aPosX, float aPosY, int aRenderOrder);    // 皮纳塔模式掉落
    // ===== 掉头小函数结束 =====
    bool                            CanTargetPlant(Plant* thePlant, ZombieAttackType theAttackType);
    void                            UpdateZombieCatapult();
    Plant*                          FindCatapultTarget();
    void                            ZombieCatapultFire(Plant* thePlant);
    void                            UpdateClimbingLadder();
    void                            UpdateZombieGargantuar();
    // ===== 伽刚特尔小函数（提取自 UpdateZombieGargantuar）=====
    void                            GargantuarSmashAttack();       // 砸击攻击（魅惑砸僵尸/普通砸植物）
    void                            GargantuarThrowImp();          // 投掷小鬼
    bool                            GargantuarShouldSmash();       // 检查是否需要砸击
    // ===== 伽刚特尔小函数结束 =====
    // ===== ZombieInitialize 小函数（提取自 ZombieInitialize）=====
    void                            InitZombieTypeNormal();        // 普通僵尸初始化
    void                            InitZombieTypeDuckyTube();     // 鸭子救生圈僵尸初始化
    void                            InitZombieTypeConehead();      // 路障僵尸初始化
    void                            InitZombieTypeBuckethead();    // 铁桶僵尸初始化
    void                            InitZombieTypeDoor();          // 铁栅门僵尸初始化
    void                            InitZombieTypeLadder();        // 梯子僵尸初始化
    void                            InitZombieTypeBungee(RenderLayer& aRenderLayer, int& aRenderOffset);  // 蹦极僵尸初始化
    void                            InitZombieTypeFootball();      // 橄榄球僵尸初始化
    void                            InitZombieTypeDigger(int& aRenderOffset);  // 矿工僵尸初始化
    void                            InitZombieTypePolevaulter();   // 撑杆僵尸初始化
    void                            InitZombieTypeDolphinRider();  // 海豚骑士僵尸初始化
    void                            InitZombieTypeGargantuar(int& aRenderOffset);  // 伽刚特尔初始化
    void                            InitZombieTypeZamboni(int& aRenderOffset);  // 雪犁僵尸初始化
    void                            InitZombieTypeCatapult();      // 投石车僵尸初始化
    void                            InitZombieTypeSnorkel();       // 潜水僵尸初始化
    void                            InitZombieTypeJackInTheBox();  // 开罐器僵尸初始化
    void                            InitZombieTypeBobsled(Zombie* theParentZombie, int& aRenderOffset);  // 雪橇僵尸初始化
    void                            InitZombieTypePogo();          // 弹跳僵尸初始化
    void                            InitZombieTypeNewspaper();     // 读报僵尸初始化
    void                            InitZombieTypeBalloon();       // 气球僵尸初始化
    void                            InitZombieTypeDancing();       // 舞王僵尸初始化
    void                            InitZombieTypeBackupDancer();  // 伴舞僵尸初始化
    void                            InitZombieTypeImp();           // 小鬼僵尸初始化
    void                            InitZombieTypeBoss(RenderLayer& aRenderLayer);  // Boss僵尸初始化
    // ===== ZombieInitialize 小函数结束 =====
    // ===== UpdateBoss 小函数（提取自 UpdateBoss）=====
    void                            UpdateBossIdle();              // Boss空闲阶段
    void                            UpdateBossSpawning();          // Boss召唤阶段
    void                            UpdateBossBungeesEnter();      // Boss蹦极进入阶段
    void                            UpdateBossStomping();          // Boss踩踏阶段
    void                            UpdateBossBungeeExit();        // Boss蹦极退出阶段
    void                            UpdateBossDropRV();            // Boss砸车阶段
    void                            UpdateBossHeadEnter();         // Boss头部进入阶段
    void                            UpdateBossHeadSpit();          // Boss头部吐球阶段
    void                            UpdateBossHeadLeave();         // Boss头部离开阶段
    // ===== UpdateBoss 小函数结束 =====
    // ===== UpdateDeath 小函数（提取自 UpdateDeath）=====
    float                           GetZombieFallTime(Reanimation* aBodyReanim);  // 获取僵尸倒地时间
    void                            UpdateBossDeathExplosions(Reanimation* aBodyReanim);  // Boss死亡爆炸序列
    bool                            UpdateZamboniCatapultDeath();  // 雪犁/投石车死亡爆炸
    // ===== UpdateDeath 小函数结束 =====
    // ===== TakeBodyDamage 小函数（提取自 TakeBodyDamage）=====
    void                            TakeBodyDamageZamboni(unsigned int theDamageFlags, int aDamageIndexBeforeDamage, int aDamageIndexAfterDamage);  // 雪犁受伤状态切换
    void                            TakeBodyDamageCatapult(unsigned int theDamageFlags, int aDamageIndexBeforeDamage, int aDamageIndexAfterDamage);  // 投石车受伤状态切换
    void                            TakeBodyDamageGargantuar(int aDamageIndexBeforeDamage, int aDamageIndexAfterDamage);  // 伽刚特尔受伤状态切换
    void                            TakeBodyDamageBoss(unsigned int theDamageFlags, int aDamageIndexBeforeDamage, int aDamageIndexAfterDamage, int aBodyHealthOrigin);  // Boss受伤状态切换
    // ===== TakeBodyDamage 小函数结束 =====
    int                             GetBodyDamageIndex();
    void                            ApplyBurn();
    // ===== 燃烧小函数（提取自 ApplyBurn）=====
    bool                            IsBurnInstantDeathPhase();                                      // 判断是否处于立即死亡阶段
    void                            HandleBurnFrozenState();                                        // 处理冻结/特殊状态燃烧（不立即死亡）
    void                            GetCharredReanimInfo(ReanimationType& aReanimType, float& aCharredPosX, float& aCharredPosY);  // 获取焦黑动画类型和位置
    void                            SetupCharredReanim(Reanimation* aCharredReanim);                // 设置焦黑动画帧/缩放/镜像
    // ===== 燃烧小函数结束 =====
    void                            UpdateBurn();
    bool                            ZombieNotWalking();
    Zombie*                         FindZombieTarget();
    /*inline*/ void                 PlayZombieReanim(const char* theTrackName, ReanimLoopType theLoopType, int theBlendTime, float theAnimRate);
    void                            UpdateZombieBackupDancer();
    ZombiePhase                     GetDancerPhase();
    bool                            IsMovingAtChilledSpeed();
    void                            StartWalkAnim(int theBlendTime);
    Reanimation*                    AddAttachedReanim(int thePosX, int thePosY, ReanimationType theReanimType);
    void                            DragUnder();
    static /*inline*/ void          SetupDoorArms(Reanimation* aReanim, bool theShow);
    static void                     SetupReanimLayers(Reanimation* aReanim, ZombieType theZombieType);
    /*inline*/ bool                 IsOnBoard();
    void                            DrawButter(Graphics* g, const ZombieDrawPosition& theDrawPos);
    bool                            IsImmobilizied();
    void                            ApplyButter();
    float                           ZombieTargetLeadX(float theTime);
    void                            UpdateZombieImp();
    void                            SquishAllInSquare(int theX, int theY, ZombieAttackType theAttackType);
    void                            RemoveIceTrap();
    bool                            IsBouncingPogo();
    int                             GetBobsledPosition();
    void                            DrawBobsledReanim(Graphics* g, const ZombieDrawPosition& theDrawPos, bool theBeforeZombie);
    void                            BobsledDie();
    void                            BobsledBurn();
    bool                            IsBobsledTeamWithSled();
    bool                            CanBeFrozen();
    bool                            CanBeChilled();
    void                            UpdateZombieSnorkel();

    // ===== 潜水僵尸更新小函数 =====
    void                            UpdateSnorkelWalking();
    void                            UpdateSnorkelIntoPool();
    void                            UpdateSnorkelWalkingInPool(bool aBackwards);
    void                            UpdateSnorkelUpToEat();
    void                            UpdateSnorkelEatingInPool();
    void                            UpdateSnorkelDownFromEat();
    void                            ReanimIgnoreClipRect(const char* theTrackName, bool theIgnoreClipRect);
    void                            SetAnimRate(float theAnimRate);
    void                            ApplyAnimRate(float theAnimRate);
    /*inline*/ bool                 IsDeadOrDying();
    void                            DrawDancerReanim(Graphics* g);
    void                            DrawBungeeReanim(Graphics* g);
    void                            DrawBungeeTarget(Graphics* g);
    void                            BungeeDie();
    void                            ZamboniDeath(unsigned int theDamageFlags);
    void                            CatapultDeath(unsigned int theDamageFlags);
    bool                            SetupDrawZombieWon(Graphics* g);
    void                            WalkIntoHouse();
    void                            UpdateZamboni();
    void                            UpdateZombieChimney();
    void                            UpdateLadder();
    void                            DropArm(unsigned int theDamageFlags);
    bool                            CanLoseBodyParts();
    void                            DropHelm(unsigned int theDamageFlags);
    void                            DropShield(unsigned int theDamageFlags);
    void                            ReanimReenableClipping();
    void                            UpdateBoss();
    void                            BossPlayIdle();
    void                            BossRVLanding();
    void                            BossStompContact();
    bool                            BossAreBungeesDone();
    void                            BossBungeeSpawn();
    void                            BossSpawnAttack();
    void                            BossBungeeAttack();
    void                            BossRVAttack();
    void                            BossSpawnContact();
    void                            BossBungeeLeave();
    void                            BossStompAttack();
    bool                            BossCanStompRow(int theRow);
    void                            BossDie();
    void                            BossHeadAttack();
    void                            BossHeadSpitContact();
    void                            BossHeadSpit();
    void                            UpdateBossFireball();
    void                            BossDestroyFireball();
    void                            BossDestroyIceballInRow();
    void                            DiggerLoseAxe();
    void                            BungeeDropZombie(Zombie* theDroppedZombie, int theGridX, int theGridY);
    void                            ShowYuckyFace(bool theShow);
    void                            AnimateChewSound();
    void                            AnimateChewEffect();
    void                            UpdateActions();
    void                            UpdateActionsByHeight();
    void                            UpdateActionsByType();
    void                            CheckForBoardEdge();
    void                            UpdateYeti();
    void                            DrawBossPart(Graphics* g, BossPart theBossPart);
    void                            BossSetupReanim();
    void                            MowDown();
    // ===== 割草机碾压小函数 =====
    bool                            HandleMowDownSpecialDeath();   // Catapult/Zamboni 特殊爆炸死亡，返回 true 表示已处理
    void                            HandleMowDownDrops();          // 掉落物分派（Flag、Polevaulter、Newspaper/Balloon、Pogo）
    // ===== 割草机碾压小函数结束 =====
    void                            UpdateMowered();
    void                            DropFlag();
    void                            DropPole();
    void                            DrawBossBackArm(Graphics* g, const ZombieDrawPosition& theDrawPos);
    static void                     PreloadZombieResources(ZombieType theZombieType);
    void                            BossStartDeath();
    void                            RemoveColdEffects();
    void                            BossHeadSpitEffect();
    void                            DrawBossFireBall(Graphics* g);
    void                            UpdateZombiePeaHead();
    void                            UpdateZombieJalapenoHead();
    void                            ApplyBossSmokeParticles(bool theEnable);
    void                            UpdateZombiquarium();
    // ===== 水族箱僵尸更新小函数 =====
    void                            UpdateZombiquariumBite(Reanimation* aBodyReanim);
    void                            UpdateZombiquariumAccel(float& aMaxSpeed);
    void                            UpdateZombiquariumBackAndForth(float aVelX, float& aMaxSpeed);
    void                            UpdateZombiquariumDrift(float& aMaxSpeed);
    bool                            ZombiquariumFindClosestBrain();
    void                            UpdateZombieGatlingHead();
    void                            UpdateZombieSquashHead();
    // ===== 豌豆头/加特林头小函数（提取自 UpdateZombiePeaHead/UpdateZombieGatlingHead）=====
    void                            PeaHeadShootProjectile(float aOriginX, float aOriginY);  // 射击豌豆（魅惑射普通豌豆/普通射僵尸豌豆）
    // ===== 豌豆头/加特林头小函数结束 =====
    // ===== 辣椒头小函数（提取自 UpdateZombieJalapenoHead）=====
    void                            JalapenoHeadIgnite();   // 点火（魅惑烧同行僵尸/普通烧同行植物）
    // ===== 辣椒头小函数结束 =====
    // ===== 倭瓜头小函数（提取自 UpdateZombieSquashHead）=====
    int                             SquashHeadGetDestX();           // 获取目标X坐标（魅惑追踪僵尸/普通落植物格）
    void                            SquashHeadSmashAttack(int aDestX); // 砸击攻击（魅惑伤害僵尸/普通压扁植物）
    // ===== 倭瓜头小函数结束 =====
    bool                            IsTanglekelpTarget();
    bool                            HasYuckyFaceImage();
    bool                            IsTangleKelpTarget();
    bool                            IsFireResistant();
    /*inline*/ void                 EnableMustache(bool theEnableMustache);
    /*inline*/ void                 EnableFuture(bool theEnableFuture);
    /*inline*/ void                 EnableDance();
    void                            BungeeDropPlant();
    void                            RemoveButter();
    void                            BalloonPropellerHatSpin(bool theSpinning);
    void                            DoDaisies();
    static /*inline*/ bool          ZombieTypeCanGoOnHighGround(ZombieType theZombieType);
    static /*inline*/ bool          ZombieTypeCanGoInPool(ZombieType theZombieType);
    void                            SetupWaterTrack(const char* theTrackName);
    void                            BurnRow(int theRow);
    void                            SetupReanimForLostHead();
    void                            SetupReanimForLostArm(unsigned int theDamageFlags);
    // ===== 断臂动画设置小函数 =====
    void                            HideLostArmTracks();
    void                            SetupLostArmImageOverride(float& aPosX, float& aPosY);
    void                            SetupLostArmParticleImage(unsigned int theDamageFlags, float aPosX, float aPosY);
    bool                            IsSquashTarget(Plant* theExcept);
    static /*inline*/ bool			IsZombotany(ZombieType theZombieType);
};

class ZombieDefinition
{
public:
    ZombieType                      mZombieType;
    ReanimationType                 mReanimationType = ReanimationType::REANIM_NONE;
    int                             mZombieValue = 0;
    int                             mStartingLevel = 0;
    int                             mFirstAllowedWave = 0;
    int                             mPickWeight = 0;
    std::string                     mZombieName;   // Mod API: 改为 std::string 以支持运行时修改
    // Mod API: 图鉴显示用字段（自定义僵尸专用，原版僵尸仍走资源文件）
    std::string                     mAlmanacName;        // 图鉴标题（如 "My Custom Zombie"）
    std::string                     mAlmanacDescription; // 图鉴描述正文
};
extern ZombieDefinition gZombieDefs[NUM_ZOMBIE_TYPES];  // Mod API: 移除 const

/*inline*/ const ZombieDefinition&            GetZombieDefinition(ZombieType theZombieType);

// Mod API: 动态注册新僵尸类型，返回分配的 ZombieType（>= NUM_ZOMBIE_TYPES）
ZombieType RegisterZombieDefinition(const ZombieDefinition& theDef);

// Mod API: 自定义僵尸定义的运行时存储（供 AlmanacDialog 查询）
extern std::vector<ZombieDefinition> gCustomZombieDefs;
inline int GetCustomZombieCount() { return static_cast<int>(gCustomZombieDefs.size()); }
inline int GetTotalZombieCount() { return static_cast<int>(ZombieType::NUM_ZOMBIE_TYPES) + GetCustomZombieCount(); }
// 把自定义僵尸的 ZombieType 转换为 gCustomZombieDefs 的索引（0-based）
inline int CustomZombieTypeToIndex(ZombieType z) { return static_cast<int>(z) - static_cast<int>(ZombieType::NUM_ZOMBIE_TYPES); }
inline bool IsCustomZombieType(ZombieType z) { return static_cast<int>(z) >= static_cast<int>(ZombieType::NUM_ZOMBIE_TYPES); }

#endif
