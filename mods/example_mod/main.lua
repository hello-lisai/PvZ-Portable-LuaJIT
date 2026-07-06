-- example_mod/main.lua
-- Mod 入口文件：返回一个包含事件回调的 table
--
-- 可用的事件回调（在 pvz.api_version_minor 对应版本中）：
--   on_app_init()                      引擎初始化完成
--   on_loading_completed()             资源加载完成，进入主菜单
--   on_board_update_pre(board)         每帧 Board::Update 入口
--   on_board_update_post(board)        每帧 Board::Update 末尾
--   on_update_game_objects_pre(board)  每帧 UpdateGameObjects 入口（对象 Update 之前）
--   on_plant_created(plant)            植物创建
--   on_zombie_created(zombie)          僵尸创建
--   on_projectile_created(projectile)  弹道创建
--   on_coin_created(coin)              金币/阳光创建
--   on_griditem_created(griditem)      场地物品创建（墓碑/梯子/坑洞/花盆/传送门等）
--   on_lawnmower_created(mower)        割草机创建
--   on_object_destroyed(type, obj)     对象销毁（type 为字符串："plant"/"zombie"/"griditem"/"lawnmower"/...）
--   on_zombie_take_damage(z, dmg, flags) -> {cancel=?, damage=?}  可拦截：僵尸受伤
--   on_projectile_impact(proj, zombie) -> {cancel=?}              可拦截：弹道撞击
--   on_spawn_zombie_wave(board)        -> {cancel=?}              可拦截：僵尸波次生成
--   on_pick_zombie_waves(board, level) -> {waves=N, plan=...}     可拦截：覆盖整张波次表
--   on_pick_zombie_type(board, level, wave, points) -> {zombie_type=N}  可拦截：替换选中僵尸类型
--   on_level_init(board)               关卡初始化
--   on_level_start(board)              关卡开始
--   on_level_end(board)                关卡失败
--   on_key_down(board, key) -> {cancel=?}                         可拦截：按键
--   on_mouse_down(board, x, y, btn) -> {cancel=?}                可拦截：鼠标按下
--   on_mouse_up(board, x, y, btn) -> {cancel=?}                  可拦截：鼠标抬起
--   on_sun_changed(board, delta)       阳光变化
--
-- 全局 API：
--   get_board()         获取当前 Board
--   pvz.ZombieType.XXX  僵尸类型枚举
--   pvz.SeedType.XXX    植物类型枚举
--   pvz.CoinType.XXX    金币类型枚举
--   pvz.api_version_major / pvz.api_version_minor
--   pvz.config.get(key)           读取当前 mod 的配置项（不存在返回 nil）
--   pvz.config.set(key, value)    写入当前 mod 的配置项（立即持久化到 mods/config.json）
--   pvz.config.all()              返回当前 mod 的全部配置（table）
--
-- 出怪系统 API：
--   pvz.waves.get_num_waves(level)                获取某关卡默认总波数
--   pvz.waves.set_num_waves(level, count)         修改某关卡默认总波数
--   pvz.waves.is_zombie_allowed(ztype, level)     该僵尸类型是否允许在该关卡出现
--   pvz.waves.allow_zombie(ztype, level, allowed) 允许/禁止某僵尸在某关卡出现
--
-- 动态注册 API（路线 C+D）：
--   pvz.reanim.register(file_path, flags=0)       注册新动画类型，返回 ReanimationType
--   pvz.plants.register(def_table)                注册新植物类型，返回 SeedType
--   pvz.zombies.register(def_table)               注册新僵尸类型，返回 ZombieType
--
-- 配置持久化：
--   用户配置保存在 mods/config.json，结构如下：
--   { "version": 1, "mods": { "<mod目录名>": { "enabled": bool, "config": {...} } } }
--   config.json 的 enabled 字段会覆盖 mod.lua 清单中的 enabled（用户设置优先级最高）
--   每个 mod 的 config 子对象是隔离的，mod A 无法读写 mod B 的配置
--
-- LuaJIT FFI：
--   mod 可使用 require'ffi' 直接声明并调用 C 函数，无需 C++ 侧写绑定。
--   ffi.C 默认访问进程符号表（C 标准库 + 游戏导出符号）。
--   示例见 on_app_init 中的 FFI 自检。
--
--   未来路线：游戏侧逐步把关键函数加 PVZ_API 导出宏，mod 即可直接通过
--   ffi.cdef + ffi.C 调用，无需在 LuaRuntime.cpp 里写 C++ 绑定函数。
--   这是「少改源码，mod 完成一切」的核心机制。

