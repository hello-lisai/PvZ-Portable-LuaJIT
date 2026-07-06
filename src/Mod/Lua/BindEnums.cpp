#include "LuaBindUtil.h"
#include "../../ConstEnums.h"

namespace ModLua {

// 注册所有游戏枚举到全局 pvz 表的子表中
// 使用方式: lua 中 ZombieType.GARGANTUAR, SeedType.SUNFLOWER 等
void BindEnums(lua_State* L) {
    // 获取或创建全局 pvz 表
    lua_getglobal(L, "pvz");
    if (!lua_istable(L, -1)) {
        lua_newtable(L);
    }

    // === ZombieType ===
    lua_newtable(L);
    static const EnumEntry zombie_types[] = {
        {"INVALID",            static_cast<lua_Integer>(ZombieType::ZOMBIE_INVALID)},
        {"NORMAL",             static_cast<lua_Integer>(ZombieType::ZOMBIE_NORMAL)},
        {"FLAG",               static_cast<lua_Integer>(ZombieType::ZOMBIE_FLAG)},
        {"TRAFFIC_CONE",       static_cast<lua_Integer>(ZombieType::ZOMBIE_TRAFFIC_CONE)},
        {"POLEVAULTER",        static_cast<lua_Integer>(ZombieType::ZOMBIE_POLEVAULTER)},
        {"PAIL",               static_cast<lua_Integer>(ZombieType::ZOMBIE_PAIL)},
        {"NEWSPAPER",          static_cast<lua_Integer>(ZombieType::ZOMBIE_NEWSPAPER)},
        {"DOOR",               static_cast<lua_Integer>(ZombieType::ZOMBIE_DOOR)},
        {"FOOTBALL",           static_cast<lua_Integer>(ZombieType::ZOMBIE_FOOTBALL)},
        {"DANCER",             static_cast<lua_Integer>(ZombieType::ZOMBIE_DANCER)},
        {"BACKUP_DANCER",      static_cast<lua_Integer>(ZombieType::ZOMBIE_BACKUP_DANCER)},
        {"DUCKY_TUBE",         static_cast<lua_Integer>(ZombieType::ZOMBIE_DUCKY_TUBE)},
        {"SNORKEL",            static_cast<lua_Integer>(ZombieType::ZOMBIE_SNORKEL)},
        {"ZAMBONI",            static_cast<lua_Integer>(ZombieType::ZOMBIE_ZAMBONI)},
        {"BOBSLED",            static_cast<lua_Integer>(ZombieType::ZOMBIE_BOBSLED)},
        {"DOLPHIN_RIDER",      static_cast<lua_Integer>(ZombieType::ZOMBIE_DOLPHIN_RIDER)},
        {"JACK_IN_THE_BOX",    static_cast<lua_Integer>(ZombieType::ZOMBIE_JACK_IN_THE_BOX)},
        {"BALLOON",            static_cast<lua_Integer>(ZombieType::ZOMBIE_BALLOON)},
        {"DIGGER",             static_cast<lua_Integer>(ZombieType::ZOMBIE_DIGGER)},
        {"POGO",               static_cast<lua_Integer>(ZombieType::ZOMBIE_POGO)},
        {"YETI",               static_cast<lua_Integer>(ZombieType::ZOMBIE_YETI)},
        {"BUNGEE",             static_cast<lua_Integer>(ZombieType::ZOMBIE_BUNGEE)},
        {"LADDER",             static_cast<lua_Integer>(ZombieType::ZOMBIE_LADDER)},
        {"CATAPULT",           static_cast<lua_Integer>(ZombieType::ZOMBIE_CATAPULT)},
        {"GARGANTUAR",         static_cast<lua_Integer>(ZombieType::ZOMBIE_GARGANTUAR)},
        {"IMP",                static_cast<lua_Integer>(ZombieType::ZOMBIE_IMP)},
        {"BOSS",               static_cast<lua_Integer>(ZombieType::ZOMBIE_BOSS)},
        {"PEA_HEAD",           static_cast<lua_Integer>(ZombieType::ZOMBIE_PEA_HEAD)},
        {"WALLNUT_HEAD",       static_cast<lua_Integer>(ZombieType::ZOMBIE_WALLNUT_HEAD)},
        {"JALAPENO_HEAD",      static_cast<lua_Integer>(ZombieType::ZOMBIE_JALAPENO_HEAD)},
        {"GATLING_HEAD",       static_cast<lua_Integer>(ZombieType::ZOMBIE_GATLING_HEAD)},
        {"SQUASH_HEAD",        static_cast<lua_Integer>(ZombieType::ZOMBIE_SQUASH_HEAD)},
        {"TALLNUT_HEAD",       static_cast<lua_Integer>(ZombieType::ZOMBIE_TALLNUT_HEAD)},
        {"REDEYE_GARGANTUAR",  static_cast<lua_Integer>(ZombieType::ZOMBIE_REDEYE_GARGANTUAR)},
    };
    RegisterEnum(L, zombie_types, sizeof(zombie_types)/sizeof(zombie_types[0]));
    lua_setfield(L, -2, "ZombieType");

    // === SeedType ===
    lua_newtable(L);
    static const EnumEntry seed_types[] = {
        {"PEASHOOTER",      static_cast<lua_Integer>(SeedType::SEED_PEASHOOTER)},
        {"SUNFLOWER",       static_cast<lua_Integer>(SeedType::SEED_SUNFLOWER)},
        {"CHERRYBOMB",      static_cast<lua_Integer>(SeedType::SEED_CHERRYBOMB)},
        {"WALLNUT",         static_cast<lua_Integer>(SeedType::SEED_WALLNUT)},
        {"POTATOMINE",      static_cast<lua_Integer>(SeedType::SEED_POTATOMINE)},
        {"SNOWPEA",         static_cast<lua_Integer>(SeedType::SEED_SNOWPEA)},
        {"CHOMPER",         static_cast<lua_Integer>(SeedType::SEED_CHOMPER)},
        {"REPEATER",        static_cast<lua_Integer>(SeedType::SEED_REPEATER)},
        {"PUFFSHROOM",      static_cast<lua_Integer>(SeedType::SEED_PUFFSHROOM)},
        {"SUNSHROOM",       static_cast<lua_Integer>(SeedType::SEED_SUNSHROOM)},
        {"FUMESHROOM",      static_cast<lua_Integer>(SeedType::SEED_FUMESHROOM)},
        {"GRAVEBUSTER",     static_cast<lua_Integer>(SeedType::SEED_GRAVEBUSTER)},
        {"HYPNOSHROOM",     static_cast<lua_Integer>(SeedType::SEED_HYPNOSHROOM)},
        {"SCAREDYSHROOM",   static_cast<lua_Integer>(SeedType::SEED_SCAREDYSHROOM)},
        {"ICESHROOM",       static_cast<lua_Integer>(SeedType::SEED_ICESHROOM)},
        {"DOOMSHROOM",      static_cast<lua_Integer>(SeedType::SEED_DOOMSHROOM)},
        {"LILYPAD",         static_cast<lua_Integer>(SeedType::SEED_LILYPAD)},
        {"SQUASH",          static_cast<lua_Integer>(SeedType::SEED_SQUASH)},
        {"THREEPEATER",     static_cast<lua_Integer>(SeedType::SEED_THREEPEATER)},
        {"TANGLEKELP",      static_cast<lua_Integer>(SeedType::SEED_TANGLEKELP)},
        {"JALAPENO",        static_cast<lua_Integer>(SeedType::SEED_JALAPENO)},
        {"SPIKEWEED",       static_cast<lua_Integer>(SeedType::SEED_SPIKEWEED)},
        {"TORCHWOOD",       static_cast<lua_Integer>(SeedType::SEED_TORCHWOOD)},
        {"TALLNUT",         static_cast<lua_Integer>(SeedType::SEED_TALLNUT)},
        {"SEASHROOM",       static_cast<lua_Integer>(SeedType::SEED_SEASHROOM)},
        {"PLANTERN",        static_cast<lua_Integer>(SeedType::SEED_PLANTERN)},
        {"CACTUS",          static_cast<lua_Integer>(SeedType::SEED_CACTUS)},
        {"BLOVER",          static_cast<lua_Integer>(SeedType::SEED_BLOVER)},
        {"SPLITPEA",        static_cast<lua_Integer>(SeedType::SEED_SPLITPEA)},
        {"STARFRUIT",       static_cast<lua_Integer>(SeedType::SEED_STARFRUIT)},
        {"PUMPKINSHELL",    static_cast<lua_Integer>(SeedType::SEED_PUMPKINSHELL)},
        {"MAGNETSHROOM",    static_cast<lua_Integer>(SeedType::SEED_MAGNETSHROOM)},
        {"CABBAGEPULT",     static_cast<lua_Integer>(SeedType::SEED_CABBAGEPULT)},
        {"FLOWERPOT",       static_cast<lua_Integer>(SeedType::SEED_FLOWERPOT)},
        {"KERNELPULT",      static_cast<lua_Integer>(SeedType::SEED_KERNELPULT)},
        {"INSTANT_COFFEE",  static_cast<lua_Integer>(SeedType::SEED_INSTANT_COFFEE)},
        {"GARLIC",          static_cast<lua_Integer>(SeedType::SEED_GARLIC)},
        {"UMBRELLA",        static_cast<lua_Integer>(SeedType::SEED_UMBRELLA)},
        {"MARIGOLD",        static_cast<lua_Integer>(SeedType::SEED_MARIGOLD)},
        {"MELONPULT",       static_cast<lua_Integer>(SeedType::SEED_MELONPULT)},
        {"GATLINGPEA",      static_cast<lua_Integer>(SeedType::SEED_GATLINGPEA)},
        {"TWINSUNFLOWER",   static_cast<lua_Integer>(SeedType::SEED_TWINSUNFLOWER)},
        {"GLOOMSHROOM",     static_cast<lua_Integer>(SeedType::SEED_GLOOMSHROOM)},
        {"CATTAIL",         static_cast<lua_Integer>(SeedType::SEED_CATTAIL)},
        {"WINTERMELON",     static_cast<lua_Integer>(SeedType::SEED_WINTERMELON)},
        {"GOLD_MAGNET",     static_cast<lua_Integer>(SeedType::SEED_GOLD_MAGNET)},
        {"SPIKEROCK",       static_cast<lua_Integer>(SeedType::SEED_SPIKEROCK)},
        {"COBCANNON",       static_cast<lua_Integer>(SeedType::SEED_COBCANNON)},
        {"IMITATER",        static_cast<lua_Integer>(SeedType::SEED_IMITATER)},
        {"EXPLODE_O_NUT",   static_cast<lua_Integer>(SeedType::SEED_EXPLODE_O_NUT)},
        {"GIANT_WALLNUT",   static_cast<lua_Integer>(SeedType::SEED_GIANT_WALLNUT)},
    };
    RegisterEnum(L, seed_types, sizeof(seed_types)/sizeof(seed_types[0]));
    lua_setfield(L, -2, "SeedType");

    // === CoinType ===
    lua_newtable(L);
    static const EnumEntry coin_types[] = {
        {"SUN",         static_cast<lua_Integer>(CoinType::COIN_SUN)},
        {"SMALL_SUN",   static_cast<lua_Integer>(CoinType::COIN_SMALLSUN)},
        {"BIG_SUN",     static_cast<lua_Integer>(CoinType::COIN_LARGESUN)},
        {"COIN_SILVER", static_cast<lua_Integer>(CoinType::COIN_SILVER)},
        {"COIN_GOLD",   static_cast<lua_Integer>(CoinType::COIN_GOLD)},
        {"DIAMOND",     static_cast<lua_Integer>(CoinType::COIN_DIAMOND)},
    };
    RegisterEnum(L, coin_types, sizeof(coin_types)/sizeof(coin_types[0]));
    lua_setfield(L, -2, "CoinType");

    // 设置全局 pvz 表
    lua_setglobal(L, "pvz");
}

} // namespace ModLua
