"""Local design preview with clearly labelled synthetic allowance data."""
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path
import json
import time
from urllib.parse import urlparse, parse_qs

ROOT = Path(__file__).resolve().parents[1]


class Preview(BaseHTTPRequestHandler):
    def state(self):
        state=dict(plan='PREVIEW',ready=True,migrationRequired=False,savedCredentials=True,status='Updated OK',
                    ssid='Preview network',ip='192.0.2.10',imu=True,landscape=True,
                    temperature=46.5,ageSeconds=120,
                    short=dict(present=True,left=62,resetAt=int(time.time())+12840),
                    weekly=dict(present=True,left=13,resetAt=int(time.time())+406800))
        case=parse_qs(urlparse(self.path).query).get('case',['mixed'])[0]
        if case in ('full','empty'):
            for key in ('short','weekly'):state[key]['left']=100 if case=='full' else 0
        if case in ('missing','setup'):
            for key in ('short','weekly'):state[key]['present']=False
        if case=='setup':state.update(ready=False,savedCredentials=False,status='Connect Codex account')
        if case=='long':state.update(ssid='NetworkWithAVeryLongName1234567890',temperature=None)
        if case=='error':state.update(status='HTTPS: read timeout',ageSeconds=300)
        return state

    def do_GET(self):
        path=urlparse(self.path).path
        if path.startswith('/fonts/'):
            font=ROOT/'web/fonts'/Path(path).name
            if not font.is_file():self.send_error(404);return
            data=font.read_bytes();kind='font/woff2'
        elif path.startswith('/api/'):
            data=json.dumps(self.state()).encode();kind='application/json'
        else:
            css=(ROOT/'web/style.css').read_text()
            body=(ROOT/'web/dashboard.html').read_text().replace('{{DATA}}',json.dumps(self.state()))
            data=(f'<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Codex · Design preview</title><style>{css}</style></head><body><main>{body}</main></body></html>').encode()
            kind='text/html; charset=utf-8'
        self.send_response(200);self.send_header('Content-Type',kind)
        self.send_header('Content-Length',str(len(data)));self.end_headers();self.wfile.write(data)

    def do_POST(self):
        self.do_GET()


if __name__ == '__main__':
    print('Synthetic design preview: http://127.0.0.1:8765',flush=True)
    HTTPServer(('127.0.0.1',8765),Preview).serve_forever()