local M = {}

-- FFI 自检：启动时用 ffi.C.printf 打印一条消息，验证 LuaJIT FFI 链路通畅
local ffi_ok, ffi = pcall(require, "ffi")
if ffi_ok then
    ffi.cdef[[
        int printf(const char* fmt, ...);
        size_t strlen(const char* s);
    ]]
end

-- 引擎初始化完成时打印
function M.on_app_init()
    print("===== [示例Mod] Mod API 已加载 =====")
    print(string.format("  API 版本: %d.%d", pvz.api_version_major, pvz.api_version_minor))

    -- FFI 自检：用 C 的 printf 打印（而非 Lua 的 print），证明 FFI 可用
    if ffi_ok then
        ffi.C.printf("[示例Mod] FFI 自检通过: strlen(\"hello\") = %d\n", ffi.C.strlen("hello"))
    else
        print("[示例Mod] FFI 不可用（require 'ffi' 失败）")
    end

    -- 首次运行时写入默认配置
    if not pvz.config.get("initialized") then
        pvz.config.set("initialized", true)
        pvz.config.set("cheat_key", "K")           -- 作弊按键
        pvz.config.set("sun_bonus", 9999)           -- 作弊阳光数
        pvz.config.set("difficulty", 1.0)           -- 难度倍率
        print("[示例Mod] 首次运行，已写入默认配置到 mods/config.json")
    end

    -- 读取配置
    local cheat_key = pvz.config.get("cheat_key")
    local sun_bonus = pvz.config.get("sun_bonus")
    print(string.format("[示例Mod] 当前配置: cheat_key=%s sun_bonus=%d", cheat_key, sun_bonus))
end

-- 资源加载完成、进入主菜单
function M.on_loading_completed()
    print("[示例Mod] 资源加载完成，已进入主菜单")
end

-- 关卡开始时提示
function M.on_level_start(board)
    if not board then return end
    print(string.format("[示例Mod] 关卡 %d 开始，初始阳光 %d", board.level, board.sun))
end

-- 每帧末尾：每 2 秒（120 帧）打印一次场上僵尸数量
function M.on_board_update_post(board)
    if not board then return end
    if board.frame % 120 == 0 then
        local n = board:count_zombies()
        if n > 0 then
            print(string.format("[示例Mod] 当前场上僵尸数: %d", n))
        end
    end
end

-- 僵尸创建：如果是巨人僵尸，血量翻倍
function M.on_zombie_created(zombie)
    if not zombie then return end
    if zombie.type == pvz.ZombieType.GARGANTUAR then
        zombie.max_health = zombie.max_health * 2
        zombie.health = zombie.max_health
        print("[示例Mod] 检测到巨人僵尸，血量翻倍至 " .. zombie.health)
    end
end

-- 对象销毁：只打印僵尸销毁
function M.on_object_destroyed(type_str, obj)
    if type_str == "zombie" and obj then
        print(string.format("[示例Mod] 僵尸被消灭 (row=%d)", obj.row))
    end
end

-- 场地物品创建：默认静默，取消注释可看到所有场地物品（墓碑/梯子/坑洞/花盆等）
function M.on_griditem_created(item)
    if not item then return end
    -- print(string.format("[示例Mod] 场地物品创建 type=%d grid=(%d,%d)", item.type, item.grid_x, item.grid_y))
end

-- 割草机创建：默认静默
function M.on_lawnmower_created(mower)
    if not mower then return end
    -- print(string.format("[示例Mod] 割草机创建 row=%d", mower.row))
end

-- 僵尸波次生成（可拦截）：演示返回 cancel 取消波次
-- 注意：取消波次会影响游戏进度，默认不取消，仅演示
function M.on_spawn_zombie_wave(board)
    -- 取消下面一行的注释可以阻止所有僵尸波次生成（仅用于演示）
    -- return { cancel = true }
end

-- 阳光变化时记录（默认静默，取消注释可看到所有阳光变化）
function M.on_sun_changed(board, delta)
    if not board then return end
    -- print(string.format("[示例Mod] 阳光变化 %+d，当前 %d", delta, board.sun))
