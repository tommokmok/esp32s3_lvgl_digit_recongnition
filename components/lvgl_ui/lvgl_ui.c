/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "lvgl_ui.h"
#include "sketchpad.h"
#include "string.h"
/* LCD size */
#define EXAMPLE_LCD_H_RES (240)
#define EXAMPLE_LCD_V_RES (320)

#define CANVAS_WIDTH (25)  // row
#define CANVAS_HEIGHT (30) // col

/* LCD settings */
#define EXAMPLE_LCD_SPI_NUM (SPI3_HOST)
#define EXAMPLE_LCD_PIXEL_CLK_HZ (40 * 1000 * 1000)
#define EXAMPLE_LCD_CMD_BITS (8)
#define EXAMPLE_LCD_PARAM_BITS (8)
#define EXAMPLE_LCD_COLOR_SPACE (ESP_LCD_COLOR_SPACE_RGB)
#define EXAMPLE_LCD_BITS_PER_PIXEL (16)
#define EXAMPLE_LCD_DRAW_BUFF_DOUBLE (1)
#define EXAMPLE_LCD_DRAW_BUFF_HEIGHT (50)
#define EXAMPLE_LCD_BL_ON_LEVEL (0)

/* LCD pins */
#define EXAMPLE_LCD_GPIO_SCLK (GPIO_NUM_12)
#define EXAMPLE_LCD_GPIO_MOSI (GPIO_NUM_11)
#define EXAMPLE_LCD_GPIO_MISO (GPIO_NUM_13)
#define EXAMPLE_LCD_GPIO_RST (GPIO_NUM_14)
#define EXAMPLE_LCD_GPIO_DC (GPIO_NUM_9)
#define EXAMPLE_LCD_GPIO_CS (GPIO_NUM_10)
#define EXAMPLE_LCD_GPIO_BL (GPIO_NUM_4)

// /* LCD touch pins */

#define EXAMPLE_TOUCH_GPIO_INT (GPIO_NUM_40)

#define TOUCH_CS_PIN (GPIO_NUM_15)

static const char *TAG = "EXAMPLE";
static lv_timer_t *_predicTimer = NULL;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static lv_obj_t *_sketchpad;
// static uint8_t _drawline_flag = 0;
static void sketchpad_toolbar_event_cb(lv_event_t *e);

static Interface_send_to_dl_t *_pPredicFunc;

// LVGL image declare
LV_IMG_DECLARE(esp_logo)

/* LCD IO and panel */
static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

/* LVGL display and touch */
static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;

static uint8_t _grayScaleBuffer[25 * 30] = {0};

static esp_err_t app_lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    /* LCD backlight */
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_LCD_GPIO_BL};
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    /* LCD initialization */
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_LCD_GPIO_SCLK,
        .mosi_io_num = EXAMPLE_LCD_GPIO_MOSI,
        .miso_io_num = EXAMPLE_LCD_GPIO_MISO,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(EXAMPLE_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_LCD_GPIO_DC,
        .cs_gpio_num = EXAMPLE_LCD_GPIO_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLK_HZ,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)EXAMPLE_LCD_SPI_NUM, &io_config, &lcd_io), err, TAG, "New panel IO failed");

    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_LCD_GPIO_RST,
        .color_space = EXAMPLE_LCD_COLOR_SPACE,
        .bits_per_pixel = EXAMPLE_LCD_BITS_PER_PIXEL,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7789(lcd_io, &panel_config, &lcd_panel), err, TAG, "New panel failed");

    esp_lcd_panel_reset(lcd_panel);
    esp_lcd_panel_init(lcd_panel);
    esp_lcd_panel_mirror(lcd_panel, false, false);
    esp_lcd_panel_disp_on_off(lcd_panel, true);

    /* LCD backlight on */
    ESP_ERROR_CHECK(gpio_set_level(EXAMPLE_LCD_GPIO_BL, EXAMPLE_LCD_BL_ON_LEVEL));

    return ret;

