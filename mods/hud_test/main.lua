-- hud_test/main.lua
-- 自定义 HUD 绘制测试 mod
--
-- 测试项：
--   1. on_board_draw_hud 事件回调被触发
--   2. g:draw_string / g:draw_string_color 画文字
--   3. g:fill_rect / g:draw_rect 画矩形
--   4. g:draw_line 画线条
--   5. g:set_color 设置颜色
--   6. g:set_font + pvz.fonts 设置字体
--   7. g:push_state / g:pop_state 状态栈
--   8. g:draw_image + pvz.images 画图片
--   9. g:string_width 测量文字宽度

local log = function(msg)
    print("[hud_test] " .. tostring(msg))
end

local frame_count = 0

return {
    on_app_init = function()
        log("hud_test mod loaded")
        log("pvz.fonts available keys:")
        for k, v in pairs(pvz.fonts or {}) do
            log("  font: " .. tostring(k))
        end
        log("pvz.images available keys:")
        local img_count = 0
        for k, v in pairs(pvz.images or {}) do
            img_count = img_count + 1
        end
        log("  total images: " .. img_count)
    end,

    on_board_draw_hud = function(board, g)
        frame_count = frame_count + 1

        -- 只在前 3 帧打日志，避免刷屏
        if frame_count <= 3 then
            log("on_board_draw_hud called, frame=" .. frame_count)
        end

        -- ====== 1. 画半透明背景条 ======
        g:push_state()
        g:set_color(0x80000000)  -- 半透明黑
        g:fill_rect(10, 10, 300, 60)
        g:pop_state()

        -- ====== 2. 画边框 ======
        g:push_state()
        g:set_color(0xFFFF0000)  -- 红色
        g:draw_rect(10, 10, 300, 60)
        g:pop_state()

        -- ====== 3. 画文字（用默认字体）======
        g:push_state()
        g:set_color(0xFFFFFFFF)  -- 白色
        g:draw_string("HUD Mod Test - Frame " .. frame_count, 20, 25)
        g:pop_state()

        -- ====== 4. 用 pvz.fonts 设置字体画文字 ======
        local font = pvz.fonts and pvz.fonts.BRIANNETOD16
        if font then
            g:push_state()
            g:set_font(font)
            g:set_color(0xFFFFFF00)  -- 黄色
            g:draw_string("Custom Font: BRIANNETOD16", 20, 45)
            g:pop_state()
        else
            if frame_count <= 3 then
                log("WARNING: pvz.fonts.BRIANNETOD16 not available")
            end
        end

        -- ====== 5. 画彩色文字（支持 ^RRGGBB^ 标签）======
        g:push_state()
        g:draw_string_color("^00FF00Green^FF0000Red^0000FFBlue", 20, 80)
        g:pop_state()

        -- ====== 6. 画线条 ======
        g:push_state()
        g:set_color(0xFF00FFFF)  -- 青色
        g:draw_line(10, 100, 310, 100)
        g:draw_line_aa(10, 102, 310, 102)
        g:pop_state()

        -- ====== 7. 画图片（如果有）======
        local img = pvz.images and pvz.images.SUNBANK
        if img then
            g:push_state()
            g:draw_image(img, 320, 10)
            g:pop_state()
            if frame_count == 1 then
                log("drew SUNBANK image, width=" .. tostring(img.width) .. " height=" .. tostring(img.height))
            end
        end

        -- ====== 8. 右上角 FPS 风格信息 ======
        g:push_state()
        g:set_color(0x80000000)
        g:fill_rect(680, 10, 140, 25)
        g:set_color(0xFF00FF00)
        g:draw_string("MOD HUD ACTIVE", 690, 18)
        g:pop_state()
    end,
}
