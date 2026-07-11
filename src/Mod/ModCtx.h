#pragma once

#include "ModEvent.h"
#include "../ConstEnums.h"  // ZombieType

// Mod 事件上下文：携带事件发生时的"现场信息"
// 设计原则：
//   - 指针仅在事件回调期间有效（不跨帧持有）
//   - 可拦截事件通过 result 字段回传 Mod 的修改意图
//   - 不直接暴露给 Lua，由 LuaBridge 翻译成 Lua 友好的形式
struct ModCtx {
    ModEvent event = ModEvent::COUNT;

    // ====== 通用上下文 ======
    LawnApp*    app    = nullptr;
    Board*      board  = nullptr;

    // ====== 绘制事件 (ON_BOARD_DRAW_HUD) ======
    // Graphics* 指针，仅在绘制事件回调期间有效
    // mod 通过 LuaBridge 的 MT_GRAPHICS 元表访问
    void*       graphics = nullptr;

    // ====== 对象创建/销毁 ======
    // 用 void* 是为了避免引入具体类型头文件（ModBus.cpp 内部会 static_cast）
    // 外部 Mod 代码通过 LuaBridge 拿到强类型 userdata
    void*           object      = nullptr;
    ModObjectType   objectType  = ModObjectType::UNKNOWN;
    int32_t         objectID    = 0;  // DataArray 的 32 位稳定句柄

    // ====== 输入事件 ======
    int32_t     keyCode    = 0;
    int32_t     mouseX     = 0;
    int32_t     mouseY     = 0;
    int32_t     mouseBtn   = 0;

    // ====== 阳光变化 ======
    int32_t     sunDelta   = 0;     // 变化量（正=获得，负=消耗）
    int32_t     sunResult  = 0;     // 变化后的阳光数（可被 Mod 改）

    // ====== 对象更新事件 ======
    class Plant*    plant       = nullptr;

    // ====== 伤害事件 (ON_ZOMBIE_TAKE_DAMAGE_PRE) ======
    Zombie*     zombie      = nullptr;
    int32_t     damage      = 0;
    uint32_t    damageFlags = 0;

    // ====== 弹道撞击 (ON_PROJECTILE_IMPACT_PRE) ======
    Projectile* projectile  = nullptr;

    // ====== 出怪事件 (ON_PICK_ZOMBIE_WAVES_PRE / ON_PICK_ZOMBIE_TYPE_PRE) ======
    int32_t         level       = 0;     // 当前关卡号 (1-50)
    int32_t         waveIndex   = 0;     // 当前波次 (0-based)
    int32_t         zombiePoints= 0;     // 当前波次剩余点数（PickZombieType 时）
    ZombieType      zombieType  = ZombieType::ZOMBIE_INVALID;  // PickZombieType 选中的类型（可被 Mod 替换）

    // ====== 可拦截事件结果 ======
    // Mod 通过设置这些字段来修改游戏行为
    bool        cancel      = false;  // true = 跳过原逻辑（如取消伤害、取消波次）
    int32_t     newDamage   = -1;     // >=0 时用此值替换原伤害

    // ====== 波次表覆盖 (ON_PICK_ZOMBIE_WAVES_PRE) ======
    // Mod 返回自定义出怪表时填这些字段，C++ 侧会用它替换默认 mZombiesInWave
    bool        useCustomWaves      = false;   // true = 用 customWaves 替换整张波次表
    int32_t     customNumWaves      = 0;       // 自定义总波数
    // customWaves[wave][i] = ZombieType，每波最多 MAX_ZOMBIES_IN_WAVE 个
    // 用静态数组避免堆分配（MAX_ZOMBIE_WAVES=100, MAX_ZOMBIES_IN_WAVE=50）
    static constexpr int MAX_CUSTOM_WAVES = 100;
    static constexpr int MAX_CUSTOM_PER_WAVE = 50;
    ZombieType  customWaves[MAX_CUSTOM_WAVES][MAX_CUSTOM_PER_WAVE];
    // 每波实际长度（customWaves[i] 的有效元素数）
    int32_t     customWaveLengths[MAX_CUSTOM_WAVES];
};

// 可拦截事件的便捷构造
inline ModCtx MakeCtx(ModEvent e) {
    ModCtx c;
    c.event = e;
    return c;
}
