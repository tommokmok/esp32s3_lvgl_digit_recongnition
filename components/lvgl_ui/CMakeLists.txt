idf_component_register(SRCS "lvgl_ui.c" "sketchpad.c"
                    INCLUDE_DIRS "."
                    REQUIRES "atanisoft__esp_lcd_touch_xpt2046 espressif__esp_lvgl_port lvgl__lvgl"
                    )

# lvgl_port_create_c_image("images/esp_logo.png" "images/" "ARGB8888" "NONE")
# lvgl_port_add_images(${COMPONENT_LIB} "images/")
