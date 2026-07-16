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

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "GameObject.h"

#define MAX_MAGNET_ITEMS 5

enum PlantSubClass : int32_t
{
    SUBCLASS_NORMAL = 0,
    SUBCLASS_SHOOTER = 1
};

enum PlantWeapon : int32_t
{
    WEAPON_PRIMARY,
    WEAPON_SECONDARY
};

enum PlantOnBungeeState : int32_t
{
    NOT_ON_BUNGEE,
    GETTING_GRABBED_BY_BUNGEE,
    RISING_WITH_BUNGEE
};

enum PlantState : int32_t
{
    STATE_NOTREADY,
    STATE_READY,
    STATE_DOINGSPECIAL,
    STATE_SQUASH_LOOK,
    STATE_SQUASH_PRE_LAUNCH,
    STATE_SQUASH_RISING,
    STATE_SQUASH_FALLING,
    STATE_SQUASH_DONE_FALLING,
    STATE_GRAVEBUSTER_LANDING,
    STATE_GRAVEBUSTER_EATING,
    STATE_CHOMPER_BITING,
    STATE_CHOMPER_BITING_GOT_ONE,
    STATE_CHOMPER_BITING_MISSED,
    STATE_CHOMPER_DIGESTING,
    STATE_CHOMPER_SWALLOWING,
    STATE_POTATO_RISING,
    STATE_POTATO_ARMED,
    STATE_POTATO_MASHED,
    STATE_SPIKEWEED_ATTACKING,
    STATE_SPIKEWEED_ATTACKING_2,
    STATE_SCAREDYSHROOM_LOWERING,
    STATE_SCAREDYSHROOM_SCARED,
    STATE_SCAREDYSHROOM_RAISING,
    STATE_SUNSHROOM_SMALL,
    STATE_SUNSHROOM_GROWING,
    STATE_SUNSHROOM_BIG,
    STATE_MAGNETSHROOM_SUCKING,
    STATE_MAGNETSHROOM_CHARGING,
    STATE_BOWLING_UP,
    STATE_BOWLING_DOWN,
    STATE_CACTUS_LOW,
    STATE_CACTUS_RISING,
    STATE_CACTUS_HIGH,
    STATE_CACTUS_LOWERING,
    STATE_TANGLEKELP_GRABBING,
    STATE_COBCANNON_ARMING,
    STATE_COBCANNON_LOADING,
    STATE_COBCANNON_READY,
    STATE_COBCANNON_FIRING,
    STATE_KERNELPULT_BUTTER,
    STATE_UMBRELLA_TRIGGERED,
    STATE_UMBRELLA_REFLECTING,
    STATE_IMITATER_MORPHING,
    STATE_ZEN_GARDEN_WATERED,
    STATE_ZEN_GARDEN_NEEDY,
    STATE_ZEN_GARDEN_HAPPY,
    STATE_MARIGOLD_ENDING,
    STATE_FLOWERPOT_INVULNERABLE,
    STATE_LILYPAD_INVULNERABLE
};

enum PLANT_LAYER : int32_t
{
    PLANT_LAYER_BELOW = -1,
    PLANT_LAYER_MAIN,
    PLANT_LAYER_REANIM,
    PLANT_LAYER_REANIM_HEAD,
    PLANT_LAYER_REANIM_BLINK,
    PLANT_LAYER_ON_TOP,
    NUM_PLANT_LAYERS
};

enum PLANT_ORDER : int32_t
{
    PLANT_ORDER_LILYPAD,
    PLANT_ORDER_NORMAL,
    PLANT_ORDER_PUMPKIN,
    PLANT_ORDER_FLYER,
    PLANT_ORDER_CHERRYBOMB
};

enum MagnetItemType : int32_t
{
    MAGNET_ITEM_NONE,
    MAGNET_ITEM_PAIL_1,
    MAGNET_ITEM_PAIL_2,
    MAGNET_ITEM_PAIL_3,
    MAGNET_ITEM_FOOTBALL_HELMET_1,
    MAGNET_ITEM_FOOTBALL_HELMET_2,
    MAGNET_ITEM_FOOTBALL_HELMET_3,
    MAGNET_ITEM_DOOR_1,
    MAGNET_ITEM_DOOR_2,
    MAGNET_ITEM_DOOR_3,
    //MAGNET_ITEM_PROPELLER,
    MAGNET_ITEM_POGO_1,
    MAGNET_ITEM_POGO_2,
    MAGNET_ITEM_POGO_3,
    MAGNET_ITEM_JACK_IN_THE_BOX,
    MAGNET_ITEM_LADDER_1,
    MAGNET_ITEM_LADDER_2,
    MAGNET_ITEM_LADDER_3,
    MAGNET_ITEM_LADDER_PLACED,
    MAGNET_ITEM_SILVER_COIN,
    MAGNET_ITEM_GOLD_COIN,
    MAGNET_ITEM_DIAMOND,
    MAGNET_ITEM_PICK_AXE
};

