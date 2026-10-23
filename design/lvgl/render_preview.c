#include "ui_preview.h"
#include "CodexUi.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
static lv_color_t draw_buffer[320*40];
static uint16_t framebuffer[320*172];
static int width,height;
static void flush(lv_disp_drv_t *driver,const lv_area_t *area,lv_color_t *pixels){
 for(int y=area->y1;y<=area->y2;y++)for(int x=area->x1;x<=area->x2;x++){
  if(x>=0&&x<width&&y>=0&&y<height)framebuffer[y*width+x]=pixels->full;
  pixels++;
 }
 lv_disp_flush_ready(driver);
}
static void save(const char *path){
 FILE *f=fopen(path,"wb");if(!f){perror(path);exit(2);}fprintf(f,"FRAME %d\n",width*height*2);
 for(int i=0;i<width*height;i++){fputc(framebuffer[i]>>8,f);fputc(framebuffer[i]&255,f);}fclose(f);
}
int main(int argc,char **argv){
 if(argc<6){fprintf(stderr,"Usage: preview width height short_left weekly_left output_prefix\n");return 2;}
 width=atoi(argv[1]);height=atoi(argv[2]);int a=atoi(argv[3]),b=atoi(argv[4]);const char *prefix=argv[5];
 if(width*height!=320*172||a < -1||a>100||b < -1||b>100)return 2;
 lv_init();static lv_disp_draw_buf_t buf;lv_disp_draw_buf_init(&buf,draw_buffer,NULL,320*40);
 static lv_disp_drv_t drv;lv_disp_drv_init(&drv);drv.hor_res=width;drv.ver_res=height;drv.draw_buf=&buf;drv.flush_cb=flush;lv_disp_drv_register(&drv);
 lv_obj_t *main_screen=codex_usage_screen(width,height,a,b),*device_screen=codex_device_screen(width,height);
 int errors=codex_verify_geometry(main_screen)+codex_verify_geometry(device_screen);if(errors)return 1;
 CodexUiModel edge={0};edge.short_left=edge.weekly_left=-1;
 strcpy(edge.badge,"PREVIEW");strcpy(edge.ssid,"A very long Wi-Fi network name");
 strcpy(edge.setup_password,"setup-1234567890");strcpy(edge.address,"192.168.100.200");
 strcpy(edge.temperature,"66.5 C");strcpy(edge.rotation,"Automatic");
 strcpy(edge.network,"Connecting Wi-Fi");strcpy(edge.freshness,"Saved / retrying");
 strcpy(edge.short_reset,"Awaiting data");strcpy(edge.weekly_reset,"Awaiting data");
 for(int page=0;page<3;page++){edge.page=page;lv_obj_t *s=codex_screen(width,height,&edge);errors+=codex_verify_geometry(s);lv_obj_del(s);}
 if(errors)return 1;
 lv_scr_load(main_screen);lv_timer_handler();lv_refr_now(NULL);
 char file[256];snprintf(file,sizeof(file),"%s-main.raw",prefix);save(file);
 lv_scr_load_anim(device_screen,LV_SCR_LOAD_ANIM_MOVE_LEFT,192,0,false);
 for(int frame=0;frame<=12;frame++){
  lv_tick_inc(16);lv_timer_handler();lv_refr_now(NULL);
  snprintf(file,sizeof(file),"%s-transition-%02d.raw",prefix,frame);save(file);
 }
 lv_tick_inc(100);lv_timer_handler();lv_refr_now(NULL);snprintf(file,sizeof(file),"%s-device.raw",prefix);save(file);
 lv_scr_load_anim(main_screen,LV_SCR_LOAD_ANIM_MOVE_RIGHT,192,0,false);
 for(int frame=0;frame<=12;frame++){
  lv_tick_inc(16);lv_timer_handler();lv_refr_now(NULL);
  snprintf(file,sizeof(file),"%s-return-%02d.raw",prefix,frame);save(file);
 }
 // Repeated rotation/page replacement must release the old widget trees.
 uint32_t baseline=0;lv_mem_monitor_t memory;
 for(int cycle=0;cycle<44;cycle++){
  width=cycle%4<2?320:172;height=cycle%4<2?172:320;
  drv.hor_res=width;drv.ver_res=height;lv_disp_drv_update(lv_disp_get_default(),&drv);
  edge.page=cycle%2;edge.short_left=62;edge.weekly_left=13;
  lv_obj_t *replacement=codex_screen(width,height,&edge);
  if(codex_verify_geometry(replacement))return 1;
  lv_scr_load_anim(replacement,LV_SCR_LOAD_ANIM_MOVE_LEFT,192,0,true);
  for(int frame=0;frame<14;frame++){lv_tick_inc(16);lv_timer_handler();}
  lv_mem_monitor(&memory);
  if(cycle==3)baseline=memory.free_size;
 }
 if(memory.free_size+1024<baseline){fprintf(stderr,"UI lifecycle leaked memory\n");return 1;}
 puts("44 rotation/page transitions passed the memory check.");
 // Requests every 48 ms exercise coalescing while a 192 ms slide is active.
 // Calling lv_scr_load_anim directly on each request crashes LVGL 8.3.11.
 int dirty=0,loads=0;
 for(int frame=0;frame<160;frame++){
  if(frame<120&&frame%3==0){edge.page=(frame/3)%2;dirty=1;}
  if(dirty&&lv_anim_count_running()==0){
   lv_obj_t *pending=codex_screen(width,height,&edge);
   lv_scr_load_anim(pending,LV_SCR_LOAD_ANIM_MOVE_LEFT,192,0,true);dirty=0;loads++;
  }
  lv_tick_inc(16);lv_timer_handler();
 }
 if(dirty||lv_anim_count_running()||loads<2){fprintf(stderr,"Pending transition did not settle\n");return 1;}
 lv_mem_monitor(&memory);if(memory.free_size+1024<baseline)return 1;
 puts("40 overlapping page requests safely coalesced and settled.");
 puts("LVGL screens rendered; all objects fit their parent bounds.");return 0;
}
