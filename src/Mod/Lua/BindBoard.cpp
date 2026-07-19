#include "LuaBindUtil.h"
#include "../../Lawn/Board.h"
#include "../../Lawn/SeedPacket.h"
#include "../../LawnApp.h"
#include "../../Resources.h"            // IMAGE_SEEDBANK（set_seed_packet 更新 SeedBank 宽度）

namespace ModLua {

void PushZombie(lua_State* L, Zombie* z);
void PushPlant(lua_State* L, Plant* p);
void PushProjectile(lua_State* L, Projectile* p);
void PushCoin(lua_State* L, Coin* c);

namespace {

// === Board 字段读写（getter/setter）===

// board.sun_money (读写)
int l_board_get_sun(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, b->mSunMoney);
    return 1;
}
int l_board_set_sun(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    b->mSunMoney = static_cast<int32_t>(luaL_checkinteger(L, 2));
    return 0;
}

// board.level
int l_board_get_level(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, b->mLevel);
    return 1;
}

// board.frame (mMainCounter)
int l_board_get_frame(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, b->mMainCounter);
    return 1;
}

// board.current_wave
int l_board_get_wave(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, b->mCurrentWave);
    return 1;
}

// board.num_waves
int l_board_get_num_waves(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, b->mNumWaves);
    return 1;
}

// board.paused
int l_board_get_paused(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushboolean(L, b->mPaused);
    return 1;
}
int l_board_set_paused(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    b->mPaused = lua_toboolean(L, 2) != 0;
    return 0;
}

// === Board 方法 ===

// board:add_sun(amount)  -- 增加/减少阳光（正数加，负数扣）
int l_board_add_sun(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    int amount = static_cast<int>(luaL_checkinteger(L, 2));
    b->AddSunMoney(amount);
    return 0;
}

// board:take_sun(amount) -> bool  -- 尝试扣除阳光
int l_board_take_sun(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    int amount = static_cast<int>(luaL_checkinteger(L, 2));
    lua_pushboolean(L, b->TakeSunMoney(amount));
    return 1;
}

// board:add_zombie(zombie_type, row, from_wave) -> Zombie
int l_board_add_zombie(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    ZombieType zt = static_cast<ZombieType>(luaL_checkinteger(L, 2));
    int fromWave = static_cast<int>(luaL_optinteger(L, 3, b->mCurrentWave));
    Zombie* z = b->AddZombie(zt, fromWave);
    PushZombie(L, z);
    return 1;
}

// board:add_plant(grid_x, grid_y, seed_type) -> Plant
int l_board_add_plant(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    int gx = static_cast<int>(luaL_checkinteger(L, 2));
    int gy = static_cast<int>(luaL_checkinteger(L, 3));
    SeedType st = static_cast<SeedType>(luaL_checkinteger(L, 4));
    Plant* p = b->AddPlant(gx, gy, st);
    PushPlant(L, p);
    return 1;
}

// board:add_projectile(x, y, row, projectile_type) -> Projectile
// 注意：mod 手动创建的投射物默认 mDamageRangeFlags=1（DAMAGES_GROUND），
// 与原版豌豆射手 GetDamageRangeFlags 的 default 返回值一致，
// 确保投射物能正确碰撞地面僵尸（mDamageRangeFlags=0 会导致部分碰撞判定异常）
int l_board_add_projectile(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    int x = static_cast<int>(luaL_checkinteger(L, 2));
    int y = static_cast<int>(luaL_checkinteger(L, 3));
    int row = static_cast<int>(luaL_checkinteger(L, 4));
    ProjectileType pt = static_cast<ProjectileType>(luaL_checkinteger(L, 5));
    Projectile* p = b->AddProjectile(x, y, 0, row, pt);
    if (p) {
        // 默认 DAMAGE_RANGE_FLAGS_GROUND = 1（bit 0 = DAMAGES_GROUND）
        p->mDamageRangeFlags = 1;
    }
    PushProjectile(L, p);
    return 1;
}

