#include "ui_preview.h"
#include "CodexUi.h"
#include <stdio.h>
#include <string.h>
static CodexUiModel sample(void){
 CodexUiModel m={0};m.online=1;
 strcpy(m.badge,"PLUS");strcpy(m.short_reset,"Resets 4h 59m");strcpy(m.weekly_reset,"Resets 6d 23h");
 strcpy(m.network,"Demo / Online");strcpy(m.freshness,"Updated 1m");strcpy(m.ssid,"Demo");
 strcpy(m.address,"192.0.2.10");strcpy(m.temperature,"51 C");strcpy(m.rotation,"Automatic");return m;
}
lv_obj_t *codex_usage_screen(int w,int h,int a,int b){CodexUiModel m=sample();m.short_left=a;m.weekly_left=b;return codex_screen(w,h,&m);}
lv_obj_t *codex_device_screen(int w,int h){CodexUiModel m=sample();m.page=1;return codex_screen(w,h,&m);}
static int verify(lv_obj_t *obj){
 int errors=0;lv_area_t parent;lv_obj_get_coords(obj,&parent);
 for(uint32_t i=0;i<lv_obj_get_child_cnt(obj);i++){
  lv_obj_t *child=lv_obj_get_child(obj,i);lv_area_t a;lv_obj_get_coords(child,&a);
  if(a.x1<parent.x1||a.y1<parent.y1||a.x2>parent.x2||a.y2>parent.y2){
   fprintf(stderr,"Clipped %s: (%d,%d)-(%d,%d) outside (%d,%d)-(%d,%d)\n",lv_obj_check_type(child,&lv_label_class)?lv_label_get_text(child):"object",a.x1,a.y1,a.x2,a.y2,parent.x1,parent.y1,parent.x2,parent.y2);errors++;
  }
  errors+=verify(child);
 }
 return errors;
}
int codex_verify_geometry(lv_obj_t *screen){lv_obj_update_layout(screen);return verify(screen);}
