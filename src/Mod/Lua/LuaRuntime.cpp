#include "LuaRuntime.h"

#include "../ModBus.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// Lua C API：通过 LuaBindUtil.h 统一入口引入（含 LuaJIT 兼容宏 lua_absindex / lua_isinteger）
// LuaBindUtil.h 已在 extern "C" 块内 include 了 lua.h / lauxlib.h / lualib.h
#include "LuaBindUtil.h"
#include "../Export/PvzExport.h"

// 极简 JSON 读写（header-only）
#include "../Utils/MiniJson.h"

// 项目头文件：用于获取 mods/ 目录路径
#include "../../LawnApp.h"
#include "../../SexyAppFramework/SexyAppBase.h"
#include "../../Sexy.TodLib/TodCommon.h"  // ClampInt
#include "../../Lawn/Challenge.h"        // gZombieWaves / gZombieAllowedLevels
#include "../../Lawn/Board.h"            // MAX_ZOMBIE_WAVES / MAX_ZOMBIES_IN_WAVE
#include "../../Lawn/Plant.h"           // PlantDefinition / RegisterPlantDefinition
#include "../../SexyAppFramework/paklib/PakInterface.h"  // gPakInterface / AddModOverlayDir
#include "../../Lawn/Zombie.h"          // ZombieDefinition / RegisterZombieDefinition
#include "../../Sexy.TodLib/Reanimator.h"  // ReanimatorRegisterAnimation
#include "../../Sexy.TodLib/Definition.h" // DefinitionClearCompiledCache
#include "../../SexyAppFramework/graphics/Graphics.h"  // Graphics（ON_BOARD_DRAW_HUD 事件）
#include "../../Resources.h"            // FONT_* / IMAGE_* 全局指针（pvz.fonts / pvz.images）
#include "../../SexyAppFramework/widget/Widget.h"  // Widget 系统暴露
#include "../../SexyAppFramework/widget/WidgetManager.h"   // WidgetManager

// 对象绑定（在各自 Bind*.cpp 中实现）
namespace ModLua {
    void BindBoard(lua_State* L);
    void BindZombie(lua_State* L);
    void BindPlant(lua_State* L);
    void BindProjectile(lua_State* L);
    void BindReanimation(lua_State* L);
    void BindCoin(lua_State* L);
    void BindGridItem(lua_State* L);
    void BindLawnMower(lua_State* L);
    void BindGraphics(lua_State* L);   // Graphics/Image/Font 元表
    void BindEnums(lua_State* L);  // ZombieType / SeedType / KeyCode 等
    void BindSol2(lua_State* L);   // sol2 批量绑定（在所有 Bind* 之后调用，覆盖旧 metatable）
    void BindWidget(lua_State* L); // Widget 系统绑定

    // 自定义类型 Lua 回调派发（由 Zombie.cpp / Plant.cpp 在 Update 中调用）
    void CallLuaZombieUpdate(Zombie* z);
    void CallLuaPlantUpdate(Plant* p);

    // 把 C++ 对象 push 成 Lua userdata（各 Bind*.cpp 提供）
    void PushBoard(lua_State* L, Board* board);
    void PushZombie(lua_State* L, Zombie* z);
    void PushPlant(lua_State* L, Plant* p);
    void PushProjectile(lua_State* L, Projectile* p);
    void PushReanimation(lua_State* L, Reanimation* r);
    void PushCoin(lua_State* L, Coin* c);
    void PushGridItem(lua_State* L, GridItem* g);
    void PushLawnMower(lua_State* L, LawnMower* m);
    void PushGraphics(lua_State* L, Graphics* g);  // ON_BOARD_DRAW_HUD 事件用
    void PushFont(lua_State* L, _Font* f);          // pvz.fonts 表用
    void PushImage(lua_State* L, Image* img);        // pvz.images 表用
}

