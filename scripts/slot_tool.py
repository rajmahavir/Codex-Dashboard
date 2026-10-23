#!/usr/bin/env python3
"""Select an existing app on the verified ESPINFO two-slot layout.

Uses ESP-IDF's OTA sequence/CRC format. Never writes apps, partitions or NVS.
Enter ROM download mode first; supply the current loader port explicitly.
"""
import argparse
import binascii
import datetime
import pathlib
import struct
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_TOOL = ROOT.parent / 'ESPINFO-macOS-fix/BuildTools/data/packages/esp32/tools/esptool_py/5.3.1/esptool'


def records(data):
    if len(data) != 8192:
        raise ValueError('Expected exactly two OTA sectors')
    result = []
    for sector in range(2):
        seq, = struct.unpack_from('<I', data, sector * 4096)
        state, crc = struct.unpack_from('<II', data, sector * 4096 + 24)
        valid = (seq not in (0, 0xffffffff) and state in (0, 1, 2, 0xffffffff)
                 and crc == binascii.crc32(struct.pack('<I', seq), 0xffffffff) & 0xffffffff)
        result.append(dict(sector=sector, seq=seq, state=state, valid=valid))
    return result


def selection(data, slot):
    valid = [r for r in records(data) if r['valid']]
    latest = max(valid, key=lambda r: r['seq']) if valid else None
    if latest is None:
        raise ValueError('No valid fallback; inspect manually')
    if latest['state'] not in (2, 0xffffffff):
        raise ValueError('Current image is not confirmed; refusing selection')
    target = 0 if slot == 'A' else 1
    if (latest['seq'] - 1) % 2 == target:
        return None
    seq = latest['seq'] + 1
    if seq >= 0xffffffff:
        raise ValueError('OTA sequence rollover requires manual recovery')
    sector = 1 - latest['sector']
    crc = binascii.crc32(struct.pack('<I', seq), 0xffffffff) & 0xffffffff
    entry = struct.pack('<I20sII', seq, b'\xff' * 20, 0, crc)
    return sector, entry + b'\xff' * (4096 - len(entry))


def validate_layout(table):
    found = {}
    for offset in range(0, len(table), 32):
        entry = table[offset:offset + 32]
        if entry[:2] != b'\xaa\x50':
            break
        _, kind, subtype, address, size, label, _ = struct.unpack('<HBBII16sI', entry)
        found[label.rstrip(b'\0').decode()] = (kind, subtype, address, size)
    expected = {'nvs': (1, 2, 0x9000, 0x5000), 'otadata': (1, 0, 0xe000, 0x2000),
                'app0': (0, 16, 0x10000, 0x300000), 'app1': (0, 17, 0x310000, 0x300000)}
    if any(found.get(k) != v for k, v in expected.items()):
        raise ValueError('Partition layout differs from the verified ESPINFO layout')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('slot', choices=['A', 'B'])
    parser.add_argument('--port', required=True)
    parser.add_argument('--enter-loader', action='store_true',
                        help='Reset the native USB application port into ROM first (macOS)')
    parser.add_argument('--esptool', type=pathlib.Path, default=DEFAULT_TOOL)
    args = parser.parse_args()
    if args.enter_loader:
        from enter_loader import enter_loader
        args.port = enter_loader(args.port)
        print('Loader port:', args.port, flush=True)
    folder = ROOT / '.local' / ('select-' + datetime.datetime.now().strftime('%Y%m%d-%H%M%S-%f'))
    folder.mkdir(mode=0o700, parents=True)
    base = [str(args.esptool), '--chip', 'esp32s3', '--port', args.port,
            '--before', 'no-reset', '--after', 'no-reset']

    def run(*cmd):
        subprocess.run(base + list(map(str, cmd)), check=True)

    table, metadata, header = [folder / name for name in ('partitions.bin', 'otadata-before.bin', 'app-header.bin')]
    run('read-flash', '0x8000', '0x1000', table)
    validate_layout(table.read_bytes())
    run('read-flash', '0xe000', '0x2000', metadata)
    address = 0x10000 if args.slot == 'A' else 0x310000
    run('read-flash', hex(address), '0x1000', header)
    image = header.read_bytes()
    if image[0] != 0xe9 or struct.unpack_from('<H', image, 12)[0] != 9:
        raise ValueError('Target has no ESP32-S3 application header')
    change = selection(metadata.read_bytes(), args.slot)
    if change:
        sector, content = change
        candidate = folder / 'selection.bin'
        candidate.write_bytes(content)
        address = hex(0xe000 + sector * 4096)
        run('write-flash', address, candidate)
        run('verify-flash', address, candidate)
    print('Selected slot', args.slot, '; metadata backup:', metadata)
    subprocess.run([str(args.esptool), '--chip', 'esp32s3', '--port', args.port,
                    '--before', 'no-reset', '--after', 'watchdog-reset', 'read-mac'], check=True)


if __name__ == '__main__':
    main()
