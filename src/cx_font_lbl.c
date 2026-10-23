/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: --bpp 1 --size 14 --font /tmp/M5StickS3-main/M5StickS3/assets/AGENCYR.TTF -r 0x20-0x7f,0xB0 --format lvgl --no-compress --no-prefilter --lv-font-name cx_font_lbl -o out/cx_font_lbl.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef CX_FONT_LBL
#define CX_FONT_LBL 1
#endif

#if CX_FONT_LBL

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0x20,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x14, 0x28, 0x93, 0xf2, 0x45, 0xa, 0x7e, 0x48,
    0x91, 0x40,

    /* U+0024 "$" */
    0x27, 0xeb, 0x5a, 0x30, 0x86, 0xad, 0x6b, 0xf2,
    0x0,

    /* U+0025 "%" */
    0xe2, 0xa4, 0xa4, 0xa8, 0xa8, 0xf7, 0x15, 0x25,
    0x25, 0x45, 0x47,

    /* U+0026 "&" */
    0xf4, 0xa5, 0x8, 0x3e, 0x52, 0x94, 0xbc,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x49, 0x49, 0x24, 0x91, 0x22,

    /* U+0029 ")" */
    0x89, 0x12, 0x49, 0x25, 0x24,

    /* U+002A "*" */
    0x4b, 0xe4,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+002C "," */
    0xc0,

    /* U+002D "-" */
    0xc0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x4, 0x20, 0x84, 0x10, 0x42, 0x8, 0x41, 0x8,
    0x0,

    /* U+0030 "0" */
    0xf9, 0x99, 0x99, 0x99, 0x99, 0xf0,

    /* U+0031 "1" */
    0xff, 0xe0,

    /* U+0032 "2" */
    0xf9, 0x91, 0x12, 0x24, 0x48, 0xf0,

    /* U+0033 "3" */
    0xf9, 0x91, 0x22, 0x19, 0x99, 0xf0,

    /* U+0034 "4" */
    0x21, 0x8, 0xa5, 0x2a, 0x5f, 0x10, 0x84,

    /* U+0035 "5" */
    0xf8, 0x88, 0xf1, 0x19, 0x99, 0xf0,

    /* U+0036 "6" */
    0xf9, 0x98, 0x8f, 0x99, 0x99, 0xf0,

    /* U+0037 "7" */
    0xf9, 0x91, 0x22, 0x22, 0x24, 0x40,

    /* U+0038 "8" */
    0xf9, 0x99, 0xf6, 0x99, 0x99, 0xf0,

    /* U+0039 "9" */
    0xf9, 0x99, 0x9f, 0x11, 0x99, 0xf0,

    /* U+003A ":" */
    0x84,

    /* U+003B ";" */
    0x86,

    /* U+003C "<" */
    0x12, 0x48, 0x63,

    /* U+003D "=" */
    0xf0, 0xf,

    /* U+003E ">" */
    0xc, 0x63, 0x48,

    /* U+003F "?" */
    0xfc, 0x62, 0x11, 0x8, 0x84, 0x20, 0x8,

    /* U+0040 "@" */
    0xfe, 0x1b, 0xeb, 0x8e, 0xfa, 0xeb, 0xbe, 0xf,
    0xc0,

    /* U+0041 "A" */
    0x21, 0x8c, 0x65, 0x29, 0x49, 0x7a, 0x62,

    /* U+0042 "B" */
    0xf9, 0x99, 0x9e, 0x99, 0x99, 0xf0,

    /* U+0043 "C" */
    0xf9, 0x98, 0x88, 0x89, 0x99, 0xf0,

    /* U+0044 "D" */
    0xe9, 0x99, 0x99, 0x99, 0x99, 0xe0,

    /* U+0045 "E" */
    0xf8, 0x88, 0x8e, 0x88, 0x88, 0xf0,

    /* U+0046 "F" */
    0xf2, 0x49, 0xe4, 0x92, 0x0,

    /* U+0047 "G" */
    0xf9, 0x98, 0x8b, 0x99, 0x99, 0xf0,

    /* U+0048 "H" */
    0x99, 0x99, 0x9f, 0x99, 0x99, 0x90,

    /* U+0049 "I" */
    0xff, 0xe0,

    /* U+004A "J" */
    0x11, 0x11, 0x11, 0x19, 0x99, 0xf0,

    /* U+004B "K" */
    0x94, 0xa9, 0x4c, 0x63, 0x14, 0xa4, 0xa4,

    /* U+004C "L" */
    0x92, 0x49, 0x24, 0x93, 0x80,

    /* U+004D "M" */
    0x8c, 0x77, 0xbd, 0xef, 0xb5, 0xad, 0x62,

    /* U+004E "N" */
    0x99, 0x9d, 0xdd, 0xbb, 0x99, 0x90,

    /* U+004F "O" */
    0xfc, 0x63, 0x18, 0xc6, 0x31, 0x8c, 0x7e,

    /* U+0050 "P" */
    0xf9, 0x99, 0x99, 0xf8, 0x88, 0x80,

    /* U+0051 "Q" */
    0xfa, 0x28, 0xa2, 0x8a, 0x28, 0xa2, 0x9a, 0x2f,
    0xc0,

    /* U+0052 "R" */
    0xf9, 0x99, 0x9b, 0xaa, 0xa9, 0x90,

    /* U+0053 "S" */
    0xf9, 0x98, 0x42, 0x29, 0x99, 0xf0,

    /* U+0054 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x84, 0x21, 0x8,

    /* U+0055 "U" */
    0x99, 0x99, 0x99, 0x99, 0x99, 0xf0,

    /* U+0056 "V" */
    0x99, 0x99, 0x96, 0x66, 0x66, 0x60,

    /* U+0057 "W" */
    0x93, 0x26, 0x4d, 0xa6, 0xcd, 0x9b, 0x36, 0x6c,
    0xd9, 0x10,

    /* U+0058 "X" */
    0x4a, 0x94, 0xa2, 0x10, 0xca, 0x52, 0x62,

    /* U+0059 "Y" */
    0x8c, 0x54, 0xa5, 0x10, 0x84, 0x21, 0x8,

    /* U+005A "Z" */
    0x71, 0x11, 0x22, 0x24, 0x44, 0xf0,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x4e,

    /* U+005C "\\" */
    0x81, 0x4, 0x8, 0x20, 0x81, 0x4, 0x8, 0x20,
    0x40,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x9e,

    /* U+005E "^" */
    0x46, 0xa9,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0x90,

    /* U+0061 "a" */
    0xf9, 0x1f, 0x99, 0xf0,

    /* U+0062 "b" */
    0x88, 0x88, 0xf9, 0x99, 0x99, 0xf0,

    /* U+0063 "c" */
    0xf9, 0x88, 0x89, 0xf0,

    /* U+0064 "d" */
    0x11, 0x11, 0xf9, 0x99, 0x99, 0xf0,

    /* U+0065 "e" */
    0xf9, 0x9f, 0x89, 0xf0,

    /* U+0066 "f" */
    0x69, 0x2e, 0x92, 0x49, 0x0,

    /* U+0067 "g" */
    0xf9, 0x99, 0x99, 0xf1, 0x9f,

    /* U+0068 "h" */
    0x88, 0x88, 0xf9, 0x99, 0x99, 0x90,

    /* U+0069 "i" */
    0x9f, 0xc0,

    /* U+006A "j" */
    0x45, 0x55, 0x57,

    /* U+006B "k" */
    0x88, 0x88, 0xaa, 0xcc, 0xca, 0x90,

    /* U+006C "l" */
    0xff, 0xe0,

    /* U+006D "m" */
    0xff, 0x26, 0x4c, 0x99, 0x32, 0x64, 0x80,

    /* U+006E "n" */
    0xf9, 0x99, 0x99, 0x90,

    /* U+006F "o" */
    0xf9, 0x99, 0x99, 0xf0,

    /* U+0070 "p" */
    0xf9, 0x99, 0x99, 0xf8, 0x88,

    /* U+0071 "q" */
    0xf9, 0x99, 0x99, 0xf1, 0x11,

    /* U+0072 "r" */
    0xf9, 0x88, 0x88, 0x80,

    /* U+0073 "s" */
    0xf9, 0x86, 0x19, 0xf0,

    /* U+0074 "t" */
    0x4b, 0xa4, 0x92, 0x60,

    /* U+0075 "u" */
    0x99, 0x99, 0x99, 0xf0,

    /* U+0076 "v" */
    0x95, 0x55, 0x62, 0x20,

    /* U+0077 "w" */
    0x92, 0xa9, 0xd3, 0x66, 0xcd, 0x8a, 0x0,

    /* U+0078 "x" */
    0x55, 0x62, 0x65, 0x90,

    /* U+0079 "y" */
    0x95, 0x55, 0x52, 0x22, 0x24,

    /* U+007A "z" */
    0xf2, 0x24, 0x48, 0xf0,

    /* U+007B "{" */
    0x74, 0x44, 0x44, 0x84, 0x44, 0x44, 0x70,

    /* U+007C "|" */
    0xff, 0xfc,

    /* U+007D "}" */
    0xe2, 0x22, 0x22, 0x12, 0x22, 0x22, 0xe0,

    /* U+007E "~" */
    0xcd, 0xc0,

    /* U+00B0 "°" */
    0xf6, 0xdb, 0xc0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 44, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 45, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 69, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 5, .adv_w = 117, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 15, .adv_w = 93, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 24, .adv_w = 150, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 35, .adv_w = 97, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 42, .adv_w = 37, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 43, .adv_w = 59, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 48, .adv_w = 59, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 53, .adv_w = 89, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 55, .adv_w = 92, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 59, .adv_w = 37, .box_w = 1, .box_h = 2, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 60, .adv_w = 66, .box_w = 2, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 61, .adv_w = 37, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 97, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 97, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 46, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 87, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 95, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 86, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 92, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 95, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 110, .adv_w = 81, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 97, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 95, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 37, .box_w = 1, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 37, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 130, .adv_w = 76, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 133, .adv_w = 99, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 135, .adv_w = 76, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 138, .adv_w = 86, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 126, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 90, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 161, .adv_w = 95, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 95, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 97, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 80, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 76, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 99, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 202, .adv_w = 45, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 88, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 86, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 75, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 222, .adv_w = 112, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 98, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 100, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 242, .adv_w = 92, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 101, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 94, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 77, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 98, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 87, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 288, .adv_w = 123, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 86, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 87, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 312, .adv_w = 77, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 318, .adv_w = 65, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 323, .adv_w = 97, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 332, .adv_w = 65, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 337, .adv_w = 90, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 339, .adv_w = 81, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 340, .adv_w = 90, .box_w = 2, .box_h = 2, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 341, .adv_w = 84, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 345, .adv_w = 85, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 351, .adv_w = 83, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 84, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 84, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 53, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 370, .adv_w = 84, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 375, .adv_w = 85, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 40, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 383, .adv_w = 40, .box_w = 2, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 386, .adv_w = 75, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 40, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 131, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 401, .adv_w = 85, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 405, .adv_w = 85, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 409, .adv_w = 84, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 414, .adv_w = 84, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 419, .adv_w = 74, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 423, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 57, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 431, .adv_w = 85, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 78, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 439, .adv_w = 113, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 446, .adv_w = 74, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 450, .adv_w = 78, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 455, .adv_w = 72, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 459, .adv_w = 76, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 466, .adv_w = 46, .box_w = 1, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 468, .adv_w = 76, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 475, .adv_w = 113, .box_w = 5, .box_h = 2, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 477, .adv_w = 73, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 6}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 176, .range_length = 1, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t cx_font_lbl = {
#else
lv_font_t cx_font_lbl = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if CX_FONT_LBL*/
