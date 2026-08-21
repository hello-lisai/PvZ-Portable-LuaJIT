-- mod_custom_levels main.lua
-- 生存模式第二页：6个自定义测试关卡
--
-- 关卡设计（均为单阶段生存模式，默认10波，引擎自动生成波次表）：
--   Lv.1  白天草地  150阳光  简单
--   Lv.2  黑夜草地  100阳光  蘑菇醒着
--   Lv.3  泳池      150阳光  水路
--   Lv.4  浓雾      100阳光  视野受限
--   Lv.5  屋顶      200阳光  旗帜波出巨人
--   Lv.6  白天草地  无限阳光  每5波出红眼巨人

local M = {}

-- ========================================================================
-- 关卡配置表
-- num_waves 字段仅用于日志显示，实际波数由引擎 GetNumWavesPerSurvivalStage() 决定（默认10波）
-- 如需自定义波数，通过 on_pick_zombie_waves 事件返回 { waves = N, plan = {...} }
-- ========================================================================
local LEVEL_CONFIG = {
    [1] = {
        name      = "草地入侵",
        terrain   = pvz.BackgroundType.BACKGROUND_1_DAY,
        sun       = 150,
        sun_floor = -1,
    },
    [2] = {
        name      = "黑夜突袭",
        terrain   = pvz.BackgroundType.BACKGROUND_2_NIGHT,
        sun       = 100,
        sun_floor = -1,
    },
    [3] = {
        name      = "泳池防御",
        terrain   = pvz.BackgroundType.BACKGROUND_3_POOL,
        sun       = 150,
        sun_floor = -1,
    },
    [4] = {
        name      = "迷雾重重",
        terrain   = pvz.BackgroundType.BACKGROUND_4_FOG,
        sun       = 100,
        sun_floor = -1,
    },
    [5] = {
        name      = "屋顶决战",
        terrain   = pvz.BackgroundType.BACKGROUND_5_ROOF,
        sun       = 200,
        sun_floor = -1,
    },
    [6] = {
        name      = "无尽狂欢",
        terrain   = pvz.BackgroundType.BACKGROUND_1_DAY,
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

-- ========================================================================
-- 一次性重置 MOD_CUSTOM_1~6 的通关记录
-- 通过 config 标志位 records_reset_version 控制：
--   - 标志位不存在或版本号变化时，清零六关通关记录并立即保存
--   - 用户如需再次重置，删除 config.json 中本 mod 的 records_reset_version 字段即可
-- 在 on_loading_completed 中执行（此时 PlayerInfo 已加载完毕）
-- ========================================================================
local RECORDS_RESET_VERSION = 1  -- 递增此版本号可强制重新重置

local function reset_custom_level_records()
    -- 检查是否已执行过此版本的重置
    local last_version = pvz.config.get("records_reset_version")
    if last_version == RECORDS_RESET_VERSION then
        return  -- 已重置过，跳过
    end

    -- 重置 MOD_CUSTOM_1~6 的通关记录
    for i = 0, 5 do
        local mode = pvz.GameMode.MOD_CUSTOM_1 + i
        pvz.reset_challenge_record(mode)
    end
    print("[mod_custom_levels] 已重置 MOD_CUSTOM_1~6 通关记录")

    -- 标记已完成此版本的重置
    pvz.config.set("records_reset_version", RECORDS_RESET_VERSION)
end

-- on_loading_completed: 资源加载完成后执行（PlayerInfo 此时已加载）
function M.on_loading_completed()
    if not icons_registered then
        icons_registered = pcall(register_icons)
    end
    -- 执行一次性通关记录重置（pcall 保护，避免 API 未就绪时崩溃）
    pcall(reset_custom_level_records)
end

-- ========================================================================
-- on_level_init: 关卡初始化（Board::InitLevel 末尾触发）
-- 此时地形、波次已由引擎设置完毕，mod 可覆盖地形和阳光
-- 注意：不要在此设置 board.num_waves，因为波次表已在 PickZombieWaves 中按引擎默认值生成
--       自定义关卡走生存模式逻辑，mNumWaves = GetNumWavesPerSurvivalStage() = 10
--       如需自定义波数，请通过 on_pick_zombie_waves 事件返回 { waves = N, plan = {...} }
-- ========================================================================
function M.on_level_init(board)
    local idx = get_custom_level_index(board.game_mode)
    if not idx then return end

    local cfg = LEVEL_CONFIG[idx]
    print(string.format("[mod_custom_levels] on_level_init: Lv.%d terrain=%d sun=%d",
        idx, cfg.terrain, cfg.sun))

    -- 切换地形（背景+行类型+网格类型）
    board:set_terrain(cfg.terrain)

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
    -- 生存模式默认每10波一个旗帜（GetNumWavesPerFlag 返回 10）
    local waves_per_flag = 10

    -- Lv.5 屋顶关：每个旗帜波追加一个巨人僵尸
    if idx == 5 then
        local GARGANTUAR = pvz.ZombieType.GARGANTUAR
        for wave = 0, num_waves - 1 do
            -- 旗帜波：第 (waves_per_flag-1) 波（0-based），即第10波、第20波...
            if (wave + 1) % waves_per_flag == 0 then
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

-- ========================================================================
-- on_award_screen_draw: 自定义奖杯页面
-- 在 AwardScreen::Draw 开头触发，返回 {cancel=true} 跳过默认绘制
-- 仅对 mod 自定义关卡（MOD_CUSTOM_1~6）生效，其他模式不接管
-- 参数: (graphics, award_type, level, game_mode, showing_achievements)
-- ========================================================================
local function draw_centered_string(g, font, text, cx, y)
    g:set_font(font)
    local w = g:string_width(text)
    g:draw_string(text, cx - w / 2, y)
end

function M.on_award_screen_draw(g, award_type, level, game_mode, showing_achievements)
    local idx = get_custom_level_index(game_mode)
    if not idx then return nil end  -- 非 mod 自定义关卡，不接管

    -- 仅对通关奖杯页面接管（AWARD_FORLEVEL）；其他类型（如成就页）不接管
    if award_type ~= pvz.AwardType.FORLEVEL then
        return nil
    end

    -- 如果正在显示成就列表，让默认绘制跑完（不接管），成就显示完后再接管
    if showing_achievements then
        return nil
    end

    -- 1. 绘制奖杯页面背景（与原版 DrawBottom 一致）
    local bg = pvz.images.AWARDSCREEN_BACK
    if bg then
        g:draw_image(bg, 0, 0)
    end

    -- 2. 绘制奖杯图片（居中）
    local trophy = pvz.images.TROPHY_HI_RES
    if trophy then
        g:draw_image(trophy, 450 - trophy:width() / 2, 137)
    end

    -- 3. 绘制自定义标题
    local cfg = LEVEL_CONFIG[idx]
    draw_centered_string(g, pvz.fonts.DWARVENTODCRAFT24,
        string.format("[ Lv.%d  %s  通关 ]", idx, cfg.name), 450, 58)

    -- 4. 绘制奖项名称
    draw_centered_string(g, pvz.fonts.DWARVENTODCRAFT18YELLOW,
        "[TROPHY]", 450, 326)

    -- 5. 绘制消息文本（手动按行换行以适配 230x90 区域）
    local message_lines
    if idx < 6 then
        message_lines = {
            string.format("[ 恭喜通关 Lv.%d ]", idx),
            string.format("[ 已解锁 Lv.%d: %s ]", idx + 1, LEVEL_CONFIG[idx + 1].name),
            "[ 点击下方按钮继续 ]",
        }
    else
        message_lines = {
            "[ 恭喜通关全部 6 个自定义关卡 ]",
            "[ 你已证明自己的实力 ]",
            "[ 点击下方按钮返回菜单 ]",
        }
    end
    local msg_y = 360
    for _, line in ipairs(message_lines) do
        draw_centered_string(g, pvz.fonts.BRIANNETOD16, line, 450, msg_y)
        msg_y = msg_y + 22  -- 行距
    end

    -- 返回 cancel=true 跳过默认绘制（按钮和淡入遮罩仍由原代码绘制）
    return { cancel = true }
end

return M
