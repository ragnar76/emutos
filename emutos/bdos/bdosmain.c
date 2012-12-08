/*
 * bdosmain.c - GEMDOS main function dispatcher
 *
 * Copyright (c) 2001 Lineo, Inc.
 *               2002 - 2010 The EmuTOS development team
 *
 * Authors:
 *  EWF  Eric W. Fleischman
 *  JSL  Jason S. Loveman
 *  SCC  Steven C. Cavender
 *  LTG  Louis T. Garavaglia
 *  KTB  Karl T. Braun (kral)
 *  ACH  Anthony C. Hay (DR UK)
 *  MAD  Martin Doering
 *  THH  Thomas Huth
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */


#define DBGOSIF 0


#include "config.h"
#include "portab.h"
#include "fs.h"
#include "bpb.h"
#include "mem.h"
#include "proc.h"
#include "console.h"
#include "time.h"
#include "gemerror.h"
#include "biosbind.h"
#include "kprint.h"


/*
**  externals
*/

/* 
 * in rwa.S
 */

extern long xsuper(long);
extern long oscall(int, ...);

extern void enter(void);
extern void bdos_trap2(void);
extern void (*old_trap2)(void);

/*
 *  prototypes / forward declarations
 */

static long ni(void);
static long xgetver(void);



/*
 * FND - Function Descriptor
 *
 * Each entry in the function table (below) consists of the address of
 * the function which corresponds to the function number, and a function
 * type.
 */

#define FND struct _fnd
FND
{
        long    (*fncall)();
        UBYTE   stdio_typ;    /* Standard IO channel (highest bit must be set, too) */
        UBYTE   wparms;       /* Amount of parameters in WORDs */
};



/*
 * funcs - table of os functions, indexed by function number
 *
 * Each entry is for an FND structure. The function 'ni' is used
 * as the address for functions not implemented.
 */

