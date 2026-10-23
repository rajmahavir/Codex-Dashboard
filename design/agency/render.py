from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];b=r/'.local/lvgl-design';f=r/'.local/reference-fonts';(f/'ui.h').write_text('#include "lvgl.h"\n')
if not (b/'objects').exists() or not list((b/'objects').glob('.local_lvgl-design_lvgl-8.3.11_src_*.o')):
 raise SystemExit('First run: python3 scripts/render_lvgl_design.py --fetch')
for name in ('ui_font_BIG.c','ui_font_MID.c','agency_hero.c'):
 if not (f/name).exists():raise SystemExit('Reference font missing: '+str(f/name)+'; see design/agency/README.md')
objects=[str(p) for p in (b/'objects').glob('.local_lvgl-design_lvgl-8.3.11_src_*.o')]
for p in [f/'ui_font_BIG.c',f/'ui_font_MID.c',f/'agency_hero.c',r/'design/agency/preview.c']:
 o=f/(p.stem+'.o');subprocess.run(['cc','-c','-O2','-std=c99','-DLV_CONF_INCLUDE_SIMPLE','-I'+str(b),'-I'+str(b/'lvgl-8.3.11'),'-I'+str(f),str(p),'-o',str(o)],check=True);objects.append(str(o))
subprocess.run(['cc',*objects,'-o',str(f/'preview'),'-lm'],check=True)
out=r/'build/agency-preview';out.mkdir(exist_ok=True)
for name,w,h,a,c in [('landscape',320,172,62,13),('full',320,172,100,100),('empty',320,172,0,0),('portrait',172,320,62,13),('portrait-full',172,320,100,100)]:
 raw=out/(name+'.raw');subprocess.run([str(f/'preview'),str(w),str(h),str(a),str(c),str(raw)],check=True)
 subprocess.run(['python3','scripts/frame_png.py',str(raw),str(out/(name+'.png')),'--width',str(w),'--height',str(h)],check=True)
