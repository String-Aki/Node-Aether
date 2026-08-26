/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --font /home/deepak/Stash/DEV/Projects/ESP32-S3/Node-Aether/Socket_Controller/main/fonts/vt323.ttf --size 16 --bpp 1 --format lvgl --range 0x20-0x7E --no-compress -o /home/deepak/Stash/DEV/Projects/ESP32-S3/Node-Aether/Socket_Controller/main/fonts/ui_font_vt323_16.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef UI_FONT_VT323_16
#define UI_FONT_VT323_16 1
#endif

#if UI_FONT_VT323_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfe, 0x40,

    /* U+0022 "\"" */
    0x99, 0x90,

    /* U+0023 "#" */
    0x51, 0x4f, 0xd4, 0x51, 0x4f, 0xd4, 0x50,

    /* U+0024 "$" */
    0x27, 0xeb, 0x5a, 0x53, 0xe5, 0xad, 0x7e, 0x42,
    0x0,

    /* U+0025 "%" */
    0x4a, 0xb4, 0xa2, 0x11, 0x4b, 0x94, 0x80,

    /* U+0026 "&" */
    0x23, 0x14, 0x52, 0xb7, 0x5d, 0xeb, 0x40,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x2a, 0x49, 0x24, 0x91, 0x10,

    /* U+0029 ")" */
    0x95, 0x55, 0x56,

    /* U+002A "*" */
    0x22, 0xb6, 0x2b, 0x20,

    /* U+002B "+" */
    0x21, 0x9, 0xf2, 0x10, 0x80,

    /* U+002C "," */
    0xfd, 0x60,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0xe0,

    /* U+002F "/" */
    0x8, 0x42, 0x21, 0x10, 0x84, 0x42, 0x11, 0x0,

    /* U+0030 "0" */
    0x20, 0x99, 0x99, 0x99, 0x96,

    /* U+0031 "1" */
    0x21, 0x18, 0x42, 0x10, 0x84, 0x27, 0xc0,

    /* U+0032 "2" */
    0x79, 0x91, 0x11, 0x24, 0x4f,

    /* U+0033 "3" */
    0x79, 0x91, 0x17, 0x19, 0x97,

    /* U+0034 "4" */
    0x38, 0x22, 0x92, 0x49, 0x2f, 0xc2, 0x8, 0x20,

    /* U+0035 "5" */
    0xf4, 0x21, 0x69, 0x68, 0x32, 0x93, 0x80,

    /* U+0036 "6" */
    0x38, 0x11, 0x8, 0x5f, 0x29, 0x49, 0x80,

    /* U+0037 "7" */
    0xf8, 0x42, 0x21, 0x8, 0x84, 0x22, 0x0,

    /* U+0038 "8" */
    0x79, 0x99, 0x97, 0x99, 0x97,

    /* U+0039 "9" */
    0x64, 0x25, 0x39, 0xf4, 0x22, 0x7, 0x0,

    /* U+003A ":" */
    0xe7,

    /* U+003B ";" */
    0xfc, 0x3f, 0x58,

    /* U+003C "<" */
    0x12, 0x24, 0x88, 0x42, 0x21,

    /* U+003D "=" */
    0xf0, 0xf,

    /* U+003E ">" */
    0x88, 0x42, 0x21, 0x24, 0x88,

    /* U+003F "?" */
    0x74, 0xa2, 0x10, 0x88, 0x80, 0x1, 0x0,

    /* U+0040 "@" */
    0x72, 0xa5, 0x7a, 0xd6, 0xb7, 0x42, 0x1c,

    /* U+0041 "A" */
    0x31, 0x9c, 0xa5, 0x25, 0xf1, 0x8c, 0x40,

    /* U+0042 "B" */
    0xfc, 0x63, 0x18, 0xfe, 0x31, 0x8f, 0xc0,

    /* U+0043 "C" */
    0x72, 0x93, 0x8, 0x42, 0x9, 0x53, 0x80,

    /* U+0044 "D" */
    0xf4, 0xa5, 0x18, 0xc6, 0x32, 0x97, 0x80,

    /* U+0045 "E" */
    0xf8, 0x88, 0x8f, 0x88, 0x8f,

    /* U+0046 "F" */
    0xf8, 0x88, 0x8f, 0x88, 0x88,

    /* U+0047 "G" */
    0x71, 0x24, 0x60, 0x82, 0x78, 0x51, 0x45, 0xe0,

    /* U+0048 "H" */
    0x8c, 0x63, 0x18, 0xfe, 0x31, 0x8c, 0x40,

    /* U+0049 "I" */
    0xe9, 0x24, 0x92, 0x5c,

    /* U+004A "J" */
    0x38, 0x84, 0x21, 0x8, 0x52, 0xa7, 0x0,

    /* U+004B "K" */
    0x8c, 0x63, 0x2a, 0x72, 0x51, 0x8c, 0x40,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0x87, 0xc0,

    /* U+004D "M" */
    0x8c, 0x77, 0xbf, 0xd6, 0xb5, 0x8c, 0x40,

    /* U+004E "N" */
    0x8c, 0x73, 0xda, 0xd6, 0xf3, 0x9c, 0x40,

    /* U+004F "O" */
    0x72, 0x93, 0x18, 0xc6, 0x29, 0x4b, 0x80,

    /* U+0050 "P" */
    0xfc, 0x63, 0x18, 0xfe, 0x10, 0x84, 0x0,

    /* U+0051 "Q" */
    0x72, 0x95, 0x18, 0xc6, 0x2a, 0x53, 0x84, 0x20,
    0x80,

    /* U+0052 "R" */
    0xf4, 0xa7, 0x29, 0x7a, 0x52, 0x94, 0x40,

    /* U+0053 "S" */
    0xfc, 0x63, 0x8, 0x7c, 0x31, 0x8f, 0xc0,

    /* U+0054 "T" */
    0xfc, 0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x80,

    /* U+0055 "U" */
    0x8c, 0x63, 0x18, 0xc6, 0x31, 0x8f, 0xc0,

    /* U+0056 "V" */
    0x8c, 0x63, 0x14, 0xa5, 0x4e, 0x71, 0x0,

    /* U+0057 "W" */
    0xbb, 0xbb, 0xbb, 0xbd, 0xd5,

    /* U+0058 "X" */
    0x8c, 0x52, 0xa5, 0x19, 0x49, 0x4c, 0x40,

    /* U+0059 "Y" */
    0x8c, 0x62, 0xa5, 0x38, 0x84, 0x21, 0x0,

    /* U+005A "Z" */
    0xf1, 0x11, 0x22, 0x48, 0x8f,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x70,

    /* U+005C "\\" */
    0x82, 0x10, 0x82, 0x10, 0x82, 0x10, 0x82, 0x10,

    /* U+005D "]" */
    0xd5, 0x55, 0x57,

    /* U+005E "^" */
    0x21, 0x1c, 0xa5, 0x44,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0xa4,

    /* U+0061 "a" */
    0x70, 0x85, 0xe9, 0x4b, 0xa0,

    /* U+0062 "b" */
    0x84, 0x21, 0x69, 0x6a, 0x32, 0x87, 0x0,

    /* U+0063 "c" */
    0x72, 0x93, 0x4, 0xa9, 0xc0,

    /* U+0064 "d" */
    0x8, 0x42, 0xd4, 0xae, 0x2b, 0x4b, 0x40,

    /* U+0065 "e" */
    0x39, 0x14, 0x7f, 0x41, 0x3, 0x80,

    /* U+0066 "f" */
    0x74, 0x4f, 0x44, 0x44, 0x4e,

    /* U+0067 "g" */
    0x7a, 0x53, 0x18, 0xad, 0xa2, 0x13, 0x80,

    /* U+0068 "h" */
    0x84, 0x21, 0x78, 0xe6, 0x31, 0x8c, 0x40,

    /* U+0069 "i" */
    0x20, 0x0, 0xc2, 0x10, 0x84, 0x27, 0xc0,

    /* U+006A "j" */
    0x20, 0x72, 0x49, 0x25, 0x2c,

    /* U+006B "k" */
    0x82, 0x8, 0x26, 0xa2, 0x8e, 0x28, 0xa2, 0x70,

    /* U+006C "l" */
    0x61, 0x8, 0x42, 0x10, 0x84, 0x27, 0xc0,

    /* U+006D "m" */
    0xd6, 0xab, 0x5a, 0xd6, 0xa0,

    /* U+006E "n" */
    0xbc, 0x73, 0x18, 0xc6, 0x20,

    /* U+006F "o" */
    0x72, 0x53, 0x14, 0xa9, 0xc0,

    /* U+0070 "p" */
    0xb4, 0xb5, 0x19, 0x4b, 0xd0, 0x84, 0x0,

    /* U+0071 "q" */
    0x7a, 0x53, 0x18, 0xad, 0xa1, 0x8, 0x40,

    /* U+0072 "r" */
    0xbd, 0x5a, 0x84, 0x23, 0x80,

    /* U+0073 "s" */
    0x78, 0x87, 0x11, 0x70,

    /* U+0074 "t" */
    0x21, 0x9, 0xf2, 0x10, 0x84, 0x21, 0xc0,

    /* U+0075 "u" */
    0x8c, 0x63, 0x1d, 0xa5, 0xe0,

    /* U+0076 "v" */
    0x8a, 0x52, 0xa7, 0x38, 0x80,

    /* U+0077 "w" */
    0xbb, 0xbb, 0xdd, 0x50,

    /* U+0078 "x" */
    0x95, 0x56, 0x59, 0x90,

    /* U+0079 "y" */
    0x8c, 0x52, 0xa5, 0x28, 0xc4, 0x23, 0x0,

    /* U+007A "z" */
    0xf8, 0x84, 0x88, 0x43, 0xe0,

    /* U+007B "{" */
    0x34, 0x44, 0x44, 0xc4, 0x44, 0x43,

    /* U+007C "|" */
    0xff, 0xf8,

    /* U+007D "}" */
    0xc4, 0x44, 0x44, 0x34, 0x44, 0x4c,

    /* U+007E "~" */
    0x4f, 0x7b, 0x79, 0x48
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 102, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 102, .box_w = 1, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 102, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 5, .adv_w = 102, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 12, .adv_w = 102, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 21, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 28, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 35, .adv_w = 102, .box_w = 1, .box_h = 3, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 36, .adv_w = 102, .box_w = 3, .box_h = 12, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 41, .adv_w = 102, .box_w = 2, .box_h = 12, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 44, .adv_w = 102, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 48, .adv_w = 102, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 53, .adv_w = 102, .box_w = 2, .box_h = 6, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 55, .adv_w = 102, .box_w = 3, .box_h = 1, .ofs_x = 2, .ofs_y = 4},
    {.bitmap_index = 56, .adv_w = 102, .box_w = 1, .box_h = 3, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 57, .adv_w = 102, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 65, .adv_w = 102, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 70, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 102, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 102, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 102, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 95, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 102, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 102, .box_w = 1, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 102, .box_w = 2, .box_h = 11, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 132, .adv_w = 102, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 102, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 139, .adv_w = 102, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 151, .adv_w = 102, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 158, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 102, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 102, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 102, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 102, .box_w = 3, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 215, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 222, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 236, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 250, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 102, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 273, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 102, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 295, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 302, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 102, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 328, .adv_w = 102, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 102, .box_w = 3, .box_h = 12, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 338, .adv_w = 102, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 346, .adv_w = 102, .box_w = 2, .box_h = 12, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 349, .adv_w = 102, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 353, .adv_w = 102, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 354, .adv_w = 102, .box_w = 2, .box_h = 3, .ofs_x = 2, .ofs_y = 9},
    {.bitmap_index = 355, .adv_w = 102, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 360, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 102, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 372, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 102, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 102, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 397, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 404, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 102, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 416, .adv_w = 102, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 424, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 431, .adv_w = 102, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 102, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 441, .adv_w = 102, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 446, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 453, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 460, .adv_w = 102, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 465, .adv_w = 102, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 469, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 476, .adv_w = 102, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 481, .adv_w = 102, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 486, .adv_w = 102, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 490, .adv_w = 102, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 494, .adv_w = 102, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 501, .adv_w = 102, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 506, .adv_w = 102, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 512, .adv_w = 102, .box_w = 1, .box_h = 13, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 514, .adv_w = 102, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 520, .adv_w = 102, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3}
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
    .cmap_num = 1,
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
const lv_font_t ui_font_vt323_16 = {
#else
lv_font_t ui_font_vt323_16 = {
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



#endif /*#if UI_FONT_VT323_16*/