namespace {

lua_State* g_L = nullptr;
// 使用递归锁：Lua 回调中可能调用 C API（如 board:add_zombie）触发新事件，
// 新事件的 DispatchEvent 会在同一线程上再次获取锁，std::mutex 会死锁。
// std::recursive_mutex 允许同一线程多次加锁，避免此问题。
std::recursive_mutex g_lua_mutex;

std::vector<ModLua::ModInfo> g_mods;

// 每个 mod 加载后返回的 table，存在 registry 里
// key = mod 索引（0-based），value = mod 的返回 table
// 同时维护一个"事件名 → 所有 mod 的回调函数"的缓存
struct ModCallbackEntry {
    int         ref;        // Lua 函数引用
    std::string modDir;     // 该回调所属的 mod 目录（用于 pvz.config 隔离）
};
struct ModCallbacks {
    // 对每个事件名，存所有 mod 注册的回调函数
    std::unordered_map<std::string, std::vector<ModCallbackEntry>> callbacks;
};
ModCallbacks g_callbacks;

// 自定义僵尸/植物类型的 Lua Update 回调引用
// key = 类型索引 (ZombieType / SeedType 的数值)
// value = Lua registry reference（on_update 函数）
std::unordered_map<int, int> g_zombieUpdateCallbacks;
std::unordered_map<int, int> g_plantUpdateCallbacks;

// 用户配置持久化：mods/config.json
// 结构: { "version": 1, "mods": { "<dir>": { "enabled": bool, "config": {...} } } }
ModJson::JsonValue g_userConfig;
std::string        g_configPath;

// pvz.images 动态查询表：key → 指向全局 Image* 指针的地址
// 资源在异步线程 LoadingThreadProc 中加载，ModBus::Initialize 时机早于资源加载完成，
// 因此不能在 mod 初始化时缓存 image 指针，必须每次访问时实时读取全局变量。
std::unordered_map<std::string, Image**> g_imageMap;

// pvz.images 表的 __index 元方法：根据 key 实时读取对应全局 Image* 指针
int l_images_index(lua_State* L) {
    const char* key = luaL_checkstring(L, 2);
    auto it = g_imageMap.find(key);
    if (it == g_imageMap.end() || it->second == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    Image* img = *(it->second);  // 读取当前指针值（资源加载后会被赋值）
    if (img) {
        ModLua::PushImage(L, img);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// ModEvent → Lua 回调名映射
const char* EventToLuaName(ModEvent e) {
    switch (e) {
    case ModEvent::ON_APP_INIT_POST:           return "on_app_init";
    case ModEvent::ON_LOADING_COMPLETED:       return "on_loading_completed";
    case ModEvent::ON_BOARD_UPDATE_PRE:        return "on_board_update_pre";
    case ModEvent::ON_BOARD_UPDATE_POST:       return "on_board_update_post";
    case ModEvent::ON_UPDATE_GAME_OBJECTS_PRE: return "on_update_game_objects_pre";
    case ModEvent::ON_ZOMBIE_UPDATE_PRE:       return "on_zombie_update_pre";
    case ModEvent::ON_PLANT_UPDATE_PRE:        return "on_plant_update_pre";
    case ModEvent::ON_PLANT_CREATED:           return "on_plant_created";
    case ModEvent::ON_ZOMBIE_CREATED:          return "on_zombie_created";
    case ModEvent::ON_PROJECTILE_CREATED:      return "on_projectile_created";
    case ModEvent::ON_COIN_CREATED:            return "on_coin_created";
    case ModEvent::ON_GRIDITEM_CREATED:        return "on_griditem_created";
    case ModEvent::ON_LAWNMOWER_CREATED:       return "on_lawnmower_created";
    case ModEvent::ON_OBJECT_DESTROYED:        return "on_object_destroyed";
    case ModEvent::ON_ZOMBIE_TAKE_DAMAGE_PRE:  return "on_zombie_take_damage";
    case ModEvent::ON_PROJECTILE_IMPACT_PRE:   return "on_projectile_impact";
    case ModEvent::ON_SPAWN_ZOMBIE_WAVE_PRE:   return "on_spawn_zombie_wave";
    case ModEvent::ON_PICK_ZOMBIE_WAVES_PRE:   return "on_pick_zombie_waves";
    case ModEvent::ON_PICK_ZOMBIE_WAVES_POST:  return "on_pick_zombie_waves_post";
    case ModEvent::ON_PICK_ZOMBIE_TYPE_PRE:    return "on_pick_zombie_type";
    case ModEvent::ON_PLANT_DIE_PRE:           return "on_plant_die";
    case ModEvent::ON_PLANT_TAKE_DAMAGE_PRE:   return "on_plant_take_damage";
    case ModEvent::ON_LEVEL_INIT_POST:         return "on_level_init";
    case ModEvent::ON_LEVEL_START_POST:        return "on_level_start";
    case ModEvent::ON_LEVEL_END:               return "on_level_end";
    case ModEvent::ON_KEY_DOWN_PRE:            return "on_key_down";
    case ModEvent::ON_MOUSE_DOWN_PRE:          return "on_mouse_down";
    case ModEvent::ON_MOUSE_UP_PRE:            return "on_mouse_up";
    case ModEvent::ON_SUN_CHANGED:             return "on_sun_changed";
    case ModEvent::ON_BOARD_DRAW_HUD:          return "on_board_draw_hud";
    default: return nullptr;
    }
}

// 安全调用 Lua 函数：把错误转成字符串记日志，不抛 C++ 异常
bool SafePCall(lua_State* L, int nargs, int nresults) {
    if (lua_pcall(L, nargs, nresults, 0) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        std::fprintf(stderr, "[ModLua] %s\n", err ? err : "(unknown error)");
        std::fflush(stderr);
        lua_pop(L, 1);
        return false;
    }
    return true;
}

// 读取文件内容到字符串
bool ReadFile(const std::string& path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

// 获取 mods/ 目录绝对路径
std::string GetModsDir() {
    // 优先用资源目录（与可执行同级）
    if (gLawnApp && !gLawnApp->mResourceDir.empty()) {
        return gLawnApp->mResourceDir + "mods/";
    }
    return "mods/";
}

// 取路径的 basename（用于做 config.json 的 key）
// 兼容 Windows 反斜杠和 Unix 正斜杠
std::string ModDirBasename(const std::string& dir) {
    size_t pos = dir.find_last_of("/\\");
    return pos == std::string::npos ? dir : dir.substr(pos + 1);
}

// ============ 配置持久化 ============

// 加载 mods/config.json；不存在或损坏时按空配置处理
void LoadUserConfig() {
    g_configPath = GetModsDir() + "config.json";
    g_userConfig = ModJson::JsonValue::MakeObject({});
    std::string content;
    if (!ReadFile(g_configPath, content)) {
        return; // 不存在，保持空配置
    }
    ModJson::JsonValue parsed;
    if (!ModJson::JsonParse(content, parsed) || !parsed.isObject()) {
        std::fprintf(stderr, "[ModLua] WARNING: config.json 解析失败，按空配置处理\n");
        return;
    }
    g_userConfig = std::move(parsed);
    // 确保顶层结构完整
    if (!g_userConfig.find("version")) g_userConfig["version"] = ModJson::JsonValue::MakeInt(1);
    if (!g_userConfig.find("mods"))    g_userConfig["mods"]    = ModJson::JsonValue::MakeObject({});
}

// 保存 config.json 到磁盘
void SaveUserConfig() {
    if (g_configPath.empty()) return;
    std::string content = ModJson::JsonDump(g_userConfig);
    std::ofstream f(g_configPath, std::ios::binary);
    if (!f) {
        std::fprintf(stderr, "[ModLua] WARNING: 无法写入 %s\n", g_configPath.c_str());
        return;
    }
    f << content;
}

// 从 g_userConfig 读取某 mod 的 enabled，不存在返回 def
bool GetModEnabled(const std::string& modDir, bool def) {
    auto* mods = g_userConfig.find("mods");
    if (!mods) return def;
    auto* entry = mods->find(ModDirBasename(modDir));
    if (!entry) return def;
    auto* enabled = entry->find("enabled");
    return enabled ? enabled->asBool(def) : def;
}

// 写入某 mod 的 enabled（用于回写）
void SetModEnabled(const std::string& modDir, bool enabled) {
    auto& mods = g_userConfig["mods"];
    if (!mods.isObject()) mods = ModJson::JsonValue::MakeObject({});
    auto& entry = mods[ModDirBasename(modDir)];
    if (!entry.isObject()) entry = ModJson::JsonValue::MakeObject({});
    entry["enabled"] = ModJson::JsonValue::MakeBool(enabled);
}

// 获取某 mod 的 config 子对象（可写引用，不存在则创建）
ModJson::JsonValue& GetModConfig(const std::string& modDir) {
    auto& mods = g_userConfig["mods"];
    if (!mods.isObject()) mods = ModJson::JsonValue::MakeObject({});
    auto& entry = mods[ModDirBasename(modDir)];
    if (!entry.isObject()) entry = ModJson::JsonValue::MakeObject({});
    if (!entry.find("config")) {
        entry["config"] = ModJson::JsonValue::MakeObject({});
    }
    return entry["config"];
}

// ============ JsonValue <-> Lua 转换 ============

void PushJsonValue(lua_State* L, const ModJson::JsonValue& v) {
    switch (v.type()) {
    case ModJson::JsonValue::Type::Null:    lua_pushnil(L); break;
    case ModJson::JsonValue::Type::Bool:    lua_pushboolean(L, v.asBool()); break;
    case ModJson::JsonValue::Type::Int:     lua_pushinteger(L, v.asInt()); break;
    case ModJson::JsonValue::Type::Double:  lua_pushnumber(L, v.asDouble()); break;
    case ModJson::JsonValue::Type::String:  lua_pushstring(L, v.asString().c_str()); break;
    case ModJson::JsonValue::Type::Array: {
        lua_newtable(L);
        int i = 1;
        for (auto& item : v.asArray()) {
            PushJsonValue(L, item);
            lua_rawseti(L, -2, i++);
        }
        break;
    }
    case ModJson::JsonValue::Type::Object: {
        lua_newtable(L);
        for (auto& [k, val] : v.asObject()) {
            PushJsonValue(L, val);
            lua_setfield(L, -2, k.c_str());
        }
        break;
    }
    }
}

ModJson::JsonValue LuaToJsonValue(lua_State* L, int idx) {
    idx = lua_absindex(L, idx);  // 转成绝对索引，避免递归时栈变化导致相对索引失效
    switch (lua_type(L, idx)) {
    case LUA_TNIL:     return ModJson::JsonValue{};
    case LUA_TBOOLEAN: return ModJson::JsonValue::MakeBool(lua_toboolean(L, idx) != 0);
    case LUA_TNUMBER:
        if (lua_isinteger(L, idx))
            return ModJson::JsonValue::MakeInt(lua_tointeger(L, idx));
        return ModJson::JsonValue::MakeDouble(lua_tonumber(L, idx));
    case LUA_TSTRING:
        return ModJson::JsonValue::MakeString(lua_tostring(L, idx));
    case LUA_TTABLE: {
        // 统一按 object 处理（字符串键）；整数键跳过以避免歧义
        ModJson::JsonObject obj;
        lua_pushnil(L);
        while (lua_next(L, idx)) {
            if (lua_type(L, -2) == LUA_TSTRING) {
                obj[lua_tostring(L, -2)] = LuaToJsonValue(L, -1);
            }
            lua_pop(L, 1);
        }
        return ModJson::JsonValue::MakeObject(std::move(obj));
    }
    default:
        return ModJson::JsonValue{}; // 不支持的类型静默转 null
    }
}

// 读取全局 __MOD_DIR__（在 LoadMod / DispatchEvent 中设置）
std::string GetCurrentModDir() {
    lua_getglobal(g_L, "__MOD_DIR__");
    std::string dir;
    if (lua_isstring(g_L, -1)) dir = lua_tostring(g_L, -1);
    lua_pop(g_L, 1);
    return dir;
}

// pvz.config.get(key) → 当前 mod 的配置值，不存在返回 nil
int l_config_get(lua_State* L) {
    std::string modDir = GetCurrentModDir();
    if (modDir.empty()) { lua_pushnil(L); return 1; }
    const char* key = luaL_checkstring(L, 1);
    auto& config = GetModConfig(modDir);
    auto* v = config.find(key);
    PushJsonValue(L, v ? *v : ModJson::JsonValue{});
    return 1;
}

// pvz.config.set(key, value) → 立即持久化到 config.json
int l_config_set(lua_State* L) {
    std::string modDir = GetCurrentModDir();
    if (modDir.empty()) return 0;
    const char* key = luaL_checkstring(L, 1);
    GetModConfig(modDir)[key] = LuaToJsonValue(L, 2);
    SaveUserConfig();
    return 0;
}

// pvz.config.all() → 返回当前 mod 的全部配置（table）
int l_config_all(lua_State* L) {
    std::string modDir = GetCurrentModDir();
    if (modDir.empty()) { lua_newtable(L); return 1; }
    PushJsonValue(L, GetModConfig(modDir));
    return 1;
}

// =============================================================================
// pvz.offset_of.* —— 内存偏移查询 API（第二步：直接读写内存）
// mod 用 ffi.cast("char*", ptr) + offset 直接读写对象字段，绕过所有 getter/setter
// =============================================================================

// pvz.offset_of.zombie(field_name) → 返回字段在 Zombie 对象内的字节偏移，-1 表示未知
int l_offset_of_zombie(lua_State* L) {
    const char* field = luaL_checkstring(L, 1);
    lua_pushinteger(L, pvz_offset_of_zombie(field));
    return 1;
}

int l_offset_of_plant(lua_State* L) {
    const char* field = luaL_checkstring(L, 1);
    lua_pushinteger(L, pvz_offset_of_plant(field));
    return 1;
}

int l_offset_of_board(lua_State* L) {
    const char* field = luaL_checkstring(L, 1);
    lua_pushinteger(L, pvz_offset_of_board(field));
    return 1;
}

int l_offset_of_projectile(lua_State* L) {
    const char* field = luaL_checkstring(L, 1);
    lua_pushinteger(L, pvz_offset_of_projectile(field));
    return 1;
}

int l_offset_of_coin(lua_State* L) {
    const char* field = luaL_checkstring(L, 1);
    lua_pushinteger(L, pvz_offset_of_coin(field));
    return 1;
}

int l_offset_of_mower(lua_State* L) {
    const char* field = luaL_checkstring(L, 1);
    lua_pushinteger(L, pvz_offset_of_mower(field));
    return 1;
}

int l_offset_of_griditem(lua_State* L) {
    const char* field = luaL_checkstring(L, 1);
    lua_pushinteger(L, pvz_offset_of_griditem(field));
    return 1;
}

// pvz.field_type_of(object_kind, field_name) → 返回字段类型代号
//   1=int32, 2=int64, 3=float, 4=double, 5=bool, 6=pointer, 0=未知
// object_kind: 0=Zombie, 1=Plant, 2=Board, 3=Projectile, 4=Coin, 5=Mower, 6=GridItem
int l_field_type_of(lua_State* L) {
    int kind = static_cast<int>(luaL_checkinteger(L, 1));
    const char* field = luaL_checkstring(L, 2);
    lua_pushinteger(L, pvz_field_type_of(kind, field));
    return 1;
}

// =============================================================================
// pvz.waves.* —— 出怪系统运行时修改 API
// =============================================================================

// pvz.waves.get_num_waves(level) → 返回该关卡默认总波数
int l_waves_get_num_waves(lua_State* L) {
    int level = static_cast<int>(luaL_checkinteger(L, 1));
    if (level < 1 || level > NUM_LEVELS) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, gZombieWaves[ClampInt(level - 1, 0, NUM_LEVELS - 1)]);
    return 1;
}

// pvz.waves.set_num_waves(level, count) → 修改某关卡的默认总波数
int l_waves_set_num_waves(lua_State* L) {
    int level = static_cast<int>(luaL_checkinteger(L, 1));
    int count = static_cast<int>(luaL_checkinteger(L, 2));
    if (level < 1 || level > NUM_LEVELS) return 0;
    gZombieWaves[ClampInt(level - 1, 0, NUM_LEVELS - 1)] = ClampInt(count, 0, MAX_ZOMBIE_WAVES);
    return 0;
}

// pvz.waves.is_zombie_allowed(zombie_type, level) → 该僵尸类型是否允许在该关卡出现
int l_waves_is_zombie_allowed(lua_State* L) {
    int zt = static_cast<int>(luaL_checkinteger(L, 1));
    int level = static_cast<int>(luaL_checkinteger(L, 2));
    if (zt < 0 || zt >= NUM_ZOMBIE_TYPES) { lua_pushboolean(L, false); return 1; }
    int lvlIdx = ClampInt(level - 1, 0, 49);
    lua_pushboolean(L, gZombieAllowedLevels[zt].mAllowedOnLevel[lvlIdx] != 0);
    return 1;
}

// pvz.waves.allow_zombie(zombie_type, level, allowed) → 允许/禁止某僵尸在某关卡出现
int l_waves_allow_zombie(lua_State* L) {
    int zt = static_cast<int>(luaL_checkinteger(L, 1));
    int level = static_cast<int>(luaL_checkinteger(L, 2));
    bool allowed = lua_toboolean(L, 3) != 0;
    if (zt < 0 || zt >= NUM_ZOMBIE_TYPES) return 0;
    int lvlIdx = ClampInt(level - 1, 0, 49);
    gZombieAllowedLevels[zt].mAllowedOnLevel[lvlIdx] = allowed ? 1 : 0;
    return 0;
}

// =============================================================================
// pvz.reanim.register / pvz.plants.register / pvz.zombies.register
// 路线 C+D：动态注册新动画/植物/僵尸
// =============================================================================

// pvz.reanim.register(file_path, flags=0) → 注册新动画类型，返回 ReanimationType
// file_path: .reanim 文件的资源路径（如 "mods/my_mod/reanim/xxx.reanim"）
int l_reanim_register(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int32_t flags = static_cast<int32_t>(luaL_optinteger(L, 2, 0));
    ReanimationType t = ReanimatorRegisterAnimation(path, flags);
    lua_pushinteger(L, static_cast<lua_Integer>(t));
    return 1;
}

// pvz.get_reanimation(reanim_id) → 通过 ReanimationID 获取 Reanimation userdata
// reanim_id: 通常是 zombie.body_reanim_id 或 plant.body_reanim_id 字段值
// 返回 Reanimation userdata 或 nil（ID 无效时）
// 例：local r = pvz.get_reanimation(zombie.body_reanim_id)
//     if r then print(r.current_track_name, r.anim_time) end
int l_get_reanimation(lua_State* L) {
    lua_Integer id = luaL_checkinteger(L, 1);
    if (id == 0 || !gLawnApp) { lua_pushnil(L); return 1; }
    Reanimation* r = gLawnApp->ReanimationTryToGet(static_cast<ReanimationID>(id));
    if (!r) { lua_pushnil(L); return 1; }
    ModLua::PushReanimation(L, r);
    return 1;
}

// pvz.resources.mount(dir) → 挂载额外资源覆盖目录
// dir: 目录路径（绝对路径或相对于资源目录）
// 挂载后，该目录下的文件会优先于 pak 被读取
// 返回 true 表示挂载成功
int l_resources_mount(lua_State* L) {
    const char* dir = luaL_checkstring(L, 1);
    if (gPakInterface) {
        gPakInterface->AddModOverlayDir(dir);
        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

// pvz.resources.unmount(dir) → 移除已挂载的资源覆盖目录
int l_resources_unmount(lua_State* L) {
    const char* dir = luaL_checkstring(L, 1);
    if (gPakInterface) {
        gPakInterface->RemoveModOverlayDir(dir);
    }
    return 0;
}

// pvz.resources.clear_compiled_cache() → 清理编译定义的磁盘缓存
// 更换 main.pak 后调用此函数可强制游戏重新从 pak 读取并编译定义文件
// 注意：仅在游戏启动时（on_app_init）调用有效，关卡运行中调用可能导致已加载的资源状态不一致
// 返回成功删除的文件数量
int l_resources_clear_compiled_cache(lua_State* L) {
    int deleted = DefinitionClearCompiledCache();
    lua_pushinteger(L, deleted);
    return 1;
}

// pvz.plants.register(definition_table) → 注册新植物类型，返回 SeedType
// definition_table 字段：
//   name (string, 必需)         植物名称
//   cost (int, 默认 0)           阳光花费
//   refresh (int, 默认 0)        冷却时间（毫秒）
//   reanim_type (int, 默认 0)    对应的 ReanimationType
//   packet_index (int, 默认 0)   卡包索引（保留字段，当前未使用）
//   subclass (int, 默认 0)       PlantSubClass (0=NORMAL, 1=SHOOTER)
//   launch_rate (int, 默认 0)    发射间隔
//   almanac_name (string, 可选)        图鉴标题
//   almanac_description (string, 可选) 图鉴描述
//   makes_sun (bool, 可选)       是否产阳光
//   seed_sort_order (int, 可选)  选种界面/图鉴排序权重（-1=追加，>=0=按值排序）
//   packet_background_cel (int, 可选) 卡包背景 cel 索引（IMAGE_SEEDS 0-8，-1=自动）
//   packet_scale (float, 可选)   卡包内图标缩放（默认 0.5）
//   packet_offset_x (float, 可选) 卡包内图标 X 偏移（默认 5.0）
//   packet_offset_y (float, 可选) 卡包内图标 Y 偏移（默认 8.0）
//   almanac_track (string, 可选) 图鉴大预览使用的 reanim track（默认空 = 用 anim_idle）
int l_plants_register(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    int tblIdx = lua_absindex(L, 1);

    PlantDefinition def;
    lua_getfield(L, tblIdx, "name");
    if (lua_isstring(L, -1)) def.mPlantName = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "cost");
    if (lua_isinteger(L, -1)) def.mSeedCost = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "refresh");
    if (lua_isinteger(L, -1)) def.mRefreshTime = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "reanim_type");
    if (lua_isinteger(L, -1)) def.mReanimationType = static_cast<ReanimationType>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "packet_index");
    if (lua_isinteger(L, -1)) def.mPacketIndex = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "subclass");
    if (lua_isinteger(L, -1)) def.mSubClass = static_cast<PlantSubClass>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "launch_rate");
    if (lua_isinteger(L, -1)) def.mLaunchRate = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    // Mod API: 图鉴显示用字段（可选）
    lua_getfield(L, tblIdx, "almanac_name");
    if (lua_isstring(L, -1)) def.mAlmanacName = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "almanac_description");
    if (lua_isstring(L, -1)) def.mAlmanacDescription = lua_tostring(L, -1);
    lua_pop(L, 1);

    // Mod API: 自定义植物是否产阳光（MakesSun/UpdateProductionPlant 读取）
    lua_getfield(L, tblIdx, "makes_sun");
    if (lua_isboolean(L, -1)) def.mMakesSun = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);

    // Mod API: 选种界面/图鉴显示排序权重（-1 = 自动追加，>=0 = 按值排序）
    // 例如 seed_sort_order = 49 表示排在第二页首位
    lua_getfield(L, tblIdx, "seed_sort_order");
    if (lua_isinteger(L, -1)) def.mSeedSortOrder = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    // Mod API: 卡包背景样式字段（可选）
    // packet_background_cel: 卡包背景 cel 索引（IMAGE_SEEDS 0-8），-1 = 走原版自动逻辑
    // packet_scale/packet_offset_x/packet_offset_y: 卡包内植物图标的缩放和偏移
    lua_getfield(L, tblIdx, "packet_background_cel");
    if (lua_isinteger(L, -1)) def.mPacketBackgroundCel = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "packet_scale");
    if (lua_isnumber(L, -1)) def.mPacketScale = static_cast<float>(lua_tonumber(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "packet_offset_x");
    if (lua_isnumber(L, -1)) def.mPacketOffsetX = static_cast<float>(lua_tonumber(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "packet_offset_y");
    if (lua_isnumber(L, -1)) def.mPacketOffsetY = static_cast<float>(lua_tonumber(L, -1));
    lua_pop(L, 1);

    // Mod API: 图鉴大预览使用的 reanim track（可选，默认空 = 用 anim_idle）
    lua_getfield(L, tblIdx, "almanac_track");
    if (lua_isstring(L, -1)) def.mAlmanacTrack = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "on_update");
    if (lua_isfunction(L, -1)) {
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        // 暂存：注册返回的 SeedType 在 RegisterPlantDefinition 之后才确定
        // 我们先把 ref 和 def 关联，等拿到 SeedType 再存 map
        SeedType st = RegisterPlantDefinition(def);
        g_plantUpdateCallbacks[static_cast<int>(st)] = ref;
        lua_pushinteger(L, static_cast<lua_Integer>(st));
    } else {
        lua_pop(L, 1); // pop nil/non-function
        SeedType st = RegisterPlantDefinition(def);
        lua_pushinteger(L, static_cast<lua_Integer>(st));
    }
    return 1;
}

