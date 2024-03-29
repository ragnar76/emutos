/*
 * console.c - GEMDOS console system
 *
 * Copyright (c) 2001 Lineo, Inc.
 *
 * Authors:
 *  JSL   Jason S. Loveman
 *  SCC   Steve C. Cavender
 *  EWF   Eric W. Fleischman
 *  KTB   Karl T. Braun (kral)
 *  MAD   Martin Doering
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */



#include        "config.h"
#include        "portab.h"                      /*  M01.01.02           */
#include        "fs.h"
#include        "proc.h"
#include        "console.h"
#include        "biosbind.h"

/* The following data structures are used for the typeahead buffer */
static long glbkbchar[3][KBBUFSZ]; /* The actual typeahead buffer */
int add[3] ;                    /*  index of add position, used from bdosmain.c */
int remove[3] ;                 /*  index of remove position, used from bdosmain.c */
static int glbcolumn[3];

/* 
 * forward declarations (internal prototypes) 
 */

static void buflush(int h);
static long constat(int h);
static void conbrk(int h);
static void buflush(int h);
static void conout(int h, int ch);
static void cookdout(int h, int ch);
static long getch(int h);
static void prt_line(int h, char *p);
static void newline(int h, int startcol);
static int backsp(int h, char *cbuf, int retlen, int col);

#define UBWORD(x) (((int) x) & 0x00ff)

#define   ctrlc  0x03
#define   ctrle  0x05
#define   ctrlq  0x11
#define   ctrlr  0x12
#define   ctrls  0x13
#define   ctrlu  0x15
#define   ctrlx  0x18

#define   cr      0x0d
#define   lf      0x0a
#define   tab     0x09
#define   rub     0x7f
#define   bs      0x08
#define   space   0x20

#define warmboot() xterm(-32)



/*
 * constat - console status
 *
 * @h - device handle
 */
static long constat(int h)
{
    if (h > 2)
        return(0);

    return( add[h] > remove[h] ? -1L : Bconstat(h) );
}

/*
 * xconstat - Function 0x0B - Console input status
 */
long xconstat(void)
{
    return(constat(HXFORM(run->p_uft[0])));
}

/*
 * xconostat - Function 0x10 - console output status
 */
long xconostat(void)
{
    return(Bcostat(HXFORM(run->p_uft[1])));
}

/*
 * xprtostat - Function 0x11 - Printer output status
 */
long xprtostat(void)
{
    return(Bcostat(HXFORM(run->p_uft[4])));
}

/*
 * xauxistat - Function 0x12 - Auxillary input status
 */
long xauxistat(void)
{
    return(constat(HXFORM(run->p_uft[3])));
}

/*
 * xauxostat - Function 0x13 - Auxillary output status
 */
long xauxostat(void)
{
    return(Bcostat(HXFORM(run->p_uft[3])));
}

/*
 * conbrk - check for ctrl/s, used internally
 *
 * @h - device handle
 */
static void conbrk(int h)
{
    register long ch;
    register int stop, c;

    stop = 0;
    if ( Bconstat(h) ) {
        do {
            c = (ch = Bconin(h)) & 0xFF;
            if ( c == ctrlc ) {
                buflush(h);     /* flush BDOS & BIOS buffers */
                warmboot();
            }

            if ( c == ctrls )
                stop = 1;
            else if ( c == ctrlq )
                stop = 0;
            else if ( c == ctrlx ) {
                buflush(h);
                glbkbchar[h][add[h]++ & KBBUFMASK] = ch;
            }
            else {
                if ( add[h] < remove[h] + KBBUFSZ ) {
                    glbkbchar[h][add[h]++ & KBBUFMASK] = ch;
                }
                else {
                    Bconout(h, 7);
                }
            }
        } while (stop);
    }
}



/*
 * buflush - flush BDOS type-ahead buffer
 *
 * @h - device handle
 */
static void buflush(int h)
{
    add[h] = remove[h] = 0;
}



/*
 * conout - console output - used internally
 *
 * @h - device handle
 */

static void conout(int h, int ch)
{
    conbrk(h);                  /* check for control-s break */
    Bconout(h,ch);              /* output character to console */
    if (ch >= ' ')
        glbcolumn[h]++;         /* keep track of screen column */
    else if (ch == cr)
        glbcolumn[h] = 0;
    else if (ch == bs)
        glbcolumn[h]--;
}



/*
 * xtabout - Function 0x02 - console output with tab expansion
 */
long xtabout(int ch)
{
    tabout(HXFORM(run->p_uft[1]),ch);
    return 1;
}



/*
 * tabout -
 *
 * @h - device handle
 * @ch - character to output to console
 */
void tabout(int h, int ch)
{
    if (ch == tab)
        do {
            conout(h,' ');
        } while (glbcolumn[h] & 7);
    else
        conout(h,ch);
}



/*
 * cookdout - console output with tab and control character expansion
 *
 * @h - device handle
 * @ch - character to output to console
 */
static void cookdout(int h, int ch)
{
    if (ch == tab) tabout(h,ch); /* if tab, expand it   */
    else {
        if ( ch < ' ' ) {
            conout( h,'^' );
            ch |= 0x40;
        }
        conout(h,ch);                       /* output the character */
    }
}

/*
 * xauxout - Function 0x04 - auxillary output
 */
long xauxout(int ch)
{
    return Bconout(HXFORM(run->p_uft[3]), ch);
}

