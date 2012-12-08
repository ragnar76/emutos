/*
 * fsio.c - read/write routines for the file system                     
 *
 * Copyright (c) 2001 Lineo, Inc.
 *               2002 - 2010 The EmuTOS development team
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#include "config.h"
#include "portab.h"
#include "fs.h"
#include "gemerror.h"
#include "biosbind.h"
#include "kprint.h"

#define DBGFSIO 0

/*
 * forward prototypes
 */

static void xfr2usr(int n, char *s, char *d);
static void usr2xfr(int n, char *d, char *s);
static long divmod(unsigned int *modp, long divdnd, int divsor);
static void addit(OFD *p, long siz, int flg);
static long xrw(int wrtflg, OFD *p, long len, char *ubufr,
                void (*bufxfr)(int, char *, char *));
static void usrio(int rwflg, int num, long strt, char *ubuf, DMD *dm);


/*
 *  xfr2usr -
 */

static void xfr2usr(int n, char *s, char *d)
{
    while (n--)
        *d++ = *s++;
}



/*
 *  usr2xfr -
 */

static void usr2xfr(int n, char *d, char *s)
{
    while (n--)
        *d++ = *s++;
}



/*
 * divmod - do divide and modulo arithmetic
 *
 * the divide is accomplished with the log2 shift factor passed in as
 * as psuedo divisor, the remainder (modulo) is left in the varable
 * pointed to by the third argument.
 */

/* divsor is log2 of actual divisor */
static long divmod(unsigned int *modp, long divdnd, int divsor)
{
    *modp = (unsigned int)(divdnd % (1L<<divsor));

    return (long)(divdnd >> divsor);
}



/*
 * xlseek - seek to byte position n on file with handle h
 *
 * Function 0x42        f_seek
 *
 * Error returns
 *   EIHNDL
 *
 *   EINVFN
 *      ixlseek()
 */

long    xlseek(long n, int h, int flg)
{
    OFD *f;

    f = getofd(h);
    if ( !f )
        return(EIHNDL);

    if (flg == 2)
        n += f->o_fileln;
    else if (flg == 1)
        n += f->o_bytnum;
    else if (flg)
        return(EINVFN);

    return(ixlseek(f,n));
}

/*
 * ixlseek - file position seek
 *
 * Error returns
 *   ERANGE
 *   EINTRN
 *
 * NOTE: This function returns ERANGE and EINTRN errors, which are new
 *       error numbers I just made up (that is, they were not defined
 *       by the BIOS or by PC DOS).
 *
 * p: file descriptor for file in use
 * n: number of bytes to seek
 */

long    ixlseek(register OFD *p, long n)
{
    CLNO clnum,clx,curnum,i;    /*  M01.01.03   */
    int curflg ;                /****  M00.01.01b  ****/
    register DMD *dm;

    if (n > p->o_fileln)
        return(ERANGE);

    if (n < 0)
        return(ERANGE);

    dm = p->o_dmd;
    if (!n)
    {
        clx = 0;
        p->o_curbyt = 0;
        goto fillin;
    }

    /* do we need to start from the beginning ? */

    /***  M00.01.01b ***/
    if( ((!p->o_curbyt) || (p->o_curbyt == dm->m_clsizb)) && p->o_bytnum )
        curflg = 1 ;
    else
        curflg = 0 ;
    /***  end  ***/

    clnum = divmod(&p->o_curbyt,n,dm->m_clblog);

    if (p->o_curcl && (n >= p->o_bytnum))
    {
        curnum = p->o_bytnum >> dm->m_clblog;
        clnum -= curnum;
        clnum += curflg ;               /***   M00.01.01b       ***/

        /*****
         M00.01.01 - original code (fix to Jason's  attempt to fix)
         clnum +=
         ((!p->o_curbyt) || (p->o_curbyt == dm->m_clsizb))&&p->o_bytnum;
         *****/

        clx = p->o_curcl;

    }
    else
        clx = p->o_strtcl;

    for (i=1; i < clnum; i++) {                 /*** M00.01.01b ***/
        clx = getclnum(clx,p);
        if (clx == 0xffff)
            return(-1);
    }

    /* go one more except on cluster boundary */

    if (p->o_curbyt && clnum)                   /*** M00.01.01b ***/
        clx = getclnum(clx,p);

fillin:
    p->o_curcl = clx;
    p->o_currec = cl2rec(clx,dm);
    p->o_bytnum = n;

    return(n);
}



/*
 * xread - read 'len' bytes  from handle 'h'
 *
 * Function 0x3F        f_read
 *
 * Error returns
 *   EIHNDL
 *   bios()
 */