// pvz.zombies.register(definition_table) → 注册新僵尸类型，返回 ZombieType
// definition_table 字段：
//   name (string, 必需)              僵尸名称
//   reanim_type (int, 默认 0)        对应的 ReanimationType（pvz.reanim.types.* 或原版整数）
//   value (int, 默认 0)              僵尸价值（用于波次成本预算）
//   starting_level (int, 默认 0)     出场起始关卡
//   first_allowed_wave (int, 默认 0) 首个允许出现的波次
//   pick_weight (int, 默认 0)        随机挑选权重
//   body_health (int, 默认 270)      初始血量
//   abilities (int, 默认 1)          能力位（ABILITY_WALK=1，可组合 |，详见 src/Lawn/Zombie.h ABILITY_*）
//   helm_type (int, 默认 0)          头盔类型（HelmType，详见 src/ConstEnums.h：0=NONE/1=CONE/2=PAIL/3=FOOTBALL 等）
//   helm_health (int, 默认 0)        头盔血量
//   shield_type (int, 默认 0)        护甲类型（ShieldType：0=NONE/1=DOOR/2=NEWSPAPER/3=LADDER）
//   shield_health (int, 默认 0)      护甲血量
//   almanac_name (string, 可选)      图鉴标题
//   almanac_description (string, 可选) 图鉴描述
//   almanac_track (string, 可选)     图鉴大预览使用的 reanim track（默认空 = 自动选 anim_idle2/anim_idle）
//   preview_track (string, 可选)     关卡开始右侧预览使用的 reanim track（默认空 = StartWalkAnim 走路动画）
//   on_update (function, 可选)       逐帧行为回调 function(zombie)
int l_zombies_register(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    int tblIdx = lua_absindex(L, 1);

    ZombieDefinition def;
    lua_getfield(L, tblIdx, "name");
    if (lua_isstring(L, -1)) def.mZombieName = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "reanim_type");
    if (lua_isinteger(L, -1)) def.mReanimationType = static_cast<ReanimationType>(lua_tointeger(L, -1));
    lua_pop(L, 1);
    // Mod API: 默认使用 REANIM_ZOMBIE（普通僵尸动画），避免 mod 忘记指定 reanim_type
    // 导致 MakeCachedZombieFrame 中的 TOD_ASSERT(mReanimationType != REANIM_NONE) 失败
    if (def.mReanimationType == ReanimationType::REANIM_NONE)
        def.mReanimationType = ReanimationType::REANIM_ZOMBIE;

    lua_getfield(L, tblIdx, "value");
    if (lua_isinteger(L, -1)) def.mZombieValue = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "starting_level");
    if (lua_isinteger(L, -1)) def.mStartingLevel = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "first_allowed_wave");
    if (lua_isinteger(L, -1)) def.mFirstAllowedWave = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "pick_weight");
    if (lua_isinteger(L, -1)) def.mPickWeight = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    // Mod API: 自定义僵尸初始化字段（ZombieInitialize 的 default 分支读取）
    lua_getfield(L, tblIdx, "body_health");
    if (lua_isinteger(L, -1)) def.mBodyHealth = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "abilities");
    if (lua_isinteger(L, -1)) def.mAbilities = static_cast<unsigned int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "helm_type");
    if (lua_isinteger(L, -1)) def.mHelmType = static_cast<HelmType>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "helm_health");
    if (lua_isinteger(L, -1)) def.mHelmHealth = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "shield_type");
    if (lua_isinteger(L, -1)) def.mShieldType = static_cast<ShieldType>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "shield_health");
    if (lua_isinteger(L, -1)) def.mShieldHealth = static_cast<int>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    // Mod API: 图鉴显示用字段（可选）
    lua_getfield(L, tblIdx, "almanac_name");
    if (lua_isstring(L, -1)) def.mAlmanacName = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "almanac_description");
    if (lua_isstring(L, -1)) def.mAlmanacDescription = lua_tostring(L, -1);
    lua_pop(L, 1);

    // Mod API: 图鉴大预览使用的 reanim track（可选，默认空 = 走原版自动逻辑 anim_idle2/anim_idle）
    lua_getfield(L, tblIdx, "almanac_track");
    if (lua_isstring(L, -1)) def.mAlmanacTrack = lua_tostring(L, -1);
    lua_pop(L, 1);

    // Mod API: 关卡开始右侧预览使用的 reanim track（可选，默认空 = 走原版 StartWalkAnim 走路动画）
    lua_getfield(L, tblIdx, "preview_track");
    if (lua_isstring(L, -1)) def.mPreviewTrack = lua_tostring(L, -1);
    lua_pop(L, 1);

    // Mod API: 外观基础类型（可选，默认 ZOMBIE_NORMAL）
    // 指定后，断臂/断头等受伤动画会使用该类型对应的 track 名和图像覆盖
    // 例如 base_type = pvz.ZombieType.ZOMBIE_FOOTBALL 时使用橄榄球僵尸的断臂逻辑
    lua_getfield(L, tblIdx, "base_type");
    if (lua_isinteger(L, -1)) def.mBaseZombieType = static_cast<ZombieType>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "on_update");
    if (lua_isfunction(L, -1)) {
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        ZombieType zt = RegisterZombieDefinition(def);
        g_zombieUpdateCallbacks[static_cast<int>(zt)] = ref;
        lua_pushinteger(L, static_cast<lua_Integer>(zt));
    } else {
        lua_pop(L, 1);
        ZombieType zt = RegisterZombieDefinition(def);
        lua_pushinteger(L, static_cast<lua_Integer>(zt));
    }
    return 1;
}

