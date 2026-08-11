-- mod_custom_levels main.lua
-- 生存模式第二页：6个自定义测试关卡
--
-- 关卡设计：
--   Lv.1  白天草地  10波  150阳光  简单
--   Lv.2  黑夜草地  15波  100阳光  蘑菇醒着
--   Lv.3  泳池      15波  150阳光  水路
--   Lv.4  浓雾      20波  100阳光  视野受限
--   Lv.5  屋顶      20波  200阳光  斜坡
--   Lv.6  白天草地  30波  无限阳光 娱乐关

local M = {}

-- ========================================================================
-- 关卡配置表
-- ========================================================================
local LEVEL_CONFIG = {
    [1] = {
        name      = "草地入侵",
        terrain   = pvz.BackgroundType.BACKGROUND_1_DAY,
        num_waves = 10,
        sun       = 150,
        sun_floor = -1,
    },
    [2] = {
        name      = "黑夜突袭",
        terrain   = pvz.BackgroundType.BACKGROUND_2_NIGHT,
        num_waves = 15,
        sun       = 100,
        sun_floor = -1,
    },
    [3] = {
        name      = "泳池防御",
        terrain   = pvz.BackgroundType.BACKGROUND_3_POOL,
        num_waves = 15,
        sun       = 150,
        sun_floor = -1,
    },
    [4] = {
        name      = "迷雾重重",
        terrain   = pvz.BackgroundType.BACKGROUND_4_FOG,
        num_waves = 20,
        sun       = 100,
        sun_floor = -1,
    },
    [5] = {
        name      = "屋顶决战",
        terrain   = pvz.BackgroundType.BACKGROUND_5_ROOF,
        num_waves = 20,
        sun       = 200,
        sun_floor = -1,
    },
    [6] = {
        name      = "无尽狂欢",
        terrain   = pvz.BackgroundType.BACKGROUND_1_DAY,
        num_waves = 30,
        sun       = 9990,
        sun_floor = 9990,  -- 无限阳光
    },
}

-- 根据当前 game_mode 获取关卡序号（1-6），非自定义关卡返回 nil
local function get_custom_level_index(game_mode)
    local base = pvz.GameMode.MOD_CUSTOM_1
    local last = pvz.GameMode.MOD_CUSTOM_6
    if game_mode < base or game_mode > last then return nil end
    return game_mode - base + 1
end

-- ========================================================================
-- mod 加载时：注册6个关卡的封面图标和显示名称
-- ========================================================================
-- set_challenge_icon / set_challenge_name 在 mod 加载时调用一次即可，全局生效
-- 图片路径相对于 mod 目录（通过 PakInterface overlay 解析）
local function register_icons()
    for i = 1, 6 do
        local mode = pvz.GameMode.MOD_CUSTOM_1 + (i - 1)
        local path = string.format("images/custom_level_%d.png", i)
        pvz.set_challenge_icon(mode, path)
        pvz.set_challenge_name(mode, LEVEL_CONFIG[i].name)
    end
    print("[mod_custom_levels] 已注册6个自定义关卡封面图标和名称")
end

-- 安全调用 register_icons（pvz API 可能在 mod 加载时还未就绪）
local icons_registered = pcall(register_icons)

-- on_loading_completed: 如果加载时注册失败，在资源加载完成后重试
function M.on_loading_completed()
    if not icons_registered then
        icons_registered = pcall(register_icons)
    end
end

-- ========================================================================
-- on_level_init: 关卡初始化（Board::InitLevel 末尾触发）
-- 此时地形、波次已由引擎设置完毕，mod 可覆盖
-- ========================================================================
function M.on_level_init(board)
    local idx = get_custom_level_index(board.game_mode)
    if not idx then return end

    local cfg = LEVEL_CONFIG[idx]
    print(string.format("[mod_custom_levels] on_level_init: Lv.%d terrain=%d waves=%d sun=%d",
        idx, cfg.terrain, cfg.num_waves, cfg.sun))

    -- 切换地形（背景+行类型+网格类型）
    board:set_terrain(cfg.terrain)

    -- 设置波数
    board.num_waves = cfg.num_waves

    -- 设置阳光
    board.sun = cfg.sun

    -- 设置阳光下限（无限阳光模式）
    board.sun_floor = cfg.sun_floor

    -- Lv.2 黑夜关卡：通过 per-grid 地形让蘑菇在全图醒着
    if idx == 2 then
        for x = 0, 8 do
            for y = 0, 4 do
                board:set_grid_terrain(x, y, pvz.GridTerrain.NIGHT_GRASS)
            end
        end
        print("[mod_custom_levels] Lv.2: 全图设为黑夜草地（蘑菇不睡觉）")
    end

    -- Lv.6 娱乐关：per-grid 混合地形演示
    if idx == 6 then
        -- 中间一列设为水池
        for y = 0, 4 do
            board:set_grid_terrain(4, y, pvz.GridTerrain.POOL)
        end
        print("[mod_custom_levels] Lv.6: 中间列设为水池")
    end
end

-- ========================================================================
-- on_load_game: 暂停继续后恢复非存档状态
-- mSunMoneyFloor 和 mGridTerrainOverride 不存档，需重新设置
-- ========================================================================
function M.on_load_game(board)
    local idx = get_custom_level_index(board.game_mode)
    if not idx then return end

    local cfg = LEVEL_CONFIG[idx]
    print(string.format("[mod_custom_levels] on_load_game: 恢复 Lv.%d 状态", idx))

    -- 恢复阳光下限
    board.sun_floor = cfg.sun_floor

    -- Lv.2 恢复 per-grid 黑夜地形
    if idx == 2 then
        for x = 0, 8 do
            for y = 0, 4 do
                board:set_grid_terrain(x, y, pvz.GridTerrain.NIGHT_GRASS)
            end
        end
    end

    -- Lv.6 恢复 per-grid 水池地形
    if idx == 6 then
        for y = 0, 4 do
            board:set_grid_terrain(4, y, pvz.GridTerrain.POOL)
        end
    end
end

-- ========================================================================
-- on_pick_zombie_waves_post: 在默认波次表生成后追加僵尸
-- 让后期关卡更难
-- 参数: (board, level, num_waves)
-- 返回: { append = { [wave] = {zombie_type, ...}, ... } }
-- ========================================================================
function M.on_pick_zombie_waves_post(board, level, num_waves)
    local idx = get_custom_level_index(board.game_mode)
    if not idx then return nil end

    local append = {}

    -- Lv.5 屋顶关：每个旗帜波追加一个巨人僵尸
    if idx == 5 then
        local GARGANTUAR = pvz.ZombieType.GARGANTUAR
        for wave = 0, num_waves - 1 do
            if board:is_flag_wave(wave) then
                append[wave] = { GARGANTUAR }
            end
        end
        print("[mod_custom_levels] Lv.5: 旗帜波追加巨人僵尸")
    end

    -- Lv.6 娱乐关：每隔5波追加一个红眼巨人
    if idx == 6 then
        local REDEYE = pvz.ZombieType.REDEYE_GARGANTUAR
        for wave = 4, num_waves - 1, 5 do
            append[wave] = { REDEYE }
        end
        print("[mod_custom_levels] Lv.6: 每5波追加红眼巨人")
    end

    if next(append) then
        return { append = append }
    end
    return nil
end

return M
