#ifndef CODEX_UI_PREVIEW_H
#define CODEX_UI_PREVIEW_H
#include "lvgl.h"

lv_obj_t *codex_usage_screen(int width,int height,int short_left,int weekly_left);
lv_obj_t *codex_device_screen(int width,int height);
int codex_verify_geometry(lv_obj_t *screen);
#endif
