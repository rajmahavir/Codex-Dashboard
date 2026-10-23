#!/usr/bin/env python3
"""Build an app-only image using the pinned, existing ESPINFO toolchain."""
import argparse
import json
import pathlib
import shutil
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--toolchain', type=pathlib.Path,
                    default=ROOT.parent / 'ESPINFO-macOS-fix/BuildTools')
args = parser.parse_args()
subprocess.run([__import__("sys").executable,str(ROOT/"scripts/pack_web.py")],check=True)
base = args.toolchain.resolve()
local = ROOT / '.local'
local.mkdir(exist_ok=True)
config = local / 'arduino-cli.yaml'
config.write_text(json.dumps({'directories': {'data': str(base / 'data'),
    'downloads': str(local / 'downloads'), 'user': str(local / 'user')}}))
sketch = local / 'sketch/CodexDashboard'
sketch.mkdir(parents=True, exist_ok=True)
(sketch / 'CodexDashboard.ino').touch()
for prepared in sketch.iterdir():
    if prepared.suffix in ('.cpp', '.c', '.h') and not (ROOT / 'src' / prepared.name).exists():
        prepared.unlink()
for source in (ROOT / 'src').iterdir():
    if source.suffix in ('.cpp', '.c', '.h'):
        shutil.copy2(source, sketch / source.name)
# Arduino's recipe parser splits include paths containing spaces.
config_include = tempfile.TemporaryDirectory(prefix='codex-lvgl-')
shutil.copy2(ROOT / 'src/lv_conf.h', pathlib.Path(config_include.name) / 'lv_conf.h')
subprocess.run([str(base / 'arduino-cli'), 'compile', '--config-file', str(config),
    '--fqbn', 'esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,USBMode=default,CDCOnBoot=cdc',
    '--libraries', str(ROOT / 'libraries'), '--libraries', str(base / 'user/libraries'),
    '--build-property', 'compiler.c.extra_flags=-DLV_CONF_INCLUDE_SIMPLE -I'+config_include.name,
    '--build-property', 'compiler.cpp.extra_flags=-DLV_CONF_INCLUDE_SIMPLE -I'+config_include.name,
    '--build-path', str(ROOT / 'build/arduino'), str(sketch)], check=True)
image = ROOT / 'build/arduino/CodexDashboard.ino.bin'
if image.stat().st_size > 0x300000:
    raise SystemExit('Image exceeds Slot B capacity')
print('APP ONLY:', image, 'at 0x310000. Never upload the generated merged image.')
