#pragma once

#include "ModCtx.h"

// ModBus: Mod 事件总线（间谍层入口）
//
// 设计目标：
//   1. 原代码每个 hook 点只加 1 行: ModBus::Fire(event, ctx);
//   2. 内部多播给所有已注册监听器（LuaBridge 是其中之一）
//   3. 监听器抛异常时被 catch，记日志不崩游戏
//   4. 当没有监听器时，Fire 几乎零开销（一次原子读）
//   5. 可通过 PVZ_MOD_API 编译选项完全禁用（宏 MOD_API_ENABLED）
//
// 用法：
//   // 在原代码中：
//   void Board::Update() {
//       MOD_HOOK(ModEvent::ON_BOARD_UPDATE_PRE, ctx.board = this);
//       // ... 原逻辑
//       MOD_HOOK(ModEvent::ON_BOARD_UPDATE_POST, ctx.board = this);
//   }
//
//   // 可拦截事件：
//   void Zombie::TakeDamage(int dmg, uint flags) {
//       ModCtx ctx = MakeCtx(ModEvent::ON_ZOMBIE_TAKE_DAMAGE_PRE);
//       ctx.zombie = this; ctx.damage = dmg; ctx.damageFlags = flags;
//       ModBus::Fire(ctx.event, ctx);
//       if (ctx.cancel) return;             // Mod 取消
//       if (ctx.newDamage >= 0) dmg = ctx.newDamage;
//       // ... 原伤害逻辑用 dmg
//   }

namespace ModBus {

#ifdef PVZ_MOD_API_ENABLED
// ====== PVZ_MOD_API=ON: 实际实现（在 ModBus.cpp 中）======

// 伤害来源传递（thread_local，避免修改 Zombie::TakeDamage 签名）
// 在调用 TakeDamage 前用 DamageSourceGuard 设置来源，
// TakeDamage 内部派发 ON_ZOMBIE_TAKE_DAMAGE_PRE 事件时读取并填入 ModCtx。
// 作用域结束后自动恢复上一级来源（支持嵌套调用，如溅射伤害）。
struct DamageSource {
    class Zombie*      zombie      = nullptr;  // 僵尸互伤时的攻击者
    class Plant*       plant       = nullptr;  // 植物直接伤害时的攻击者
    class Projectile*  projectile  = nullptr;  // 投射物伤害时的来源
};
extern thread_local DamageSource g_damageSource;

// RAII guard：设置当前伤害来源，作用域结束时恢复
// 用法：{ DamageSourceGuard g(nullptr, plantPtr, nullptr); zombie->TakeDamage(...); }
struct DamageSourceGuard {
    DamageSource prev;
    DamageSourceGuard(Zombie* z = nullptr, Plant* p = nullptr, Projectile* pr = nullptr);
    ~DamageSourceGuard();
};
// 初始化（在 LawnApp::Init 末尾调用）
// 会触发 ModLoader 扫描 mods/ 目录、加载所有 mod
void Initialize();

// 关闭（在 LawnApp::Shutdown 调用）
void Shutdown();

// 当前是否已初始化且至少有一个监听器
bool HasListeners();

// 检查特定事件是否有监听器（用于性能优化：高频事件可先检查再构造 ctx）
bool HasListenersFor(ModEvent e);

// 触发事件（同步调用所有监听器）
// ctx 在调用期间被监听器读写；可拦截事件监听器可修改 ctx 的 result 字段
void Fire(ModEvent event, ModCtx& ctx);

#else
// ====== PVZ_MOD_API=OFF: 空实现，编译期零开销 ======
// 所有调用在编译期被优化掉，不会产生任何函数调用
inline void Initialize() {}
inline void Shutdown() {}
inline bool HasListeners() { return false; }
inline bool HasListenersFor(ModEvent) { return false; }
inline void Fire(ModEvent, ModCtx&) {}
#endif

} // namespace ModBus

// ====== 便捷宏 ======
#ifdef PVZ_MOD_API_ENABLED
    #define MOD_HOOK(event, init_expr) do { \
        if (ModBus::HasListenersFor(event)) { \
            ModCtx _ctx = MakeCtx(event); \
            _ctx.app = gLawnApp; \
            (void)(init_expr); \
            ModBus::Fire(event, _ctx); \
        } \
    } while(0)
#else
    #define MOD_HOOK(event, init_expr) ((void)0)
#endif
