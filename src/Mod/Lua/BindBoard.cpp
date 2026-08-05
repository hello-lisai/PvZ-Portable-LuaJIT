#include "LuaBindUtil.h"
#include "../../Lawn/Board.h"
#include "../../Lawn/SeedPacket.h"
#include "../../Lawn/GridItem.h"
#include "../../Lawn/Zombie.h"
#include "../../LawnApp.h"
#include "../../Resources.h"            // IMAGE_SEEDBANK（set_seed_packet 更新 SeedBank 宽度）
#include "../../SexyAppFramework/graphics/MemoryImage.h"
#include "../../SexyAppFramework/imagelib/ImageLib.h"
#include <cstdio>
#include <cstring>

namespace ModLua {

void PushZombie(lua_State* L, Zombie* z);
void PushPlant(lua_State* L, Plant* p);
void PushProjectile(lua_State* L, Projectile* p);
void PushCoin(lua_State* L, Coin* c);
void PushGridItem(lua_State* L, GridItem* g);

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

// board.game_mode —— 当前游戏模式（GameMode 枚举值）
// 用于判断关卡类型：冒险模式、生存模式、挑战模式（如 POGO_PARTY）等
// mod 中可用 pvz.GameMode.POGO_PARTY 等常量进行比较
int l_board_get_game_mode(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b || !b->mApp) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, static_cast<lua_Integer>(b->mApp->mGameMode));
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

// board.background —— 关卡背景类型（BackgroundType 枚举值）
// 用于判断关卡场景：白天/黑夜/泳池/雾/屋顶/Boss 等
// mod 中可用 pvz.BackgroundType.BACKGROUND_1_DAY 等常量进行比较
int l_board_get_background(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, static_cast<lua_Integer>(b->mBackground));
    return 1;
}
int l_board_set_background(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    b->mBackground = static_cast<BackgroundType>(luaL_checkinteger(L, 2));
    return 0;
}

// board.waves_per_flag —— 每多少波一个旗帜（0=用原版默认逻辑）
// mod 可设置此值控制旗帜 UI 的数量和分布
// 例：设为 8 时，32 波会有 4 面旗帜（第8/16/24/32波）
int l_board_get_waves_per_flag(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, b->mWavesPerFlagOverride);
    return 1;
}
int l_board_set_waves_per_flag(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    b->mWavesPerFlagOverride = static_cast<int32_t>(luaL_checkinteger(L, 2));
    return 0;
}

// board.num_waves_per_flag —— 当前生效的每多少波一个旗帜（只读）
// 返回 GetNumWavesPerFlag() 的结果（考虑 override 后的实际值）
int l_board_get_num_waves_per_flag(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, b->GetNumWavesPerFlag());
    return 1;
}

// board.progress_meter_width —— 进度条宽度（0-150，只读）
// 反映当前关卡进度，用于自定义 HUD 绘制
int l_board_get_progress_meter_width(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, b->mProgressMeterWidth);
    return 1;
}

// board.zombie_count_down —— 下一波僵尸生成倒计时（读写）
// mod 可修改此值控制僵尸生成节奏
int l_board_get_zombie_count_down(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, b->mZombieCountDown);
    return 1;
}
int l_board_set_zombie_count_down(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    b->mZombieCountDown = static_cast<int32_t>(luaL_checkinteger(L, 2));
    return 0;
}

// board.huge_wave_count_down —— 大波僵尸倒计时（读写）
// 旗帜波前的"巨大浪潮"倒计时
int l_board_get_huge_wave_count_down(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, b->mHugeWaveCountDown);
    return 1;
}
int l_board_set_huge_wave_count_down(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    b->mHugeWaveCountDown = static_cast<int32_t>(luaL_checkinteger(L, 2));
    return 0;
}

// board.total_spawned_waves —— 已生成的总波数（只读）
int l_board_get_total_spawned_waves(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    lua_pushinteger(L, b->mTotalSpawnedWaves);
    return 1;
}

// === Board 方法 ===

// board:is_flag_wave(wave) -> bool —— 判断指定波次是否是旗帜波
int l_board_is_flag_wave(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) { lua_pushboolean(L, 0); return 1; }
    int wave = static_cast<int>(luaL_checkinteger(L, 2));
    lua_pushboolean(L, b->IsFlagWave(wave) ? 1 : 0);
    return 1;
}