// ===== 自定义类型 Lua Update 回调派发 =====
// 在 UpdateActionsByType 中被调用（自定义僵尸类型不会走 C++ switch）

} // namespace

namespace ModLua {

void CallLuaZombieUpdate(Zombie* z) {
    if (!g_L || !z) return;
    int typeIdx = static_cast<int>(z->mZombieType);
    auto it = g_zombieUpdateCallbacks.find(typeIdx);
    if (it == g_zombieUpdateCallbacks.end()) return;

    lua_rawgeti(g_L, LUA_REGISTRYINDEX, it->second); // push callback
    PushZombie(g_L, z);
    SafePCall(g_L, 1, 0);
}

void CallLuaPlantUpdate(Plant* p) {
    if (!g_L || !p) return;
    int typeIdx = static_cast<int>(p->mSeedType);
    auto it = g_plantUpdateCallbacks.find(typeIdx);
    if (it == g_plantUpdateCallbacks.end()) return;

    lua_rawgeti(g_L, LUA_REGISTRYINDEX, it->second);
    PushPlant(g_L, p);
    SafePCall(g_L, 1, 0);
}

} // namespace ModLua

namespace {

// ===== Widget 绑定 =====

// LuaWidget：把 Sexy::Widget 的子类暴露给 Lua，允许 Mod 创建自定义 UI
// Widget 的 Draw/Update/MouseDown/KeyDown 等方法由 Lua 回调实现
class LuaWidget : public Sexy::Widget {
public:
    int     mOnDrawRef      = LUA_NOREF;
    int     mOnUpdateRef    = LUA_NOREF;
    int     mOnKeyDownRef   = LUA_NOREF;
    int     mOnMouseDownRef = LUA_NOREF;
    int     mOnMouseUpRef   = LUA_NOREF;
    int     mOnMouseMoveRef = LUA_NOREF;
    int     mOnMouseDragRef = LUA_NOREF;
    int     mOnKeyCharRef   = LUA_NOREF;

    void Draw(Sexy::Graphics* g) override {
        Sexy::Widget::Draw(g);
        if (!g_L || mOnDrawRef == LUA_NOREF) return;
        lua_rawgeti(g_L, LUA_REGISTRYINDEX, mOnDrawRef);
        ModLua::PushGraphics(g_L, g);
        SafePCall(g_L, 1, 0);
    }

    void Update() override {
        Sexy::Widget::Update();
        if (!g_L || mOnUpdateRef == LUA_NOREF) return;
        lua_rawgeti(g_L, LUA_REGISTRYINDEX, mOnUpdateRef);
        SafePCall(g_L, 0, 0);
    }

    void KeyDown(Sexy::KeyCode key) override {
        if (!g_L || mOnKeyDownRef == LUA_NOREF) { Sexy::Widget::KeyDown(key); return; }
        lua_rawgeti(g_L, LUA_REGISTRYINDEX, mOnKeyDownRef);
        lua_pushinteger(g_L, static_cast<lua_Integer>(key));
        if (SafePCall(g_L, 1, 1)) {
            if (lua_toboolean(g_L, -1)) return; // return true = 已处理
            lua_pop(g_L, 1);
        }
        Sexy::Widget::KeyDown(key);
    }

    void MouseDown(int x, int y, int btn, int clickCount) override {
        if (!g_L || mOnMouseDownRef == LUA_NOREF) { Sexy::Widget::MouseDown(x, y, btn, clickCount); return; }
        lua_rawgeti(g_L, LUA_REGISTRYINDEX, mOnMouseDownRef);
        lua_pushinteger(g_L, x); lua_pushinteger(g_L, y); lua_pushinteger(g_L, btn);
        if (SafePCall(g_L, 3, 1)) {
            if (lua_toboolean(g_L, -1)) return;
            lua_pop(g_L, 1);
        }
        Sexy::Widget::MouseDown(x, y, btn, clickCount);
    }

    void MouseUp(int x, int y, int btn, int clickCount) override {
        if (!g_L || mOnMouseUpRef == LUA_NOREF) { Sexy::Widget::MouseUp(x, y, btn, clickCount); return; }
        lua_rawgeti(g_L, LUA_REGISTRYINDEX, mOnMouseUpRef);
        lua_pushinteger(g_L, x); lua_pushinteger(g_L, y); lua_pushinteger(g_L, btn);
        if (SafePCall(g_L, 3, 1)) {
            if (lua_toboolean(g_L, -1)) return;
            lua_pop(g_L, 1);
        }
        Sexy::Widget::MouseUp(x, y, btn, clickCount);
    }

    // Mod API: MouseMove — 鼠标在 widget 内移动（未按下）
    void MouseMove(int x, int y) override {
        if (!g_L || mOnMouseMoveRef == LUA_NOREF) { Sexy::Widget::MouseMove(x, y); return; }
        lua_rawgeti(g_L, LUA_REGISTRYINDEX, mOnMouseMoveRef);
        lua_pushinteger(g_L, x); lua_pushinteger(g_L, y);
        if (SafePCall(g_L, 2, 1)) {
            if (lua_toboolean(g_L, -1)) return;
            lua_pop(g_L, 1);
        }
        Sexy::Widget::MouseMove(x, y);
    }

    // Mod API: MouseDrag — 鼠标按住拖动
    void MouseDrag(int x, int y) override {
        if (!g_L || mOnMouseDragRef == LUA_NOREF) { Sexy::Widget::MouseDrag(x, y); return; }
        lua_rawgeti(g_L, LUA_REGISTRYINDEX, mOnMouseDragRef);
        lua_pushinteger(g_L, x); lua_pushinteger(g_L, y);
        if (SafePCall(g_L, 2, 1)) {
            if (lua_toboolean(g_L, -1)) return;
            lua_pop(g_L, 1);
        }
        Sexy::Widget::MouseDrag(x, y);
    }