long    xread(int h, long len, void *ubufr) 
{
    OFD *p;
    long ret;

    p = getofd(h);
    if ( p )
        ret = ixread(p,len,ubufr);
    else
        ret = EIHNDL;
#if DBGFSIO
    kprintf("xread(%d, %ld) => %ld\n", h, len, ret);
#endif
    return ret;
}

/*
 * ixread -
 */

long    ixread(OFD *p, long len, void *ubufr)
{
    long maxlen;

#if DBGFSIO  // bug to fix ?
    // kprintf("ixread(%p, %li, %p)\n", p, len, ubufr);
    if(p->o_dmd == NULL)
    {
        kprintf("xread() dm NULL ofd: 0x%p\n", p);
        // return(EIHNDL);
    }
#endif

    /* Make sure file not opened as write only */
    if ((p->o_mod&MODE_FAC) == WO_MODE)
        return (EACCDN);

    if (len > (maxlen = p->o_fileln - p->o_bytnum))
        len = maxlen;

    if (len > 0)
        return(xrw(0,p,len,ubufr,xfr2usr));

    return(0L); /* zero bytes read for zero requested */
}



/*
 * xwrite - write 'len' bytes to handle 'h'.
 *
 * Function 0x40  f_write
 *
 * Error returns
 *   EIHNDL
 *   bios()
 */

long    xwrite(int h, long len, void *ubufr) 
{
    register OFD *p;
    long ret;

    p = getofd(h);
    if ( p ) {
        /* Make sure not read only.*/
        if (((p->o_mod&MODE_FAC) == RO_MODE) && !(p->o_mod&RO_WRITE_OK))
            ret = EACCDN;
        else
            ret = ixwrite(p,len,ubufr);
    } else {
        ret = EIHNDL;
    }
#if DBGFSIO
    kprintf("xwrite(%d, %ld) => %ld\n", h, len, ret);
#endif
    return ret;
}

/*
 *  ixwrite -
 */

long    ixwrite(OFD *p, long len, void *ubufr)
{
    return(xrw(1,p,len,ubufr,usr2xfr));
}

/*
 * addit - update the OFD for the file
 *
 * update the OFD for the file to reflect the fact that 'siz' bytes
 * have been written to it.
 *
 * flg: update curbyt ? (yes if less than 1 cluster transferred)
 */

static void addit(OFD *p, long siz, int flg)
{
    p->o_bytnum += siz;

    if (flg)
        p->o_curbyt += siz;

    if (p->o_bytnum > p->o_fileln)
    {
        p->o_fileln = p->o_bytnum;
        p->o_flag |= O_DIRTY;
    }
}



/*
 * xrw -
 *
 * This has two (related) functions:
 * 1. transfer 'len' bytes between the file indicated by the OFD and the
 *    buffer pointed to by 'ubufr'
 * 2. if 'ubufr' is NULL, return a pointer to the current file position
 *    within an internal buffer.  this is used internally within fsdir.c
 *    and fsopnclo.c to request ixread() [which calls xrw()] to return a
 *    pointer to a directory entry.
 *
 * We wish to do the i/o in whole clusters as much as possible.
 * Therefore, we break the i/o up into 5 sections.  Data which occupies
 * part of a logical record (e.g., sector) with data not in the request
 * (both at the start and the end of the the request) are handled
 * separateley and are called header (tail) bytes.  Data which are
 * contained complete in sectors but share part of a cluster with data not
 * in the request are called header (tail) records.  These are also
 * handled separately.  In between handling of header and tail sections,
 * we do i/o in terms of whole clusters.
 *
 *  returns
 *      1. nbr of bytes read/written from/to the file, or
 *      2. pointer (see above)
 */

