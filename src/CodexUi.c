#include "CodexUi.h"
#include <stdio.h>
#include <string.h>
/* Agency FB fonts, generated from the same AGENCYB/AGENCYR TTFs as the VolosR repos (see fonts/) */
LV_FONT_DECLARE(cx_font_lbl);   /* Agency regular 14  - labels, footer   */
LV_FONT_DECLARE(cx_font_val);   /* Agency bold    20  - values, header   */
LV_FONT_DECLARE(cx_font_pct);   /* Agency bold    29  - percent sign     */
LV_FONT_DECLARE(cx_font_num);   /* Agency bold    52  - weekly number    */
LV_FONT_DECLARE(cx_font_hero);  /* Agency bold    80  - 5H hero number   */
#define LBL (&cx_font_lbl)
#define VAL (&cx_font_val)
#define PCT (&cx_font_pct)
#define NUM (&cx_font_num)
#define HERO (&cx_font_hero)
#define BG     0x000000
#define BLUE   0x082C39
#define PURPLE 0x210429
#define TEAL   0x06232A
#define ORANGE 0xFFA500
#define GREEN  0x29AE0B
#define RED    0xFF2A2A
#define INK    0xDBDBDB
#define SOFT   0xB7B7B7
#define MUTED  0x818181
#define DIM    0x4B4B4B
#define CYAN   0x7B9AA5
#define BLACK  0x000000

static lv_obj_t *box(lv_obj_t *parent,int x,int y,int w,int h,unsigned color,int radius){
 lv_obj_t *obj=lv_obj_create(parent);lv_obj_remove_style_all(obj);
 lv_obj_set_pos(obj,x,y);lv_obj_set_size(obj,w,h);lv_obj_clear_flag(obj,LV_OBJ_FLAG_SCROLLABLE);
 lv_obj_set_style_bg_color(obj,lv_color_hex(color),0);lv_obj_set_style_bg_opa(obj,LV_OPA_COVER,0);
 lv_obj_set_style_radius(obj,radius,0);return obj;
}
static lv_obj_t *tile(lv_obj_t *parent,int x,int y,int w,int h,unsigned color){return box(parent,x,y,w,h,color,4);}
static lv_obj_t *text(lv_obj_t *parent,const char *value,int x,int y,const lv_font_t *font,unsigned color){
 lv_obj_t *obj=lv_label_create(parent);lv_obj_remove_style_all(obj);
 lv_obj_set_style_text_font(obj,font,0);lv_obj_set_style_text_color(obj,lv_color_hex(color),0);
 lv_label_set_text(obj,value);lv_obj_set_pos(obj,x,y);
 lv_obj_update_layout(parent);
 int maxw=lv_obj_get_width(parent)-x-8;
 if(maxw>0){lv_obj_update_layout(obj);if(lv_obj_get_width(obj)>maxw){lv_obj_set_width(obj,maxw);lv_label_set_long_mode(obj,LV_LABEL_LONG_DOT);}}
 return obj;
}
static void fit(lv_obj_t *label,int width){
 lv_obj_update_layout(label);if(lv_obj_get_width(label)>width){lv_obj_set_width(label,width);lv_label_set_long_mode(label,LV_LABEL_LONG_DOT);}
}
static void right(lv_obj_t *o,int edge){lv_obj_update_layout(o);lv_obj_set_x(o,edge-lv_obj_get_width(o));}
static void center(lv_obj_t *o,int cx){lv_obj_update_layout(o);lv_obj_set_x(o,cx-lv_obj_get_width(o)/2);}
static void align_after(lv_obj_t *parent,lv_obj_t *o,lv_obj_t *ref,int dx,int dy){
 lv_obj_update_layout(parent);lv_obj_align_to(o,ref,LV_ALIGN_OUT_RIGHT_BOTTOM,dx,dy);
}

static unsigned tone(int left){return left<0?DIM:left<=10?RED:left<=30?ORANGE:GREEN;}