// board:add_coin(x, y, coin_type, coin_motion) -> Coin
int l_board_add_coin(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    int x = static_cast<int>(luaL_checkinteger(L, 2));
    int y = static_cast<int>(luaL_checkinteger(L, 3));
    CoinType ct = static_cast<CoinType>(luaL_checkinteger(L, 4));
    CoinMotion cm = static_cast<CoinMotion>(luaL_optinteger(L, 5, 0));
    Coin* c = b->AddCoin(x, y, ct, cm);
    PushCoin(L, c);
    return 1;
}

// board:remove_all_zombies()
int l_board_remove_all_zombies(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    b->RemoveAllZombies();
    return 0;
}

// board:pause(true/false)
int l_board_pause(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    b->Pause(lua_toboolean(L, 2) != 0);
    return 0;
}

// board:grid_to_pixel_x(row, col) -> x, y
int l_board_grid_to_pixel(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    int row = static_cast<int>(luaL_checkinteger(L, 2));
    int col = static_cast<int>(luaL_checkinteger(L, 3));
    int x = b->GridToPixelX(col, row);
    int y = b->GridToPixelY(col, row);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

// === Board 迭代器 ===
// board:for_each_zombie(function(zombie) ... end)
// board:for_each_plant(function(plant) ... end)
// board:for_each_projectile(function(proj) ... end)
// board:for_each_coin(function(coin) ... end)

int l_board_for_each_zombie(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    luaL_checktype(L, 2, LUA_TFUNCTION);
    Zombie* z = nullptr;
    while (b->IterateZombies(z)) {
        if (!z) continue;
        lua_pushvalue(L, 2);          // 复制回调函数
        PushZombie(L, z);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            lua_pop(L, 1); // 丢弃错误继续
        }
    }
    return 0;
}

int l_board_for_each_plant(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    luaL_checktype(L, 2, LUA_TFUNCTION);
    Plant* p = nullptr;
    while (b->IteratePlants(p)) {
        if (!p) continue;
        lua_pushvalue(L, 2);
        PushPlant(L, p);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            lua_pop(L, 1);
        }
    }
    return 0;
}

int l_board_for_each_projectile(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    luaL_checktype(L, 2, LUA_TFUNCTION);
    Projectile* p = nullptr;
    while (b->IterateProjectiles(p)) {
        if (!p) continue;
        lua_pushvalue(L, 2);
        PushProjectile(L, p);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            lua_pop(L, 1);
        }
    }
    return 0;
}

int l_board_for_each_coin(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    luaL_checktype(L, 2, LUA_TFUNCTION);
    Coin* c = nullptr;
    while (b->IterateCoins(c)) {
        if (!c) continue;
        lua_pushvalue(L, 2);
        PushCoin(L, c);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            lua_pop(L, 1);
        }
    }
    return 0;
}

// board:count_zombies() -> int
int l_board_count_zombies(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) { lua_pushinteger(L, 0); return 1; }
    int n = 0;
    Zombie* z = nullptr;
    while (b->IterateZombies(z)) if (z) ++n;
    lua_pushinteger(L, n);
    return 1;
}

int l_board_count_plants(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) { lua_pushinteger(L, 0); return 1; }
    int n = 0;
    Plant* p = nullptr;
    while (b->IteratePlants(p)) if (p) ++n;
    lua_pushinteger(L, n);
    return 1;
}

// board:set_seed_packet(slot_index, seed_type) -> bool
// 在指定卡槽设置种子卡片（支持自定义植物）。若 slot >= mNumPackets，自动扩展卡槽数量
// 典型用法：on_level_start 时 board:set_seed_packet(6, custom_seed_type) 添加自定义植物到种子栏
int l_board_set_seed_packet(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) { lua_pushboolean(L, false); return 1; }
    int slot = static_cast<int>(luaL_checkinteger(L, 2));
    SeedType st = static_cast<SeedType>(luaL_checkinteger(L, 3));

    if (slot < 0 || slot >= SEEDBANK_MAX) { lua_pushboolean(L, false); return 1; }
    if (!b->mSeedBank) { lua_pushboolean(L, false); return 1; }

    // 扩展卡槽数量（若 slot 超出当前 mNumPackets）
    if (slot + 1 > b->mSeedBank->mNumPackets) {
        b->mSeedBank->mNumPackets = slot + 1;
        // mNumPackets 变化后，重新计算 SeedBank 宽度和所有卡槽位置
        // 不能调用 UpdateWidth()，因为它会用 GetNumSeedsInBank() 重置 mNumPackets
        b->mSeedBank->mWidth = Sexy::IMAGE_SEEDBANK->GetWidth() + b->GetSeedBankExtraWidth();
        for (int i = 0; i < b->mSeedBank->mNumPackets; i++) {
            b->mSeedBank->mSeedPackets[i].mX = b->GetSeedPacketPositionX(i);
        }
    }

    SeedPacket& packet = b->mSeedBank->mSeedPackets[slot];
    packet.SetPacketType(st);
    packet.mIndex = slot;
    packet.mY = 8;
    lua_pushboolean(L, true);
    return 1;
}