static const FND funcs[0x58] =
{
    
     { (long(*)()) x0term, 0, 0 }, /* 0x00 */

    /*
     * console functions
     *
     * on these functions, the 0x80 flag indicates std file used
     * 0x80 is std in, 0x81 is stdout, 0x82 is stdaux, 0x83 stdprn
     */

    { xconin,   0x80, 0 },   /* 0x01 */
    { xtabout,  0x81, 1 },   /* 0x02 */
    { xauxin,   0x82, 0 },   /* 0x03 */
    { xauxout,  0x82, 1 },   /* 0x04 */
    { xprtout,  0x83, 1 },   /* 0x05 */
    { rawconio, 0, 1 },      /* 0x06 */
    { x7in,     0x80, 0 },   /* 0x07 */
    { x8in,     0x80, 0 },   /* 0x08 */
    { (long(*)()) xprt_line,  0x81, 2 }, /* 0x09 */
    { (long(*)()) readline,   0x80, 2 }, /* 0x0A */
    { xconstat, 0x80, 0 },   /* 0x0B */

    /*
     * disk functions
     *
     * on these functions the 0x80 flag indicates whether a handle
     * is required, the low bits represent the parameter ordering,
     * as usual.
     */

    { ni,       0, 0 },
    { ni,       0, 0 },

    { xsetdrv,  0, 1 },      /* 0x0E */

    { ni,       0, 0 },

    /*
     * extended console functions
     *
     * Here the 0x80 flag indicates std file used, as above
     */

    { xconostat, 0x81, 0 },  /* 0x10 */
    { xprtostat, 0x83, 0 },  /* 0x11 */
    { xauxistat, 0x82, 0 },  /* 0x12 */
    { xauxostat, 0x82, 0 },  /* 0x13 */

#if CONF_WITH_ALT_RAM
    { xmaddalt, 0, 4 },      /* 0x14 */
#else
    { ni,       0, 0 },      /* 0x14 */
#endif
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },

    { xgetdrv,  0, 0 },      /* 0x19 */
    { (long(*)()) xsetdta, 0, 2 }, /* 0x1A */

    { xsuper,   0, 2 },      /* 0x20 - switch to supervisor mode */
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },

    /* xgsps */

    { ni,       0, 0 },      /* 0x20 */
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },

    { xgetdate, 0, 0 },      /* 0x2A */
    { xsetdate, 0, 1 },      /* 0x2B */
    { xgettime, 0, 0 },      /* 0x2C */
    { xsettime, 0, 1 },      /* 0x2D */

    { ni,       0, 0 },

    { (long(*)()) xgetdta, 0, 0 }, /* 0x2F */
    { xgetver,  0, 0 },      /* 0x30 */
    { (long(*)()) xtermres, 0, 3 }, /* 0x31 */

    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },

    { xgetfree, 0, 3 },      /* 0x36 */

    { ni,       0, 0 },
    { ni,       0, 0 },

    { xmkdir,   0, 2 },      /* 0x39 */
    { xrmdir,   0, 2 },      /* 0x3A */
    { xchdir,   0, 2 },      /* 0x3B */
    { (long(*)()) xcreat, 0, 3 },  /* 0x3C */
    { xopen,    0, 3 },      /* 0x3D */
    { xclose,   0, 1 },      /* 0x3E - will handle its own redirection */
    { xread,    0x82, 5 },   /* 0x3F */
    { xwrite,   0x82, 5 },   /* 0x40 */
    { xunlink,  0, 2 },      /* 0x41 */
    { xlseek,   0x81, 4 },   /* 0x42 */
    { (long(*)()) xchmod, 0, 4 },  /* 0x43 */
    { xmxalloc, 0, 3 },      /* 0x44 */
    { dup,      0, 2 },      /* 0x45 */
    { xforce,   0, 2 },      /* 0x46 */
    { xgetdir,  0, 3 },      /* 0x47 */
    { xmalloc,  0, 2 },      /* 0x48 */
    { xmfree,   0, 2 },      /* 0x49 */
    { xsetblk,  0, 5 },      /* 0x4A */
    { (long(*)()) xexec, 0, 7 },   /* 0x4B */
    { (long(*)()) xterm, 0, 1 },   /* 0x4C */

    { ni,       0, 0 },
                
    { xsfirst,  0, 3 },      /* 0x4E */
    { xsnext,   0, 0 },      /* 0x4F */

    { ni,       0, 0 },    /* 0x50 */
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },
    { ni,       0, 0 },

    { xrename,  0, 5 },    /* 0x56 */
    { xgsdtof,  0, 4 }       /* 0x57 */
};




/*
 *  xgetver -
 *      return current version number
 */

static long    xgetver(void)
{
        return(0x2000L);                /*  minor.major */
#if DBGOSIF
        kprintf("BDOS: xgetver - Get version  successful ...\n");
#endif
}


/*
 *  ni -
 */

static long    ni(void)
{
        return(EINVFN);
}



/*
 *  osinit - the bios calls this routine to initialize the os
 */

void    osinit(void)
{
    /* take over the handling of TRAP #1 */
    Setexc(0x21, (long)enter);
    /* intercept TRAP #2 only for xterm(), keeping the old value
     * so that our trap handler can call the old one
     */
    old_trap2 = (void(*)()) Setexc(0x22, (long)bdos_trap2);

    bufl_init();                /* initialize BDOS buffer list */
    osmem_init();
    umem_init();
    time_init();

    run = MGET(PD);
#if DBGOSIF
    kprintf("BDOS: Address of basepage = %08lx\n", (LONG)&run);
#endif

    /* set up system initial standard handles */

    run->p_uft[0] = H_Console;          /* stdin        =       con:    */
    run->p_uft[1] = H_Console;          /* stdout       =       con:    */
    run->p_uft[2] = H_Console;          /* stderr       =       con:    */
    run->p_uft[3] = H_Aux;              /* stdaux       =       aux:    */
    run->p_uft[4] = H_Print;            /* stdprn       =       prn:    */

    add[0] = remove[0] = add[1] = remove[1] = add[2] = remove[2] = 0 ;

#if DBGOSIF
    kprintf("BDOS: cinit - osinit successful ...\n");
#endif
}



/*
 *  ncmps -  compare two text strings, ingoreing case.
 */

static int ncmps(int n, char *s, char *d)
{
    while (n--)
        if (uc(*s++) != uc(*d++))
            return(0);

    return(1);
}