err:
    if (lcd_panel)
    {
        esp_lcd_panel_del(lcd_panel);
    }
    if (lcd_io)
    {
        esp_lcd_panel_io_del(lcd_io);
    }
    spi_bus_free(EXAMPLE_LCD_SPI_NUM);
    return ret;
}

static esp_err_t app_touch_init(void)
{
    /* Initilize touch */

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_spi_config_t tp_io_config = ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(TOUCH_CS_PIN);
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)EXAMPLE_LCD_SPI_NUM, &tp_io_config, &tp_io_handle));

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,
        .y_max = EXAMPLE_LCD_V_RES,
        .rst_gpio_num = -1,
        .int_gpio_num = EXAMPLE_TOUCH_GPIO_INT,
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    xpt2046_set_cali_param(&tp_cfg, 280.0, 3600.0, 280.0, 3600.0);
    ESP_LOGI(TAG, "Initialize touch controller XPT2046");
    return esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &touch_handle);
}

static esp_err_t app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 4096,       /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5      /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUFF_HEIGHT,
        .double_buffer = EXAMPLE_LCD_DRAW_BUFF_DOUBLE,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
#if LVGL_VERSION_MAJOR >= 9
        .color_format = LV_COLOR_FORMAT_RGB565,
#endif
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
            .buff_spiram = true,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = true,
#endif
        }};
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    /* Add touch input (for selected screen) */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_disp,
        .handle = touch_handle,
    };
    lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);

    return ESP_OK;
}

// Convert ARGB8888 buffer to grayscale (1 byte per pixel)
void lv_color_argb8888_to_grayscale(const uint8_t *src, uint8_t *dst, int width, int height)
{
    int num_pixels = width * height;
    for (int i = 0; i < num_pixels; i++)
    {
        // ARGB8888: src[4*i+0]=A, src[4*i+1]=R, src[4*i+2]=G, src[4*i+3]=B
        uint8_t r = src[4 * i + 1];
        uint8_t g = src[4 * i + 2];
        uint8_t b = src[4 * i + 3];
        // Standard luminance formula
        uint8_t gray = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
        // if (gray > 128) gray = 1; else gray = 0; // Binarization
        dst[i] = gray;
    }
}

// Convert RGB565 buffer to grayscale (1 byte per pixel)
void lv_color_rgb565_to_grayscale(const uint16_t *src, uint8_t *dst, int width, int height)
{
    int num_pixels = width * height;
    for (int i = 0; i < num_pixels; i++)
    {
        uint16_t pixel = src[i];
        // Extract RGB components
        uint8_t r = (pixel >> 11) & 0x1F;

        uint8_t gray = 0;
        if ((r - 25) > 0)
            gray = 1; // Test on the output buffer. Min=25 for no point

        dst[i] = gray;
    }
}

/**
 * Convert a 128x128 RGB565 buffer to a 25x30 grayscale buffer (downsample).
 * src: pointer to 128x128 RGB565 buffer
 * dst: pointer to 25x30 grayscale buffer (1 byte per pixel)
 */
void lvgl_downsample_128x128_to_25x30(const uint16_t *src, uint8_t *dst)
{
    const int src_width = 128;
    const int src_height = 128;
    const int dst_width = 30;
    const int dst_height = 25;

    // Calculate scale factors
    float scale_x = (float)src_width / dst_width;
    float scale_y = (float)src_height / dst_height;

    for (int dy = 0; dy < dst_height; dy++)
    {
        for (int dx = 0; dx < dst_width; dx++)
        {
            // Find the corresponding source pixel (nearest neighbor)
            int sx = (int)(dx * scale_x);
            int sy = (int)(dy * scale_y);
            if (sx >= src_width)
                sx = src_width - 1;
            if (sy >= src_height)
                sy = src_height - 1;
            uint16_t pixel = src[sy * src_width + sx];

            // Convert RGB565 to grayscale
            uint8_t r = (pixel >> 11) & 0x1F;

            uint8_t gray = 0;
            if ((r - 25) > 0)
            {
                gray = 1; // Test on the output buffer. Min=25 for no point

                dst[dy * dst_width + dx] = gray;
            }
        }
    }
}
void print_pixel()
{
    for (int y = 0; y < CANVAS_WIDTH; y++)
    {
        for (int x = 0; x < CANVAS_HEIGHT; x++)
        {
            // printf(" %c ", _grayScaleBuffer[y * CANVAS_HEIGHT + x] == 0 ? '-' : '*');
            printf(" %3d ", _grayScaleBuffer[y * CANVAS_HEIGHT + x]);
        }
        printf("\n");
    }
}

