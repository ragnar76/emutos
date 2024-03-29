/*
 * videl.c - Falcon VIDEL support
 *
 * Copyright (c) 2012 The EmuTOS development team
 *
 * Authors:
 *  PES   Petr Stehlik
 *  RFB   Roger Burrows
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#define DBG_VIDEL 0

#include "config.h"  
#include "machine.h"
#include "screen.h"
#include "videl.h"
#include "asm.h"
#include "tosvars.h"
#include "lineavars.h"
#include "nvram.h"
#include "kprint.h"
#include "font.h"
#include "vt52.h"
#include "xbiosbind.h"
#include "vectors.h"

#if CONF_WITH_VIDEL

static const LONG videl_dflt_palette[] = {
    FRGB_WHITE, FRGB_RED, FRGB_GREEN, FRGB_YELLOW,
    FRGB_BLUE, FRGB_MAGENTA, FRGB_CYAN, FRGB_LTGRAY,
    FRGB_GRAY, FRGB_LTRED, FRGB_LTGREEN, FRGB_LTYELLOW,
    FRGB_LTBLUE, FRGB_LTMAGENTA, FRGB_LTCYAN, FRGB_BLACK,
    0xffff00ff, 0xeded00ed, 0xdddd00dd, 0xcccc00cc,
    0xbaba00ba, 0xaaaa00aa, 0x99990099, 0x87870087,
    0x77770077, 0x66660066, 0x54540054, 0x44440044,
    0x33330033, 0x21210021, 0x11110011, 0x00000000,
    0xff000000, 0xff000011, 0xff000021, 0xff000033,
    0xff000044, 0xff000054, 0xff000066, 0xff000077,
    0xff000087, 0xff000099, 0xff0000aa, 0xff0000ba,
    0xff0000cc, 0xff0000dd, 0xff0000ed, 0xff0000ff,
    0xed0000ff, 0xdd0000ff, 0xcc0000ff, 0xba0000ff,
    0xaa0000ff, 0x990000ff, 0x870000ff, 0x770000ff,
    0x660000ff, 0x540000ff, 0x440000ff, 0x330000ff,
    0x210000ff, 0x110000ff, 0x000000ff, 0x001100ff,
    0x002100ff, 0x003300ff, 0x004400ff, 0x005400ff,
    0x006600ff, 0x007700ff, 0x008700ff, 0x009900ff,
    0x00aa00ff, 0x00ba00ff, 0x00cc00ff, 0x00dd00ff,
    0x00ed00ff, 0x00ff00ff, 0x00ff00ed, 0x00ff00dd,
    0x00ff00cc, 0x00ff00ba, 0x00ff00aa, 0x00ff0099,
    0x00ff0087, 0x00ff0077, 0x00ff0066, 0x00ff0054,
    0x00ff0044, 0x00ff0033, 0x00ff0021, 0x00ff0011,
    0x00ff0000, 0x11ff0000, 0x21ff0000, 0x33ff0000,
    0x44ff0000, 0x54ff0000, 0x66ff0000, 0x77ff0000,
    0x87ff0000, 0x99ff0000, 0xaaff0000, 0xbaff0000,
    0xccff0000, 0xddff0000, 0xedff0000, 0xffff0000,
    0xffed0000, 0xffdd0000, 0xffcc0000, 0xffba0000,
    0xffaa0000, 0xff990000, 0xff870000, 0xff770000,
    0xff660000, 0xff540000, 0xff440000, 0xff330000,
    0xff210000, 0xff110000, 0xba000000, 0xba000011,
    0xba000021, 0xba000033, 0xba000044, 0xba000054,
    0xba000066, 0xba000077, 0xba000087, 0xba000099,
    0xba0000aa, 0xba0000ba, 0xaa0000ba, 0x990000ba,
    0x870000ba, 0x770000ba, 0x660000ba, 0x540000ba,
    0x440000ba, 0x330000ba, 0x210000ba, 0x110000ba,
    0x000000ba, 0x001100ba, 0x002100ba, 0x003300ba,
    0x004400ba, 0x005400ba, 0x006600ba, 0x007700ba,
    0x008700ba, 0x009900ba, 0x00aa00ba, 0x00ba00ba,
    0x00ba00aa, 0x00ba0099, 0x00ba0087, 0x00ba0077,
    0x00ba0066, 0x00ba0054, 0x00ba0044, 0x00ba0033,
    0x00ba0021, 0x00ba0011, 0x00ba0000, 0x11ba0000,
    0x21ba0000, 0x33ba0000, 0x44ba0000, 0x54ba0000,
    0x66ba0000, 0x77ba0000, 0x87ba0000, 0x99ba0000,
    0xaaba0000, 0xbaba0000, 0xbaaa0000, 0xba990000,
    0xba870000, 0xba770000, 0xba660000, 0xba540000,
    0xba440000, 0xba330000, 0xba210000, 0xba110000,
    0x77000000, 0x77000011, 0x77000021, 0x77000033,
    0x77000044, 0x77000054, 0x77000066, 0x77000077,
    0x66000077, 0x54000077, 0x44000077, 0x33000077,
    0x21000077, 0x11000077, 0x00000077, 0x00110077,
    0x00210077, 0x00330077, 0x00440077, 0x00540077,
    0x00660077, 0x00770077, 0x00770066, 0x00770054,
    0x00770044, 0x00770033, 0x00770021, 0x00770011,
    0x00770000, 0x11770000, 0x21770000, 0x33770000,
    0x44770000, 0x54770000, 0x66770000, 0x77770000,
    0x77660000, 0x77540000, 0x77440000, 0x77330000,
    0x77210000, 0x77110000, 0x44000000, 0x44000011,
    0x44000021, 0x44000033, 0x44000044, 0x33000044,
    0x21000044, 0x11000044, 0x00000044, 0x00110044,
    0x00210044, 0x00330044, 0x00440044, 0x00440033,
    0x00440021, 0x00440011, 0x00440000, 0x11440000,
    0x21440000, 0x33440000, 0x44440000, 0x44330000,
    0x44210000, 0x44110000, FRGB_WHITE, FRGB_BLACK
};

GLOBAL LONG falcon_shadow_palette[256];   /* real Falcon does this */
static WORD ste_shadow_palette[16];