class MagnetItem
{
public:
    float                   mPosX;
    float                   mPosY;
    float                   mDestOffsetX;
    float                   mDestOffsetY;
    MagnetItemType          mItemType;
};

class Coin;
class Zombie;
class Reanimation;
class TodParticleSystem;
class PlantDefinition;
class Projectile;
class GridItem;

class Plant : public GameObject
{
public:
    SeedType                mSeedType;
    int32_t                 mPlantCol;
    int32_t                 mAnimCounter;
    int32_t                 mFrame;
    int32_t                 mFrameLength;
    int32_t                 mNumFrames;
    PlantState              mState;
    int32_t                 mPlantHealth;
    int32_t                 mPlantMaxHealth;
    int32_t                 mSubclass;
    int32_t                 mDisappearCountdown;
    int32_t                 mDoSpecialCountdown;
    int32_t                 mStateCountdown;
    int32_t                 mLaunchCounter;
    int32_t                 mLaunchRate;
    Rect                    mPlantRect;
    Rect                    mPlantAttackRect;
    int32_t                 mTargetX;
    int32_t                 mTargetY;
    int32_t                 mStartRow;
    ParticleSystemID        mParticleID;
    int32_t                 mShootingCounter;
    ReanimationID           mBodyReanimID;
    ReanimationID           mHeadReanimID;
    ReanimationID           mHeadReanimID2;
    ReanimationID           mHeadReanimID3;
    ReanimationID           mBlinkReanimID;
    ReanimationID           mLightReanimID;
    ReanimationID           mSleepingReanimID;
    int32_t                 mBlinkCountdown;
    int32_t                 mRecentlyEatenCountdown;
    int32_t                 mEatenFlashCountdown;
    int32_t                 mBeghouledFlashCountdown;
    float                   mShakeOffsetX;
    float                   mShakeOffsetY;
    MagnetItem              mMagnetItems[MAX_MAGNET_ITEMS];
    ZombieID                mTargetZombieID;
    int32_t                 mWakeUpCounter;
    PlantOnBungeeState      mOnBungeeState;
    SeedType                mImitaterType;
    int32_t                 mPottedPlantIndex;
    bool                    mAnimPing;
    bool                    mDead;
    bool                    mSquished;
    bool                    mIsAsleep;
    bool                    mIsOnBoard;
    bool                    mHighlighted;

public:
    Plant();

