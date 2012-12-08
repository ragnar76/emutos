/*
 * keyb_gr.h - Greek keyboard layout definition
 *
 * Copyright (c) 2002 EmuTOS development team
 *
 * Authors:
 *  GGN   ggn@atari.org
 *
 * This file may not be distributed at all, or I'll have yer head off!
 * 
 * Only kidding! Actually the keymaps took a bit of time to rip!
 * Yes, they were ripped of an Accessoy written by D. Gizis and
 * M.Statharas of ELKAT, Greece (which was Atari Greece if you wonder).
 *
 * Note: this file was not automatically generated, although I took this
 * header from an automatically generated header, just for the looks!
 * 
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

static const BYTE keytbl_gr_norm[];
static const BYTE keytbl_gr_shft[];
static const BYTE keytbl_gr_caps[];
static const BYTE keytbl_gr_altnorm[];
static const BYTE keytbl_gr_altshft[];
static const BYTE keytbl_gr_altcaps[];

static const struct keytbl keytbl_gr = {
    keytbl_gr_norm, 
    keytbl_gr_shft, 
    keytbl_gr_caps, 
    keytbl_gr_altnorm, 
    keytbl_gr_altshft, 
    keytbl_gr_altcaps, 
    NULL
}; 

static const BYTE keytbl_gr_altnorm[] = {
        0, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
     0x37, 0x38, 0x39, 0x30, 0x2D, 0x3D, 0x08, 0x09,
     0xF1, 0xAA, 0x9C, 0xA8, 0xAB, 0xAC, 0x9F, 0xA0,
     0xA6, 0xA7, 0x5B, 0x5D, 0x0D,    0, 0x98, 0xA9,
     0x9B, 0xAD, 0x9A, 0x9E, 0xA5, 0xA1, 0xA2, 0x3B,
     0x27, 0x60,    0, 0x23, 0x9D, 0xAE, 0xAF, 0xE0,
     0x99, 0xA4, 0xA3, 0x2C, 0x2E, 0x2F,    0,    0,
        0, 0x20,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0, 0x2D,    0,    0,    0, 0x2B,    0,
        0,    0,    0, 0x7F,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
     0x5C,    0,    0, 0x28, 0x29, 0x2F, 0x2A, 0x37,
     0x38, 0x39, 0x34, 0x35, 0x36, 0x31, 0x32, 0x33,
     0x30, 0x2E, 0x0D,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
};

static const BYTE keytbl_gr_altshft[] = {
        0, 0x1B, 0x21, 0x22, 0xF9, 0x24, 0x25, 0x5E,
     0x26, 0x2A, 0x28, 0x29, 0x5F, 0x2B, 0x08, 0x09,
     0xF2, 0xAA, 0x84, 0x90, 0x92, 0x93, 0x87, 0x88,
     0x8E, 0x8F, 0x7B, 0x7D, 0x0D,    0, 0x80, 0x91,
     0x83, 0x94, 0x82, 0x86, 0x8D, 0x89, 0x8A, 0x3A,
     0x40, 0xFF,    0, 0x7E, 0x85, 0x95, 0x96, 0x97,
     0x81, 0x8C, 0x8B, 0x3C, 0x3E, 0x3F,    0,    0,
        0, 0x20,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0, 0x2D,    0,    0,    0, 0x2B,    0,
        0,    0,    0, 0x7F,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
     0x7C,    0,    0, 0x28, 0x29, 0x2F, 0x2A, 0x37,
     0x38, 0x39, 0x34, 0x35, 0x36, 0x31, 0x32, 0x33,
     0x30, 0x2E, 0x0D,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
};

static const BYTE keytbl_gr_altcaps[] = {
        0, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
     0x37, 0x38, 0x39, 0x30, 0x2D, 0x3D, 0x08, 0x09,
     0xF1, 0xFB, 0x84, 0x90, 0x92, 0x93, 0x87, 0x88,
     0x8E, 0x8F, 0x5B, 0x5D, 0x0D,    0, 0x80, 0x91,
     0x83, 0x94, 0x82, 0x86, 0x8D, 0x89, 0x8A, 0x3B,
     0x27, 0x60,    0, 0x23, 0x85, 0x95, 0x96, 0x97,
     0x81, 0x8C, 0x8B, 0x2C, 0x2E, 0x2F,    0,    0,
        0, 0x20,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0, 0x2D,    0,    0,    0, 0x2B,    0,
        0,    0,    0, 0x7F,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
     0x5C,    0,    0, 0x28, 0x29, 0x2F, 0x2A, 0x37,
     0x38, 0x39, 0x34, 0x35, 0x36, 0x31, 0x32, 0x33,
     0x30, 0x2E, 0x0D,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
};

static const BYTE keytbl_gr_norm[] = {
        0, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
     0x37, 0x38, 0x39, 0x30, 0x2D, 0x3D, 0x08, 0x09,
     0xE8, 0xE4, 0xE2, 0xA8, 0xAB, 0xE7, 0x9F, 0xE5,
     0xE6, 0xA7, 0x5B, 0x5D, 0x0D,    0, 0xE1, 0xA9,
     0x9B, 0xAD, 0x9A, 0xE3, 0xA5, 0xA1, 0xA2, 0x3B,
     0x27, 0x60,    0, 0x23, 0x9D, 0xAE, 0xAF, 0xE9,
     0x99, 0xA4, 0xA3, 0x2C, 0x2E, 0x2F,    0,    0,
        0, 0x20,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0, 0x2D,    0,    0,    0, 0x2B,    0,
        0,    0,    0, 0x7F,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
     0x5C,    0,    0, 0x28, 0x29, 0x2F, 0x2A, 0x37,
     0x38, 0x39, 0x34, 0x35, 0x36, 0x31, 0x32, 0x33,
     0x30, 0x2E, 0x0D,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
};

static const BYTE keytbl_gr_shft[] = {
        0, 0x1B, 0x21, 0x22, 0xF9, 0x24, 0x25, 0x5E,
     0x26, 0x2A, 0x28, 0x29, 0x5F, 0x2B, 0x08, 0x09,
     0xF2, 0xAA, 0xEB, 0x90, 0x92, 0xEF, 0x87, 0xED,
     0xEE, 0x8F, 0x7B, 0x7D, 0x0D,    0, 0xEA, 0x91,
     0x83, 0x94, 0x82, 0xEC, 0x8D, 0x89, 0x8A, 0x3A,
     0x40, 0xFF,    0, 0x7E, 0x85, 0x95, 0x96, 0xF0,
     0x81, 0x8C, 0x8B, 0x3C, 0x3E, 0x3F,    0,    0,
        0, 0x20,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0, 0x2D,    0,    0,    0, 0x2B,    0,
        0,    0,    0, 0x7F,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
     0x7C,    0,    0, 0x28, 0x29, 0x2F, 0x2A, 0x37,
     0x38, 0x39, 0x34, 0x35, 0x36, 0x31, 0x32, 0x33,
     0x30, 0x2E, 0x0D,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
};

static const BYTE keytbl_gr_caps[] = {
        0, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
     0x37, 0x38, 0x39, 0x30, 0x2D, 0x3D, 0x08, 0x09,
     0xF1, 0xFB, 0xEB, 0x90, 0x92, 0xEF, 0x87, 0xED,
     0xEE, 0x8F, 0x5B, 0x5D, 0x0D,    0, 0xEA, 0x91,
     0x83, 0x94, 0x82, 0xEC, 0x8D, 0x89, 0x8A, 0x3B,
     0x27, 0x60,    0, 0x23, 0x85, 0x95, 0x96, 0xF0,
     0x81, 0x8C, 0x8B, 0x2C, 0x2E, 0x2F,    0,    0,
        0, 0x20,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
        0,    0, 0x2D,    0,    0,    0, 0x2B,    0,
        0,    0,    0, 0x7F,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
     0x5C,    0,    0, 0x28, 0x29, 0x2F, 0x2A, 0x37,
     0x38, 0x39, 0x34, 0x35, 0x36, 0x31, 0x32, 0x33,
     0x30, 0x2E, 0x0D,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,
};

