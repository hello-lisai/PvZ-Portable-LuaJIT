#pragma once

#include <cstdint>
#include <string>

// 前向声明，避免引入大量头文件
class Board;
class LawnApp;
class Zombie;
class Plant;
class Projectile;
class Coin;
class GridItem;
class LawnMower;

// Mod API 版本号：当事件契约发生变化时递增
//  - 主版本号变更：不兼容的 API 重构
//  - 次版本号变更：新增事件/字段（向后兼容）
// Mod 在 mod.lua 中声明 api_version，游戏会做兼容性检查
constexpr int MOD_API_VERSION_MAJOR = 0;
constexpr int MOD_API_VERSION_MINOR = 2;  // v0.2: 出怪系统数据驱动化

// 所有 Mod 事件类型枚举
// 命名约定: ON_<对象>_<动作>[_PRE|POST]
//   _PRE  = 可拦截事件（Mod 可修改参数/取消）
//   _POST = 只读事件（仅观察）
enum class ModEvent : int32_t {
    // ====== 生命周期 ======
    ON_APP_INIT_POST,           // LawnApp::Init 完成后（Mod 加载时机）
    ON_LOADING_COMPLETED,       // 加载完成进入主菜单
    ON_BOARD_UPDATE_PRE,        // Board::Update 入口（每帧）
    ON_BOARD_UPDATE_POST,       // Board::Update 末尾（每帧）
    ON_UPDATE_GAME_OBJECTS_PRE, // Board::UpdateGameObjects 入口

    // ====== 对象更新预钩子（可拦截，Mod 可完全接管 Update）======
    ON_ZOMBIE_UPDATE_PRE,       // Zombie::Update 入口，可取消（Mod 自行控制行为）
    ON_PLANT_UPDATE_PRE,        // Plant::Update 入口，可取消

    // ====== 对象创建（只读观察）======
    ON_PLANT_CREATED,           // Plant 创建后
    ON_ZOMBIE_CREATED,          // Zombie 创建后
    ON_PROJECTILE_CREATED,      // Projectile 创建后
    ON_COIN_CREATED,            // Coin 创建后
    ON_GRIDITEM_CREATED,        // GridItem 创建后
    ON_LAWNMOWER_CREATED,       // LawnMower 创建后

    // ====== 对象销毁（统一在 ProcessDeleteQueue）======
    ON_OBJECT_DESTROYED,        // 任意对象销毁前（带类型标记）

    // ====== 可拦截事件 ======
    ON_ZOMBIE_TAKE_DAMAGE_PRE,  // Zombie::TakeDamage 入口，可改伤害值
    ON_PROJECTILE_IMPACT_PRE,   // Projectile::DoImpact 入口，可改效果
    ON_SPAWN_ZOMBIE_WAVE_PRE,   // Board::SpawnZombieWave 入口，可改波次
    ON_PICK_ZOMBIE_WAVES_PRE,   // Board::PickZombieWaves 入口，可覆盖整张波次表
    ON_PICK_ZOMBIE_TYPE_PRE,    // Board::PickZombieType 入口，可替换选中的僵尸类型
    ON_PLANT_DIE_PRE,           // Plant::Die 入口，可取消（ctx.cancel=true 让植物免死）
    ON_PLANT_TAKE_DAMAGE_PRE,   // Zombie::EatPlant 入口，可改伤害值（ctx.newDamage）

    // ====== 关卡 ======
    ON_LEVEL_INIT_POST,         // Board::InitLevel 末尾
    ON_LEVEL_START_POST,        // Board::StartLevel 末尾
    ON_LEVEL_END,               // Board::ZombiesWon 入口（关卡失败）

    // ====== 输入 ======
    ON_KEY_DOWN_PRE,            // Board::KeyDown 入口（可拦截）
    ON_MOUSE_DOWN_PRE,          // Board::MouseDown 入口
    ON_MOUSE_UP_PRE,            // Board::MouseUp 入口

    // ====== 经济 ======
    ON_SUN_CHANGED,             // 阳光变化（AddSunMoney/TakeSunMoney）