    void                    PlantInitialize(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType);
    // ===== 植物初始化小函数（提取自 PlantInitialize switch-case）=====
    void                    InitPlantBodyReanim(const PlantDefinition& aPlantDef, Reanimation*& aBodyReanim);  // 创建 Body Reanimation 并基础设置
    void                    InitPlantPeashooterFamily(const PlantDefinition& aPlantDef);   // 豌豆射手家族（含 repeater/leftpeater/gatlingpea）
    void                    InitPlantSplitPea(const PlantDefinition& aPlantDef);           // 双发射手
    void                    InitPlantThreepeater(const PlantDefinition& aPlantDef);        // 三线射手
    void                    InitPlantWallnutFamily(Reanimation* aBodyReanim);              // 坚果家族（坚果/爆炸坚果/巨型坚果/高坚果）
    void                    InitPlantCherryJalapeno(Reanimation* aBodyReanim);             // 樱桃/辣椒
    void                    InitPlantPotatoMine(Reanimation* aBodyReanim);                 // 土豆雷
    void                    InitPlantGraveBuster(Reanimation* aBodyReanim);                // 墓碑吞噬者
    void                    InitPlantSunShroom(Reanimation* aBodyReanim);                  // 阳光菇
    void                    InitPlantPumpkinShell(Reanimation* aBodyReanim);               // 南瓜头
    void                    InitPlantCobCannon(Reanimation* aBodyReanim);                  // 玉米加农炮
    void                    InitPlantFlowerPotOrLilyPad();                                 // 花盆/荷叶（无敌状态设置）
    void                    InitPlantByType(SeedType theSeedType, const PlantDefinition& aPlantDef, Reanimation* aBodyReanim);  // switch 分发
    // ===== 植物初始化小函数结束 =====
    void                    Update();
    void                    Animate();
    void                    Draw(Graphics* g);
    void                    MouseDown(int x, int y, int theClickCount);
    void                    DoSpecial();
    // ===== 特殊技能小函数（提取自 DoSpecial）=====
    void                    DoSpecialBlover();          // 三叶草：吹走飞行僵尸
    void                    DoSpecialCherryBomb();      // 樱桃炸弹：范围杀伤
    void                    DoSpecialDoomShroom();      // 毁灭菇：大范围杀伤+留坑
    void                    DoSpecialJalapeno();        // 火爆辣椒：烧一行
    void                    DoSpecialUmbrella();        // 伞叶：反弹弹射物
    void                    DoSpecialIceShroom();       // 寒冰菇：冰冻所有僵尸
    void                    DoSpecialPotatoMine();      // 土豆雷：爆炸
    void                    DoSpecialInstantCoffee();   // 咖啡豆：唤醒植物
    // ===== 特殊技能小函数结束 =====
    void                    Fire(Zombie* theTargetZombie, int theRow, PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
    // ===== 发射投射物相关小函数（提取自 Fire）=====
    ProjectileType          GetFireProjectileType(PlantWeapon thePlantWeapon);              // 按植物类型返回投射物类型
    void                    GetFireOrigin(PlantWeapon thePlantWeapon, int& aOriginX, int& aOriginY);  // 计算发射点坐标
    void                    PlayFireSound();                                                // 播放发射音效
    void                    PlayFireMuzzleParticle(int aOriginX, int aOriginY);            // 创建枪口粒子特效
    void                    SetupProjectileMotion(Projectile* aProjectile, Zombie* theTargetZombie, int theRow, PlantWeapon thePlantWeapon, int aOriginX, int aOriginY);  // 设置投射物运动类型
    void                    SetupLobbedProjectileMotion(Projectile* aProjectile, Zombie* theTargetZombie, int aOriginX, int aOriginY);  // 抛投类投射物运动（射程计算+抛物线参数）
    // ===== 发射投射物相关小函数结束 =====
    Zombie*                 FindTargetZombie(int theRow, PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
    // ===== 目标查找小函数（提取自 FindTargetZombie）=====
    bool                    FilterZombieTarget(Zombie* aZombie, int theRow, int aDamageRangeFlags, bool needPortalCheck, Rect& aAttackRect, int& aExtraRange);  // 检查僵尸是否为有效目标，返回 false 表示跳过
    int                     CalcZombieTargetWeight(Zombie* aZombie, const Rect& aZombieRect);  // 计算僵尸目标权重（越大越优先）
    // ===== 目标查找小函数结束 =====
    void                    Die();
    void                    UpdateProductionPlant();
    void                    UpdateShooter();
    bool                    FindTargetAndFire(int theRow, PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
    void                    LaunchThreepeater();
    static Image*           GetImage(SeedType theSeedType);
    static int              GetCost(SeedType theSeedType, SeedType theImitaterType = SeedType::SEED_NONE);
    static std::string       GetNameString(SeedType theSeedType, SeedType theImitaterType = SeedType::SEED_NONE);
    static std::string       GetToolTip(SeedType theSeedType);
    static int              GetRefreshTime(SeedType theSeedType, SeedType theImitaterType = SeedType::SEED_NONE);
    static /*inline*/ bool  IsNocturnal(SeedType theSeedtype);
    static /*inline*/ bool  IsFungus(SeedType theSeedType);
    static /*inline*/ bool  IsAquatic(SeedType theSeedType);
    static /*inline*/ bool  IsFlying(SeedType theSeedtype);
    static /*inline*/ bool  IsUpgrade(SeedType theSeedtype);
    void                    UpdateAbilities();
    void                    UpdatePlantAbilityByType();
    void                    Squish();
    void                    DoRowAreaDamage(int theDamage, unsigned int theDamageFlags);
    int                     GetDamageRangeFlags(PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
    Rect                    GetPlantRect();
    Rect                    GetPlantAttackRect(PlantWeapon thePlantWeapon = PlantWeapon::WEAPON_PRIMARY);
    Zombie*                 FindSquashTarget();
    void                    UpdateSquash();
    /*inline*/ bool         NotOnGround();
    void                    DoSquashDamage();
    void                    BurnRow(int theRow);
    void                    IceZombies();
    void                    BlowAwayFliers();
    void                    UpdateGraveBuster();
    TodParticleSystem*      AddAttachedParticle(int thePosX, int thePosY, int theRenderPosition, ParticleEffect theEffect);
    void                    GetPeaHeadOffset(int& theOffsetX, int& theOffsetY);
    /*inline*/ bool         MakesSun();
    static void             DrawSeedType(Graphics* g, SeedType theSeedType, SeedType theImitaterType, DrawVariation theDrawVariation, float thePosX, float thePosY);
    void                    KillAllPlantsNearDoom();
    bool                    IsOnHighGround();
    void                    UpdateTorchwood();
    void                    LaunchStarFruit();
    bool                    FindStarFruitTarget();
    void                    UpdateChomper();
    // ===== 食人花小函数（提取自 UpdateChomper）=====
    void                    UpdateChomperBiting();       // 咬合阶段：查找目标、判定命中/未命中
    // ===== 食人花小函数结束 =====
    void                    DoBlink();
    void                    UpdateBlink();
    void                    PlayBodyReanim(const char* theTrackName, ReanimLoopType theLoopType, int theBlendTime, float theAnimRate);
    void                    UpdateMagnetShroom();
    // ===== 磁力菇小函数（提取自 UpdateMagnetShroom）=====
    void                    UpdateMagnetItemPositions();  // 更新磁铁物品位置（移动到植物）
    void                    UpdateMagnetShroomCharging(); // 充能状态：恢复准备
    void                    UpdateMagnetShroomSucking();  // 吸引状态：等待动画完成
    Zombie*                 FindClosestMagnetTarget();    // 查找最近的磁铁目标僵尸
    GridItem*               FindClosestMagnetLadder();    // 查找最近的梯子
    // ===== 磁力菇小函数结束 =====
    MagnetItem*             GetFreeMagnetItem();
    void                    DrawMagnetItems(Graphics* g);
    void                    UpdateDoomShroom();
    void                    UpdateIceShroom();
    void                    UpdatePotato();
    int                     CalcRenderOrder();
    void                    AnimateNuts();
    void                    SetSleeping(bool theIsAsleep);
    void                    UpdateShooting();
    // ===== 射击更新小函数（提取自 UpdateShooting）=====
    void                    UpdateFumeShroomShooting();                    // 大喷菇粒子触发
    void                    UpdateGloomShroomShooting();                   // 多发喷菇多次发射+粒子
    void                    UpdateGatlingPeaShooting();                    // 机枪射手多次发射
    void                    UpdateCattailShooting();                       // 猫尾草目标查找发射
    void                    FireThreepeaterAtCounterOne();                 // 三线射手 counter==1 发射
    void                    FireSplitPeaAtCounterOne();                    // 双发射手 counter==1 发射
    void                    FireCatapultAtCounterOne();                    // 投掷类 counter==1 发射
    void                    FireAtCounterOne();                            // counter==1 发射分发
    void                    ResetThreepeaterShootingAnim();                // 三线射手动画复位
    void                    ResetSplitPeaShootingAnim();                   // 双发射手动画复位
    void                    ResetShootingAnim();                           // counter==0 动画重置分发
    // ===== 射击更新小函数结束 =====
    void                    DrawShadow(Graphics* g, float theOffsetX, float theOffsetY);
    void                    UpdateScaredyShroom();
    int                     DistanceToClosestZombie();
    void                    UpdateSpikeweed();
    void                    MagnetShroomAttactItem(Zombie* theZombie);
    // ===== 磁铁吸引装备小函数（提取自 MagnetShroomAttactItem）=====
    void                    MagnetAttractPail(Zombie* theZombie, MagnetItem* aMagnetItem);          // 铁桶
    void                    MagnetAttractFootball(Zombie* theZombie, MagnetItem* aMagnetItem);     // 橄榄球头盔
    void                    MagnetAttractDoor(Zombie* theZombie, MagnetItem* aMagnetItem);          // 纱门
    void                    MagnetAttractLadder(Zombie* theZombie, MagnetItem* aMagnetItem);       // 梯子
    void                    MagnetAttractPogo(Zombie* theZombie, MagnetItem* aMagnetItem);         // 弹跳杆
    void                    MagnetAttractJackInTheBox(Zombie* theZombie, MagnetItem* aMagnetItem); // 小丑盒
    void                    MagnetAttractDiggerAxe(Zombie* theZombie, MagnetItem* aMagnetItem);    // 镐
    void                    UpdateSunShroom();
    void                    UpdateBowling();
    // ===== 保龄球小函数（提取自 UpdateBowling）=====
    bool                    UpdateBowlingMovement(PlantState& aNewState);    // 水平+垂直移动及边界反弹状态计算（返回 false 表示未到网格需提前 return）
    bool                    HandleBowlingZombieImpact(PlantState& aNewState); // 撞击僵尸处理（伤害/金币/方向变更，返回 false 表示已爆炸需终止）
    void                    ApplyBowlingState(PlantState aNewState);       // 应用新状态（更新行/状态/渲染顺序）
    // ===== 保龄球小函数结束 =====
    void                    AnimatePumpkin();
    void                    UpdateBlover();
    void                    UpdateCactus();
    void                    StarFruitFire();
    void                    UpdateTanglekelp();
    Reanimation*            AttachBlinkAnim(Reanimation* theReanimBody);
    void                    UpdateReanimColor();
    bool                    IsUpgradableTo(SeedType theUpgradedType);
    bool                    IsPartOfUpgradableTo(SeedType theUpgradedType);
    void                    UpdateCobCannon();
    void                    CobCannonFire(int theTargetX, int theTargetY);
    void                    UpdateGoldMagnetShroom();
    /*inline*/ bool         IsOnBoard();
    void                    RemoveEffects();
    void                    UpdateCoffeeBean();
    void                    UpdateUmbrella();
    void                    EndBlink();
    void                    AnimateGarlic();
    Coin*                   FindGoldMagnetTarget();
    void                    SpikeweedAttack();
    void                    ImitaterMorph();
    void                    UpdateImitater();
    void                    UpdateReanim();
    // ===== UpdateReanim 小函数（提取自 UpdateReanim）=====
    void                    CalcReanimBaseTransform(float& aOffsetX, float& aOffsetY, float& aScaleX, float& aScaleY);  // 计算基础偏移/缩放（按植物类型/状态）
    void                    ApplyPottedPlantTransform(float& aOffsetX, float& aOffsetY, float& aScaleX, float& aScaleY);  // 盆栽植物偏移/缩放/生长动画
    // ===== UpdateReanim 小函数结束 =====
    void                    SpikeRockTakeDamage();
    bool                    IsSpiky();
    static /*inline*/ void  PreloadPlantResources(SeedType theSeedType);
    /*inline*/ bool         IsInPlay();
    void                    UpdateNeedsFood() { ; }
    void                    PlayIdleAnim(float theRate);
    void                    UpdateFlowerPot();
    void                    UpdateLilypad();
    void                    GoldMagnetFindTargets();
    bool                    IsAGoldMagnetAboutToSuck();
    bool                    DrawMagnetItemsOnTop();
};

float                       PlantDrawHeightOffset(Board* theBoard, Plant* thePlant, SeedType theSeedType, int theCol, int theRow);
float                       PlantFlowerPotHeightOffset(SeedType theSeedType, float theFlowerPotScale);

class PlantDefinition
{
public:
    SeedType                mSeedType;
    Image**                 mPlantImage = nullptr;
    ReanimationType         mReanimationType = ReanimationType::REANIM_NONE;
    int                     mPacketIndex = 0;
    int                     mSeedCost = 0;
    int                     mRefreshTime = 0;
    PlantSubClass           mSubClass = PlantSubClass::SUBCLASS_NORMAL;
    int                     mLaunchRate = 0;
    std::string             mPlantName;   // Mod API: 改为 std::string 以支持运行时修改
    // Mod API: 图鉴显示用字段（自定义植物专用，原版植物仍走资源文件）
    std::string             mAlmanacName;        // 图鉴标题（如 "My Custom Plant"）
    std::string             mAlmanacDescription; // 图鉴描述正文
};
extern PlantDefinition gPlantDefs[SeedType::NUM_SEED_TYPES];  // Mod API: 移除 const

/*inline*/ const PlantDefinition& GetPlantDefinition(SeedType theSeedType);

// Mod API: 动态注册新植物类型，返回分配的 SeedType（>= NUM_SEED_TYPES）
SeedType RegisterPlantDefinition(const PlantDefinition& theDef);

// Mod API: 自定义植物定义的运行时存储（供 AlmanacDialog 查询）
extern std::vector<PlantDefinition> gCustomPlantDefs;
inline int GetCustomPlantCount() { return static_cast<int>(gCustomPlantDefs.size()); }
inline int GetTotalPlantCount() { return static_cast<int>(SeedType::NUM_SEED_TYPES) + GetCustomPlantCount(); }
// 把自定义植物的 SeedType 转换为 gCustomPlantDefs 的索引（0-based）
inline int CustomSeedTypeToIndex(SeedType s) { return static_cast<int>(s) - static_cast<int>(SeedType::NUM_SEED_TYPES); }
inline bool IsCustomSeedType(SeedType s) { return static_cast<int>(s) >= static_cast<int>(SeedType::NUM_SEED_TYPES); }