    // Mod API: KeyChar — 字符输入（文本编辑用）
    void KeyChar(char theChar) override {
        if (!g_L || mOnKeyCharRef == LUA_NOREF) { Sexy::Widget::KeyChar(theChar); return; }
        lua_rawgeti(g_L, LUA_REGISTRYINDEX, mOnKeyCharRef);
        char buf[2] = { theChar, 0 };
        lua_pushstring(g_L, buf);
        if (SafePCall(g_L, 1, 1)) {
            if (lua_toboolean(g_L, -1)) return;
            lua_pop(g_L, 1);
        }
        Sexy::Widget::KeyChar(theChar);
    }
};

// 从 Lua userdata 获取 LuaWidget*
LuaWidget* CheckLuaWidget(lua_State* L, int idx) {
    void** pp = static_cast<void**>(luaL_checkudata(L, idx, "PvZ.Widget"));
    return pp ? static_cast<LuaWidget*>(*pp) : nullptr;
}

int l_widget_new(lua_State* L) {
    int x = static_cast<int>(luaL_optinteger(L, 1, 0));
    int y = static_cast<int>(luaL_optinteger(L, 2, 0));
    int w = static_cast<int>(luaL_optinteger(L, 3, 400));
    int h = static_cast<int>(luaL_optinteger(L, 4, 300));

    LuaWidget* wgt = new LuaWidget();
    wgt->Resize(x, y, w, h);

    void** pp = static_cast<void**>(lua_newuserdata(L, sizeof(void*)));
    *pp = wgt;
    luaL_setmetatable(L, "PvZ.Widget");
    return 1;
}

int l_widget_gc(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (w) {
        if (w->mOnDrawRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnDrawRef);
        if (w->mOnUpdateRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnUpdateRef);
        if (w->mOnKeyDownRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnKeyDownRef);
        if (w->mOnMouseDownRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnMouseDownRef);
        if (w->mOnMouseUpRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnMouseUpRef);
        if (w->mOnMouseMoveRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnMouseMoveRef);
        if (w->mOnMouseDragRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnMouseDragRef);
        if (w->mOnKeyCharRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnKeyCharRef);
        if (w->mWidgetManager) w->mWidgetManager->RemoveWidget(w);
        delete w;
    }
    return 0;
}

int l_widget_set_on_draw(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    if (w->mOnDrawRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnDrawRef);
    w->mOnDrawRef = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

int l_widget_set_on_update(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    if (w->mOnUpdateRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnUpdateRef);
    w->mOnUpdateRef = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

int l_widget_set_on_key_down(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    if (w->mOnKeyDownRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnKeyDownRef);
    w->mOnKeyDownRef = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

int l_widget_set_on_mouse_down(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    if (w->mOnMouseDownRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnMouseDownRef);
    w->mOnMouseDownRef = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

int l_widget_set_on_mouse_up(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    if (w->mOnMouseUpRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnMouseUpRef);
    w->mOnMouseUpRef = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

// Mod API: set_on_mouse_move(x, y) — 鼠标在 widget 内移动（未按下时）
int l_widget_set_on_mouse_move(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    if (w->mOnMouseMoveRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnMouseMoveRef);
    w->mOnMouseMoveRef = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

// Mod API: set_on_mouse_drag(x, y) — 鼠标按住拖动
int l_widget_set_on_mouse_drag(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    if (w->mOnMouseDragRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnMouseDragRef);
    w->mOnMouseDragRef = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

// Mod API: set_on_key_char(char) — 字符输入（文本编辑用）
int l_widget_set_on_key_char(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    if (w->mOnKeyCharRef != LUA_NOREF) luaL_unref(L, LUA_REGISTRYINDEX, w->mOnKeyCharRef);
    w->mOnKeyCharRef = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

int l_widget_add_to_manager(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    if (gLawnApp && gLawnApp->mWidgetManager) {
        gLawnApp->mWidgetManager->AddWidget(w);
        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

int l_widget_remove_from_manager(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    if (w->mWidgetManager) {
        w->mWidgetManager->RemoveWidget(w);
    }
    return 0;
}

int l_widget_set_visible(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    w->SetVisible(lua_toboolean(L, 2) != 0);
    return 0;
}

int l_widget_set_pos(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    int x = static_cast<int>(luaL_checkinteger(L, 2));
    int y = static_cast<int>(luaL_checkinteger(L, 3));
    w->Move(x, y);
    return 0;
}

int l_widget_set_size(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    int w2 = static_cast<int>(luaL_checkinteger(L, 2));
    int h = static_cast<int>(luaL_checkinteger(L, 3));
    w->Resize(w->mX, w->mY, w2, h);
    return 0;
}

// 设置 widget 的标题（可选，纯数据字段，Mod 可自行管理）
int l_widget_set_title(lua_State* L) {
    LuaWidget* w = CheckLuaWidget(L, 1);
    if (!w) return 0;
    const char* title = luaL_checkstring(L, 2);
    (void)w; (void)title;
    // 这是一个纯 Lua 侧属性，Mod 通过 widget.__title 自行维护
    // 我们用 light userdata 做 key 存到 Lua 侧的 table 里
    return 0;
}

} // namespace

namespace ModLua {

void BindWidget(lua_State* L) {
    luaL_newmetatable(L, "PvZ.Widget");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    static const luaL_Reg methods[] = {
        {"set_on_draw",         l_widget_set_on_draw},
        {"set_on_update",       l_widget_set_on_update},
        {"set_on_key_down",     l_widget_set_on_key_down},
        {"set_on_mouse_down",   l_widget_set_on_mouse_down},
        {"set_on_mouse_up",     l_widget_set_on_mouse_up},
        {"set_on_mouse_move",   l_widget_set_on_mouse_move},
        {"set_on_mouse_drag",   l_widget_set_on_mouse_drag},
        {"set_on_key_char",     l_widget_set_on_key_char},
        {"add_to_manager",      l_widget_add_to_manager},
        {"remove_from_manager", l_widget_remove_from_manager},
        {"set_visible",         l_widget_set_visible},
        {"set_pos",             l_widget_set_pos},
        {"set_size",            l_widget_set_size},
        {nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);

    lua_pushcfunction(L, l_widget_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    // 注册 pvz.widget.new() 全局函数
    lua_getglobal(L, "pvz");
    if (!lua_istable(L, -1)) { lua_newtable(L); lua_setglobal(L, "pvz"); lua_getglobal(L, "pvz"); }
    lua_newtable(L);
    lua_pushcfunction(L, l_widget_new);
    lua_setfield(L, -2, "new");
    lua_setfield(L, -2, "widget");
    lua_pop(L, 1);
}

} // namespace ModLua

namespace {

// 解析 mod.lua 清单（返回 table）
// 期望 mod.lua 返回一个 table，包含 name/version/author/description/main
bool ParseManifest(lua_State* L, const std::string& manifestPath, ModLua::ModInfo& info) {
    std::string content;
    if (!ReadFile(manifestPath, content)) {
        info.error = "cannot read mod.lua";
        return false;
    }

    std::string chunk = "return " + content; // 确保返回值
    if (luaL_loadstring(L, chunk.c_str()) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        info.error = std::string("mod.lua parse error: ") + (err ? err : "?");
        lua_pop(L, 1);
        return false;
    }
    if (!SafePCall(L, 0, 1)) {
        info.error = "mod.lua runtime error";
        return false;
    }
    if (!lua_istable(L, -1)) {
        info.error = "mod.lua must return a table";
        lua_pop(L, 1);
        return false;
    }

    auto get_str = [&](const char* key) -> std::string {
        lua_getfield(L, -1, key);
        std::string v = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
        lua_pop(L, 1);
        return v;
    };
    auto get_bool = [&](const char* key, bool def) -> bool {
        lua_getfield(L, -1, key);
        bool v = lua_isboolean(L, -1) ? (lua_toboolean(L, -1) != 0) : def;
        lua_pop(L, 1);
        return v;
    };
    auto get_int = [&](const char* key, int def) -> int {
        lua_getfield(L, -1, key);
        int v = lua_isinteger(L, -1) ? static_cast<int>(lua_tointeger(L, -1)) :
                (lua_isnumber(L, -1) ? static_cast<int>(lua_tonumber(L, -1)) : def);
        lua_pop(L, 1);
        return v;
    };

    info.name        = get_str("name");
    info.version     = get_str("version");
    info.author      = get_str("author");
    info.description = get_str("description");
    info.enabled     = get_bool("enabled", true);
    info.api_version = get_int("api_version", 0);
    info.priority    = get_int("priority", 100);
    // main 字段稍后由调用者读取
    return true;
}

// 加载单个 mod：解析清单 + 加载 main.lua + 收集回调
void LoadMod(lua_State* L, const std::string& modDir) {
    ModLua::ModInfo info;
    info.dir     = modDir;
    info.loaded  = false;
    info.enabled = true;

    std::string manifestPath = modDir + "/mod.lua";
    if (!std::filesystem::exists(manifestPath)) {
        info.error = "mod.lua not found";
        g_mods.push_back(info);
        return;
    }

    // 保存 mod 目录到全局，供 main.lua 中 require/package.path 使用
    lua_pushstring(L, modDir.c_str());
    lua_setglobal(L, "__MOD_DIR__");

    // 解析清单（结果在栈顶）
    if (!ParseManifest(L, manifestPath, info)) {
        g_mods.push_back(info);
        return;
    }

    // 读取 main 字段
    std::string mainFile = "main.lua";
    lua_getfield(L, -1, "main");
    if (lua_isstring(L, -1)) mainFile = lua_tostring(L, -1);
    lua_pop(L, 1);

    // 弹出清单 table
    lua_pop(L, 1);

    // api_version 兼容性检查（只警告不阻止——符合"方便篡改"意图）
    if (info.api_version > 0 && info.api_version != MOD_API_VERSION_MAJOR) {
        std::fprintf(stderr, "[ModLua] WARNING: mod '%s' declares api_version=%d, runtime=%d (may have compatibility issues)\n",
                     info.name.c_str(), info.api_version, MOD_API_VERSION_MAJOR);
    }

    // config.json 的 enabled 覆盖 manifest 的 enabled（用户设置优先级最高）
    info.enabled = GetModEnabled(modDir, info.enabled);
    // 同步回写到 config.json（确保新发现的 mod 有条目）
    SetModEnabled(modDir, info.enabled);

    if (!info.enabled) {
        info.error = "disabled";
        g_mods.push_back(info);
        return;
    }

    // Mod API: 自动挂载 mod 目录为资源覆盖目录
    // 这让 mod 能覆盖 pak 内的同名文件（如 .reanim/.xml/.txt/.ogg/.png 等）
    // mod 目录下的文件路径需与 pak 内路径一致才能覆盖
    // 例如：mod 目录下放 reanim/peashooter.reanim 可覆盖原版 peashooter 动画
    if (gPakInterface) {
        gPakInterface->AddModOverlayDir(modDir);
    }

    // 加载 main.lua
    std::string mainPath = modDir + "/" + mainFile;
    std::string content;
    if (!ReadFile(mainPath, content)) {
        info.error = "cannot read " + mainFile;
        g_mods.push_back(info);
        return;
    }

    // 设置 package.path 让 mod 能 require 同目录文件
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    std::string oldPath = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
    lua_pop(L, 1);
    std::string newPath = modDir + "/?.lua;" + modDir + "/?/init.lua;" + oldPath;
    lua_pushstring(L, newPath.c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);

    // 编译 + 运行 main.lua（期望返回一个 table）
    if (luaL_loadbuffer(L, content.c_str(), content.size(), mainFile.c_str()) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        info.error = std::string("load error: ") + (err ? err : "?");
        lua_pop(L, 1);
        g_mods.push_back(info);
        return;
    }
    if (!SafePCall(L, 0, 1)) {
        info.error = "runtime error in " + mainFile;
        g_mods.push_back(info);
        return;
    }
    if (!lua_istable(L, -1)) {
        info.error = mainFile + " must return a table";
        lua_pop(L, 1);
        g_mods.push_back(info);
        return;
    }

    // 收集所有回调函数
    for (int i = 0; i < static_cast<int>(ModEvent::COUNT); ++i) {
        ModEvent e = static_cast<ModEvent>(i);
        const char* cb_name = EventToLuaName(e);
        if (!cb_name) continue;

        lua_getfield(L, -1, cb_name);
        if (lua_isfunction(L, -1)) {
            int ref = luaL_ref(L, LUA_REGISTRYINDEX); // 弹出并存储
            g_callbacks.callbacks[cb_name].push_back({ref, modDir});
        } else {
            lua_pop(L, 1);
        }
    }

    // 弹出 main.lua 返回的 table
    lua_pop(L, 1);

    info.loaded = true;
    g_mods.push_back(info);

    std::fprintf(stdout, "[ModLua] Loaded mod: %s v%s (%s)\n",
                 info.name.c_str(), info.version.c_str(), modDir.c_str());
}

} // namespace

namespace ModLua {

void Initialize() {
    if (g_L) return; // 已初始化

    // stdout 默认在重定向到文件时是全缓冲模式，崩溃时会丢失末尾日志
    // 设置为行缓冲，确保每行 print 实时写入 log.txt
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    std::setvbuf(stderr, nullptr, _IOLBF, 0);

    g_L = luaL_newstate();
    if (!g_L) {
        std::fprintf(stderr, "[ModLua] Failed to create lua_State\n");
        return;
    }

    // 加载所有标准库（用户要求宽松沙箱，方便篡改）
    luaL_openlibs(g_L);

    // 替换全局 print：调用原版 print 后立即 fflush(stdout)
    // 崩溃时 stdout 缓冲区内容会丢失，这确保每行 print 实时写入 log.txt
    {
        // upvalue[1] = 原 print 函数
        lua_getglobal(g_L, "print");
        lua_pushcclosure(g_L, [](lua_State* L) -> int {
            // 调用 upvalue[1]（原 print），参数原样传递
            int n = lua_gettop(L);
            lua_pushvalue(L, lua_upvalueindex(1));
            lua_insert(L, 1);
            lua_call(L, n, 0);
            std::fflush(stdout);
            return 0;
        }, 1);
        lua_setglobal(g_L, "print");
    }

    // 加载用户配置（mods/config.json），必须在扫描 mod 目录前完成
    LoadUserConfig();

    // 注册全局 pvz 表 + 各对象绑定
    BindEnums(g_L);
    BindBoard(g_L);
    BindZombie(g_L);
    BindPlant(g_L);
    BindProjectile(g_L);
    BindReanimation(g_L);
    BindCoin(g_L);
    BindGridItem(g_L);
    BindLawnMower(g_L);
    BindGraphics(g_L);
    BindWidget(g_L);

    // sol2 批量绑定：注册所有 usertype，并覆盖原有 metatable
    // 必须在所有 Bind*() 之后调用，以便覆盖旧 metatable
    BindSol2(g_L);

    // 提供全局 pvz 表（含 api_version + config 子表）
    // 注意：BindEnums 已创建 pvz 表并设置了 ZombieType/SeedType 等枚举子表，
    // 这里必须获取已有表而非创建新表，否则会覆盖枚举导致 mod 中 pvz.ZombieType 为 nil
    lua_getglobal(g_L, "pvz");
    if (!lua_istable(g_L, -1)) {
        lua_pop(g_L, 1);
        lua_newtable(g_L);
    }
    lua_pushstring(g_L, "api_version_major");
    lua_pushinteger(g_L, MOD_API_VERSION_MAJOR);
    lua_settable(g_L, -3);
    lua_pushstring(g_L, "api_version_minor");
    lua_pushinteger(g_L, MOD_API_VERSION_MINOR);
    lua_settable(g_L, -3);

    // pvz.config 子表：get/set/all（操作当前调用 mod 的配置，通过 __MOD_DIR__ 识别）
    lua_newtable(g_L);
    lua_pushcfunction(g_L, l_config_get);
    lua_setfield(g_L, -2, "get");
    lua_pushcfunction(g_L, l_config_set);
    lua_setfield(g_L, -2, "set");
    lua_pushcfunction(g_L, l_config_all);
    lua_setfield(g_L, -2, "all");
    lua_setfield(g_L, -2, "config");

    // pvz.waves 子表：出怪系统运行时修改
    lua_newtable(g_L);
    lua_pushcfunction(g_L, l_waves_get_num_waves);
    lua_setfield(g_L, -2, "get_num_waves");
    lua_pushcfunction(g_L, l_waves_set_num_waves);
    lua_setfield(g_L, -2, "set_num_waves");
    lua_pushcfunction(g_L, l_waves_is_zombie_allowed);
    lua_setfield(g_L, -2, "is_zombie_allowed");
    lua_pushcfunction(g_L, l_waves_allow_zombie);
    lua_setfield(g_L, -2, "allow_zombie");
    lua_setfield(g_L, -2, "waves");

    // pvz.reanim 子表：动画系统注册
    lua_newtable(g_L);
    lua_pushcfunction(g_L, l_reanim_register);
    lua_setfield(g_L, -2, "register");
    // pvz.reanim.types 子表：暴露常用 ReanimationType 枚举值，供注册植物/僵尸时引用
    lua_newtable(g_L);
    ModLua::SetIntField(g_L, "NONE",            static_cast<lua_Integer>(ReanimationType::REANIM_NONE));
    ModLua::SetIntField(g_L, "PEASHOOTER",      static_cast<lua_Integer>(ReanimationType::REANIM_PEASHOOTER));
    ModLua::SetIntField(g_L, "WALLNUT",         static_cast<lua_Integer>(ReanimationType::REANIM_WALLNUT));
    ModLua::SetIntField(g_L, "SUNFLOWER",       static_cast<lua_Integer>(ReanimationType::REANIM_SUNFLOWER));
    ModLua::SetIntField(g_L, "CHERRYBOMB",      static_cast<lua_Integer>(ReanimationType::REANIM_CHERRYBOMB));
    ModLua::SetIntField(g_L, "REPEATER",        static_cast<lua_Integer>(ReanimationType::REANIM_REPEATER));
    ModLua::SetIntField(g_L, "SNOWPEA",         static_cast<lua_Integer>(ReanimationType::REANIM_SNOWPEA));
    ModLua::SetIntField(g_L, "CHOMPER",         static_cast<lua_Integer>(ReanimationType::REANIM_CHOMPER));
    ModLua::SetIntField(g_L, "ZOMBIE",          static_cast<lua_Integer>(ReanimationType::REANIM_ZOMBIE));
    ModLua::SetIntField(g_L, "ZOMBIE_FOOTBALL", static_cast<lua_Integer>(ReanimationType::REANIM_ZOMBIE_FOOTBALL));
    ModLua::SetIntField(g_L, "ZOMBIE_NEWSPAPER",static_cast<lua_Integer>(ReanimationType::REANIM_ZOMBIE_NEWSPAPER));
    ModLua::SetIntField(g_L, "ZOMBIE_ZAMBONI",  static_cast<lua_Integer>(ReanimationType::REANIM_ZOMBIE_ZAMBONI));
    lua_setfield(g_L, -2, "types");
    lua_setfield(g_L, -2, "reanim");

    // pvz.ReanimLoop 子表：ReanimLoopType 枚举值
    // mod 在 play_reanim/play_zombie_reanim/play_body_reanim 调用时使用
    lua_newtable(g_L);
    ModLua::SetIntField(g_L, "LOOP",                            static_cast<lua_Integer>(REANIM_LOOP));
    ModLua::SetIntField(g_L, "LOOP_FULL_LAST_FRAME",            static_cast<lua_Integer>(REANIM_LOOP_FULL_LAST_FRAME));
    ModLua::SetIntField(g_L, "PLAY_ONCE",                       static_cast<lua_Integer>(REANIM_PLAY_ONCE));
    ModLua::SetIntField(g_L, "PLAY_ONCE_AND_HOLD",              static_cast<lua_Integer>(REANIM_PLAY_ONCE_AND_HOLD));
    ModLua::SetIntField(g_L, "PLAY_ONCE_FULL_LAST_FRAME",       static_cast<lua_Integer>(REANIM_PLAY_ONCE_FULL_LAST_FRAME));
    ModLua::SetIntField(g_L, "PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD", static_cast<lua_Integer>(REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD));
    lua_setfield(g_L, -2, "ReanimLoop");

    // pvz.RenderGroup 子表：轨道渲染组常量
    // mod 在 reanim_show_prefix / reanim_show_track / assign_render_group_to_track 等调用中使用
    // 用于动态显示/隐藏动画轨道（如掉胳膊、头盔损坏等受损表现）
    lua_newtable(g_L);
    ModLua::SetIntField(g_L, "HIDDEN",  RENDER_GROUP_HIDDEN);
    ModLua::SetIntField(g_L, "NORMAL",  RENDER_GROUP_NORMAL);
    lua_setfield(g_L, -2, "RenderGroup");

    // pvz.get_reanimation(reanim_id) → 通过 ReanimationID 获取 Reanimation userdata
    // 用于从 zombie.body_reanim_id / plant.body_reanim_id 获取动画对象
    lua_pushcfunction(g_L, l_get_reanimation);
    lua_setfield(g_L, -2, "get_reanimation");

    // pvz.resources 子表：资源覆盖目录管理
    // mod 目录默认已自动挂载，此 API 用于挂载额外目录
    lua_newtable(g_L);
    lua_pushcfunction(g_L, l_resources_mount);
    lua_setfield(g_L, -2, "mount");
    lua_pushcfunction(g_L, l_resources_unmount);
    lua_setfield(g_L, -2, "unmount");
    // 清理编译定义的磁盘缓存（更换 main.pak 后调用）
    lua_pushcfunction(g_L, l_resources_clear_compiled_cache);
    lua_setfield(g_L, -2, "clear_compiled_cache");
    lua_setfield(g_L, -2, "resources");

    // pvz.plants 子表：植物定义注册
    lua_newtable(g_L);
    lua_pushcfunction(g_L, l_plants_register);
    lua_setfield(g_L, -2, "register");
    lua_setfield(g_L, -2, "plants");

    // pvz.zombies 子表：僵尸定义注册
    lua_newtable(g_L);
    lua_pushcfunction(g_L, l_zombies_register);
    lua_setfield(g_L, -2, "register");
    lua_setfield(g_L, -2, "zombies");

    // pvz.offset_of 子表：内存偏移查询（第二步：直接读写内存）
    // 返回字段在对象内的字节偏移，mod 用 ffi.cast + offset 直接读写
    lua_newtable(g_L);
    lua_pushcfunction(g_L, l_offset_of_zombie);
    lua_setfield(g_L, -2, "zombie");
    lua_pushcfunction(g_L, l_offset_of_plant);
    lua_setfield(g_L, -2, "plant");
    lua_pushcfunction(g_L, l_offset_of_board);
    lua_setfield(g_L, -2, "board");
    lua_pushcfunction(g_L, l_offset_of_projectile);
    lua_setfield(g_L, -2, "projectile");
    lua_pushcfunction(g_L, l_offset_of_coin);
    lua_setfield(g_L, -2, "coin");
    lua_pushcfunction(g_L, l_offset_of_mower);
    lua_setfield(g_L, -2, "mower");
    lua_pushcfunction(g_L, l_offset_of_griditem);
    lua_setfield(g_L, -2, "griditem");
    lua_setfield(g_L, -2, "offset_of");

    // pvz.field_type_of(object_kind, field_name) → 字段类型代号
    //   1=int32, 2=int64, 3=float, 4=double, 5=bool, 6=pointer, 0=未知
    // object_kind: 0=Zombie, 1=Plant, 2=Board, 3=Projectile, 4=Coin, 5=Mower, 6=GridItem
    lua_pushcfunction(g_L, l_field_type_of);
    lua_setfield(g_L, -2, "field_type_of");

    // pvz.fonts 子表：暴露所有全局 FONT_* 指针给 mod
    // mod 用法: local f = pvz.fonts.BRIANNETOD16; g:set_font(f)
    lua_newtable(g_L);
    auto push_font = [&](const char* key, _Font* font) {
        if (font) { PushFont(g_L, font); lua_setfield(g_L, -2, key); }
    };
    push_font("BRIANNETOD12",                    FONT_BRIANNETOD12);
    push_font("BRIANNETOD16",                    FONT_BRIANNETOD16);
    push_font("BRIANNETOD32",                    FONT_BRIANNETOD32);
    push_font("BRIANNETOD32BLACK",               FONT_BRIANNETOD32BLACK);
    push_font("CONTINUUMBOLD14",                 FONT_CONTINUUMBOLD14);
    push_font("CONTINUUMBOLD14OUTLINE",          FONT_CONTINUUMBOLD14OUTLINE);
    push_font("DWARVENTODCRAFT12",               FONT_DWARVENTODCRAFT12);
    push_font("DWARVENTODCRAFT15",               FONT_DWARVENTODCRAFT15);
    push_font("DWARVENTODCRAFT18",               FONT_DWARVENTODCRAFT18);
    push_font("DWARVENTODCRAFT18BRIGHTGREENINSET",FONT_DWARVENTODCRAFT18BRIGHTGREENINSET);
    push_font("DWARVENTODCRAFT18GREENINSET",     FONT_DWARVENTODCRAFT18GREENINSET);
    push_font("DWARVENTODCRAFT18YELLOW",         FONT_DWARVENTODCRAFT18YELLOW);
    push_font("DWARVENTODCRAFT24",               FONT_DWARVENTODCRAFT24);
    push_font("DWARVENTODCRAFT36BRIGHTGREENINSET",FONT_DWARVENTODCRAFT36BRIGHTGREENINSET);
    push_font("DWARVENTODCRAFT36GREENINSET",     FONT_DWARVENTODCRAFT36GREENINSET);
    push_font("HOUSEOFTERROR16",                 FONT_HOUSEOFTERROR16);
    push_font("HOUSEOFTERROR20",                 FONT_HOUSEOFTERROR20);
    push_font("HOUSEOFTERROR28",                 FONT_HOUSEOFTERROR28);
    push_font("PICO129",                         FONT_PICO129);
    push_font("TINYBOLD",                        FONT_TINYBOLD);
    lua_setfield(g_L, -2, "fonts");

    // pvz.images 子表：暴露常用 IMAGE_* 指针给 mod（共数百个，这里选最常用的）
    // mod 用法: local img = pvz.images.BACKGROUND1; g:draw_image(img, 0, 0)
    // 注意：资源在异步线程 LoadingThreadProc 中加载，ModBus::Initialize 时机早于资源加载完成，
    // 因此这里不直接缓存指针，而是建立 key→全局指针地址 的映射，通过 __index 元方法动态查询。
    g_imageMap.clear();
    auto push_img = [&](const char* key, Image*& img) {
        g_imageMap[std::string(key)] = &img;  // 存储全局变量地址，访问时实时读取
    };
    push_img("BACKGROUND1",          IMAGE_BACKGROUND1);
    push_img("BACKGROUND2",          IMAGE_BACKGROUND2);
    push_img("BACKGROUND3",          IMAGE_BACKGROUND3);
    push_img("BACKGROUND4",          IMAGE_BACKGROUND4);
    push_img("BACKGROUND5",          IMAGE_BACKGROUND5);
    push_img("BACKGROUND6BOSS",      IMAGE_BACKGROUND6BOSS);
    push_img("BACKGROUND1UNSODDED",  IMAGE_BACKGROUND1UNSODDED);
    push_img("SOD1ROW",              IMAGE_SOD1ROW);
    push_img("SOD3ROW",              IMAGE_SOD3ROW);
    push_img("SUNBANK",              IMAGE_SUNBANK);
    push_img("SHOVEL",               IMAGE_SHOVEL);
    push_img("SHOVELBANK",           IMAGE_SHOVELBANK);
    push_img("BLANK",                IMAGE_BLANK);
    push_img("FOG",                  IMAGE_FOG);
    push_img("SEEDPACKETFLASH",      IMAGE_SEEDPACKETFLASH);
    push_img("SEEDPACKETSILHOUETTE", IMAGE_SEEDPACKETSILHOUETTE);
    push_img("SEEDPACKET_LARGER",    IMAGE_SEEDPACKET_LARGER);
    push_img("LOADBAR_DIRT",         IMAGE_LOADBAR_DIRT);
    push_img("LOADBAR_GRASS",        IMAGE_LOADBAR_GRASS);
    push_img("BUTTON_LEFT",          IMAGE_BUTTON_LEFT);
    push_img("BUTTON_MIDDLE",        IMAGE_BUTTON_MIDDLE);
    push_img("BUTTON_RIGHT",         IMAGE_BUTTON_RIGHT);
    push_img("BUTTON_DOWN_LEFT",     IMAGE_BUTTON_DOWN_LEFT);
    push_img("BUTTON_DOWN_MIDDLE",   IMAGE_BUTTON_DOWN_MIDDLE);
    push_img("BUTTON_DOWN_RIGHT",    IMAGE_BUTTON_DOWN_RIGHT);
    push_img("LOCK",                 IMAGE_LOCK);
    push_img("LOCK_OPEN",            IMAGE_LOCK_OPEN);
    push_img("ALMANAC_PLANTCARD",    IMAGE_ALMANAC_PLANTCARD);
    push_img("ALMANAC_ZOMBIECARD",   IMAGE_ALMANAC_ZOMBIECARD);
    // 橄榄球僵尸头盔受损三阶段（用于 set_image_override 实现动态受损表现）
    push_img("ZOMBIE_FOOTBALL_HELMET",  IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET);
    push_img("ZOMBIE_FOOTBALL_HELMET2", IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET2);
    push_img("ZOMBIE_FOOTBALL_HELMET3", IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET3);
    // 创建 pvz.images 表，设置 __index 元方法实现动态查询
    lua_newtable(g_L);
    lua_newtable(g_L);  // 元表
    lua_pushcfunction(g_L, l_images_index);
    lua_setfield(g_L, -2, "__index");
    lua_setmetatable(g_L, -2);
    lua_setfield(g_L, -2, "images");

    lua_setglobal(g_L, "pvz");

    // 扫描 mods/ 目录
    std::string modsDir = GetModsDir();
    if (!std::filesystem::exists(modsDir)) {
        std::fprintf(stdout, "[ModLua] mods/ dir not found at %s (skipping)\n", modsDir.c_str());
        return;
    }

    // 第一阶段：预扫描所有 mod 目录，读取 priority 用于排序
    struct ModEntry {
        std::string dir;
        int priority;
    };
    std::vector<ModEntry> entries;
    for (auto& entry : std::filesystem::directory_iterator(modsDir)) {
        if (!entry.is_directory()) continue;
        std::string dir = entry.path().string();
        ModLua::ModInfo info;
        std::string manifestPath = dir + "/mod.lua";
        int prio = 100;
        if (std::filesystem::exists(manifestPath) && ParseManifest(g_L, manifestPath, info)) {
            prio = info.priority;
            lua_pop(g_L, 1); // 弹出 manifest table
        }
        entries.push_back({dir, prio});
    }

    // 按 priority 排序（priority 相同则按目录名字母序，保证可预测）
    std::sort(entries.begin(), entries.end(), [](const ModEntry& a, const ModEntry& b) {
        if (a.priority != b.priority) return a.priority < b.priority;
        return a.dir < b.dir;
    });

    // 第二阶段：按 priority 顺序加载 main.lua
    for (auto& e : entries) {
        LoadMod(g_L, e.dir);
    }

    // 加载完成后回写 config.json（补齐新发现的 mod 条目）
    SaveUserConfig();

    std::fprintf(stdout, "[ModLua] Initialized: %zu mod(s) loaded\n", g_mods.size());
}

void Shutdown() {
    std::lock_guard<std::recursive_mutex> lk(g_lua_mutex);
    if (!g_L) return;

    // 最终保存配置
    SaveUserConfig();

    // 释放所有 callback ref
    for (auto& [name, entries] : g_callbacks.callbacks) {
        for (auto& entry : entries) {
            luaL_unref(g_L, LUA_REGISTRYINDEX, entry.ref);
        }
    }
    g_callbacks.callbacks.clear();
    g_mods.clear();

    lua_close(g_L);
    g_L = nullptr;
}

void Reload() {
    Shutdown();
    Initialize();
}

void DispatchEvent(ModCtx& ctx) {
    std::lock_guard<std::recursive_mutex> lk(g_lua_mutex);
    if (!g_L) return;

    const char* cb_name = EventToLuaName(ctx.event);
    if (!cb_name) return;

    auto it = g_callbacks.callbacks.find(cb_name);
    if (it == g_callbacks.callbacks.end() || it->second.empty()) return;

    // 根据事件类型决定 push 什么参数
    int nargs = 0;
    for (auto& entry : it->second) {
        lua_rawgeti(g_L, LUA_REGISTRYINDEX, entry.ref);
        // 设置当前 mod 目录（供 pvz.config.get/set/all 识别调用者）
        lua_pushstring(g_L, entry.modDir.c_str());
        lua_setglobal(g_L, "__MOD_DIR__");
        // 构造参数表
        switch (ctx.event) {
        case ModEvent::ON_APP_INIT_POST:
        case ModEvent::ON_LOADING_COMPLETED:
            // 无参数
            nargs = 0;
            break;
        case ModEvent::ON_BOARD_UPDATE_PRE:
        case ModEvent::ON_BOARD_UPDATE_POST:
        case ModEvent::ON_UPDATE_GAME_OBJECTS_PRE:
        case ModEvent::ON_LEVEL_INIT_POST:
        case ModEvent::ON_LEVEL_START_POST:
        case ModEvent::ON_LEVEL_END:
            PushBoard(g_L, ctx.board);
            nargs = 1;
            break;
        case ModEvent::ON_PLANT_CREATED:
            PushPlant(g_L, static_cast<Plant*>(ctx.object));
            nargs = 1;
            break;
        case ModEvent::ON_PLANT_DIE_PRE:
            // 用 ctx.plant（MOD_HOOK 在 Plant::Die 中设置），fallback 到 ctx.object
            PushPlant(g_L, ctx.plant ? ctx.plant : static_cast<Plant*>(ctx.object));
            nargs = 1;
            break;
        case ModEvent::ON_ZOMBIE_CREATED:
            PushZombie(g_L, static_cast<Zombie*>(ctx.object));
            nargs = 1;
            break;
        case ModEvent::ON_PROJECTILE_CREATED:
            PushProjectile(g_L, static_cast<Projectile*>(ctx.object));
            nargs = 1;
            break;
        case ModEvent::ON_COIN_CREATED:
            PushCoin(g_L, static_cast<Coin*>(ctx.object));
            nargs = 1;
            break;
        case ModEvent::ON_GRIDITEM_CREATED:
            PushGridItem(g_L, static_cast<GridItem*>(ctx.object));
            nargs = 1;
            break;
        case ModEvent::ON_LAWNMOWER_CREATED:
            PushLawnMower(g_L, static_cast<LawnMower*>(ctx.object));
            nargs = 1;
            break;
        case ModEvent::ON_OBJECT_DESTROYED:
            // push 类型 + 对象（按类型强转）
            lua_pushstring(g_L, [](ModObjectType t) -> const char* {
                switch (t) {
                case ModObjectType::PLANT:     return "plant";
                case ModObjectType::ZOMBIE:    return "zombie";
                case ModObjectType::PROJECTILE:return "projectile";
                case ModObjectType::COIN:      return "coin";
                case ModObjectType::GRIDITEM:  return "griditem";
                case ModObjectType::LAWNMOWER: return "lawnmower";
                default: return "unknown";
                }
            }(ctx.objectType));
            switch (ctx.objectType) {
            case ModObjectType::PLANT:     PushPlant(g_L, static_cast<Plant*>(ctx.object)); break;
            case ModObjectType::ZOMBIE:    PushZombie(g_L, static_cast<Zombie*>(ctx.object)); break;
            case ModObjectType::PROJECTILE:PushProjectile(g_L, static_cast<Projectile*>(ctx.object)); break;
            case ModObjectType::COIN:      PushCoin(g_L, static_cast<Coin*>(ctx.object)); break;
            case ModObjectType::GRIDITEM:  PushGridItem(g_L, static_cast<GridItem*>(ctx.object)); break;
            case ModObjectType::LAWNMOWER: PushLawnMower(g_L, static_cast<LawnMower*>(ctx.object)); break;
            default: lua_pushnil(g_L); break;
            }
            nargs = 2;
            break;
        case ModEvent::ON_ZOMBIE_UPDATE_PRE:
            // 参数：zombie。返回 {cancel=true} 可完全接管原版 Update
            PushZombie(g_L, ctx.zombie);
            nargs = 1;
            break;
        case ModEvent::ON_PLANT_UPDATE_PRE:
            // 参数：plant。返回 {cancel=true} 可完全接管原版 Update
            PushPlant(g_L, ctx.plant);
            nargs = 1;
            break;
        case ModEvent::ON_ZOMBIE_TAKE_DAMAGE_PRE:
            // 参数：zombie, damage, damageFlags, source
            // source 是一个表：{plant=..., projectile=..., zombie=...}
            // 三者最多有一个非 nil，表示伤害来源；全为 nil 表示来源未知（如范围秒杀）
            PushZombie(g_L, ctx.zombie);
            lua_pushinteger(g_L, ctx.damage);
            lua_pushinteger(g_L, ctx.damageFlags);
            lua_newtable(g_L);
            if (ctx.plant)       { PushPlant(g_L, ctx.plant);       lua_setfield(g_L, -2, "plant"); }
            if (ctx.projectile)  { PushProjectile(g_L, ctx.projectile); lua_setfield(g_L, -2, "projectile"); }
            if (ctx.sourceZombie){ PushZombie(g_L, ctx.sourceZombie); lua_setfield(g_L, -2, "zombie"); }
            nargs = 4;
            break;
        case ModEvent::ON_PLANT_TAKE_DAMAGE_PRE:
            PushPlant(g_L, ctx.plant);
            PushZombie(g_L, ctx.zombie);
            lua_pushinteger(g_L, ctx.damage);
            nargs = 3;
            break;
        case ModEvent::ON_PROJECTILE_IMPACT_PRE:
            PushProjectile(g_L, ctx.projectile);
            PushZombie(g_L, ctx.zombie);
            nargs = 2;
            break;
        case ModEvent::ON_SPAWN_ZOMBIE_WAVE_PRE:
            PushBoard(g_L, ctx.board);
            nargs = 1;
            break;
        case ModEvent::ON_PICK_ZOMBIE_WAVES_PRE:
            // 参数：board, level
            PushBoard(g_L, ctx.board);
            lua_pushinteger(g_L, ctx.level);
            nargs = 2;
            break;
        case ModEvent::ON_PICK_ZOMBIE_WAVES_POST:
            // 参数：board, level, num_waves（原版已生成的波数）
            PushBoard(g_L, ctx.board);
            lua_pushinteger(g_L, ctx.level);
            lua_pushinteger(g_L, ctx.customNumWaves);  // 复用 customNumWaves 传 mNumWaves
            nargs = 3;
            break;
        case ModEvent::ON_PICK_ZOMBIE_TYPE_PRE:
            // 参数：board, level, wave_index, points
            PushBoard(g_L, ctx.board);
            lua_pushinteger(g_L, ctx.level);
            lua_pushinteger(g_L, ctx.waveIndex);
            lua_pushinteger(g_L, ctx.zombiePoints);
            nargs = 4;
            break;
        case ModEvent::ON_KEY_DOWN_PRE:
            PushBoard(g_L, ctx.board);
            lua_pushinteger(g_L, ctx.keyCode);
            nargs = 2;
            break;
        case ModEvent::ON_MOUSE_DOWN_PRE:
        case ModEvent::ON_MOUSE_UP_PRE:
            PushBoard(g_L, ctx.board);
            lua_pushinteger(g_L, ctx.mouseX);
            lua_pushinteger(g_L, ctx.mouseY);
            lua_pushinteger(g_L, ctx.mouseBtn);
            nargs = 4;
            break;
        case ModEvent::ON_SUN_CHANGED:
            PushBoard(g_L, ctx.board);
            lua_pushinteger(g_L, ctx.sunDelta);
            nargs = 2;
            break;
        case ModEvent::ON_BOARD_DRAW_HUD:
            // 参数：board, graphics
            // graphics 是 light userdata，仅在回调期间有效
            PushBoard(g_L, ctx.board);
            PushGraphics(g_L, static_cast<Graphics*>(ctx.graphics));
            nargs = 2;
            break;
        default:
            nargs = 0;
            break;
        }

        // 调用并处理返回值（可拦截事件读返回 table）
        if (lua_pcall(g_L, nargs, 1, 0) != LUA_OK) {
            const char* err = lua_tostring(g_L, -1);
            std::fprintf(stderr, "[ModLua] %s callback error: %s\n", cb_name, err ? err : "?");
            lua_pop(g_L, 1);
            continue;
        }

        // 处理可拦截事件的返回值
        if (lua_istable(g_L, -1)) {
            int retIdx = lua_absindex(g_L, -1);

            lua_getfield(g_L, retIdx, "cancel");
            if (lua_toboolean(g_L, -1)) ctx.cancel = true;
            lua_pop(g_L, 1);

            lua_getfield(g_L, retIdx, "damage");
            if (lua_isinteger(g_L, -1)) ctx.newDamage = static_cast<int32_t>(lua_tointeger(g_L, -1));
            lua_pop(g_L, 1);

            lua_getfield(g_L, retIdx, "sun");
            if (lua_isinteger(g_L, -1)) ctx.sunResult = static_cast<int32_t>(lua_tointeger(g_L, -1));
            lua_pop(g_L, 1);

            // ON_PICK_ZOMBIE_TYPE_PRE: 返回 {zombie_type = N} 替换选中的僵尸类型
            if (ctx.event == ModEvent::ON_PICK_ZOMBIE_TYPE_PRE) {
                lua_getfield(g_L, retIdx, "zombie_type");
                if (lua_isinteger(g_L, -1)) {
                    ctx.zombieType = static_cast<ZombieType>(lua_tointeger(g_L, -1));
                }
                lua_pop(g_L, 1);
            }

            // ON_PICK_ZOMBIE_WAVES_PRE: 返回 {waves = N, plan = {...}} 覆盖整张波次表
            // plan 格式：{ [0] = {ZOMBIE_NORMAL, ZOMBIE_CONE, ...}, [1] = {...}, ... }
            // 或 Lua 风格 1-based：{ {ZOMBIE_NORMAL, ZOMBIE_CONE}, {...} }
            if (ctx.event == ModEvent::ON_PICK_ZOMBIE_WAVES_PRE) {
                lua_getfield(g_L, retIdx, "waves");
                if (lua_isinteger(g_L, -1)) {
                    int w = static_cast<int>(lua_tointeger(g_L, -1));
                    ctx.customNumWaves = ClampInt(w, 0, ModCtx::MAX_CUSTOM_WAVES);
                }
                lua_pop(g_L, 1);

                lua_getfield(g_L, retIdx, "plan");
                if (lua_istable(g_L, -1)) {
                    int planIdx = lua_absindex(g_L, -1);
                    // 遍历 plan 表
                    lua_pushnil(g_L);
                    int maxWaveFilled = -1;
                    while (lua_next(g_L, planIdx)) {
                        // key 在 -2，value 在 -1
                        int waveKey = -1;
                        if (lua_isinteger(g_L, -2)) {
                            // Mod 使用 0-based 索引（wave 0 = 第一波），无需转换
                            int k = static_cast<int>(lua_tointeger(g_L, -2));
                            waveKey = k;
                        }
                        if (waveKey >= 0 && waveKey < ModCtx::MAX_CUSTOM_WAVES && lua_istable(g_L, -1)) {
                            int waveListIdx = lua_absindex(g_L, -1);
                            int len = 0;
                            lua_pushnil(g_L);
                            while (lua_next(g_L, waveListIdx) && len < ModCtx::MAX_CUSTOM_PER_WAVE) {
                                if (lua_isinteger(g_L, -1)) {
                                    int zt = static_cast<int>(lua_tointeger(g_L, -1));
                                    // 简单范围检查
                                    if (zt >= 0 && zt < 100) {
                                        ctx.customWaves[waveKey][len] = static_cast<ZombieType>(zt);
                                        ++len;
                                    }
                                }
                                lua_pop(g_L, 1);
                            }
                            ctx.customWaveLengths[waveKey] = len;
                            if (waveKey > maxWaveFilled) maxWaveFilled = waveKey;
                        }
                        lua_pop(g_L, 1);  // 弹出 value，保留 key 给 lua_next
                    }
                    // 如果未显式设置 waves 字段，用 plan 的最大 key+1
                    if (ctx.customNumWaves == 0 && maxWaveFilled >= 0) {
                        ctx.customNumWaves = maxWaveFilled + 1;
                    }
                    if (ctx.customNumWaves > 0) {
                        ctx.useCustomWaves = true;
                    }
                }
                lua_pop(g_L, 1);  // 弹出 plan
            }

            // ON_PICK_ZOMBIE_WAVES_POST: 返回 {append = {[wave] = {type1, type2, ...}}}
            // 把指定波次的僵尸追加到已生成的 mZombiesInWave（不替换原版波次）
            if (ctx.event == ModEvent::ON_PICK_ZOMBIE_WAVES_POST) {
                lua_getfield(g_L, retIdx, "append");
                if (lua_istable(g_L, -1)) {
                    int appendIdx = lua_absindex(g_L, -1);
                    lua_pushnil(g_L);
                    while (lua_next(g_L, appendIdx)) {
                        int waveKey = -1;
                        if (lua_isinteger(g_L, -2)) {
                            // Mod 使用 0-based 索引（wave 0 = 第一波），无需转换
                            int k = static_cast<int>(lua_tointeger(g_L, -2));
                            waveKey = k;
                        }
                        if (waveKey >= 0 && waveKey < ModCtx::MAX_CUSTOM_WAVES && lua_istable(g_L, -1)) {
                            int waveListIdx = lua_absindex(g_L, -1);
                            int len = 0;
                            lua_pushnil(g_L);
                            while (lua_next(g_L, waveListIdx) && len < ModCtx::MAX_CUSTOM_PER_WAVE) {
                                if (lua_isinteger(g_L, -1)) {
                                    int zt = static_cast<int>(lua_tointeger(g_L, -1));
                                    if (zt >= 0 && zt < 100) {
                                        ctx.appendWaves[waveKey][len] = static_cast<ZombieType>(zt);
                                        ++len;
                                    }
                                }
                                lua_pop(g_L, 1);
                            }
                            ctx.appendWaveLengths[waveKey] = len;
                        }
                        lua_pop(g_L, 1);  // 弹出 value，保留 key 给 lua_next
                    }
                    ctx.useAppendWaves = true;
                }
                lua_pop(g_L, 1);  // 弹出 append

                // hide_from_preview: {type1, type2, ...} —— 标记这些僵尸类型不参与关卡开始前的右侧预览
                lua_getfield(g_L, retIdx, "hide_from_preview");
                if (lua_istable(g_L, -1)) {
                    int hideIdx = lua_absindex(g_L, -1);
                    lua_pushnil(g_L);
                    while (lua_next(g_L, hideIdx)) {
                        if (lua_isinteger(g_L, -1)) {
                            int zt = static_cast<int>(lua_tointeger(g_L, -1));
                            if (zt >= 0 && zt < 100) {
                                MarkZombieHiddenFromPreview(static_cast<ZombieType>(zt));
                            }
                        }
                        lua_pop(g_L, 1);
                    }
                }
                lua_pop(g_L, 1);  // 弹出 hide_from_preview
            }
        }
        lua_pop(g_L, 1); // 弹出返回值
    }
}

std::string DoString(const std::string& code) {
    std::lock_guard<std::recursive_mutex> lk(g_lua_mutex);
    if (!g_L) return "Lua runtime not initialized";

    if (luaL_loadstring(g_L, code.c_str()) != LUA_OK) {
        const char* err = lua_tostring(g_L, -1);
        std::string r = err ? err : "?";
        lua_pop(g_L, 1);
        return r;
    }
    if (lua_pcall(g_L, 0, 1, 0) != LUA_OK) {
        const char* err = lua_tostring(g_L, -1);
        std::string r = err ? err : "?";
        lua_pop(g_L, 1);
        return r;
    }
    std::string result;
    if (lua_isstring(g_L, -1)) {
        result = lua_tostring(g_L, -1);
    } else if (lua_isnil(g_L, -1)) {
        result = "nil";
    } else if (lua_isboolean(g_L, -1)) {
        result = lua_toboolean(g_L, -1) ? "true" : "false";
    } else {
        result = lua_tostring(g_L, -1);
    }
    lua_pop(g_L, 1);
    return result;
}

const std::vector<ModInfo>& GetLoadedMods() {
    return g_mods;
}

} // namespace ModLua