    // ====== 绘制（只读 POST 事件，mod 可在 HUD 上画自定义内容）======
    ON_BOARD_DRAW_HUD,          // Board::DrawUITop 末尾，ctx.graphics 指向 Graphics*

    // 事件总数（用于静态数组大小）
    COUNT,
};

// 对象类型标记（用于 ON_OBJECT_DESTROYED）
enum class ModObjectType : int32_t {
    UNKNOWN = 0,
    PLANT,
    ZOMBIE,
    PROJECTILE,
    COIN,
    GRIDITEM,
    LAWNMOWER,
};

// 将事件名转为字符串（调试日志用）
inline const char* ModEventName(ModEvent e) {
    switch (e) {
    case ModEvent::ON_APP_INIT_POST:           return "ON_APP_INIT_POST";
    case ModEvent::ON_LOADING_COMPLETED:       return "ON_LOADING_COMPLETED";
    case ModEvent::ON_BOARD_UPDATE_PRE:        return "ON_BOARD_UPDATE_PRE";
    case ModEvent::ON_BOARD_UPDATE_POST:           return "ON_BOARD_UPDATE_POST";
    case ModEvent::ON_UPDATE_GAME_OBJECTS_PRE: return "ON_UPDATE_GAME_OBJECTS_PRE";
    case ModEvent::ON_ZOMBIE_UPDATE_PRE:           return "ON_ZOMBIE_UPDATE_PRE";
    case ModEvent::ON_PLANT_UPDATE_PRE:            return "ON_PLANT_UPDATE_PRE";
    case ModEvent::ON_PLANT_CREATED:           return "ON_PLANT_CREATED";
    case ModEvent::ON_ZOMBIE_CREATED:          return "ON_ZOMBIE_CREATED";
    case ModEvent::ON_PROJECTILE_CREATED:      return "ON_PROJECTILE_CREATED";
    case ModEvent::ON_COIN_CREATED:            return "ON_COIN_CREATED";
    case ModEvent::ON_GRIDITEM_CREATED:        return "ON_GRIDITEM_CREATED";
    case ModEvent::ON_LAWNMOWER_CREATED:       return "ON_LAWNMOWER_CREATED";
    case ModEvent::ON_OBJECT_DESTROYED:        return "ON_OBJECT_DESTROYED";
    case ModEvent::ON_ZOMBIE_TAKE_DAMAGE_PRE:  return "ON_ZOMBIE_TAKE_DAMAGE_PRE";
    case ModEvent::ON_PROJECTILE_IMPACT_PRE:   return "ON_PROJECTILE_IMPACT_PRE";
    case ModEvent::ON_SPAWN_ZOMBIE_WAVE_PRE:   return "ON_SPAWN_ZOMBIE_WAVE_PRE";
    case ModEvent::ON_PICK_ZOMBIE_WAVES_PRE:   return "ON_PICK_ZOMBIE_WAVES_PRE";
    case ModEvent::ON_PICK_ZOMBIE_TYPE_PRE:    return "ON_PICK_ZOMBIE_TYPE_PRE";
    case ModEvent::ON_PLANT_DIE_PRE:           return "ON_PLANT_DIE_PRE";
    case ModEvent::ON_PLANT_TAKE_DAMAGE_PRE:   return "ON_PLANT_TAKE_DAMAGE_PRE";
    case ModEvent::ON_LEVEL_INIT_POST:         return "ON_LEVEL_INIT_POST";
    case ModEvent::ON_LEVEL_START_POST:        return "ON_LEVEL_START_POST";
    case ModEvent::ON_LEVEL_END:               return "ON_LEVEL_END";
    case ModEvent::ON_KEY_DOWN_PRE:            return "ON_KEY_DOWN_PRE";
    case ModEvent::ON_MOUSE_DOWN_PRE:          return "ON_MOUSE_DOWN_PRE";
    case ModEvent::ON_MOUSE_UP_PRE:            return "ON_MOUSE_UP_PRE";
    case ModEvent::ON_SUN_CHANGED:             return "ON_SUN_CHANGED";
    case ModEvent::ON_BOARD_DRAW_HUD:          return "ON_BOARD_DRAW_HUD";
    default: return "UNKNOWN";
    }
}