/*
 *  freetree -  free the directory node tree
 */

static void freetree(DND *d)
{
    int i;

    if (d->d_left) freetree(d->d_left);
    if (d->d_right) freetree(d->d_right);
    if (d->d_ofd)
    {
        xmfreblk(d->d_ofd);
    }
    for (i = 0; i < NCURDIR; i++)
    {
        if (dirtbl[i] == d)
        {
            dirtbl[i] = 0;
            diruse[i] = 0 ;     /*  M01.01.06           */
        }
    }
    xmfreblk(d);
}



/*
 *  offree -
 */

static void offree(DMD *d)
{
    int i;
    OFD *f;
    for (i=0; i < OPNFILES; i++)
        if( ((long) (f = sft[i].f_ofd)) > 0L )
            if (f->o_dmd == d)
            {
                xmfreblk(f);
                sft[i].f_ofd = 0;
                sft[i].f_own = 0;
                sft[i].f_use = 0;
            }
}


/*
 *  osif -
 */

#if 0     // was #if    DBGOSIF
/*
 * if in debug mode, use this 'front end' so we can tell if we exit
 * from osif
 */

long    osif(int *pw )
{
    long        osif2() ;
    char        *p ;
    long        r ;

    p = (char *) &pw ;
    osifdmp( p-4 , pw ) ;               /*  pass return addr and pw ptr */

    r = osif2( pw ) ;

    osifret() ;
    return( r ) ;
}
#else
/*
 * if not in debug mode, go directory to 'osif2'.  Do not pass go, do
 * not collect $200, and do not spend time on an extra call
 */
#define osif2   osif

#endif

