"""Embed standalone web assets in flash; no filesystem partition required."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
out = ['#pragma once', '#include <pgmspace.h>']
for name, source in [('WEB_CSS','style.css'),('WEB_DASHBOARD','dashboard.html')]:
    data = (root/'web'/source).read_text()
    if ')CODEXWEB"' in data:
        raise ValueError('Raw-string delimiter collision')
    out.append(f'static const char {name}[] PROGMEM = R"CODEXWEB({data})CODEXWEB";')
(root/'src/WebAssets.h').write_text('\n'.join(out)+'\n')
# Binary fonts are served from flash, avoiding base64 expansion and external CDNs.
fonts = ['#pragma once', '#include <pgmspace.h>']
for name, source in [('WEB_FONT_REGULAR','agency-regular-v1.woff2'),('WEB_FONT_BOLD','agency-bold-v1.woff2')]:
    data = (root/'web/fonts'/source).read_bytes()
    fonts.append(f'static const uint8_t {name}[] PROGMEM = {{')
    fonts.extend(','.join(f'0x{b:02x}' for b in data[i:i+24])+',' for i in range(0,len(data),24))
    fonts.append('};')
(root/'src/WebFonts.h').write_text('\n'.join(fonts)+'\n')
