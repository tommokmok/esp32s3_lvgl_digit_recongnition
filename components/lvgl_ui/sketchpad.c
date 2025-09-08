
/**
 * @file sketchpad.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "sketchpad.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_100ask_sketchpad_class
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_100ask_sketchpad_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void lv_100ask_sketchpad_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void lv_100ask_sketchpad_event(const lv_obj_class_t *class_p, lv_event_t *e);


/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_100ask_sketchpad_class = {
    .constructor_cb = lv_100ask_sketchpad_constructor,
    .destructor_cb = lv_100ask_sketchpad_destructor,
    .event_cb = lv_100ask_sketchpad_event,
    .instance_size = sizeof(lv_100ask_sketchpad_t),
    .base_class = &lv_image_class,
    .name = "sketchpad",
};



/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t *lv_100ask_sketchpad_create(lv_obj_t *parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t *obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*=====================
 * Other functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_100ask_sketchpad_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_100ask_sketchpad_t *sketchpad = (lv_100ask_sketchpad_t *)obj;

    lv_draw_line_dsc_init(&sketchpad->line_rect_dsc);
    sketchpad->line_rect_dsc.width = 4;
    sketchpad->line_rect_dsc.round_start = true;
    sketchpad->line_rect_dsc.round_end = true;
    sketchpad->line_rect_dsc.color = lv_palette_main(LV_PALETTE_RED);

    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);



    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_100ask_sketchpad_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_canvas_t *canvas = (lv_canvas_t *)obj;
    if (canvas->draw_buf == NULL)
        return;

    lv_image_cache_drop(&canvas->draw_buf);
}

static void lv_100ask_sketchpad_event(const lv_obj_class_t *class_p, lv_event_t *e)
{
    LV_UNUSED(class_p);

    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK)
        return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_100ask_sketchpad_t *sketchpad = (lv_100ask_sketchpad_t *)obj;

    static lv_coord_t last_x = -32768, last_y = -32768;

    if (code == LV_EVENT_PRESSING)
    {
        lv_indev_t *indev = lv_indev_active();
        if (indev == NULL)
            return;

        lv_point_t point;

        lv_indev_get_point(indev, &point);

        // LV_LOG_USER("point: %d, %d", point.x, point.y);
        // LV_LOG_USER("sketchpad point: %d, %d", sketchpad->pos.x, sketchpad->pos.y);

        point.x = point.x - sketchpad->pos.x;
        point.y = point.y - sketchpad->pos.y;
        /*Release or first use*/
        if ((last_x != -32768) && (last_y != -32768))
        {
            lv_layer_t layer;
            lv_canvas_init_layer(obj, &layer);

            sketchpad->line_rect_dsc.p1.x = last_x;
            sketchpad->line_rect_dsc.p1.y = last_y;
            sketchpad->line_rect_dsc.p2.x = point.x;
            sketchpad->line_rect_dsc.p2.y = point.y;

            lv_draw_line(&layer, &sketchpad->line_rect_dsc);
            lv_canvas_finish_layer(obj, &layer);

            lv_obj_send_event(obj, LV_EVENT_VALUE_CHANGED, NULL);
        }

        last_x = point.x;
        last_y = point.y;
    }

    /*Loosen the brush*/
    else if (code == LV_EVENT_RELEASED)
    {
        last_x = -32768;
        last_y = -32768;
    }
    else if ((code == LV_EVENT_STYLE_CHANGED) || (code == LV_EVENT_SIZE_CHANGED))
    {
        lv_obj_refr_pos(obj);
        sketchpad->pos.x = lv_obj_get_x(obj);
        sketchpad->pos.y = lv_obj_get_y(obj);
        // LV_LOG_USER("pos.x,y changed: %d, %d", sketchpad->pos.x, sketchpad->pos.y);
    }
}
