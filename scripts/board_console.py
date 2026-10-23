#!/usr/bin/env python3
"""Bounded POSIX serial diagnostics; no external Python dependencies."""
import argparse
import os
import pathlib
import select
import termios
import time


def request(port, command, timeout=5):
    fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    old = None
    try:
        old = termios.tcgetattr(fd)
        cfg = termios.tcgetattr(fd)
        cfg[0] = cfg[1] = cfg[3] = 0
        cfg[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
        cfg[4] = cfg[5] = termios.B115200
        cfg[6][termios.VMIN] = cfg[6][termios.VTIME] = 0
        termios.tcsetattr(fd, termios.TCSANOW, cfg)
        os.write(fd, (command + '\n').encode('utf-8'))
        result = b''
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            ready, _, _ = select.select([fd], [], [], .1)
            if ready:
                try:
                    result += os.read(fd, 16384)
                except BlockingIOError:
                    continue
                except OSError:
                    break
                if command == 'SCREEN':
                    start = result.find(b'FRAME 110080\n')
                    if start >= 0 and len(result) >= start + 13 + 110080:
                        break
                elif b'\n' in result:
                    break
        return result
    finally:
        if old is not None:
            try:
                termios.tcsetattr(fd, termios.TCSANOW, old)
            except (OSError, termios.error):
                pass
        os.close(fd)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', required=True)
    parser.add_argument('command', choices=['STATUS', 'DETAILS', 'RESTART', 'BOOT_A', 'CANCEL', 'SCREEN', 'TLS_TEST', 'STORAGE_TEST', 'WIFI_DIAG', 'WIFI_CONNECT', 'WIFI_RESTORE', 'PREVIEW', 'LIVE'])
    parser.add_argument('--output', type=pathlib.Path)
    parser.add_argument('--rotation', type=int, choices=[0,1,2,3], default=1)
    parser.add_argument('--ssid', help='Saved SSID to try temporarily over USB')
    args = parser.parse_args()
    if args.command == 'WIFI_CONNECT' and not args.ssid:
        parser.error('--ssid is required for WIFI_CONNECT')
    command = f"PREVIEW {args.rotation}" if args.command == "PREVIEW" else f"WIFI_CONNECT {args.ssid}" if args.command == "WIFI_CONNECT" else args.command
    data = request(args.port, command, timeout=180 if args.command == 'STORAGE_TEST' else 45 if args.command in ('TLS_TEST', 'WIFI_DIAG', 'WIFI_CONNECT', 'WIFI_RESTORE', 'PREVIEW', 'LIVE') else 20 if args.command == 'SCREEN' else 5)
    if not data:
        raise SystemExit('No response before timeout')
    if args.output:
        args.output.write_bytes(data)
        print('Saved', len(data), 'bytes to', args.output)
    else:
        print(data.decode(errors='replace').strip())