#define MON_ALL     -1  /* code used in VMODE_ENTRY for match on mode only */

/*
 * tables that cover all(?) valid Falcon modes
 * note:
 *  . 256-colour and Truecolor modes are not currently supported by the VDI
 */
static const VMODE_ENTRY vga_init_table[] = {
    /* the entries in this table are for VGA/NTSC (i.e. VGA 60Hz) and VGA/PAL
     * (i.e. VGA 50Hz).  in *this* table, each entry applies to four video modes:
     * mode, mode|VIDEL_VERTICAL, mode|VIDEL_PAL, mode|VIDEL_VERTICAL|VIDEL_PAL
     */
    { 0x0011, MON_ALL,  0x0017, 0x0012, 0x0001, 0x020a, 0x0009, 0x0011, 0x0419, 0x03ff, 0x003f, 0x003f, 0x03ff, 0x0415 },
    { 0x0012, MON_ALL,  0x00c6, 0x008d, 0x0015, 0x028a, 0x006b, 0x0096, 0x0419, 0x03ff, 0x003f, 0x003f, 0x03ff, 0x0415 },
    { 0x0013, MON_ALL,  0x00c6, 0x008d, 0x0015, 0x029a, 0x007b, 0x0096, 0x0419, 0x03ff, 0x003f, 0x003f, 0x03ff, 0x0415 },
    { 0x0014, MON_ALL,  0x00c6, 0x008d, 0x0015, 0x02ac, 0x0091, 0x0096, 0x0419, 0x03ff, 0x003f, 0x003f, 0x03ff, 0x0415 },
    { 0x0018, MON_ALL,  0x00c6, 0x008d, 0x0015, 0x0273, 0x0050, 0x0096, 0x0419, 0x03ff, 0x003f, 0x003f, 0x03ff, 0x0415 },
    { 0x0019, MON_ALL,  0x0017, 0x0012, 0x0001, 0x020e, 0x000d, 0x0011, 0x0419, 0x03ff, 0x003f, 0x003f, 0x03ff, 0x0415 },
    { 0x001a, MON_ALL,  0x00c6, 0x008d, 0x0015, 0x02a3, 0x007c, 0x0096, 0x0419, 0x03ff, 0x003f, 0x003f, 0x03ff, 0x0415 },
    { 0x001b, MON_ALL,  0x00c6, 0x008d, 0x0015, 0x02ab, 0x0084, 0x0096, 0x0419, 0x03ff, 0x003f, 0x003f, 0x03ff, 0x0415 },
    { 0x0092, MON_ALL,  0x0017, 0x0012, 0x0001, 0x020e, 0x000d, 0x0011, 0x0419, 0x03af, 0x008f, 0x008f, 0x03af, 0x0415 },
    { 0x0098, MON_ALL,  0x00c6, 0x008d, 0x0015, 0x0273, 0x0050, 0x0096, 0x0419, 0x03af, 0x008f, 0x008f, 0x03af, 0x0415 },
    { 0x0099, MON_ALL,  0x0017, 0x0012, 0x0001, 0x020e, 0x000d, 0x0011, 0x0419, 0x03af, 0x008f, 0x008f, 0x03af, 0x0415 },
    { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static const VMODE_ENTRY nonvga_init_table[] = {
    /* the remaining entries are for TV+NTSC, TV+PAL, TV+NTSC+overscan, TV+PAL+overscan */
    { 0x0001, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0239, 0x0012, 0x0034, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x0002, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x000c, 0x006d, 0x00d8, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x0003, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x001c, 0x007d, 0x00d8, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x0004, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x002e, 0x008f, 0x00d8, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x0008, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x03f0, 0x009f, 0x01b4, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x0009, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0002, 0x0020, 0x0034, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x000a, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x004d, 0x00fd, 0x01b4, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x000b, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x005d, 0x010d, 0x01b4, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x0021, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0239, 0x0012, 0x0034, 0x0271, 0x0265, 0x002f, 0x007f, 0x020f, 0x026b },
    { 0x0022, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x000c, 0x006d, 0x00d8, 0x0271, 0x0265, 0x002f, 0x007f, 0x020f, 0x026b },
    { 0x0023, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x001c, 0x007d, 0x00d8, 0x0271, 0x0265, 0x002f, 0x007f, 0x020f, 0x026b },
    { 0x0024, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x002e, 0x008f, 0x00d8, 0x0271, 0x0265, 0x002f, 0x007f, 0x020f, 0x026b },
    { 0x0028, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x03ef, 0x00a0, 0x01b2, 0x0271, 0x0265, 0x002f, 0x007f, 0x020f, 0x026b },
    { 0x0029, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0002, 0x0020, 0x0034, 0x0271, 0x0265, 0x002f, 0x007f, 0x020f, 0x026b },
    { 0x002a, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x004d, 0x00fe, 0x01b2, 0x0271, 0x0265, 0x002f, 0x007f, 0x020f, 0x026b },
    { 0x002b, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x005d, 0x010e, 0x01b2, 0x0271, 0x0265, 0x002f, 0x007f, 0x020f, 0x026b },
    { 0x0041, MON_VGA,  0x003e, 0x0030, 0x0008, 0x0239, 0x0012, 0x0034, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0041, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0232, 0x001b, 0x0034, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0042, MON_VGA,  0x00fe, 0x00c9, 0x0027, 0x000c, 0x006d, 0x00d8, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0042, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x02ec, 0x008d, 0x00d8, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0043, MON_VGA,  0x00fe, 0x00c9, 0x0027, 0x001c, 0x007d, 0x00d8, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0043, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x02fc, 0x009d, 0x00d8, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0044, MON_VGA,  0x00fe, 0x00c9, 0x0027, 0x002e, 0x008f, 0x00d8, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0044, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x000e, 0x00af, 0x00d8, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0048, MON_VGA,  0x01ff, 0x0197, 0x0050, 0x03f0, 0x009f, 0x01b4, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0048, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x03b0, 0x00df, 0x01b4, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0049, MON_VGA,  0x003e, 0x0030, 0x0008, 0x023b, 0x001c, 0x0034, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0049, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0237, 0x0020, 0x0034, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x004a, MON_VGA,  0x01ff, 0x0197, 0x0050, 0x004d, 0x00fd, 0x01b4, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x004a, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x000d, 0x013d, 0x01b4, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x004b, MON_VGA,  0x01ff, 0x0197, 0x0050, 0x005d, 0x010d, 0x01b4, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x004b, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x001d, 0x014d, 0x01b4, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x0061, MON_VGA,  0x003e, 0x0030, 0x0008, 0x0239, 0x0012, 0x0034, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0061, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0232, 0x001b, 0x0034, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0062, MON_VGA,  0x00fe, 0x00cb, 0x0027, 0x000c, 0x006d, 0x00d8, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0062, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x02ec, 0x008d, 0x00d8, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0063, MON_VGA,  0x00fe, 0x00cb, 0x0027, 0x001c, 0x007d, 0x00d8, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0063, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x02fc, 0x009d, 0x00d8, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0064, MON_VGA,  0x00fe, 0x00cb, 0x0027, 0x002e, 0x008f, 0x00d8, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0064, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x000e, 0x00af, 0x00d8, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0068, MON_VGA,  0x01fe, 0x0199, 0x0050, 0x03ef, 0x00a0, 0x01b2, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0068, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x03af, 0x00e0, 0x01b2, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0069, MON_VGA,  0x003e, 0x0030, 0x0008, 0x023b, 0x001c, 0x0034, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0069, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0237, 0x0020, 0x0034, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x006a, MON_VGA,  0x01fe, 0x0199, 0x0050, 0x004d, 0x00fe, 0x01b2, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x006a, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x000d, 0x013e, 0x01b2, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x006b, MON_VGA,  0x01fe, 0x0199, 0x0050, 0x005d, 0x010e, 0x01b2, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x006b, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x001d, 0x014e, 0x01b2, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x0082, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x0088, MON_MONO, 0x001a, 0x0000, 0x0000, 0x020f, 0x000c, 0x0014, 0x03e9, 0x0000, 0x0000, 0x0043, 0x0363, 0x03e7 },
    { 0x0088, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x03f0, 0x009f, 0x01b4, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x0089, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x00a2, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x0271, 0x0265, 0x002f, 0x006f, 0x01ff, 0x026b },
    { 0x00a8, MON_MONO, 0x001a, 0x0000, 0x0000, 0x020f, 0x000c, 0x0014, 0x03e9, 0x0000, 0x0000, 0x0043, 0x0363, 0x03e7 },
    { 0x00a8, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x03ef, 0x00a0, 0x01b2, 0x0271, 0x0265, 0x002f, 0x007f, 0x020f, 0x026b },
    { 0x00a9, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x0271, 0x0265, 0x002f, 0x006f, 0x01ff, 0x026b },
    { 0x00c2, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x00c8, MON_MONO, 0x001a, 0x0000, 0x0000, 0x020f, 0x000c, 0x0014, 0x03e9, 0x0000, 0x0000, 0x0043, 0x0363, 0x03e7 },
    { 0x00c8, MON_VGA,  0x01ff, 0x0197, 0x0050, 0x03f0, 0x009f, 0x01b4, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x00c8, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x03b0, 0x00df, 0x01b4, 0x020d, 0x0201, 0x0016, 0x0025, 0x0205, 0x0207 },
    { 0x00c9, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x00e2, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x0271, 0x0265, 0x002f, 0x006f, 0x01ff, 0x026b },
    { 0x00e8, MON_MONO, 0x001a, 0x0000, 0x0000, 0x020f, 0x000c, 0x0014, 0x03e9, 0x0000, 0x0000, 0x0043, 0x0363, 0x03e7 },
    { 0x00e8, MON_VGA,  0x01fe, 0x0199, 0x0050, 0x03ef, 0x00a0, 0x01b2, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x00e8, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x03af, 0x00e0, 0x01b2, 0x0271, 0x0265, 0x002f, 0x0057, 0x0237, 0x026b },
    { 0x00e9, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x0271, 0x0265, 0x002f, 0x006f, 0x01ff, 0x026b },
    { 0x0101, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0239, 0x0012, 0x0034, 0x020c, 0x0201, 0x0016, 0x004c, 0x01dc, 0x0207 },
    { 0x0102, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x000c, 0x006d, 0x00d8, 0x020c, 0x0201, 0x0016, 0x004c, 0x01dc, 0x0207 },
    { 0x0103, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x001c, 0x007d, 0x00d8, 0x020c, 0x0201, 0x0016, 0x004c, 0x01dc, 0x0207 },
    { 0x0104, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x002e, 0x008f, 0x00d8, 0x020c, 0x0201, 0x0016, 0x004c, 0x01dc, 0x0207 },
    { 0x0108, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x03f0, 0x009f, 0x01b4, 0x020c, 0x0201, 0x0016, 0x004c, 0x01dc, 0x0207 },
    { 0x0109, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0002, 0x0020, 0x0034, 0x020c, 0x0201, 0x0016, 0x004c, 0x01dc, 0x0207 },
    { 0x010a, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x004d, 0x00fd, 0x01b4, 0x020c, 0x0201, 0x0016, 0x004c, 0x01dc, 0x0207 },
    { 0x010b, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x005d, 0x010d, 0x01b4, 0x020c, 0x0201, 0x0016, 0x004c, 0x01dc, 0x0207 },
    { 0x0121, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0239, 0x0012, 0x0034, 0x0270, 0x0265, 0x002f, 0x007e, 0x020e, 0x026b },
    { 0x0122, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x000c, 0x006d, 0x00d8, 0x0270, 0x0265, 0x002f, 0x007e, 0x020e, 0x026b },
    { 0x0123, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x001c, 0x007d, 0x00d8, 0x0270, 0x0265, 0x002f, 0x007e, 0x020e, 0x026b },
    { 0x0124, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x002e, 0x008f, 0x00d8, 0x0270, 0x0265, 0x002f, 0x007e, 0x020e, 0x026b },
    { 0x0128, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x03ef, 0x00a0, 0x01b2, 0x0270, 0x0265, 0x002f, 0x007e, 0x020e, 0x026b },
    { 0x0129, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0002, 0x0020, 0x0034, 0x0270, 0x0265, 0x002f, 0x007e, 0x020e, 0x026b },
    { 0x012a, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x004d, 0x00fe, 0x01b2, 0x0270, 0x0265, 0x002f, 0x007e, 0x020e, 0x026b },
    { 0x012b, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x005d, 0x010e, 0x01b2, 0x0270, 0x0265, 0x002f, 0x007e, 0x020e, 0x026b },
    { 0x0141, MON_VGA,  0x003e, 0x0030, 0x0008, 0x0239, 0x0012, 0x0034, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0141, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0232, 0x001b, 0x0034, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0142, MON_VGA,  0x00fe, 0x00c9, 0x0027, 0x000c, 0x006d, 0x00d8, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0142, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x02ec, 0x008d, 0x00d8, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0143, MON_VGA,  0x00fe, 0x00c9, 0x0027, 0x001c, 0x007d, 0x00d8, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0143, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x02fc, 0x009d, 0x00d8, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0144, MON_VGA,  0x00fe, 0x00c9, 0x0027, 0x002e, 0x008f, 0x00d8, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0144, MON_ALL,  0x00fe, 0x00c9, 0x0027, 0x000e, 0x00af, 0x00d8, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0148, MON_VGA,  0x01ff, 0x0197, 0x0050, 0x03f0, 0x009f, 0x01b4, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0148, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x03b0, 0x00df, 0x01b4, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0149, MON_VGA,  0x003e, 0x0030, 0x0008, 0x023b, 0x001c, 0x0034, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0149, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0237, 0x0020, 0x0034, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x014a, MON_VGA,  0x01ff, 0x0197, 0x0050, 0x004d, 0x00fd, 0x01b4, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x014a, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x000d, 0x013d, 0x01b4, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x014b, MON_VGA,  0x01ff, 0x0197, 0x0050, 0x005d, 0x010d, 0x01b4, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x014b, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x001d, 0x014d, 0x01b4, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x0161, MON_VGA,  0x003e, 0x0030, 0x0008, 0x0239, 0x0012, 0x0034, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0161, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0232, 0x001b, 0x0034, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0162, MON_VGA,  0x00fe, 0x00cb, 0x0027, 0x000c, 0x006d, 0x00d8, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0162, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x02ec, 0x008d, 0x00d8, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0163, MON_VGA,  0x00fe, 0x00cb, 0x0027, 0x001c, 0x007d, 0x00d8, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0163, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x02fc, 0x009d, 0x00d8, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0164, MON_VGA,  0x00fe, 0x00cb, 0x0027, 0x002e, 0x008f, 0x00d8, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0164, MON_ALL,  0x00fe, 0x00cb, 0x0027, 0x000e, 0x00af, 0x00d8, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0168, MON_VGA,  0x01fe, 0x0199, 0x0050, 0x03ef, 0x00a0, 0x01b2, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0168, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x03af, 0x00e0, 0x01b2, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0169, MON_VGA,  0x003e, 0x0030, 0x0008, 0x023b, 0x001c, 0x0034, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0169, MON_ALL,  0x003e, 0x0030, 0x0008, 0x0237, 0x0020, 0x0034, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x016a, MON_VGA,  0x01fe, 0x0199, 0x0050, 0x004d, 0x00fe, 0x01b2, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x016a, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x000d, 0x013e, 0x01b2, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x016b, MON_VGA,  0x01fe, 0x0199, 0x0050, 0x005d, 0x010e, 0x01b2, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x016b, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x001d, 0x014e, 0x01b2, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x0182, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x0188, MON_MONO, 0x001a, 0x0000, 0x0000, 0x020f, 0x000c, 0x0014, 0x03e9, 0x0000, 0x0000, 0x0043, 0x0363, 0x03e7 },
    { 0x0188, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x03f0, 0x009f, 0x01b4, 0x020c, 0x0201, 0x0016, 0x004c, 0x01dc, 0x0207 },
    { 0x0189, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x01a2, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x0271, 0x0265, 0x002f, 0x006f, 0x01ff, 0x026b },
    { 0x01a8, MON_MONO, 0x001a, 0x0000, 0x0000, 0x020f, 0x000c, 0x0014, 0x03e9, 0x0000, 0x0000, 0x0043, 0x0363, 0x03e7 },
    { 0x01a8, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x03ef, 0x00a0, 0x01b2, 0x0270, 0x0265, 0x002f, 0x007e, 0x020e, 0x026b },
    { 0x01a9, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x0271, 0x0265, 0x002f, 0x006f, 0x01ff, 0x026b },
    { 0x01c2, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x01c8, MON_MONO, 0x001a, 0x0000, 0x0000, 0x020f, 0x000c, 0x0014, 0x03e9, 0x0000, 0x0000, 0x0043, 0x0363, 0x03e7 },
    { 0x01c8, MON_VGA,  0x01ff, 0x0197, 0x0050, 0x03f0, 0x009f, 0x01b4, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x01c8, MON_ALL,  0x01ff, 0x0197, 0x0050, 0x03b0, 0x00df, 0x01b4, 0x020c, 0x0201, 0x0016, 0x0024, 0x0204, 0x0207 },
    { 0x01c9, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x020d, 0x0201, 0x0016, 0x004d, 0x01dd, 0x0207 },
    { 0x01e2, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x0271, 0x0265, 0x002f, 0x006f, 0x01ff, 0x026b },
    { 0x01e8, MON_MONO, 0x001a, 0x0000, 0x0000, 0x020f, 0x000c, 0x0014, 0x03e9, 0x0000, 0x0000, 0x0043, 0x0363, 0x03e7 },
    { 0x01e8, MON_VGA,  0x01fe, 0x0199, 0x0050, 0x03ef, 0x00a0, 0x01b2, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x01e8, MON_ALL,  0x01fe, 0x0199, 0x0050, 0x03af, 0x00e0, 0x01b2, 0x0270, 0x0265, 0x002f, 0x0056, 0x0236, 0x026b },
    { 0x01e9, MON_ALL,  0x003e, 0x0032, 0x0009, 0x023f, 0x001c, 0x0034, 0x0271, 0x0265, 0x002f, 0x006f, 0x01ff, 0x026b },
    { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

/*
 * functions for VIDEL programming
 */

static UWORD get_videl_bpp(void)
{
    UWORD f_shift = *(volatile UWORD *)SPSHIFT;
    UBYTE st_shift = *(volatile UBYTE *)ST_SHIFTER;
    /* to get bpp, we must examine f_shift and st_shift.
     * f_shift is valid if any of bits no. 10, 8 or 4
     * is set. Priority in f_shift is: 10 ">" 8 ">" 4, i.e.
     * if bit 10 set then bit 8 and bit 4 don't care...
     * If all these bits are 0 get display depth from st_shift
     * (as for ST and STE)
     */
    int bits_per_pixel = 1;
    if (f_shift & 0x400)         /* 2 colors */
        bits_per_pixel = 1;
    else if (f_shift & 0x100)    /* hicolor */
        bits_per_pixel = 16;
    else if (f_shift & 0x010)    /* 8 bitplanes */
        bits_per_pixel = 8;
    else if (st_shift == 0)
        bits_per_pixel = 4;
    else if (st_shift == 0x1)
        bits_per_pixel = 2;
    else /* if (st_shift == 0x2) */
        bits_per_pixel = 1;

    return bits_per_pixel;
}

static UWORD get_videl_width(void)
{
    return (*(volatile UWORD *)0xffff8210) * 16 / get_videl_bpp();
}

static UWORD get_videl_height(void)
{
    UWORD vdb = *(volatile UWORD *)0xffff82a8;
    UWORD vde = *(volatile UWORD *)0xffff82aa;
    UWORD vmode = *(volatile UWORD *)0xffff82c2;

    /* visible y resolution:
     * Graphics display starts at line VDB and ends at line
     * VDE. If interlace mode off unit of VC-registers is
     * half lines, else lines.
     */
    UWORD yres = vde - vdb;
    if (!(vmode & 0x02))        /* interlace */
        yres >>= 1;
    if (vmode & 0x01)           /* double */
        yres >>= 1;

    return yres;
}


/*
 * lookup videl initialisation data for specified mode/monitor
 * returns NULL if mode/monitor combination is invalid
 */
const VMODE_ENTRY *lookup_videl_mode(WORD mode,WORD monitor)
{
    const VMODE_ENTRY *vmode_init_table, *p;

    if (mode&VIDEL_VGA) {
        vmode_init_table = vga_init_table;
        /* ignore bits that don't affect initialisation data */
        mode &= ~(VIDEL_VERTICAL|VIDEL_PAL);
    } else {
        vmode_init_table = nonvga_init_table;
    }

    for (p = vmode_init_table; p->vmode >= 0; p++)
        if (p->vmode == mode)
            if ((p->monitor == MON_ALL) || (p->monitor == monitor))
                return p;

    return NULL;
}


/*
 * determine scanline width based on video mode
 */
WORD determine_width(WORD mode)
{
    WORD linewidth;

    linewidth = (mode&VIDEL_80COL) ? 40 : 20;
    linewidth <<= (mode & VIDEL_BPPMASK);
    if (mode&VIDEL_OVERSCAN)
        linewidth = linewidth * 12 / 10;    /* multiply by 1.2 */

    return linewidth;
}


/*
 * determine vctl based on video mode and monitor type
 */
WORD determine_vctl(WORD mode,WORD monitor)
{
    WORD vctl;

    if (mode&VIDEL_VGA) {
        vctl = (mode&VIDEL_80COL) ? 0x08 : 0x04;
        if (mode&VIDEL_VERTICAL)
            vctl |= 0x01;
    } else {
        vctl = (mode&VIDEL_80COL) ? 0x04 : 0x00;
        if (mode&VIDEL_VERTICAL)
            vctl |= 0x02;
    }

    if (!(mode&VIDEL_COMPAT))
        return vctl;

    switch(mode&VIDEL_BPPMASK) {
    case VIDEL_1BPP:
        if (!(mode&VIDEL_VGA) && (monitor == MON_MONO))
            vctl = 0x08;
        break;
    case VIDEL_2BPP:
        vctl = (mode&VIDEL_VGA)? 0x09 : 0x04;
        break;
    case VIDEL_4BPP:
        vctl = (mode&VIDEL_VGA)? 0x05 : 0x00;
        break;
    }

    return vctl;
}


/*
 * determine regc0 based on video mode & monitor type
 */
WORD determine_regc0(WORD mode,WORD monitor)
{
    if (mode&VIDEL_VGA)
        return 0x0186;

    if (!(mode&VIDEL_COMPAT))
        return (monitor==MON_TV)?0x0183:0x0181;

    /* handle ST-compatible modes */
    if ((mode&(VIDEL_80COL|VIDEL_BPPMASK)) == (VIDEL_80COL|VIDEL_1BPP)) {  /* 80-column, 2-colour */
        switch(monitor) {
        case MON_MONO:
            return 0x0080;
        case MON_TV:
            return 0x0183;
        default:
            return 0x0181;
        }
    }

    return (monitor==MON_TV)?0x0083:0x0081;
}


/*
 * this routine can set VIDEL to 1,2,4 or 8 bitplanes mode on VGA
 */
static int set_videl_vga(WORD mode)
{
    volatile char *videlregs = (char *)0xffff8200;
#define videlword(n) (*(volatile UWORD *)(videlregs+(n)))
    const VMODE_ENTRY *p;
    WORD linewidth, monitor, vctl;

    monitor = vmontype();

    p = lookup_videl_mode(mode,monitor);/* validate mode */
    if (!p)
        return -1;

    videlregs[0x0a] = (mode&VIDEL_PAL) ? 2 : 0; /* video sync to 50Hz if PAL */

    // FIXME: vsync() can't work if the screen is initially turned off
    //vsync(); /* wait for vbl so we're not interrupted :-) */

    videlword(0x82) = p->hht;           /* H hold timer */
    videlword(0x84) = p->hbb;           /* H border begin */
    videlword(0x86) = p->hbe;           /* H border end */
    videlword(0x88) = p->hdb;           /* H display begin */
    videlword(0x8a) = p->hde;           /* H display end */
    videlword(0x8c) = p->hss;           /* H SS */

    videlword(0xa2) = p->vft;           /* V freq timer */
    videlword(0xa4) = p->vbb;           /* V border begin */
    videlword(0xa6) = p->vbe;           /* V border end */
    videlword(0xa8) = p->vdb;           /* V display begin */
    videlword(0xaa) = p->vde;           /* V display end */
    videlword(0xac) = p->vss;           /* V SS */

    videlregs[0x60] = 0x00;             /* clear ST shift for safety */

    videlword(0x0e) = 0;                /* offset */

    linewidth = determine_width(mode);
    vctl = determine_vctl(mode,monitor);

    videlword(0x10) = linewidth;        /* scanline width */
    videlword(0xc2) = vctl;             /* video control */
    videlword(0xc0) = determine_regc0(mode,monitor);
    videlword(0x66) = 0x0000;           /* clear SPSHIFT */

    switch(mode&VIDEL_BPPMASK) {        /* set SPSHIFT / ST shift */
    case VIDEL_1BPP:                    /* 2 colours (mono) */
        if (monitor == MON_MONO)
            videlregs[0x60] = 0x02;
        else videlword(0x66) = 0x0400;
        break;
    case VIDEL_2BPP:                    /* 4 colours */
        videlregs[0x60] = 0x01;
        videlword(0x10) = linewidth;        /* writing to the ST shifter has    */
        videlword(0xc2) = vctl;             /* just overwritten these registers */
        break;
    case VIDEL_4BPP:                    /* 16 colours */
        /* if not ST-compatible, SPSHIFT was already set correctly above */
        if (mode&VIDEL_COMPAT)
            videlregs[0x60] = 0x00;         /* else set ST shifter */
        break;
    case VIDEL_8BPP:                    /* 256 colours */
        videlword(0x66) = 0x0010;
        break;
    case VIDEL_TRUECOLOR:               /* 65536 colours (Truecolor) */
        videlword(0x66) = 0x0100;
        break;
    }

    return 0;
}

/*
 * the current Falcon video mode; used by vsetmode() & vfixmode()
 */
WORD current_video_mode;

/*
 * Set Falcon video mode
 */
WORD vsetmode(WORD mode)
{
    WORD ret;

    if (!has_videl)
        return 0x58; /* XBIOS function number as unimplemented error code */

    if (mode == -1)
        return current_video_mode;

#if DBG_VIDEL
    kprintf("vsetmode(0x%04x)\n", mode);
#endif

    if (set_videl_vga(mode) < 0)    /* invalid mode */
        return current_video_mode;

    ret = current_video_mode;
    current_video_mode = mode;

    return ret;
}

/*
 * Get Videl monitor type
 */
WORD vmontype(void)
{
    if (!has_videl)
        return 0x59; /* XBIOS function number as unimplemented error code */

    return ((*(volatile UBYTE *)0xffff8006) >> 6) & 3;
}

/*
 * Set external video sync mode
 */
WORD vsetsync(WORD external)
{
    UWORD spshift;

    if (!has_videl)
        return 0x5a; /* XBIOS function number as unimplemented error code */

    if (external & 0x01)            /* external clock wanted? */
        *(volatile BYTE *)SYNCMODE |= 0x01;
    else *(volatile BYTE *)SYNCMODE &= 0xfe;

    spshift = *(volatile UWORD *)SPSHIFT;

    if (external&0x02)              /* external vertical sync wanted? */
        spshift |= 0x0020;
    else spshift &= 0xffdf;

    if (external&0x04)              /* external horizontal sync wanted? */
        spshift |= 0x0040;
    else spshift &= 0xffbf;

    *(volatile UWORD *)SPSHIFT = spshift;

    return 0; /* OK */
}

/*
 * get video ram size according to mode
 */
LONG vgetsize(WORD mode)
{
    const VMODE_ENTRY *p;
    int height;
    WORD vctl, monitor;

    if (!has_videl)
        return 0x5b; /* XBIOS function number as unimplemented error code */

    monitor = vmontype();

    mode &= VIDEL_VALID;        /* ignore invalid bits */
    if ((mode&VIDEL_BPPMASK) > VIDEL_TRUECOLOR) {   /* fixup invalid bpp */
        mode &= ~VIDEL_BPPMASK;
        mode |= VIDEL_TRUECOLOR;
    }

    p = lookup_videl_mode(mode,monitor);
    if (!p) {                   /* invalid mode */
        if (mode&VIDEL_COMPAT)
            return ST_VRAM_SIZE;
        mode &= ~(VIDEL_OVERSCAN|VIDEL_PAL);/* ignore less-important bits */
        p = lookup_videl_mode(mode,monitor);/* & try again */
        if (!p)                             /* "can't happen" */
            return FALCON_VRAM_SIZE;
    }

    vctl = determine_vctl(mode,monitor);
    height = p->vde - p->vdb;
    if (!(vctl&0x02))
        height >>= 1;
    if (vctl&0x01)
        height >>= 1;

    return (LONG)determine_width(mode) * 2 * height;
}

/*
 * convert from Falcon palette format to STe palette format
 */
#define falc2ste(a) ((((a)>>1)&0x08)|(((a)>>5)&0x07))
static void convert2ste(WORD *ste,LONG *falcon)
{
    union {
        LONG l;
        UBYTE b[4];
    } u;
    int i;

    for (i = 0; i < 16; i++) {
        u.l = *falcon++;
        *ste++ = (falc2ste(u.b[0])<<8) | (falc2ste(u.b[1])<<4) | falc2ste(u.b[3]);
    }
}

/*
 * determine whether to update STe or Falcon h/w palette registers
 * returns TRUE if we need to update the STE h/w palette
 */
static int use_ste_palette(WORD videomode)
{
    if (vmontype() == MON_MONO)                     /* always for ST mono monitor */
        return TRUE;

    if ((videomode&VIDEL_BPPMASK) == VIDEL_2BPP)    /* always for 4-colour modes */
        return TRUE;

    if ((videomode&VIDEL_COMPAT) && ((videomode&VIDEL_BPPMASK) == VIDEL_4BPP))
        return TRUE;                                /* and for ST low */

    return FALSE;
}

/*
 * set palette registers
 * 
 * note that the actual update of the hardware registers is done by the
 * VBL interrupt handler, according to the setting of 'colorptr'.  since
 * the address in colorptr must be even, we use bit 0 as a flag.
 * 
 * colorptr contents   VBL interrupt handler action
 * -----------------   ----------------------------
 *       0             do nothing
 * address             load STe palette regs from address
 * address | 0x01      load first 16 Falcon palette regs from address
 *       0 | 0x01      load 256 Falcon palette regs from falcon_shadow_palette[]
 */
WORD vsetrgb(WORD index,WORD count,LONG *rgb)
{
    LONG *shadow, *source;
    union {
        LONG l;
        UBYTE b[4];
    } u;
    WORD limit;

    if (!has_videl)
        return 0x5d; /* XBIOS function number as unimplemented error code */

    if ((index < 0) || (count <= 0))
        return -1; /* Generic error */

    limit = (get_videl_bpp()<=4) ? 16 : 256;
    if ((index+count) > limit)
        return -1; /* Generic error */

    /*
     * we always update the Falcon shadow palette, since that's
     * what we'll return for VgetRGB()
     */
    shadow = falcon_shadow_palette + index;
    source = rgb;
    while(count--) {
        u.l = *source++;
        u.b[0] = u.b[1];                 /* shift R & G */
        u.b[1] = u.b[2];
        u.b[2] = 0x00;
        *shadow++ = u.l;
    }

    /*
     * for ST low or 4-colour modes, we need to convert the
     * Falcon shadow registers to STe palette register format, and
     * request the VBL interrupt handler to update the STe palette
     * registers rather than the Falcon registers
     */
    if (use_ste_palette(vsetmode(-1))) {
        convert2ste(ste_shadow_palette,falcon_shadow_palette);
        colorptr = ste_shadow_palette;
        return 0; /* OK */
    }

    colorptr = (limit==256) ? (WORD *)0x01L : (WORD *)((LONG)falcon_shadow_palette|0x01L);

    return 0; /* OK */
}

/*
 * get palette registers
 */
WORD vgetrgb(WORD index,WORD count,LONG *rgb)
{
    LONG *shadow;
    union {
        LONG l;
        UBYTE b[4];
    } u;
    WORD limit;

    if (!has_videl)
        return 0x5e; /* XBIOS function number as unimplemented error code */

    if ((index < 0) || (count <= 0))
        return -1; /* Generic error */

    limit = (get_videl_bpp()<=4) ? 16 : 256;
    if ((index+count) > limit)
        return -1; /* Generic error */

    shadow = falcon_shadow_palette + index;
    while(count--) {
        u.l = *shadow++;
        u.b[2] = u.b[1];        /* shift R & G right*/
        u.b[1] = u.b[0];
        u.b[0] = 0x00;
        *rgb++ = u.l;
    }

    return 0; /* OK */
}

/*
 * Fix Videl mode
 *
 * This converts an (assumed legal) input mode into the
 * corresponding output mode for the current monitor type
 */
WORD vfixmode(WORD mode)
{
WORD monitor, currentmode;

    if (!has_videl)
        return -32;

    monitor = vmontype();
    if (monitor == MON_MONO)
        return FALCON_ST_HIGH;

    currentmode = vsetmode(-1);
    if (currentmode & VIDEL_PAL)    /* set PAL bit per current value */
        mode |= VIDEL_PAL;
    else mode &= ~VIDEL_PAL;

    /* handle VGA monitor */
    if (monitor == MON_VGA) {
        mode &= ~VIDEL_OVERSCAN;    /* turn off overscan (not used with VGA) */
        if (!(mode & VIDEL_VGA))            /* if mode doesn't have VGA set, */
            mode ^= (VIDEL_VERTICAL | VIDEL_VGA);   /* set it & flip vertical */
        if (mode & VIDEL_COMPAT) {
            if ((mode&VIDEL_BPPMASK) == VIDEL_1BPP)
                mode &= ~VIDEL_VERTICAL;    /* clear vertical for ST high */
            else mode |= VIDEL_VERTICAL;    /* set it for ST medium, low  */
        }
        return mode;
    }

    /* handle RGB or TV */
    if (mode & VIDEL_VGA)                       /* if mode has VGA set, */
        mode ^= (VIDEL_VERTICAL | VIDEL_VGA);   /* clear it & flip vertical */
    if (mode & VIDEL_COMPAT) {
        if ((mode&VIDEL_BPPMASK) == VIDEL_1BPP)
            mode |= VIDEL_VERTICAL;         /* set vertical for ST high */
        else mode &= ~VIDEL_VERTICAL;       /* clear it for ST medium, low  */
    }

    return mode;
}

WORD videl_check_moderez(WORD moderez)
{
    WORD current_mode, return_mode;

    if (moderez < 0)                /* ignore rez values */
        return 0;

    current_mode = get_videl_mode();
    return_mode = vfixmode(moderez);/* adjust */
    return (return_mode==current_mode)?0:return_mode;
}

ULONG videl_vram_size(void)
{
    return get_videl_width() / 8L * get_videl_height() * get_videl_bpp();
}

void videl_get_current_mode_info(UWORD *planes, UWORD *hz_rez, UWORD *vt_rez)
{
    *planes = get_videl_bpp();
    *hz_rez = get_videl_width();
    *vt_rez = get_videl_height();
}

/*
 * Initialise Falcon palette
 */
void initialise_falcon_palette(WORD mode)
{
    volatile WORD *col_regs = (WORD *) ST_PALETTE_REGS;
    volatile LONG *fcol_regs = (LONG *) FALCON_PALETTE_REGS;
    int i, limit;

    /* first, set up Falcon shadow palette and real registers */
    for (i = 0; i < 256; i++)
        falcon_shadow_palette[i] = videl_dflt_palette[i];

    switch(mode&VIDEL_BPPMASK) {
    case VIDEL_1BPP:        /* 2-colour mode */
        falcon_shadow_palette[1] = falcon_shadow_palette[15];
        break;
    case VIDEL_2BPP:        /* 4-colour mode */
        falcon_shadow_palette[3] = falcon_shadow_palette[15];
        break;
    }

    /* a 'feature' of the Falcon hardware: if we're in a mode with less
     * than 256 colours, and we attempt to set the Falcon hardware
     * palette registers for colours 16 & above, it will screw up the
     * values in the first 16 hardware palette registers, resulting in
     * a messed-up display ...
     * NOTE: what happens in the Truecolor case is yet to be determined,
     * although it is probably not important since we don't use those
     * registers.
     */
    limit = ((mode&VIDEL_BPPMASK)==VIDEL_8BPP) ? 256 : 16;
    for (i = 0; i < limit; i++)
        fcol_regs[i] = falcon_shadow_palette[i];

    /*
     * if appropriate, set up the STe shadow & real palette registers
     */
    if (use_ste_palette(mode)) {
        convert2ste(ste_shadow_palette,falcon_shadow_palette);
        for (i = 0; i < 16; i++)
            col_regs[i] = ste_shadow_palette[i];
    }
}

/*
 * Get videl mode
 * This is the same as vsetmode(-1) except that it returns
 * zero when there is no videl.  Used by app_save().
 */
WORD get_videl_mode(void)
{
    if (has_videl)
        return vsetmode(-1);

    return 0;
}

#endif /* CONF_WITH_VIDEL */