long    osif2(int *pw)
{
    char **pb,*pb2,*p,ctmp;
    BPB *b;
    BCB *bx;
    DND *dn;
    int typ,h,i,fn;
    int num,max;
    long rc,numl;
    const FND *f;


restrt:
    fn = pw[0];
    if (fn > 0x57)
        return(EINVFN);

#if DBGOSIF
    kprintf("bdos(fn = 0x%04x)\n", fn);
#endif
    
    if ( setjmp(errbuf) )
    {
        rc = errcode;
        /* hard error processing */
#if DBGOSIF
        kprintf("Error code gotten from some longjmp(), back in osif(): %ld\n", rc);
#endif
        /* is this a media change ? */
        if (rc == E_CHNG) {
            /* first, out with the old stuff */
            dn = drvtbl[errdrv]->m_dtl;
            offree(drvtbl[errdrv]);
            xmfreblk(drvtbl[errdrv]);
            drvtbl[errdrv] = 0;

            if (dn)
                freetree(dn);

            for (i = 0; i < 2; i++)
                for (bx = bufl[i]; bx; bx = bx->b_link)
                    if (bx->b_bufdrv == errdrv)
                        bx->b_bufdrv = -1;

            /* then, in with the new */
            b = (BPB *)Getbpb(errdrv);
            if ( (long)b <= 0 ) {
                drvsel &= ~(1L<<errdrv);
                if ( (long)b )
                    return( (long)b );
                return(rc);
            }

            if ( log_media(b,errdrv) )
                return (ENSMEM);

            rwerr = 0;
            errdrv = 0;
            goto restrt;
        }

        /* else handle as hard error on disk for now */

        for (i = 0; i < 2; i++)
            for (bx = bufl[i]; bx; bx = bx->b_link)
                if (bx->b_bufdrv == errdrv)
                    bx->b_bufdrv = -1;
        return(rc);
    }

    f = &funcs[fn];
    typ = f->stdio_typ;

    if (typ && fn && ((fn<12) || ((fn>=16) && (fn<=19)))) /* std funcs */
    {
        h = run->p_uft[typ & 0x7f];
        if ( h > 0)
        { /* do std dev function from a file */
            switch(fn)
            {
            case 6:
                if (pw[1] != 0xFF)
                    goto rawout;
            case 1:
            case 3:
            case 7:
            case 8:
                xread(h,1L,&ctmp);
                return(ctmp);

            case 2:
            case 4:
            case 5:
                /*  M01.01.07  */
                /*  write the char in the int at pw[1]  */
            rawout:
                xwrite( h , 1L , ((char*) &pw[1])+1 ) ;
                return 0; /* dummy */

            case 9:
                pb2 = *((char **) &pw[1]);
                while (*pb2) xwrite(h,1L,pb2++);
                return 0; /* dummy */

            case 10:
                pb2 = *((char **) &pw[1]);
                max = *pb2++;
                p = pb2 + 1;
                for (i = 0; max--; i++,p++)
                {
                    if (xread(h,1L,p) == 1)
                    {
                        oscall(0x40,1,1L,p);
                        if (*p == 0x0d)
                        {       /* eat the lf */
                            xread(h,1L,&ctmp);
                            break;
                        }
                    }
                    else
                        break;
                }
                *pb2 = i;
                return(0);

            case 11:
            case 16:
            case 17:
            case 18:
            case 19:
                return(0xFF);
            }
        }

        typ = 0;
    }

    if (typ & 0x80)
    {
        if (typ == 0x81)
            h = pw[3];
        else
            h = pw[1];

        if (h >= NUMSTD)
            numl = (long) sft[h-NUMSTD].f_ofd;
        else if (h >= 0)
        {
            h = run->p_uft[h];
            if ( h > 0)
                numl = (long) sft[h-NUMSTD].f_ofd;
            else
                numl = h;
        }
        else
            numl = h;

        if (!numl)
            return(EIHNDL); /* invalid handle: media change, etc */

        if (numl < 0)
        {       /* prn, aux, con */
                /* -3   -2   -1  */

            num = numl;

            /*  check for valid handle  */ /* M01.01.0528.01 */
            if( num < -3 )
                return( EIHNDL ) ;

            pb = (char **) &pw[4];

            /* only do things on read and write */

            if (fn == 0x3f) /* read */
            {
                if (pw[2])      /* disallow HUGE reads      */
                    return(0);

                if (pw[3] == 1)
                {
                    **pb = conin(HXFORM(num));
                    return(1);
                }

                return(cgets(HXFORM(num),pw[3],*pb));
            }

            if (fn == 0x40) /* write */
            {
                if (pw[2])      /* disallow HUGE writes     */
                    return(0);

                pb2 = *pb;      /* char * is buffer address */


                for (i = 0; i < pw[3]; i++)
                {
                    if( num == H_Console )
                        tabout( HXFORM(num) , (unsigned char)*pb2++ ) ;
                    else
                    {           /* M01.01.1029.01 */
                        if (Bconout( HXFORM(num), (unsigned char)*pb2++ ) == 0)
                            return(i);
                    }
                }

                return(pw[3]);
            }

            return(0);
        }
    }

    rc = 0;
    if ((fn == 0x3d) || (fn == 0x3c))  /* open, create */
    {
        p = *((char **) &pw[1]);
        if (ncmps(5,p,"CON:"))
            rc = 0xFFFFL;
        else if (ncmps(5,p,"AUX:"))
            rc = 0xFFFEL;
        else if (ncmps(5,p,"PRN:"))
            rc = 0xFFFDL;
    }

    if (!rc)
    {
        switch (f->wparms)
        {
        case 0:
            rc = (*f->fncall)();
            break;

        case 1:
            rc = (*f->fncall)(pw[1]);
            break;

        case 2:
            rc = (*f->fncall)(pw[1],pw[2]);
            break;

        case 3:
            rc = (*f->fncall)(pw[1],pw[2],pw[3]);
            break;

        case 4:
            rc = (*f->fncall)(pw[1],pw[2],pw[3],pw[4]);
            break;

        case 5:
            rc = (*f->fncall)(pw[1],pw[2],pw[3],pw[4],pw[5]);
            break;

        case 7:
            rc = (*f->fncall)(pw[1],pw[2],pw[3],pw[4],pw[5],pw[6],pw[7]);
            break;

        default:
            rc = EINTRN;    /* Internal error */
        }
    }

#if DBGOSIF
    kprintf("bdos returns: 0x%08lx\n", rc);
#endif

    return(rc);
}
