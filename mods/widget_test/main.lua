-- widget_test/main.lua
-- 验证 LuaWidget 自定义 UI 系统
--
-- 测试项：
--   1. pvz.widget.new(x, y, w, h) 创建自定义 Widget
--   2. set_on_draw(g) 绘制半透明面板 + 文字
--   3. set_on_mouse_down(x, y, btn) 开始拖拽
--   4. set_on_mouse_drag(x, y) 拖拽移动面板
--   5. set_on_mouse_move(x, y) 鼠标悬停高亮
--   6. set_on_mouse_up(x, y, btn) 结束拖拽
--   7. set_on_key_down(key) 按键切换可见性
--   8. set_on_key_char(char) 字符输入（显示在面板）
--   9. add_to_manager / remove_from_manager 生命周期
--
-- 验证方法：
--   - 启动游戏进入关卡
--   - on_level_start 时创建 Widget 并 add_to_manager
--   - 屏幕左上角出现半透明面板，可拖拽移动
--   - 按 H 键切换面板可见性
--   - 按键输入字符会显示在面板上
--   - on_level_end 时 remove_from_manager 清理

local M = {}

-- KeyCode 常量（H 键 = 72 ASCII）
local KEY_H = 72

local g_widget = nil
local g_dragging = false
local g_drag_offset_x = 0
local g_drag_offset_y = 0
local g_hover = false
local g_input_text = ""
local g_frame = 0

function M.on_level_start(board)
    if g_widget ~= nil then
        -- 已存在，先清理
        g_widget:remove_from_manager()
        g_widget = nil
    end

    if not pvz or not pvz.widget or not pvz.widget.new then
        print("[widget_test] ERROR: pvz.widget.new 不可用")
        return
    end

    -- 创建 280x120 的面板，放在 (50, 50)
    g_widget = pvz.widget.new(50, 50, 280, 120)

    g_widget:set_on_draw(function(g)
        g_frame = g_frame + 1
        -- 半透明背景
        g:push_state()
        g:set_color(0xC0000000)  -- 黑色 75% 透明
        g:fill_rect(0, 0, 280, 120)
        -- 边框（悬停时高亮）
        if g_hover then
            g:set_color(0xFFFFFF00)  -- 黄色
        else
            g:set_color(0xFFFFFFFF)  -- 白色
        end
        g:draw_rect(0, 0, 280, 120)
        g:pop_state()

        -- 标题
        g:set_color(0xFFFFFFFF)
        g:draw_string("[widget_test] Custom Panel", 8, 8)

        -- 状态信息
        if g_dragging then
            g:set_color(0xFF00FF00)  -- 绿色
            g:draw_string("DRAGGING", 8, 30)
        elseif g_hover then
            g:set_color(0xFFFFFF00)  -- 黄色
            g:draw_string("HOVER", 8, 30)
        end

        -- 帧计数
        g:set_color(0xFFCCCCCC)
        g:draw_string("frame: " .. g_frame, 8, 50)

        -- 输入文本
        g:set_color(0xFF00CCFF)
        g:draw_string("input: " .. g_input_text, 8, 70)

        -- 提示
        g:set_color(0xFF888888)
        g:draw_string("drag to move | H to toggle | type chars", 8, 95)
    end)

    g_widget:set_on_update(function()
        -- 每帧重置 hover（mouse_move 会设 true，实现"鼠标不动则失焦"）
        g_hover = false
    end)

    g_widget:set_on_mouse_down(function(x, y, btn)
        if btn == 0 then  -- 左键
            g_dragging = true
            g_drag_offset_x = x
            g_drag_offset_y = y
            return true  -- 已处理
        end
    end)

    g_widget:set_on_mouse_up(function(x, y, btn)
        if btn == 0 then
            g_dragging = false
            return true
        end
    end)

    g_widget:set_on_mouse_move(function(x, y)
        g_hover = true
    end)

    g_widget:set_on_mouse_drag(function(x, y)
        if g_dragging and g_cur_x and g_cur_y then
            -- x,y 是相对 widget 的局部坐标，增量移动
            local dx = x - g_drag_offset_x
            local dy = y - g_drag_offset_y
            g_cur_x = g_cur_x + dx
            g_cur_y = g_cur_y + dy
            g_widget:set_pos(g_cur_x, g_cur_y)
        end
    end)

    g_widget:set_on_key_down(function(key)
        if key == KEY_H then
            -- 切换可见性
            g_visible = not g_visible
            g_widget:set_visible(g_visible)
            return true  -- 已处理
        end
    end)

    g_widget:set_on_key_char(function(ch)
        -- 累加输入字符
        g_input_text = g_input_text .. ch
        if #g_input_text > 20 then
            g_input_text = ""  -- 超长清空
        end
        return true  -- 已处理
    end)

    -- 初始状态
    g_cur_x = 50
    g_cur_y = 50
    g_visible = true

    local ok = g_widget:add_to_manager()
    if ok then
        print("[widget_test] Widget created and added to manager at (50, 50)")
        print("[widget_test] drag to move | press H to toggle visibility | type chars to input")
    else
        print("[widget_test] ERROR: add_to_manager failed")
    end
end

function M.on_level_end()
    if g_widget ~= nil then
        g_widget:remove_from_manager()
        g_widget = nil
        print("[widget_test] Widget removed from manager on level end")
    end
    -- 重置状态
    g_dragging = false
    g_hover = false
    g_input_text = ""
    g_frame = 0
end

print("[widget_test] 已加载：自定义 Widget 验证（面板可拖拽，H 切换可见性）")
return M