static long xrw(int wrtflg, OFD *p, long len, char *ubufr,
                void (*bufxfr)(int, char *, char *))
{
    register DMD *dm;
    char *bufp;
    unsigned int bytn, tailrec;
    int lenxfr, lentail;
    RECNO recn, num;
    int hdrrec, lsiz;
    RECNO last, nrecs;                  /* multi-sector variables */
    int lflg;
    long nbyts;
    long rc,bytpos,lenrec,lenmid;

    /* determine where we currently are in the file */

    dm = p->o_dmd;                      /*  get drive media descriptor  */

    bytpos = p->o_bytnum;               /*  starting file position      */

    /*
     * get logical record number to start i/o with
     * (bytn will be byte offset into sector # recn)
     */

    recn = divmod(&bytn,(long) p->o_curbyt,dm->m_rblog);
    recn += p->o_currec;

    /* determine "header" of request. */

    if (bytn) /* do header */
    {
        /* xfer len is min( #bytes req'd , */
        /* #bytes left in current record ) */

        lenxfr = min(len,dm->m_recsiz-bytn);
        bufp = getrec(recn,p,wrtflg);   /* get desired record  */
        addit(p,(long) lenxfr,1);       /* update ofd          */
        len -= lenxfr;                  /* nbr left to do      */
        recn++;                         /* starting w/ next    */

        if (!ubufr)
            return (long) (bufp+bytn);

        (*bufxfr)(lenxfr,bufp+bytn,ubufr);
        ubufr += lenxfr;
    }

    /* "header" complete.      See if there is a "tail". */
    /* After that, see if there is anything left in the middle. */

    lentail = len & dm->m_rbm;

    lenmid = len - lentail;             /*  Is there a Middle ? */
    if ( lenmid )
    {
        hdrrec = recn & dm->m_clrm;

        if (hdrrec)
        {
            /*  if hdrrec != 0, then we do not start on a clus bndy;
             *  so determine the min of (the nbr sects
             *  remaining in the current cluster) and (the nbr
             *  of sects remaining in the file).  This will be
             *  the number of header records to read/write.
             */

            hdrrec = ( dm->m_clsiz - hdrrec ) ; /* M00.14.01 */
            if ( hdrrec > lenmid >> dm->m_rblog )       /* M00.14.01 */
                hdrrec = lenmid >> dm->m_rblog; /* M00.14.01 */

            usrio(wrtflg,hdrrec,recn,ubufr,dm);
            ubufr += (lsiz = hdrrec << dm->m_rblog);
            lenmid -= lsiz;
            addit(p,(long) lsiz,1);
        }

        /* do whole clusters */

        lenrec = lenmid >> dm->m_rblog;            /* nbr of records  */
        num = divmod(&tailrec,lenrec,dm->m_clrlog);/* nbr of clusters */

        last = nrecs = 0L;
        nbyts = lflg = 0;

        while (num--)           /*  for each whole cluster...   */
        {
            rc = nextcl(p,wrtflg);

            /*
             *  if eof or non-contiguous cluster, or last cluster
             *  of request, then finish pending I/O
             */

            if ((!rc) && (p->o_currec == last + nrecs))
            {
                nrecs += dm->m_clsiz;
                nbyts += dm->m_clsizb;
                if (!num) goto mulio;
            }
            else
            {
                if (!num)
                    lflg = 1;
mulio:
                if (nrecs)
                    usrio(wrtflg,nrecs,last,ubufr,dm);
                ubufr += nbyts;
                addit(p,nbyts,0);
                if (rc)
                    goto eof;
                last = p->o_currec;
                nrecs = dm->m_clsiz;
                nbyts = dm->m_clsizb;
                if ((!num) && lflg)
                {
                    lflg = 0;
                    goto mulio;
                }
            }
        }  /*  end while  */

        /* do "tail" records */

        if (tailrec)
        {
            if (nextcl(p,wrtflg))
                goto eof;
            lsiz = tailrec << dm->m_rblog;
            addit(p,(long) lsiz,1);
            usrio(wrtflg,tailrec,p->o_currec,ubufr,dm);
            ubufr += lsiz;
        }
    }

    /* do tail bytes within this cluster */

    if (lentail)
    {
        recn = divmod(&bytn,(long) p->o_curbyt,dm->m_rblog);

        if ((!recn) || (recn == (RECNO)dm->m_clsiz))
        {
            if (nextcl(p,wrtflg))
                goto eof;
            recn = 0;
        }

        bufp = getrec((RECNO)p->o_currec+recn,p,wrtflg);
        addit(p,(long) lentail,1);

        if (!ubufr)
            return (long) bufp;

        (*bufxfr)(lentail,bufp,ubufr);
    } /*  end tail bytes  */

eof:
    rc = p->o_bytnum - bytpos;

    return(rc);
}

/*
 * usrio -
 *
 * NOTE: longjmp_rwabs() is a macro that includes a longjmp() which is 
 *       executed if the BIOS returns an error, therefore usrio() does 
 *       not need to return any error codes.
 */

static void usrio(int rwflg, int num, long strt, char *ubuf, DMD *dm)
{
    register BCB *b;

    for (b = bufl[BI_DATA]; b; b = b->b_link)
        if ((b->b_bufdrv == dm->m_drvnum) &&
            (b->b_bufrec >= strt) &&
            (b->b_bufrec < strt+num))
            flush(b);

    longjmp_rwabs(rwflg, (long)ubuf, num, strt+dm->m_recoff[BT_DATA], dm->m_drvnum);
}
