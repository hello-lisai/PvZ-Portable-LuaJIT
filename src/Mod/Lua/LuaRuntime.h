#pragma once

#include "../ModCtx.h"
#include "../../SexyAppFramework/graphics/Image.h"

#include <string>
#include <vector>

// Lua 运行时管理
//
// 职责：
//   1. 创建/销毁 Lua state
//   2. 注册所有 C++ → Lua 的绑定（Board/Zombie/Plant/...）
//   3. 把 C++ 的 ModCtx 事件翻译成 Lua 回调调用
//   4. 加载 mods/ 目录下的所有 mod
//
// 沙箱策略（用户明确要求"方便篡改"，因此宽松）：
//   - 保留 io / os / math / string / table / debug 等所有标准库
//   - 不限制文件访问范围（Mod 可读写任意路径）
//   - 但仍保留异常隔离：Lua 错误不崩游戏，记日志后继续
namespace ModLua {

// 初始化：创建 lua_State、注册绑定、加载 mods/
void Initialize();

// 关闭：销毁 lua_State
void Shutdown();

// 重新加载所有 mod（运行时热重载，按 ~ 键触发）
void Reload();

// 分发事件给 Lua（被 ModBus 调用）
// 内部会根据 ctx.event 查找对应的 Lua 回调并调用
void DispatchEvent(ModCtx& ctx);

// 在 Lua 控制台执行一段代码（调试用，PVZ_DEBUG 下按 ~ 打开）
// 返回执行结果（成功/失败 + 错误信息）
std::string DoString(const std::string& code);

// 获取已加载的 mod 列表（用于 UI 显示）
struct ModInfo {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::string dir;        // mod 所在目录
    bool        enabled;
    bool        loaded;     // 是否成功加载
    std::string error;      // 加载失败时的错误信息
    int         api_version = 0;   // mod 声明的 API 主版本号（用于兼容性检查）
    int         priority = 100;    // 加载优先级（数字越小越先加载，默认 100）
};
const std::vector<ModInfo>& GetLoadedMods();

// 植物禁用列表（mod 通过 board:disable_seed 设置，选卡界面查询）
bool IsSeedDisabled(int seedType);
void ClearDisabledSeeds();

// 自定义关卡封面图标（mod 通过 pvz.set_challenge_icon 设置，ChallengeScreen 查询）
// 返回 nullptr 表示无自定义图标，使用原版 IMAGE_CHALLENGE_THUMBNAILS
// theGameMode: GameMode 枚举值
Sexy::Image* GetCustomChallengeIcon(int theGameMode);
void SetCustomChallengeIcon(int theGameMode, Sexy::Image* theImage);
void ClearCustomChallengeIcons();

// 自定义关卡显示名称（mod 通过 pvz.set_challenge_name 设置，ChallengeScreen 查询）
// 返回空字符串表示无自定义名称，使用 gChallengeDefs 中的默认名称
// theGameMode: GameMode 枚举值
std::string GetCustomChallengeName(int theGameMode);
void SetCustomChallengeName(int theGameMode, const std::string& theName);
void ClearCustomChallengeNames();

// 自定义 HUD 模式（mod 通过 pvz.set_hud_custom 设置）
// 返回 true 表示该关卡由 mod 自定义绘制 HUD（进度条等），跳过默认 DrawProgressMeter
// theGameMode: GameMode 枚举值
bool IsCustomHudMode(int theGameMode);
void SetCustomHudMode(int theGameMode, bool enabled);
void ClearCustomHudModes();

} // namespace ModLua
