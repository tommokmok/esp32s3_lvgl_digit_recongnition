
/**
 * @file sketchpad.h
 *
 */

#ifndef _SKETCHPAD_H
#define _SKETCHPAD_H

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "../lvgl__lvgl/src/lvgl_private.h"

#ifdef __cplusplus
extern "C" {
#endif




/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
	LV_100ASK_SKETCHPAD_TOOLBAR_OPT_ALL = 0,
    LV_100ASK_SKETCHPAD_TOOLBAR_OPT_DELETE,
	LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH,
	LV_100ASK_SKETCHPAD_TOOLBAR_OPT_LAST
}lv_100ask_sketchpad_toolbar_t;

/*Data of canvas*/
typedef struct {
    lv_image_t img;
    lv_draw_buf_t * draw_buf;
    lv_draw_buf_t static_buf;
    lv_draw_line_dsc_t line_rect_dsc;
    lv_layer_t layer;
    lv_point_t pos;
} lv_100ask_sketchpad_t;

/***********************
 * GLOBAL VARIABLES
 ***********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_obj_t * lv_100ask_sketchpad_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 *====================*/

/*=====================
 * Getter functions
 *====================*/

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/



#ifdef __cplusplus
} /*extern "C"*/
#endif


#endif