// board:has_progress_meter() -> bool —— 是否显示进度条
int l_board_has_progress_meter(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) { lua_pushboolean(L, 0); return 1; }
    lua_pushboolean(L, b->HasProgressMeter() ? 1 : 0);
    return 1;
}

// board:get_survival_flags_completed() -> int —— 生存模式已完成的旗帜数
int l_board_get_survival_flags_completed(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) { lua_pushinteger(L, 0); return 1; }
    lua_pushinteger(L, b->GetSurvivalFlagsCompleted());
    return 1;
}

// === 关卡类型判断便捷方法 ===
// 这些方法封装了常见的 game_mode 判断，避免 mod 重复写枚举比较

// board:is_adventure() -> bool
int l_board_is_adventure(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b || !b->mApp) { lua_pushboolean(L, 0); return 1; }
    lua_pushboolean(L, b->mApp->IsAdventureMode() ? 1 : 0);
    return 1;
}

// board:is_survival() -> bool
int l_board_is_survival(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b || !b->mApp) { lua_pushboolean(L, 0); return 1; }
    lua_pushboolean(L, b->mApp->IsSurvivalMode() ? 1 : 0);
    return 1;
}

// board:is_challenge() -> bool —— 是否是挑战模式（迷你游戏）
int l_board_is_challenge(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b || !b->mApp) { lua_pushboolean(L, 0); return 1; }
    lua_pushboolean(L, b->mApp->IsChallengeMode() ? 1 : 0);
    return 1;
}

// board:is_puzzle() -> bool —— 是否是解谜模式（砸罐/我僵尸）
int l_board_is_puzzle(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b || !b->mApp) { lua_pushboolean(L, 0); return 1; }
    lua_pushboolean(L, b->mApp->IsPuzzleMode() ? 1 : 0);
    return 1;
}

// board:is_final_boss() -> bool
int l_board_is_final_boss(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b || !b->mApp) { lua_pushboolean(L, 0); return 1; }
    lua_pushboolean(L, b->mApp->IsFinalBossLevel() ? 1 : 0);
    return 1;
}

// board:is_first_time_adventure() -> bool —— 是否是首次冒险模式（教程关）
int l_board_is_first_time_adventure(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b || !b->mApp) { lua_pushboolean(L, 0); return 1; }
    lua_pushboolean(L, b->mApp->IsFirstTimeAdventureMode() ? 1 : 0);
    return 1;
}

// === Board 原有方法（add_sun 等）===

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

// board:set_background_image(image_path) — 加载图片文件并设为自定义背景
// image_path: 相对资源目录的图片路径（如 "images/my_bg.png"），支持 mod overlay
// 传入 nil 或空字符串则清除自定义背景，恢复原版
// 图片在关卡退出时自动释放
int l_board_set_background_image(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;

    // 释放旧的自定义背景
    if (b->mCustomBackgroundImage) {
        delete b->mCustomBackgroundImage;
        b->mCustomBackgroundImage = nullptr;
    }

    // nil 或空字符串：清除自定义背景
    if (lua_isnil(L, 2) || (lua_isstring(L, 2) && lua_tostring(L, 2)[0] == '\0')) {
        std::fprintf(stdout, "[ModAPI] clear_background_image (nil/empty)\n");
        std::fflush(stdout);
        return 0;
    }

    const char* path = luaL_checkstring(L, 2);
    std::fprintf(stdout, "[ModAPI] set_background_image: path='%s'\n", path);
    std::fflush(stdout);

    // 用 ImageLib 加载图片文件（支持 PNG/JPG/GIF/TGA，通过 PakInterface 支持 mod overlay）
    ImageLib::Image* srcImg = ImageLib::GetImage(path, false);
    if (!srcImg) {
        std::fprintf(stdout, "[ModAPI] FAILED to load background image: %s\n", path);
        std::fflush(stdout);
        luaL_error(L, "failed to load background image: %s", path);
        return 0;
    }

    std::fprintf(stdout, "[ModAPI] background image loaded: %dx%d\n", srcImg->mWidth, srcImg->mHeight);
    std::fflush(stdout);

    // 创建 MemoryImage 并拷贝像素数据
    MemoryImage* memImg = new MemoryImage(gLawnApp);
    memImg->Create(srcImg->mWidth, srcImg->mHeight);
    uint32_t* bits = memImg->GetBits();
    if (srcImg->mBits) {
        std::memcpy(bits, srcImg->mBits, sizeof(uint32_t) * srcImg->mWidth * srcImg->mHeight);
    }
    memImg->BitsChanged();
    delete srcImg;

    b->mCustomBackgroundImage = memImg;
    std::fprintf(stdout, "[ModAPI] mCustomBackgroundImage set OK (ptr=%p)\n", (void*)memImg);
    std::fflush(stdout);
    return 0;
}

