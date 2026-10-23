#!/usr/bin/env python3
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess
import argparse,shutil,urllib.request,tarfile,hashlib,sys
root=Path(__file__).resolve().parents[1];base=root/'.local/lvgl-design';lib=base/'lvgl-8.3.11';build=base/'objects';build.mkdir(parents=True,exist_ok=True)
parser=argparse.ArgumentParser(description='Render the proposed LCD design using actual LVGL on this computer; never touches firmware or USB.')
parser.add_argument('--fetch',action='store_true',help='Download the pinned official LVGL source if it is missing')
args=parser.parse_args()
if not lib.exists():
 if not args.fetch:parser.error('LVGL source is missing; run with --fetch once.')
 archive=base/'lvgl-v8.3.11.tar.gz'
 with urllib.request.urlopen('https://codeload.github.com/lvgl/lvgl/tar.gz/refs/tags/v8.3.11',timeout=30) as response:archive.write_bytes(response.read())
 if hashlib.sha256(archive.read_bytes()).hexdigest()!='83b7325a4b78cbb19a39f3ac75346c8086926d7faeab5ff2d825c20da0bbdb46':raise SystemExit('LVGL archive hash mismatch')
 with tarfile.open(archive) as tar:tar.extractall(base,filter='data')
shutil.copy2(root/'design/lvgl/lv_conf.h',base/'lv_conf.h')
sources=sorted((lib/'src').rglob('*.c'))+sorted((root/'src').glob('cx_font_*.c'))+[root/'src/CodexUi.c']+[root/'design/lvgl/ui_preview.c',root/'design/lvgl/render_preview.c']
def compile_one(source):
 name=str(source.relative_to(root)).replace('/','_')+'.o';obj=build/name
 if not obj.exists() or obj.stat().st_mtime<max(source.stat().st_mtime,(base/'lv_conf.h').stat().st_mtime):
  subprocess.run(['cc','-c','-O2','-std=c99','-DLV_CONF_INCLUDE_SIMPLE','-I'+str(base),'-I'+str(lib),'-I'+str(root/'design/lvgl'),'-I'+str(root/'src'),str(source),'-o',str(obj)],check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
 return str(obj)
with ThreadPoolExecutor(max_workers=4) as pool:objects=list(pool.map(compile_one,sources))
subprocess.run(['cc',*objects,'-o',str(base/'preview'),'-lm'],check=True)
print('Built real LVGL host preview:',len(sources),'C files')

output=root/'build/lvgl-preview';output.mkdir(parents=True,exist_ok=True)
for name,w,h,a,b in [('full',320,172,100,100),('mixed',320,172,62,13),('empty',320,172,0,0),('portrait',172,320,100,100),('portrait-mixed',172,320,62,13),('portrait-empty',172,320,0,0)]:
 subprocess.run([str(base/'preview'),str(w),str(h),str(a),str(b),str(output/name)],check=True)
 for page in ['main','device']:
  subprocess.run([sys.executable,str(root/'scripts/frame_png.py'),str(output/(name+'-'+page+'.raw')),str(output/(name+'-'+page+'.png')),'--width',str(w),'--height',str(h)],check=True)
print('Design previews:',output)
