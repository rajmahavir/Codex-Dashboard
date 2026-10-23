#pragma once
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
LV_FONT_DECLARE(cx_font_lbl);
LV_FONT_DECLARE(cx_font_val);
LV_FONT_DECLARE(cx_font_pct);
LV_FONT_DECLARE(cx_font_num);
LV_FONT_DECLARE(cx_font_hero);
typedef struct {
 int short_left, weekly_left, page;
 char badge[20], short_reset[32], weekly_reset[32], network[48], freshness[48];
 char ssid[33], address[24], temperature[20], rotation[24];
 char setup_password[24];
 int online;
} CodexUiModel;
lv_obj_t *codex_screen(int width,int height,const CodexUiModel *model);
#ifdef __cplusplus
}
#endif