static void reset_label(char *out,size_t size,const char *value,int left){
 if(left<0){snprintf(out,size,"--");return;}
 const char *v=!strncmp(value,"Resets ",7)?value+7:value;
 if(!strcmp(v,"Reset due"))v="DUE";
 snprintf(out,size,"%s",v);
 for(char *c=out;*c;c++)if(*c>='a'&&*c<='z')*c-='a'-'A';
}
static void bar(lv_obj_t *p,int x,int y,int w,int h,int left,unsigned color){
 int count=20,gap=2,sw=(w-gap*(count-1))/count;if(sw<1)sw=1;
 int lit=left<0?0:(left*count+99)/100;
 for(int i=0;i<count;i++)box(p,x+i*(sw+gap),y,sw,h,i<lit?color:BG,1);
}
static void wifi_bars(lv_obj_t *s,int right_edge,unsigned c){
 box(s,right_edge-10,12,2,5,c,0);box(s,right_edge-6,9,2,8,c,0);box(s,right_edge-2,6,2,11,c,0);
}
static void stat(lv_obj_t *s,int x,int y,int w,int h,unsigned color,const char *label,const char *value){
 lv_obj_t *t=tile(s,x,y,w,h,color);
 unsigned c1=color==ORANGE?BLACK:CYAN,c2=color==ORANGE?BLACK:INK;
 text(t,label,8,1,LBL,c1);
 fit(text(t,value,8,14,VAL,c2),w-16);
}

static lv_obj_t *screen_base(int width,int height,const char *badge){
 lv_obj_t *screen=lv_obj_create(NULL);lv_obj_remove_style_all(screen);
 lv_obj_set_size(screen,width,height);lv_obj_clear_flag(screen,LV_OBJ_FLAG_SCROLLABLE);
 lv_obj_set_style_bg_color(screen,lv_color_hex(BG),0);lv_obj_set_style_bg_opa(screen,LV_OPA_COVER,0);
 lv_obj_t *brand=tile(screen,6,0,64,22,ORANGE);
 lv_obj_t *bl=text(brand,"CODEX",0,0,VAL,BLACK);
 lv_obj_set_style_text_letter_space(bl,1,0);lv_obj_center(bl);
 int bw=!strcmp(badge,"PLUS")?46:64;lv_obj_t *pill=tile(screen,76,0,bw,22,BLUE);
 lv_obj_t *label=text(pill,badge,0,0,LBL,SOFT);lv_obj_center(label);
 return screen;
}

