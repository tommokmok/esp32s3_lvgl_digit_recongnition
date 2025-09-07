
idf_component_register(SRCS "esp_lcd_touch_xpt2046.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "driver esp_lcd_touch")
                       # in a component's CMakeLists.txt file
target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLOG_LOCAL_LEVEL=ESP_LOG_VERBOSE")