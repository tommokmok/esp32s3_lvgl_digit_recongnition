if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/touch_sensor_lowlevel.c")
if(CONFIG_TOUCH_SENSOR_LOWLEVEL_SIMULATOR)
    set(sources "touch_sensor_lowlevel_simulation.c"
                "simulation_data.c")
    message(STATUS "Building touch sensor lowlevel simulator")
else()
    set(sources "touch_sensor_lowlevel.c")
    message(STATUS "Building touch sensor lowlevel")
endif()
else()
    set(sources "")
endif()

idf_component_register(SRCS "${sources}"
                        INCLUDE_DIRS "include"
                        REQUIRES "driver" "esp_driver_touch_sens")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/touch_sensor_lowlevel.c")
    add_prebuilt_library(touch_sensor_lowlevel_lib "${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_IDF_TARGET}/libtouch_sensor_lowlevel.a"
                        REQUIRES "driver" "esp_driver_touch_sens")
    target_include_directories(${COMPONENT_LIB} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(${COMPONENT_LIB} INTERFACE touch_sensor_lowlevel_lib)
    message(STATUS "Using prebuilt touch sensor lowlevel library")
else()
    include(package_manager)
    cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})
    message(STATUS "Using local touch sensor lowlevel library")
    target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-ignored-qualifiers -Wno-int-to-pointer-cast -Wno-discarded-array-qualifiers)
endif()