end

-- 按键事件：按配置的作弊键加配置的阳光数
function M.on_key_down(board, key)
    if not board then return end
    -- KeyCode.K 通常是 75，但允许用户通过 config.json 自定义
    local cheat_key = pvz.config.get("cheat_key") or "K"
    local expected = string.byte(cheat_key, 1) or 75
    if key == expected then
        local bonus = pvz.config.get("sun_bonus") or 9999
        board:add_sun(bonus)
        print(string.format("[示例Mod] 作弊：阳光 +%d", bonus))
    end
end

-- 鼠标按下（可拦截）：演示获取坐标（默认静默）
function M.on_mouse_down(board, x, y, btn)
    if not board then return end
    -- print(string.format("[示例Mod] 鼠标按下 (%d, %d) btn=%d", x, y, btn))
end

-- =============================================================================
-- 出怪系统演示（路线 B：数据驱动出怪）
-- =============================================================================

-- on_pick_zombie_waves：可覆盖整张波次表
-- 参数：board, level
-- 返回 {waves=N, plan={[0]={zt1,zt2,...}, [1]={...}, ...}} 替换默认出怪
-- 或返回 {cancel=true} 跳过默认逻辑（保持 mZombiesInWave 原样）
-- 不返回则使用默认出怪
function M.on_pick_zombie_waves(board, level)
    if not board then return end
    -- 演示：第 1 关改为 3 波，每波都是普通僵尸
    -- 取消下面注释可生效
    -- if level == 1 then
    --     return {
    --         waves = 3,
    --         plan = {
    --             [0] = {pvz.ZombieType.ZOMBIE_NORMAL, pvz.ZombieType.ZOMBIE_NORMAL},
    --             [1] = {pvz.ZombieType.ZOMBIE_NORMAL, pvz.ZombieType.ZOMBIE_CONE},
    --             [2] = {pvz.ZombieType.ZOMBIE_NORMAL, pvz.ZombieType.ZOMBIE_CONE, pvz.ZombieType.ZOMBIE_BUCKET},
    --         }
    --     }
    -- end
end

-- on_pick_zombie_type：可替换选中的僵尸类型
-- 参数：board, level, wave_index, points
-- 返回 {zombie_type=N} 替换选中的类型
-- 不返回则使用默认选中结果
function M.on_pick_zombie_type(board, level, wave_index, points)
    if not board then return end
    -- 演示：最后一波（wave_index == num_waves - 1）全部替换为巨人僵尸
    -- 取消下面注释可生效
    -- if board and wave_index == board.num_waves - 1 then
    --     return { zombie_type = pvz.ZombieType.GARGANTUAR }
    -- end
end

-- =============================================================================
-- 动态注册演示（路线 C+D：注册新动画/植物/僵尸）
-- =============================================================================

-- on_app_init 中注册自定义内容（取消注释可生效）
-- 注意：注册新类型需要对应的 .reanim 资源文件存在
function M.on_app_init_custom_register()
    -- 1. 注册新动画类型（需要 mods/example_mod/reanim/xxx.reanim 文件存在）
    -- local reanim_type = pvz.reanim.register("mods/example_mod/reanim/custom_zombie.reanim")
    -- print(string.format("[示例Mod] 注册新动画类型: %d", reanim_type))

    -- 2. 注册新植物类型（使用上面注册的动画）
    -- local plant_type = pvz.plants.register({
    --     name = "CUSTOM_PLANT",
    --     cost = 150,
    --     refresh = 750,
    --     reanim_type = reanim_type,
    --     packet_index = 50,
    --     subclass = 1,  -- SHOOTER
    --     launch_rate = 150,
    -- })
    -- print(string.format("[示例Mod] 注册新植物类型: %d", plant_type))

    -- 3. 注册新僵尸类型
    -- local zombie_type = pvz.zombies.register({
    --     name = "CUSTOM_ZOMBIE",
    --     reanim_type = reanim_type,
    --     value = 5,
    --     starting_level = 1,
    --     first_allowed_wave = 1,
    --     pick_weight = 1000,
    -- })
    -- print(string.format("[示例Mod] 注册新僵尸类型: %d", zombie_type))

    -- 4. 允许新僵尸在第 1 关出现
    -- pvz.waves.allow_zombie(zombie_type, 1, true)
end

return M
