/*
 * esclisa.c - GSX escapes for the VDI screen driver
 *
 * Copyright (c) 2002 The EmuTOS development team
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include "config.h"
#include "portab.h"
#include "vdi_defs.h"
#include "asm.h"



/* Local Constants */

#define ldri_escape             19      // last DRI escape = 19.

/* GEMDOS Function Codes */

#define rawio                   0x06    // raw i/o to standard input/output
#define wntstr                  0x09    // write null terminated string to std output



/*
 * escfn0 - Don't know, what it does...
 */

static void escfn0(Vwk * vwk)
{
}



/*
 * escfn1 - returns the maximum addressable character cell row and column.
 *
 * outputs:
 *   CONTRL[4] = 2 (# of integers returned)
 *   INTOUT[0] = maximum character cell row
 *   INTOUT[1] = maximum character cell column
 */

static void escfn1(Vwk * vwk)
{
    CONTRL[4] = 2;              // 2 integers are returned
    INTOUT[0] = 25;             // write max row number to array
    INTOUT[1] = 80;             // write max column number to array
}



/*
 * escfn2 - exit the alpha mode and enter the graphics mode.
 */

static void escfn2(Vwk * vwk)
{
    trap1(wntstr, "\033f\033E");   // hide alpha cursor
    _v_clrwk(vwk);
}



/*
 * escfn3 - enter the alpha mode and exit the graphics mode.
 */
static void escfn3(Vwk * vwk)
{
    _v_clrwk(vwk);
    trap1(wntstr, "\033E\033e");   // show alpha cursor
}



/*
 * escfn4 - moves the alpha cursor up one line.  
 */

static void escfn4(Vwk * vwk)
{
    trap1(wntstr, "\033A");   
}



/*
 * escfn5 - moves the alpha cursor down one line.
 */

static void escfn5(Vwk * vwk)
{
    trap1(wntstr, "\033B");
}



/*
 * escfn6 - moves the alpha cursor right one column.
 */

static void escfn6(Vwk * vwk)
{
    trap1(wntstr, "\033C");
}



/*
 * escfn7 - moves the alpha cursor left one column.
 */

static void escfn7(Vwk * vwk)
{
    trap1(wntstr, "\033D");
}



/*
 * escfn8 - moves the alpha cursor home.
 */

static void escfn8(Vwk * vwk)
{
    trap1(wntstr, "\033H");
}



/*
 * escfn9 - clears screen from cursor position to the end of the screen.
 */

static void escfn9(Vwk * vwk)
{
    trap1(wntstr, "\033J");
}



/*
 * escfn10 - clears screen from cursor position to the end of the line.
 */

static void escfn10(Vwk * vwk)
{
    trap1(wntstr, "\033K");      // cursor down
}



/*
 * escfn11 - sets the cursor position
 *
 * The cursor will be displayed at the new location,
 * if it is not currently hidden.
 *
 * inputs:
 *   INTIN[0] = cursor row (0 - max_y_cell)
 *   INTIN[1] = cursor column (0 - max_x_cell)
 */

static void escfn11(Vwk * vwk)
{
    char out[5];

    out[0] = '\033';
    out[1] = 'Y';
    out[2] = 0x20 + INTIN[0];   /* get row number */
    out[3] = 0x20 + INTIN[1];  /* get col number */
    out[4] = '\0';
    trap1(wntstr, out);
}



/*
 * escfn12 - outputs cursor addressable alpha text.
 *
 * The cursor will be displayed at the new location,
 * if it is not currently hidden.
 *
 * inputs:
 *   CONTRL[3] = character count
 *   INTIN = character array
 */

static void escfn12(Vwk * vwk)
{
    int cnt;
    char *chr;

    cnt = CONTRL[3];            /* get the character count */
    chr = (char*)&INTIN[0];     /* address of the character array */

    while (cnt--) {
        trap1(rawio, chr++);    /*  raw i/o to standard input/output */
    }
}



/*
 * escfn13 - switch to reverse video
 */

static void escfn13(Vwk * vwk)
{
    trap1(wntstr, "\033p");      /* enter reverse video */
}



/*
 * escfn14 - switch to normal video
 */

static void escfn14(Vwk * vwk)
{
    trap1(wntstr, "\033q");      /* enter reverse video */
}



/*
 * escfn15 - returns the current x and y-coordinates of the alpha cursor.
 *
 * This function is currently just a stub because this information
 * is not readilly available.
 */

static void escfn15(Vwk * vwk)
{
    CONTRL[4] = 2;              // 2 integers are returned
    INTOUT[0] = 0;              // dummy
    INTOUT[1] = 0;              // dummy
}



/*
 * escfn16 - returns the availability of a mouse or similar
 *
 * outputs:
 *   CONTRL[4] = 1  (# of parameters returned)
 *   INTOUT[0] = 1  (mouse is available)
 */

static void escfn16(Vwk * vwk)
{
    CONTRL[4] = 1;              // 2 integers are returned
    INTOUT[0] = 1;              // there is a mouse
}



/*
 * escfn17 - obtain hard copy of the contents of the screen.
 *
 * This function is currently just a stub.
 */

static void escfn17(Vwk * vwk)
{
}



/*
 * escfn18 - displays the graphics cursor at its current address.
 */

static void escfn18(Vwk * vwk)
{
    INTIN[0] = 0;       /* show regardless */
    _v_show_c(vwk);         /* display the graphics cursor */
}



/*
 * escfn19 - hides the graphics cursor.
 */

static void escfn19(Vwk * vwk)
{
    _v_hide_c(vwk);         /* hide the graphics cursor */
}



static void (* const esctbl[])(Vwk *) =
{
    escfn0,
    escfn1,
    escfn2,
    escfn3,
    escfn4,
    escfn5,
    escfn6,
    escfn7,
    escfn8,
    escfn9,
    escfn10,
    escfn11,
    escfn12,
    escfn13,
    escfn14,
    escfn15,
    escfn16,
    escfn17,
    escfn18,
    escfn19
};



/*
 * chk_esc -  This routine decodes the escape functions.
 *
 * The following inputs and outputs may be used by a sub-function:
 *
 * input:
 *   CONTRL[5] = escape function ID.
 *   CONTRL[6] = device handle.
 *   INTIN[]   = array of input parameters.
 *
 * output:
 *   CONTRL[2] = number of output vertices.
 *   CONTRL[4] = number of output parameters.
 *   INTOUT[]  = array of output parameters.
 *   PTSOUT[]  = array of output vertices.
 */

void chk_esc(Vwk * vwk)
{
    UWORD escfun = CONTRL[5];

#if HAVE_BEZIER
    if (escfun == 99) {
        v_bez_qual(vwk);        /* set quality of bezier curves */
        return;
    }
#endif
    if (escfun > ldri_escape)
        return;
    (*esctbl[escfun])(vwk);
}



void esc_init(Vwk * vwk)
{
    escfn2(vwk);
}



void esc_exit(Vwk * vwk)
{
    escfn3(vwk);
}