// board:clear_background_image() — 清除自定义背景，恢复原版
int l_board_clear_background_image(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    if (b->mCustomBackgroundImage) {
        delete b->mCustomBackgroundImage;
        b->mCustomBackgroundImage = nullptr;
    }
    return 0;
}

// ===== 场地控制 API =====

// board:add_crater(grid_x, grid_y) — 在指定格子创建弹坑（阻止种植，类似末日蘑菇爆炸后的坑）
int l_board_add_crater(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    int gx = static_cast<int>(luaL_checkinteger(L, 2));
    int gy = static_cast<int>(luaL_checkinteger(L, 3));
    GridItem* crater = b->AddACrater(gx, gy);
    PushGridItem(L, crater);
    return 1;
}

// board:remove_crater(grid_x, grid_y) — 移除指定格子的弹坑
int l_board_remove_crater(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    int gx = static_cast<int>(luaL_checkinteger(L, 2));
    int gy = static_cast<int>(luaL_checkinteger(L, 3));
    GridItem* crater = b->GetCraterAt(gx, gy);
    if (crater) {
        crater->GridItemDie();
        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

// board:add_gravestone(grid_x, grid_y) — 在指定格子创建墓碑（阻止种植，可被墓碑吞噬者清除）
int l_board_add_gravestone(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    int gx = static_cast<int>(luaL_checkinteger(L, 2));
    int gy = static_cast<int>(luaL_checkinteger(L, 3));
    GridItem* grave = b->AddAGraveStone(gx, gy);
    PushGridItem(L, grave);
    return 1;
}

// board:bungee_drop_zombie(zombie_type, grid_x, grid_y) — 蹦极僵尸空降指定僵尸到指定位置
// 效果与蹦极僵尸关（Bungee Blitz）相同：蹦极僵尸从天而降，将指定僵尸放在目标格子
int l_board_bungee_drop_zombie(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    ZombieType zt = static_cast<ZombieType>(luaL_checkinteger(L, 2));
    int gx = static_cast<int>(luaL_checkinteger(L, 3));
    int gy = static_cast<int>(luaL_checkinteger(L, 4));

    Zombie* aBungee = b->AddZombie(ZombieType::ZOMBIE_BUNGEE, b->mCurrentWave);
    Zombie* aDropped = b->AddZombie(zt, b->mCurrentWave);
    if (aBungee && aDropped) {
        aBungee->BungeeDropZombie(aDropped, gx, gy);
    }
    PushZombie(L, aDropped);
    return 1;
}

// board:spawn_zombie_at(zombie_type, grid_x, grid_y) — 在指定格子直接生成僵尸（无蹦极动画）
// 僵尸会出现在指定格子的像素坐标，适合墓碑冒僵尸等场景
int l_board_spawn_zombie_at(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    ZombieType zt = static_cast<ZombieType>(luaL_checkinteger(L, 2));
    int gx = static_cast<int>(luaL_checkinteger(L, 3));
    int gy = static_cast<int>(luaL_checkinteger(L, 4));

    Zombie* z = b->AddZombieInRow(zt, gy, b->mCurrentWave);
    if (z) {
        // 设置到指定格子的 X 坐标
        z->mPosX = b->GridToPixelX(gx, gy);
        z->mPosY = b->GridToPixelY(gx, gy);
    }
    PushZombie(L, z);
    return 1;
}

// board:can_plant_at(grid_x, grid_y, seed_type) -> bool — 查询能否在指定格子种植指定植物
int l_board_can_plant_at(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) return 0;
    int gx = static_cast<int>(luaL_checkinteger(L, 2));
    int gy = static_cast<int>(luaL_checkinteger(L, 3));
    SeedType st = static_cast<SeedType>(luaL_checkinteger(L, 4));
    PlantingReason reason = b->CanPlantAt(gx, gy, st);
    lua_pushboolean(L, reason == PlantingReason::PLANTING_OK);
    return 1;
}


// board:__index 分发
int l_board_index(lua_State* L) {
    Board* b = CheckUserdata<Board>(L, 1, MT_BOARD);
    if (!b) { lua_pushnil(L); return 1; }
    const char* key = luaL_checkstring(L, 2);

    // 属性
    if (strcmp(key, "sun") == 0)                  return l_board_get_sun(L);
    if (strcmp(key, "level") == 0)                return l_board_get_level(L);
    if (strcmp(key, "frame") == 0)                return l_board_get_frame(L);
    if (strcmp(key, "wave") == 0)                 return l_board_get_wave(L);
    if (strcmp(key, "num_waves") == 0)            return l_board_get_num_waves(L);
    if (strcmp(key, "game_mode") == 0)            return l_board_get_game_mode(L);
    if (strcmp(key, "paused") == 0)               return l_board_get_paused(L);
    if (strcmp(key, "background") == 0)           return l_board_get_background(L);
    if (strcmp(key, "waves_per_flag") == 0)       return l_board_get_waves_per_flag(L);
    if (strcmp(key, "num_waves_per_flag") == 0)   return l_board_get_num_waves_per_flag(L);
    if (strcmp(key, "progress_meter_width") == 0) return l_board_get_progress_meter_width(L);
    if (strcmp(key, "zombie_count_down") == 0)    return l_board_get_zombie_count_down(L);
    if (strcmp(key, "huge_wave_count_down") == 0) return l_board_get_huge_wave_count_down(L);
    if (strcmp(key, "total_spawned_waves") == 0)  return l_board_get_total_spawned_waves(L);

    // 方法（push C 闭包）
    struct { const char* name; lua_CFunction fn; } methods[] = {
        {"add_sun",                    l_board_add_sun},
        {"take_sun",                   l_board_take_sun},
        {"add_zombie",                 l_board_add_zombie},
        {"add_plant",                  l_board_add_plant},
        {"add_projectile",             l_board_add_projectile},
        {"add_coin",                   l_board_add_coin},
        {"remove_all_zombies",         l_board_remove_all_zombies},
        {"pause",                      l_board_pause},
        {"grid_to_pixel",              l_board_grid_to_pixel},
        {"for_each_zombie",            l_board_for_each_zombie},
        {"for_each_plant",             l_board_for_each_plant},
        {"for_each_projectile",        l_board_for_each_projectile},
        {"get_ptr",                    l_board_get_ptr},
        {"for_each_coin",              l_board_for_each_coin},
        {"count_zombies",              l_board_count_zombies},
        {"count_plants",               l_board_count_plants},
        {"set_seed_packet",            l_board_set_seed_packet},
        // 波次/进度条相关
        {"is_flag_wave",               l_board_is_flag_wave},
        {"has_progress_meter",         l_board_has_progress_meter},
        {"get_survival_flags_completed", l_board_get_survival_flags_completed},
        // 关卡类型判断
        {"is_adventure",               l_board_is_adventure},
        {"is_survival",                l_board_is_survival},
        {"is_challenge",               l_board_is_challenge},
        {"is_puzzle",                  l_board_is_puzzle},
        {"is_final_boss",              l_board_is_final_boss},
        {"is_first_time_adventure",    l_board_is_first_time_adventure},
        // 自定义背景图片
        {"set_background_image",       l_board_set_background_image},
        {"clear_background_image",     l_board_clear_background_image},
        // 场地控制
        {"add_crater",                 l_board_add_crater},
        {"remove_crater",              l_board_remove_crater},
        {"add_gravestone",             l_board_add_gravestone},
        {"bungee_drop_zombie",         l_board_bungee_drop_zombie},
        {"spawn_zombie_at",            l_board_spawn_zombie_at},
        {"can_plant_at",               l_board_can_plant_at},
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
    if (strcmp(key, "background") == 0) {
        b->mBackground = static_cast<BackgroundType>(luaL_checkinteger(L, 3));
        return 0;
    }
    if (strcmp(key, "waves_per_flag") == 0) {
        b->mWavesPerFlagOverride = static_cast<int32_t>(luaL_checkinteger(L, 3));
        return 0;
    }
    if (strcmp(key, "num_waves") == 0) {
        b->mNumWaves = static_cast<int32_t>(luaL_checkinteger(L, 3));
        return 0;
    }
    if (strcmp(key, "zombie_count_down") == 0) {
        b->mZombieCountDown = static_cast<int32_t>(luaL_checkinteger(L, 3));
        return 0;
    }
    if (strcmp(key, "huge_wave_count_down") == 0) {
        b->mHugeWaveCountDown = static_cast<int32_t>(luaL_checkinteger(L, 3));
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
