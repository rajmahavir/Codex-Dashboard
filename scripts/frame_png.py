"""Convert the board's public SCREEN RGB565 response to a PNG without dependencies."""
from pathlib import Path
import argparse
import struct
import zlib

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('input',type=Path)
parser.add_argument('output',type=Path)
parser.add_argument('--width',type=int,required=True)
parser.add_argument('--height',type=int,required=True)
args=parser.parse_args()
data=args.input.read_bytes()
marker=f'FRAME {args.width*args.height*2}\n'.encode()
start=data.index(marker)+len(marker)
raw=data[start:start+args.width*args.height*2]
if len(raw)!=args.width*args.height*2:
    raise SystemExit('Incomplete framebuffer')
rows=[]
for y in range(args.height):
    row=bytearray([0])
    for x in range(args.width):
        i=(y*args.width+x)*2
        value=int.from_bytes(raw[i:i+2],'big')
        row.extend((((value>>11)&31)*255//31,((value>>5)&63)*255//63,(value&31)*255//31))
    rows.append(bytes(row))

def chunk(kind,body):
    return struct.pack('>I',len(body))+kind+body+struct.pack('>I',zlib.crc32(kind+body)&0xffffffff)

image=(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',args.width,args.height,8,2,0,0,0))+
       chunk(b'IDAT',zlib.compress(b''.join(rows)))+chunk(b'IEND',b''))
args.output.write_bytes(image)
print(args.output)
