#include "ModBus.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <vector>

// 仅在 PVZ_MOD_API_ENABLED 时编译实际逻辑
#ifdef PVZ_MOD_API_ENABLED

#include "Lua/LuaRuntime.h"

namespace {

// 监听器函数类型：接收可变的 ctx，可修改 ctx 的 result 字段
using Listener = void(*)(ModCtx&);

struct ListenerEntry {
    Listener    fn;
    int         priority;   // 数字越小越先调用
};

// 每个事件一组监听器
struct EventSlot {
    std::vector<ListenerEntry> listeners;
    bool                       has_listeners = false;
};

EventSlot  g_slots[static_cast<int>(ModEvent::COUNT)];
std::mutex g_mutex;
std::atomic<bool> g_initialized{false};
std::atomic<bool> g_any_listeners{false};

// Lua 监听器统一入口（由 LuaBridge 注册）
// 真正的实现分散在各 Bind*.cpp 中，这里只做调度
void LuaListenerDispatch(ModCtx& ctx) {
    ModLua::DispatchEvent(ctx);
}

// 注册 Lua 作为单一监听器（在 Initialize 中调用）
void RegisterLuaListener() {
    std::lock_guard<std::mutex> lk(g_mutex);
    for (int i = 0; i < static_cast<int>(ModEvent::COUNT); ++i) {
        auto& slot = g_slots[i];
        slot.listeners.push_back({LuaListenerDispatch, 100}); // Lua 最后调用
        slot.has_listeners = true;
    }
    g_any_listeners.store(true, std::memory_order_release);
}

} // namespace

namespace ModBus {

void Initialize() {
    if (g_initialized.load(std::memory_order_acquire)) return;

    // 初始化 Lua 运行时 + 加载所有 mod
    ModLua::Initialize();

    // 把 Lua 注册为所有事件的监听器
    RegisterLuaListener();

    g_initialized.store(true, std::memory_order_release);

    // 触发 ON_APP_INIT_POST（虽然此时 Board 还没创建）
    ModCtx ctx = MakeCtx(ModEvent::ON_APP_INIT_POST);
    ctx.app = gLawnApp;
    Fire(ctx.event, ctx);
}

void Shutdown() {
    if (!g_initialized.load(std::memory_order_acquire)) return;

    // 清空监听器
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        for (int i = 0; i < static_cast<int>(ModEvent::COUNT); ++i) {
            g_slots[i].listeners.clear();
            g_slots[i].has_listeners = false;
        }
        g_any_listeners.store(false, std::memory_order_release);
    }

    ModLua::Shutdown();
    g_initialized.store(false, std::memory_order_release);
}

bool HasListeners() {
    return g_any_listeners.load(std::memory_order_acquire);
}

bool HasListenersFor(ModEvent e) {
    int idx = static_cast<int>(e);
    if (idx < 0 || idx >= static_cast<int>(ModEvent::COUNT)) return false;
    // has_listeners 不用加锁（容忍瞬时不一致，最坏多调用一次空 Lua dispatch）
    return g_slots[idx].has_listeners;
}

void Fire(ModEvent event, ModCtx& ctx) {
    int idx = static_cast<int>(event);
    if (idx < 0 || idx >= static_cast<int>(ModEvent::COUNT)) return;

    // 取监听器快照（避免回调中加锁）
    std::vector<ListenerEntry> snapshot;
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        snapshot = g_slots[idx].listeners;
    }

    for (auto& entry : snapshot) {
        try {
            entry.fn(ctx);
        } catch (const std::exception& e) {
            // 异常隔离：记录但不崩游戏
            std::fprintf(stderr, "[ModBus] Listener exception on %s: %s\n",
                         ModEventName(event), e.what());
        } catch (...) {
            std::fprintf(stderr, "[ModBus] Unknown listener exception on %s\n",
                         ModEventName(event));
        }
    }
}

} // namespace ModBus

#endif // PVZ_MOD_API_ENABLED
