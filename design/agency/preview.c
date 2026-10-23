/* Off-board proposal using the reference's actual Agency Bold glyphs. */
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
LV_FONT_DECLARE(ui_font_BIG);
LV_FONT_DECLARE(ui_font_MID);
LV_FONT_DECLARE(agency_hero);
#define BG 0xA6B7AD
#define INK 0x25443D
#define SOFT 0x6D877C
#define PALE 0xB7C5BD
static lv_color_t buffer[320*40];
static uint16_t pixels[320*172];
static int width,height;
static void flush(lv_disp_drv_t *d,const lv_area_t *a,lv_color_t *p){
 for(int y=a->y1;y<=a->y2;y++)for(int x=a->x1;x<=a->x2;x++){pixels[y*width+x]=p->full;p++;}lv_disp_flush_ready(d);
}
static lv_obj_t *box(lv_obj_t *parent,int x,int y,int w,int h,unsigned color,int radius){
 lv_obj_t *o=lv_obj_create(parent);lv_obj_remove_style_all(o);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
 lv_obj_clear_flag(o,LV_OBJ_FLAG_SCROLLABLE);lv_obj_set_style_bg_color(o,lv_color_hex(color),0);
 lv_obj_set_style_bg_opa(o,LV_OPA_COVER,0);lv_obj_set_style_radius(o,radius,0);return o;
}
static lv_obj_t *label(lv_obj_t *s,const char *value,int x,int y,const lv_font_t *font,unsigned color){
 lv_obj_t *o=lv_label_create(s);lv_obj_remove_style_all(o);lv_label_set_text(o,value);lv_obj_set_pos(o,x,y);
 lv_obj_set_style_text_font(o,font,0);lv_obj_set_style_text_color(o,lv_color_hex(color),0);return o;
}
static void meter(lv_obj_t *s,int x,int y,int w,int h,int left){
 int count=20,gap=2,sw=(w-gap*(count-1))/count;
 for(int i=0;i<count;i++)box(s,x+i*(sw+gap),y,sw,h,i<(left*count+99)/100?INK:SOFT,0);
}
static void right(lv_obj_t *o,int edge){lv_obj_update_layout(o);lv_obj_set_x(o,edge-lv_obj_get_width(o));}
static void center(lv_obj_t *o,int cx){lv_obj_update_layout(o);lv_obj_set_x(o,cx-lv_obj_get_width(o)/2);}
static lv_obj_t *screen(int a,int b){
 lv_obj_t *s=lv_obj_create(NULL);lv_obj_remove_style_all(s);lv_obj_set_size(s,width,height);
 lv_obj_clear_flag(s,LV_OBJ_FLAG_SCROLLABLE);lv_obj_set_style_bg_color(s,lv_color_hex(BG),0);lv_obj_set_style_bg_opa(s,LV_OPA_COVER,0);
 label(s,"CODEX",10,7,&lv_font_montserrat_12,INK);
 right(label(s,"PLUS",0,7,&lv_font_montserrat_12,INK),width-10);box(s,10,25,width-20,2,INK,0);
 char n[8];snprintf(n,sizeof(n),"%d",a);
 if(width>height){
  label(s,"5H",14,30,&ui_font_MID,INK);
  lv_obj_t *hero=label(s,n,12,54,&agency_hero,INK);
  lv_obj_t *unit=label(s,"% LEFT",0,0,&ui_font_MID,INK);lv_obj_update_layout(s);lv_obj_align_to(unit,hero,LV_ALIGN_OUT_RIGHT_BOTTOM,8,-2);
  meter(s,14,122,184,8,a);box(s,14,135,184,15,INK,0);label(s,"RESET 3H 34M",20,135,&lv_font_montserrat_12,PALE);
  box(s,206,31,2,118,INK,0);box(s,216,31,96,85,INK,2);
  label(s,"WEEKLY",222,33,&ui_font_MID,PALE);
  snprintf(n,sizeof(n),"%d",b);lv_obj_t *number=label(s,n,222,54,&ui_font_BIG,PALE);
  lv_obj_t *percent=label(s,"%",0,0,&ui_font_MID,PALE);lv_obj_update_layout(s);lv_obj_align_to(percent,number,LV_ALIGN_OUT_RIGHT_BOTTOM,3,-3);
  label(s,"LEFT",222,99,&lv_font_montserrat_12,PALE);
  meter(s,218,122,92,5,b);label(s,"4D 17H",218,131,&ui_font_MID,INK);
  right(label(s,"RESET",0,138,&lv_font_montserrat_12,INK),310);
  box(s,10,153,width-20,1,INK,0);label(s,"DP",24,156,&lv_font_montserrat_12,INK);
  box(s,11,161,2,5,INK,0);box(s,15,158,2,8,INK,0);box(s,19,155,2,11,INK,0);
  label(s,"50 C",92,156,&lv_font_montserrat_12,INK);right(label(s,"SYNC 1M",0,156,&lv_font_montserrat_12,INK),310);
 }else{
  label(s,"5H",14,34,&ui_font_MID,INK);center(label(s,n,0,64,&agency_hero,INK),width/2);
  center(label(s,"% LEFT",0,134,&ui_font_MID,INK),width/2);meter(s,14,165,width-28,9,a);
  box(s,14,184,width-28,20,INK,1);center(label(s,"RESET 3H 34M",0,187,&lv_font_montserrat_12,PALE),width/2);
  box(s,8,214,width-16,73,INK,2);label(s,"WEEKLY",16,216,&ui_font_MID,PALE);
  snprintf(n,sizeof(n),"%d",b);right(label(s,n,0,216,&ui_font_BIG,PALE),155);
  label(s,"RESET 4D 17H",16,255,&lv_font_montserrat_12,PALE);right(label(s,"% LEFT",0,272,&lv_font_montserrat_12,PALE),156);
  box(s,10,294,width-20,1,INK,0);label(s,"DP / 50 C",10,301,&lv_font_montserrat_12,INK);
  right(label(s,"SYNC 1M",0,301,&lv_font_montserrat_12,INK),width-10);
 }
 return s;
}
static int verify(lv_obj_t *o){
 int errors=0;lv_area_t p;lv_obj_get_coords(o,&p);
 for(int i=0;i<lv_obj_get_child_cnt(o);i++){
  lv_obj_t *c=lv_obj_get_child(o,i);lv_area_t a;lv_obj_get_coords(c,&a);
  if(a.x1<p.x1||a.y1<p.y1||a.x2>p.x2||a.y2>p.y2){fprintf(stderr,"Clipped child at %d,%d to %d,%d\n",a.x1,a.y1,a.x2,a.y2);errors++;}errors+=verify(c);
 }return errors;
}
int main(int argc,char **argv){
 if(argc!=6)return 2;width=atoi(argv[1]);height=atoi(argv[2]);int a=atoi(argv[3]),b=atoi(argv[4]);
 if(width*height!=320*172||a<0||a>100||b<0||b>100)return 2;
 lv_init();static lv_disp_draw_buf_t draw;lv_disp_draw_buf_init(&draw,buffer,NULL,320*40);
 static lv_disp_drv_t driver;lv_disp_drv_init(&driver);driver.hor_res=width;driver.ver_res=height;driver.draw_buf=&draw;driver.flush_cb=flush;lv_disp_drv_register(&driver);
 lv_obj_t *s=screen(a,b);lv_obj_update_layout(s);if(verify(s))return 1;lv_scr_load(s);lv_refr_now(NULL);
 FILE *f=fopen(argv[5],"wb");if(!f)return 2;fprintf(f,"FRAME %d\n",width*height*2);
 for(int i=0;i<width*height;i++){fputc(pixels[i]>>8,f);fputc(pixels[i]&255,f);}fclose(f);return 0;
}