/*
 * xprtout - Function 0x05 - printer output
 */
long xprtout(int ch)
{
#if 0 
    /* TODO - depending whether Bconout() returns a value or not,
     * use Bcostat() or not. 
     * Some doc (Le livre du développeur - the developper's book) says:
     *   void Bconout(); void Cauxout(); int Cprnout();
     * the BDOS code assumes that Bconout() returns a value;
     * our current BIOS code doesn't.
     */
    int h = HXFORM(run->p_uft[4]);
    if (Bcostat(h) == -1L) {
        Bconout(h, ch);
        return -1L;
    } else {
        return 0L;
    }
#else
    return Bconout(HXFORM(run->p_uft[4]), ch);
#endif
}



/*
 * getch - get character from device h
 *
 * @h - device handle
 */
static long getch(int h)
{
    long temp;

    if ( add[h] > remove[h] ) {
        temp = glbkbchar[h][remove[h]++ & KBBUFMASK];
        if ( add[h] == remove[h] ) {
            buflush(h);
        }
        return(temp);
    }

    return(Bconin(h));
}



/*
 * x7in - Function 0x07 - Direct console input without echo
 */

long x7in(void)
{
    return(getch(HXFORM(run->p_uft[0])));
}

/*
 * conin - BDOS console input function
 */
long conin(int h)              
{
    long ch;

    conout( h,(unsigned char)(ch = getch(h)) );
    return(ch);
}

/*
 * xconin - Function 0x01 - console input
 */
long xconin(void)
{
    int h;

    h = HXFORM( run->p_uft[0] );
    conbrk( h );
    return( conin( h ) );
}

/*
 * x8in - Function 0x08 - Console input without echo
 */
long x8in(void)
{
    register int h;
    register long ch;

    h = HXFORM(run->p_uft[0]);
    conbrk(h);
    ch = getch(h);
    if ((ch & 0xFF) == ctrlc) {
        warmboot();
    } else {
        return(ch);
    }
}

/*
 * xauxin - Function 0x03 - Auxillary input
 */
long xauxin(void)
{
    return(Bconin(HXFORM(run->p_uft[3])));
}

/*
 * rawconio - Function 0x06 - Raw console I/O
 */
long rawconio(int parm)
{
    int i;

    if (parm == 0xFF) {
        i = HXFORM(run->p_uft[0]);
        return(constat(i) ? getch(i) : 0L);
    }
    Bconout(HXFORM(run->p_uft[1]), parm);
    return 0; /* dummy */
}

/*
 * xprt_line - Function 0x09 - Print line up to nul with tab expansion
 */
void xprt_line(char *p)
{
    prt_line(HXFORM(run->p_uft[1]),p);
}



/*
 * prt_line - print line to stdout
 */
static void prt_line(int h, char *p)
{
    while( *p ) tabout( h, (unsigned char)*p++ );
}



/*
 * read line with editing and bounds checking
 */

/* Two subroutines first */
static void newline(int h, int startcol)
{
    conout(h,cr);                       /* go to new line */
    conout(h,lf);
    while(startcol) {
        conout(h,' ');
        startcol -= 1;          /* start output at starting column */
    }
}

/* backspace one character position */
/* col is the starting console column */
static int backsp(int h, char *cbuf, int retlen, int col) 
{
    register char       ch;             /* current character            */
    register int        i;
    register char       *p;             /* character pointer            */

    if (retlen) --retlen;
                                /* if buffer non-empty, decrease it by 1 */
    i = retlen;
    p = cbuf;
    while (i--)  {              /* calculate column position across entire char buffer */
        ch = *p++;              /* get next char                */
        if ( ch == tab ) {
            col += 8;
            col &= ~7;          /* for tab, go to multiple of 8 */
        }
        else if ( (unsigned char)ch < ' ' )
            col += 2;           /* control chars put out 2 printable chars */
        else
            col += 1;
    }
    while (glbcolumn[h] > col) {
        conout(h,bs);           /* backspace until we get to proper column */
        conout(h,' ');
        conout(h,bs);
    }
    return(retlen);
}

/*
 * readline - Function 0x0A - Read console string into buffer
 *
 * p - max length, return length, buffer space
 */
void readline(char *p)
{
    p[1] = cgets(HXFORM(run->p_uft[0]),(unsigned char)p[0],&p[2]);
}

/* h is special handle denoting device number */
int cgets(int h, int maxlen, char *buf)
{
    char ch;
    int i,stcol,retlen;

    stcol = glbcolumn[h];           /* set up starting column */
    for (retlen = 0; retlen < maxlen; ) {
        switch(ch = getch(h)) {
        case cr:
        case lf:
            conout(h,cr);
            return(retlen);
        case bs:
        case rub:
            retlen = backsp(h,buf,retlen,stcol);
            break;
        case ctrlc: warmboot();
        case ctrlx:
            do retlen = backsp(h,buf,retlen,stcol);
            while (retlen);
            break;
        case ctrlu:
            conout(h,'#');
            newline(h,stcol);
            retlen = 0;
            break;
        case ctrlr:
            conout(h,'#');
            newline(h,stcol);
            for (i=0; i < retlen; i++)
                cookdout(h,(unsigned char)buf[i]);
            break;
        default:
            cookdout(h,(unsigned char)(buf[retlen++] = ch));
        }
    }
    return(retlen);
}
