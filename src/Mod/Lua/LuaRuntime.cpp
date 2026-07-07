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
#include "../../SexyAppFramework/graphics/Graphics.h"  // Graphics（ON_BOARD_DRAW_HUD 事件）
#include "../../Resources.h"            // FONT_* / IMAGE_* 全局指针（pvz.fonts / pvz.images）

// 对象绑定（在各自 Bind*.cpp 中实现）
namespace ModLua {
    void BindBoard(lua_State* L);
    void BindZombie(lua_State* L);
    void BindPlant(lua_State* L);
    void BindProjectile(lua_State* L);
    void BindCoin(lua_State* L);
    void BindGridItem(lua_State* L);
    void BindLawnMower(lua_State* L);
    void BindGraphics(lua_State* L);   // Graphics/Image/Font 元表
    void BindEnums(lua_State* L);  // ZombieType / SeedType / KeyCode 等

    // 把 C++ 对象 push 成 Lua userdata（各 Bind*.cpp 提供）
    void PushBoard(lua_State* L, Board* board);
    void PushZombie(lua_State* L, Zombie* z);
    void PushPlant(lua_State* L, Plant* p);
    void PushProjectile(lua_State* L, Projectile* p);
    void PushCoin(lua_State* L, Coin* c);
    void PushGridItem(lua_State* L, GridItem* g);
    void PushLawnMower(lua_State* L, LawnMower* m);
    void PushGraphics(lua_State* L, Graphics* g);  // ON_BOARD_DRAW_HUD 事件用
    void PushFont(lua_State* L, _Font* f);          // pvz.fonts 表用
    void PushImage(lua_State* L, Image* img);        // pvz.images 表用
}

namespace {

lua_State* g_L = nullptr;
std::mutex g_lua_mutex;

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

// 用户配置持久化：mods/config.json
// 结构: { "version": 1, "mods": { "<dir>": { "enabled": bool, "config": {...} } } }
ModJson::JsonValue g_userConfig;
std::string        g_configPath;

// ModEvent → Lua 回调名映射
const char* EventToLuaName(ModEvent e) {
    switch (e) {
    case ModEvent::ON_APP_INIT_POST:           return "on_app_init";
    case ModEvent::ON_LOADING_COMPLETED:       return "on_loading_completed";
    case ModEvent::ON_BOARD_UPDATE_PRE:        return "on_board_update_pre";
    case ModEvent::ON_BOARD_UPDATE_POST:       return "on_board_update_post";
    case ModEvent::ON_UPDATE_GAME_OBJECTS_PRE: return "on_update_game_objects_pre";
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
    case ModEvent::ON_PICK_ZOMBIE_TYPE_PRE:    return "on_pick_zombie_type";
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

// pvz.plants.register(definition_table) → 注册新植物类型，返回 SeedType
// definition_table 字段：
//   name (string, 必需)         植物名称
//   cost (int, 默认 0)           阳光花费
//   refresh (int, 默认 0)        冷却时间（毫秒）
//   reanim_type (int, 默认 0)    对应的 ReanimationType
//   packet_index (int, 默认 0)   卡包索引
//   subclass (int, 默认 0)       PlantSubClass (0=NORMAL, 1=SHOOTER)
//   launch_rate (int, 默认 0)    发射间隔
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

    SeedType st = RegisterPlantDefinition(def);
    lua_pushinteger(L, static_cast<lua_Integer>(st));
    return 1;
}

// pvz.zombies.register(definition_table) → 注册新僵尸类型，返回 ZombieType
// definition_table 字段：
//   name (string, 必需)              僵尸名称
//   reanim_type (int, 默认 0)        对应的 ReanimationType
//   value (int, 默认 0)              僵尸价值（用于波次成本预算）
//   starting_level (int, 默认 0)     出场起始关卡
//   first_allowed_wave (int, 默认 0) 首个允许出现的波次
//   pick_weight (int, 默认 0)        随机挑选权重
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

    // Mod API: 图鉴显示用字段（可选）
    lua_getfield(L, tblIdx, "almanac_name");
    if (lua_isstring(L, -1)) def.mAlmanacName = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, tblIdx, "almanac_description");
    if (lua_isstring(L, -1)) def.mAlmanacDescription = lua_tostring(L, -1);
    lua_pop(L, 1);

    ZombieType zt = RegisterZombieDefinition(def);
    lua_pushinteger(L, static_cast<lua_Integer>(zt));
    return 1;
}

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

    g_L = luaL_newstate();
    if (!g_L) {
        std::fprintf(stderr, "[ModLua] Failed to create lua_State\n");
        return;
    }

    // 加载所有标准库（用户要求宽松沙箱，方便篡改）
    luaL_openlibs(g_L);

    // 加载用户配置（mods/config.json），必须在扫描 mod 目录前完成
    LoadUserConfig();

    // 注册全局 pvz 表 + 各对象绑定
    BindEnums(g_L);
    BindBoard(g_L);
    BindZombie(g_L);
    BindPlant(g_L);
    BindProjectile(g_L);
    BindCoin(g_L);
    BindGridItem(g_L);
    BindLawnMower(g_L);
    BindGraphics(g_L);

    // 提供全局 pvz 表（含 api_version + config 子表）
    lua_newtable(g_L);
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

    // pvz.resources 子表：资源覆盖目录管理
    // mod 目录默认已自动挂载，此 API 用于挂载额外目录
    lua_newtable(g_L);
    lua_pushcfunction(g_L, l_resources_mount);
    lua_setfield(g_L, -2, "mount");
    lua_pushcfunction(g_L, l_resources_unmount);
    lua_setfield(g_L, -2, "unmount");
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
    lua_newtable(g_L);
    auto push_img = [&](const char* key, Image* img) {
        if (img) { PushImage(g_L, img); lua_setfield(g_L, -2, key); }
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
    std::lock_guard<std::mutex> lk(g_lua_mutex);
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
    std::lock_guard<std::mutex> lk(g_lua_mutex);
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
        case ModEvent::ON_ZOMBIE_TAKE_DAMAGE_PRE:
            PushZombie(g_L, ctx.zombie);
            lua_pushinteger(g_L, ctx.damage);
            lua_pushinteger(g_L, ctx.damageFlags);
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
            nargs = 3;
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
                                                            // Lua 1-based 索引：自动转 0-based
                                int k = static_cast<int>(lua_tointeger(g_L, -2));
                            waveKey = (k >= 1) ? k - 1 : k;  // 0 也允许
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
        }
        lua_pop(g_L, 1); // 弹出返回值
    }
}

std::string DoString(const std::string& code) {
    std::lock_guard<std::mutex> lk(g_lua_mutex);
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