// board:get_ptr() — 返回原始指针（light userdata），供 LuaJIT FFI 使用
int l_board_get_ptr(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) { lua_pushnil(L); return 1; }
    lua_pushlightuserdata(L, b);
    return 1;
}

// board:__index 分发
int l_board_index(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) { lua_pushnil(L); return 1; }
    const char* key = luaL_checkstring(L, 2);

    // 属性
    if (strcmp(key, "sun") == 0)         return l_board_get_sun(L);
    if (strcmp(key, "level") == 0)       return l_board_get_level(L);
    if (strcmp(key, "frame") == 0)       return l_board_get_frame(L);
    if (strcmp(key, "wave") == 0)        return l_board_get_wave(L);
    if (strcmp(key, "num_waves") == 0)   return l_board_get_num_waves(L);
    if (strcmp(key, "paused") == 0)      return l_board_get_paused(L);

    // 方法（push C 闭包）
    struct { const char* name; lua_CFunction fn; } methods[] = {
        {"add_sun",               l_board_add_sun},
        {"take_sun",              l_board_take_sun},
        {"add_zombie",            l_board_add_zombie},
        {"add_plant",             l_board_add_plant},
        {"add_projectile",        l_board_add_projectile},
        {"add_coin",              l_board_add_coin},
        {"remove_all_zombies",    l_board_remove_all_zombies},
        {"pause",                 l_board_pause},
        {"grid_to_pixel",         l_board_grid_to_pixel},
        {"for_each_zombie",       l_board_for_each_zombie},
        {"for_each_plant",        l_board_for_each_plant},
        {"for_each_projectile",   l_board_for_each_projectile},
        {"get_ptr",               l_board_get_ptr},
        {"for_each_coin",         l_board_for_each_coin},
        {"count_zombies",         l_board_count_zombies},
        {"count_plants",          l_board_count_plants},
        {"set_seed_packet",       l_board_set_seed_packet},
    };
    for (auto& m : methods) {
        if (strcmp(key, m.name) == 0) {
            lua_pushcfunction(L, m.fn);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

// board:__newindex（写入属性）
int l_board_newindex(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    const char* key = luaL_checkstring(L, 2);
    if (strcmp(key, "sun") == 0) {
        b->mSunMoney = static_cast<int32_t>(luaL_checkinteger(L, 3));
        return 0;
    }
    if (strcmp(key, "paused") == 0) {
        b->mPaused = lua_toboolean(L, 3) != 0;
        return 0;
    }
    return 0;
}

} // namespace

void BindBoard(lua_State* L) {
    CreateMetatable(L, MT_BOARD);
    SetFuncField(L, "__index",    l_board_index);
    SetFuncField(L, "__newindex", l_board_newindex);
    lua_pop(L, 1); // 弹出 metatable

    // 全局函数 get_board() 返回当前 Board（gLawnApp->mBoard）
    lua_pushcfunction(L, [](lua_State* L) -> int {
        if (gLawnApp && gLawnApp->mBoard) {
            NewUserdata(L, gLawnApp->mBoard, MT_BOARD);
        } else {
            lua_pushnil(L);
        }
        return 1;
    });
    lua_setglobal(L, "get_board");
}

void PushBoard(lua_State* L, Board* board) {
    NewUserdata(L, board, MT_BOARD);
}

} // namespace ModLua