// Delayed callback function
static void predict_delayed_cb(lv_timer_t *timer)
{
    // Example: call prediction function again after delay
    if (_sketchpad && _pPredicFunc)
    {
        lv_100ask_sketchpad_t *_sketchpad_t = (lv_100ask_sketchpad_t *)_sketchpad;
        lvgl_downsample_128x128_to_25x30((const uint16_t *)(_sketchpad_t->draw_buf->data), _grayScaleBuffer);
        ESP_ERROR_CHECK((*_pPredicFunc)(_grayScaleBuffer, sizeof(_grayScaleBuffer)));
    }
    
}

static void sketchpad_toolbar_event_cb(lv_event_t *e)
{
    lv_coord_t *toolbar_opt = (lv_coord_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    // lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    lv_100ask_sketchpad_t *_sketchpad_t = (lv_100ask_sketchpad_t *)_sketchpad;

    if (code == LV_EVENT_CLICKED)
    {
        if ((*toolbar_opt) == LV_100ASK_SKETCHPAD_TOOLBAR_OPT_DELETE)
        {

            lv_canvas_fill_bg(_sketchpad, lv_color_hex3(0xccc), LV_OPA_COVER);
            memset(_grayScaleBuffer, 0, sizeof(_grayScaleBuffer));
            lvgl_ui_update_digit_label(-1);
        }
        else if ((*toolbar_opt) == LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH)
        {

            // ESP_LOGI(TAG, "Width toolbar clicked");

            // Make a conversion from 128x128 to 25x30
            lvgl_downsample_128x128_to_25x30((const uint16_t *)(_sketchpad_t->draw_buf->data), _grayScaleBuffer);
            // lv_color_rgb565_to_grayscale((const uint16_t *)(_sketchpad_t->draw_buf->data), _grayScaleBuffer, 30, 25);

            ESP_ERROR_CHECK((*_pPredicFunc)(_grayScaleBuffer, sizeof(_grayScaleBuffer)));

            // print_pixel();
        }
    }
    else if (code == LV_EVENT_RELEASED)
    {
        // When user releases touch, start a timer to call predict_delayed_cb after 500ms
        // lv_timer_t *_predicTimer = lv_timer_create(predict_delayed_cb, 200, NULL);
        // lv_timer_set_repeat_count(_predicTimer, 1); // 
        // Optionally, store timer pointer if you want to cancel it later
        // ESP_LOGI(TAG, "LV_EVENT_RELEASED");
    }
}
#if 0
static void toolbar_set_event_cb(lv_event_t *e)
{
    lv_coord_t *toolbar_opt = (lv_coord_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    lv_100ask_sketchpad_t *sketchpad = (lv_100ask_sketchpad_t *)_sketchpad;

    if (code == LV_EVENT_RELEASED)
    {
        if (*(toolbar_opt) == LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH)
        {
            lv_obj_del(obj);
        }
    }
    else if (code == LV_EVENT_VALUE_CHANGED)
    {
        if ((*toolbar_opt) == LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH)
        {
            sketchpad->line_rect_dsc.width = lv_slider_get_value(obj);
        }
    }
}
#endif

static lv_obj_t *label_prediction = NULL;
static lv_obj_t *label_digit = NULL;

// Function to update the digit label
void lvgl_ui_update_digit_label(int digit)
{
    /* Task lock */
    lvgl_port_lock(0);
    if (label_digit)
    {
        if (digit < 0 || digit > 9)
        {
            lv_label_set_text(label_digit, "?"); // Invalid digit
        }
        else
        {
            char buf[8];
            snprintf(buf, sizeof(buf), "%d", digit);
            lv_label_set_text(label_digit, buf);
        }
    }
    /* Task unlock */
    lvgl_port_unlock();
}

static void app_main_display(void)
{

    /* Task lock */
    lvgl_port_lock(0);

    /* Your LVGL objects code here .... */
#if 1
    lv_obj_t *scr = lv_screen_active();
    lv_draw_buf_t *draw_buf = lv_draw_buf_create(128, 128, LV_COLOR_FORMAT_RGB565, LV_STRIDE_AUTO);
    _sketchpad = lv_100ask_sketchpad_create(scr);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF); // See no different in PC, but may have some difference in the TFT display
    lv_obj_set_scroll_snap_x(scr, LV_SCROLL_SNAP_NONE);
    lv_obj_set_scroll_snap_y(scr, LV_SCROLL_SNAP_NONE);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLL_ELASTIC); // Disable elastic scrolling>>not work
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLL_CHAIN);   // THis work

    lv_obj_set_scrollbar_mode(_sketchpad, LV_SCROLLBAR_MODE_OFF); // See no different in PC, but may have some difference in the TFT display
    lv_obj_set_scroll_snap_x(_sketchpad, LV_SCROLL_SNAP_NONE);
    lv_obj_set_scroll_snap_y(_sketchpad, LV_SCROLL_SNAP_NONE);
    lv_obj_clear_flag(_sketchpad, LV_OBJ_FLAG_SCROLL_ELASTIC); // Disable elastic scrolling>>not work
    lv_obj_clear_flag(_sketchpad, LV_OBJ_FLAG_SCROLL_CHAIN);   // THis work

    lv_canvas_set_draw_buf(_sketchpad, draw_buf);

    lv_canvas_fill_bg(_sketchpad, lv_color_hex3(0xccc), LV_OPA_COVER);
    lv_obj_align(_sketchpad, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(_sketchpad, sketchpad_toolbar_event_cb, LV_EVENT_RELEASED, NULL);

    /* Prediction label */
    label_prediction = lv_label_create(scr);
    lv_label_set_text_static(label_prediction, "Prediction:");
    lv_obj_align(label_prediction, LV_ALIGN_BOTTOM_LEFT, 10, -10);

    /* Digit label (dynamic) */
    label_digit = lv_label_create(scr);
    lv_label_set_text_static(label_digit, "0");
    lv_obj_align(label_digit, LV_ALIGN_BOTTOM_LEFT, 120, -10);

    /* Button */
    static lv_coord_t sketchpad_toolbar_cw = LV_100ASK_SKETCHPAD_TOOLBAR_OPT_DELETE;
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_t *label = lv_label_create(btn);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text_static(label, " Clear  ");
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_add_event_cb(btn, sketchpad_toolbar_event_cb, LV_EVENT_CLICKED, &sketchpad_toolbar_cw);

    /* Button */
    static lv_coord_t sketchpad_toolbar_width = LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH;
    lv_obj_t *btn2 = lv_btn_create(scr);
    lv_obj_t *label2 = lv_label_create(btn2);
    lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(btn2, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_label_set_text_static(label2, "Predict");
    lv_obj_add_event_cb(btn2, sketchpad_toolbar_event_cb, LV_EVENT_CLICKED, &sketchpad_toolbar_width);

#endif

    /* Task unlock */
    lvgl_port_unlock();
}

void lvgl_ui_init(Interface_send_to_dl_t *pFunc)
{
    assert(pFunc);
    _pPredicFunc = pFunc;
    // esp_log_level_set("xpt2046", ESP_LOG_VERBOSE);
    /* LCD HW initialization */
    ESP_ERROR_CHECK(app_lcd_init());

    /* Touch initialization */
    ESP_ERROR_CHECK(app_touch_init());

    /* LVGL initialization */
    ESP_ERROR_CHECK(app_lvgl_init());

    /* Show LVGL objects */
    app_main_display();
}