static lv_obj_t *usage_screen(int width,int height,const CodexUiModel *m){
 int a=m->short_left,b=m->weekly_left;
 char sr[32],wr[32],n[8];
 reset_label(sr,sizeof(sr),m->short_reset,a);
 reset_label(wr,sizeof(wr),m->weekly_reset,b);
 unsigned ca=tone(a),cb=tone(b),wc=m->online?GREEN:DIM;
 lv_obj_t *s=screen_base(width,height,m->badge);
 wifi_bars(s,width-8,wc);
 if(width>height){lv_obj_t *fresh=text(s,m->freshness,0,3,LBL,CYAN);fit(fresh,100);right(fresh,width-24);}
 if(a<0)snprintf(n,sizeof(n),"--");else snprintf(n,sizeof(n),"%d",a);
 if(width>height){
  int hw=168,ww=width-12-hw-6,tw=(width-24)/3;
  lv_obj_t *t1=tile(s,6,28,hw,80,BLUE);
  right(text(t1,"5H LEFT",0,4,LBL,CYAN),hw-8);
  lv_obj_t *hero=text(t1,n,8,4,HERO,INK);
  align_after(t1,text(t1,"%",0,0,PCT,SOFT),hero,4,-2);
  bar(t1,8,70,hw-16,6,a,ca);
  lv_obj_t *t2=tile(s,6+hw+6,28,ww,80,PURPLE);
  text(t2,"WEEKLY",8,4,LBL,CYAN);
  if(b<0)snprintf(n,sizeof(n),"--");else snprintf(n,sizeof(n),"%d",b);
  lv_obj_t *num=text(t2,n,8,22,NUM,INK);
  align_after(t2,text(t2,"%",0,0,PCT,SOFT),num,4,-2);
  bar(t2,8,70,ww-16,6,b,cb);
  stat(s,6,114,tw,36,TEAL,"RESET 5H",sr);
  stat(s,6+tw+6,114,tw,36,PURPLE,"RESET WEEK",wr);
  stat(s,6+(tw+6)*2,114,width-6-(6+(tw+6)*2),36,ORANGE,"TEMP",m->temperature);
  box(s,8,158,6,6,wc,3);
  fit(text(s,m->online?m->ssid:"OFFLINE",20,154,LBL,INK),120);
  lv_obj_t *addr=text(s,m->address,0,154,LBL,MUTED);fit(addr,110);right(addr,width-8);
 }else{
  int tw=width-12,half=(width-18)/2;
  lv_obj_t *t1=tile(s,6,28,tw,118,BLUE);
  text(t1,"5H LEFT",8,4,LBL,CYAN);
  center(text(t1,n,0,16,HERO,INK),tw/2);
  center(text(t1,"% LEFT",0,84,VAL,SOFT),tw/2);
  bar(t1,8,108,tw-16,6,a,ca);
  lv_obj_t *t2=tile(s,6,152,tw,72,PURPLE);
  text(t2,"WEEKLY LEFT",8,3,LBL,CYAN);
  if(b<0)snprintf(n,sizeof(n),"--");else snprintf(n,sizeof(n),"%d",b);
  lv_obj_t *num=text(t2,n,8,16,NUM,INK);
  align_after(t2,text(t2,"%",0,0,PCT,SOFT),num,4,-2);
  bar(t2,8,60,tw-16,6,b,cb);
  stat(s,6,230,half,36,TEAL,"RESET 5H",sr);
  stat(s,12+half,230,half,36,PURPLE,"RESET WEEK",wr);
  lv_obj_t *t5=tile(s,6,272,tw,26,ORANGE);
  text(t5,"TEMP",8,5,LBL,BLACK);
  right(text(t5,m->temperature,0,2,VAL,BLACK),tw-8);
  box(s,8,306,6,6,wc,3);
  int fw=(width-36)/2;
  fit(text(s,m->online?m->ssid:"OFFLINE",20,302,LBL,INK),fw);
  lv_obj_t *fresh=text(s,m->freshness,0,302,LBL,CYAN);fit(fresh,fw);right(fresh,width-8);
 }
 return s;
}
static void detail(lv_obj_t *screen,int x,int y,int width,int height,unsigned color,const char *title,const char *value){
 lv_obj_t *t=tile(screen,x,y,width,height,color);
 unsigned c1=color==ORANGE?BLACK:CYAN,c2=color==ORANGE?BLACK:INK;
 text(t,title,10,4,LBL,c1);fit(text(t,value,10,22,VAL,c2),width-20);
}
static lv_obj_t *device_screen(int width,int height,const CodexUiModel *m){
 lv_obj_t *screen=screen_base(width,height,"DEVICE");
 if(width>height){
  detail(screen,6,28,152,56,BLUE,"Wi-Fi network",m->ssid);
  detail(screen,164,28,150,56,PURPLE,"Device address",m->address);
  detail(screen,6,90,152,56,TEAL,"Temperature",m->temperature);
  detail(screen,164,90,150,56,ORANGE,"Screen rotation",m->rotation);
  text(screen,"BOOT: usage",8,153,LBL,MUTED);
  lv_obj_t *hold=text(screen,"Hold: ESPINFO",0,153,LBL,MUTED);right(hold,width-8);
 }else{
  detail(screen,6,28,width-12,56,BLUE,"Wi-Fi network",m->ssid);
  detail(screen,6,90,width-12,56,PURPLE,"Device address",m->address);
  detail(screen,6,152,width-12,56,TEAL,"Temperature",m->temperature);
  detail(screen,6,214,width-12,56,ORANGE,"Screen rotation",m->rotation);
  text(screen,"BOOT: usage",8,287,LBL,MUTED);
  text(screen,"Hold: ESPINFO",8,302,LBL,MUTED);
 }
 return screen;
}

lv_obj_t *codex_screen(int width,int height,const CodexUiModel *m){
 if(m->page==2){
  lv_obj_t *s=screen_base(width,height,"SETUP");
  int w=width>height?152:width-12;
  detail(s,6,28,w,56,BLUE,"Join Wi-Fi network",m->ssid);
  detail(s,width>height?164:6,width>height?28:90,width>height?150:w,56,PURPLE,"Wi-Fi password",m->setup_password);
  detail(s,6,width>height?90:152,width-12,56,ORANGE,"Open in your browser","192.168.4.1");
  text(s,"Hold BOOT: ESPINFO",8,height-16,LBL,MUTED);return s;
 }
 return m->page?device_screen(width,height,m):usage_screen(width,height,m);
}
