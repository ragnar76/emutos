/*
 * xhdi.c - XHDI handler
 *
 * Copyright (c) 2001-2005 EmuTOS development team
 *
 * Authors:
 *  PES   Petr Stehlik
 *
 *  xhdi_handler() inspired by ppzip driver (by Frank Naumann)
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#include "config.h"
#include "stdarg.h"
#include "kprint.h"
#include "blkdev.h"
#include "gemerror.h"
#include "acsi.h"
#include "string.h"
#include "cookie.h"
#include "natfeat.h"
#include "ide.h"

#include "xhdi.h"
 
#define DBG_XHDI        0

#if CONF_WITH_XHDI

/*--- Global variables ---*/

typedef long (*XHDI_HANDLER)(UWORD opcode, ...);
static XHDI_HANDLER next_handler; /* Next handler installed by XHNewCookie() */

/*---Functions ---*/

void create_XHDI_cookie(void)
{
    cookie_add(COOKIE_XHDI, (long)xhdi_vec);
}

static UWORD XHGetVersion(void)
{
    UWORD version = 0x130;

    if (next_handler) {
        UWORD next_version = (UWORD)next_handler(XHGETVERSION);
        if (next_version < version)
            version = next_version;
    }

    return version;
}

static ULONG XHDrvMap(void)
{
    ULONG drvmap = blkdev_drvmap();

    /* Currently, floppy drives can't be accessed through XHDI. */
    drvmap &= ~0x03;

    if (next_handler)
        drvmap |= next_handler(XHDRVMAP);

    return drvmap;
}

static long XHNewCookie(ULONG newcookie)
{
    if (next_handler)
        return next_handler(XHNEWCOOKIE, newcookie);

    next_handler = (XHDI_HANDLER)newcookie;

    return E_OK;
}

static long XHInqDev2(UWORD drv, UWORD *major, UWORD *minor, ULONG *start,
                      BPB *bpb, ULONG *blocks, char *partid)
{
    long pstart = blkdev[drv].start;
    int mediachange = 0;
    BPB *myBPB;

#if DBG_XHDI
    kprintf("XHInqDev2(%c:) start=%ld, size=%ld, ID=%c%c%c\n",
            'A' + drv, pstart, blkdev[drv].size,
            blkdev[drv].id[0], blkdev[drv].id[1], blkdev[drv].id[2]);
#endif
    
    if (next_handler) {
        long ret = next_handler(XHINQDEV2, drv, major, minor, start, bpb, blocks, partid);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }

    if (major)
        *major = blkdev[drv].unit-2;
    if (minor)
        *minor = 0;
    if (bpb)
        bpb->recsiz = 0;

    if (!pstart)
        return EDRVNR;

    if (start)
        *start = pstart;

    myBPB = (BPB *)blkdev_getbpb(drv);
    if (bpb)
        memcpy(bpb, myBPB, sizeof(BPB));

    if (blocks)
        *blocks = blkdev[drv].size;

    if (partid) {
        partid[0] = blkdev[drv].id[0];
        partid[1] = blkdev[drv].id[1];
        partid[2] = blkdev[drv].id[2];
    }

    if (mediachange)
        return EDRVNR;

   return E_OK;
}

static long XHInqDev(UWORD drv, UWORD *major, UWORD *minor, ULONG *start,
                     BPB *bpb)
{
    if (next_handler) {
        long ret = next_handler(XHINQDEV, drv, major, minor, start, bpb);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }

    return XHInqDev2(drv, major, minor, start, bpb, NULL, NULL);
}

static long XHReserve(UWORD major, UWORD minor, UWORD do_reserve, UWORD key)
{
    if (next_handler) {
        long ret = next_handler(XHRESERVE, major, minor, do_reserve, key);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }

    return EINVFN;
}

static long XHLock(UWORD major, UWORD minor, UWORD do_lock, UWORD key)
{
    if (next_handler) {
        long ret = next_handler(XHLOCK, major, minor, do_lock, key);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }

    return EINVFN;
}

static long XHStop(UWORD major, UWORD minor, UWORD do_stop, UWORD key)
{
    if (next_handler) {
        long ret = next_handler(XHSTOP, major, minor, do_stop, key);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }

    return EINVFN;
}

static long XHEject(UWORD major, UWORD minor, UWORD do_eject, UWORD key)
{
    if (next_handler) {
        long ret = next_handler(XHEJECT, major, minor, do_eject, key);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }

    return EINVFN;
}

static long XHInqDriver(UWORD dev, char *name, char *version, char *company, UWORD *ahdi_version, UWORD *max_IPL)
{
    if (next_handler) {
        long ret = next_handler(XHSTOP, dev, name, version, company, ahdi_version, max_IPL);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }

    return EINVFN;
}

static long XHDriverSpecial(ULONG key1, ULONG key2, UWORD subopcode, void *data)
{
    if (next_handler) {
        long ret = next_handler(XHDRIVERSPECIAL, key1, key2, subopcode, data);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }

    return EINVFN;
}

static long XHMediumChanged(UWORD major, UWORD minor)
{
    if (next_handler) {
        long ret = next_handler(XHMEDIUMCHANGED, major, minor);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }

    return EINVFN;
}

static long XHDOSLimits(UWORD which, ULONG limit)
{
   if (next_handler) {
        long ret = next_handler(XHDOSLIMITS, which, limit);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }

    return EINVFN;
}

static long XHLastAccess(UWORD major, UWORD minor, ULONG *ms)
{
    if (next_handler) {
        long ret = next_handler(XHLASTACCESS, major, minor, ms);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }

    return EINVFN;
}

static long XHReaccess(UWORD major, UWORD minor)
{
    if (next_handler) {
        long ret = next_handler(XHREACCESS, major, minor);
        if (ret != EINVFN && ret != EUNDEV && ret != EDRIVE)
            return ret;
    }
    
    return EINVFN;
}

#endif  /* CONF_WITH_XHDI */

/*=========================================================================*/

static long XHInqTarget2(UWORD major, UWORD minor, ULONG *blocksize,
                         ULONG *deviceflags, char *productname, UWORD stringlen)
{
#if DBG_XHDI
    kprintf("XHInqTarget2(%d.%d)\n", major, minor);
#endif

#if CONF_WITH_XHDI
    if (next_handler) {
        long ret = next_handler(XHINQTARGET2, major, minor, blocksize, deviceflags, productname, stringlen);
        if (ret != EINVFN && ret != EUNDEV)
            return ret;
    }
#endif

#if DETECT_NATIVE_FEATURES
    /* direct access to device */
    if (get_xhdi_nfid()) {
        long ret = NFCall(get_xhdi_nfid() + XHINQTARGET2, (long)major, (long)minor, (long)blocksize, (long)deviceflags, (long)productname, (long)stringlen);
        if (ret != EINVFN && ret != EUNDEV)
            return ret;
    }
#endif

    if (blocksize) {
        /* TODO could add some heuristic here:
         * 1) create two buffers and fill first with zeros and second with $ff
         * 2) read first sector to both buffers
         * 3) find last common byte
         * 4) blocksize = index_of_last_common_byte + 1
         */
        *blocksize = 512;   /* usually physical sector size on HDD is 512 bytes */
    }
    if (deviceflags)
        *deviceflags = 0;  /* not implemented yet */
    if (productname)
        strcpy(productname, "EmuTOS Disk");

    return 0;
}

long XHInqTarget(UWORD major, UWORD minor, ULONG *blocksize,
                 ULONG *deviceflags, char *productname)
{
#if CONF_WITH_XHDI
    if (next_handler) {
        long ret = next_handler(XHINQTARGET, major, minor, blocksize, deviceflags, productname);
        if (ret != EINVFN && ret != EUNDEV)
            return ret;
    }
#endif

    return XHInqTarget2(major, minor, blocksize, deviceflags, productname, 33);
}

long XHGetCapacity(UWORD major, UWORD minor, ULONG *blocks, ULONG *blocksize)
{
#if DBG_XHDI
    kprintf("XHGetCapacity(%d.%d)\n", major, minor);
#endif
    
#if CONF_WITH_XHDI
    if (next_handler) {
        long ret = next_handler(XHGETCAPACITY, major, minor, blocks, blocksize);
        if (ret != EINVFN && ret != EUNDEV)
            return ret;
    }
#endif

#if DETECT_NATIVE_FEATURES
    if (get_xhdi_nfid()) {
        long ret = NFCall(get_xhdi_nfid() + XHGETCAPACITY, (long)major, (long)minor, (long)blocks, (long)blocksize);
        if (ret != EINVFN && ret != EUNDEV)
            return ret;
    }
#endif

    return EINVFN;
}

/*=========================================================================*/

long XHReadWrite(UWORD major, UWORD minor, UWORD rw, ULONG sector,
                 UWORD count, void *buf)
{
    WORD dev = major;

#if DBG_XHDI
    kprintf("XH%s(device=%d.%d, sector=%ld, count=%d, buf=%p)\n",
            rw ? "Write" : "Read", major, minor, sector, count, buf);
#endif

#if CONF_WITH_XHDI
    if (next_handler) {
        long ret = next_handler(XHREADWRITE, major, minor, rw, sector, count, buf);
        if (ret != EINVFN && ret != EUNDEV)
            return ret;
    }
#endif

#if DETECT_NATIVE_FEATURES
    /* direct access to device */
    if (get_xhdi_nfid()) {
        long ret = NFCall(get_xhdi_nfid() + XHREADWRITE, (long)dev, (long)0, (long)rw, (long)sector, (long)count, buf);
        if (ret != EINVFN && ret != EUNDEV)
            return ret;
    }
#endif

    if (minor != 0)
        return EUNDEV;

    /* hardware access to device */
    if (FALSE) {
        /* Dummy case for conditional compilation */
    }
#if CONF_WITH_ACSI
    else if (dev >= 0 && dev < 8) {
        long ret = acsi_rw(rw, sector, count, (LONG)buf, dev);
#if DBG_XHDI
        kprintf("acsi_rw() returned %ld\n", ret);
#endif
        return ret;
    }
#endif /* CONF_WITH_ACSI */
    else if (dev < 16) {
        return EUNDEV;  /* call scsi_rw() here when implemented */
    }
#if CONF_WITH_IDE
    else if (dev < 24) {
        long ret = ide_rw(rw, sector, count, (LONG)buf, dev - 16);
#if DBG_XHDI
        kprintf("ide_rw() returned %ld\n", ret);
#endif
        return ret;
    }
#endif /* CONF_WITH_IDE */

    return EUNDEV;      /* unknown device */
}

/*=========================================================================*/

#if CONF_WITH_XHDI

long xhdi_handler(UWORD *stack)
{
    UWORD opcode = *stack;

    switch (opcode)
    {
        case XHGETVERSION:
        {
            return XHGetVersion();
        }

        case XHINQTARGET:
        {
            struct XHINQTARGET_args
            {
                UWORD opcode;
                UWORD major;
                UWORD minor;
                ULONG *blocksize;
                ULONG *deviceflags;
                char *productname;
            } *args = (struct XHINQTARGET_args *)stack;

            return XHInqTarget(args->major, args->minor, args->blocksize, args->deviceflags, args->productname);
        }

        case XHRESERVE:
        {
            struct XHRESERVE_args
            {
                UWORD opcode;
                UWORD major;
                UWORD minor;
                UWORD do_reserve;
                UWORD key;
            } *args = (struct XHRESERVE_args *)stack;

            return XHReserve(args->major, args->minor, args->do_reserve, args->key);
        }

        case XHLOCK:
        {
            struct XHLOCK_args
            {
                UWORD opcode;
                UWORD major;
                UWORD minor;
                UWORD do_lock;
                UWORD key;
            } *args = (struct XHLOCK_args *)stack;

            return XHLock(args->major, args->minor, args->do_lock, args->key);
        }

        case XHSTOP:
        {
            struct XHSTOP_args
            {
                UWORD opcode;
                UWORD major;
                UWORD minor;
                UWORD do_stop;
                UWORD key;
            } *args = (struct XHSTOP_args *)stack;

            return XHStop(args->major, args->minor, args->do_stop, args->key);
        }

        case XHEJECT:
        {
            struct XHEJECT_args
            {
                UWORD opcode;
                UWORD major;
                UWORD minor;
                UWORD do_eject;
                UWORD key;
            } *args = (struct XHEJECT_args *)stack;

            return XHEject(args->major, args->minor, args->do_eject, args->key);
        }

        case XHDRVMAP:
        {
            return XHDrvMap();
        }

        case XHINQDEV:
        {
            struct XHINQDEV_args
            {
                UWORD opcode;
                UWORD drv;
                UWORD *major;
                UWORD *minor;
                ULONG *start;
                BPB *bpb;
            } *args = (struct XHINQDEV_args *)stack;

            return XHInqDev(args->drv, args->major, args->minor, args->start, args->bpb);
        }

        case XHINQDRIVER:
        {
            struct XHINQDRIVER_args
            {
                UWORD opcode;
                UWORD dev;
                char *name;
                char *version;
                char *company;
                UWORD *ahdi_version;
                UWORD *maxIPL;
            } *args = (struct XHINQDRIVER_args *)stack;

            return XHInqDriver(args->dev, args->name, args->version, args->company, args->ahdi_version, args->maxIPL);
        }

        case XHNEWCOOKIE:
        {
            struct XHNEWCOOKIE_args
            {
                UWORD opcode;
                ULONG newcookie;
            } *args = (struct XHNEWCOOKIE_args *)stack;

            return XHNewCookie(args->newcookie);
        }

        case XHREADWRITE:
        {
            struct XHREADWRITE_args
            {
                UWORD opcode;
                UWORD major;
                UWORD minor;
                UWORD rw;
                ULONG sector;
                UWORD count;
                void *buf;
            } *args = (struct XHREADWRITE_args *)stack;

            return XHReadWrite(args->major, args->minor, args->rw, args->sector, args->count, args->buf);
        }

        case XHINQTARGET2:
        {
            struct XHINQTARGET2_args
            {
                UWORD opcode;
                UWORD major;
                UWORD minor;
                ULONG *blocksize;
                ULONG *deviceflags;
                char *productname;
                UWORD stringlen;
            } *args = (struct XHINQTARGET2_args *)stack;

            return XHInqTarget2(args->major, args->minor, args->blocksize, args->deviceflags, args->productname, args->stringlen);
        }

        case XHINQDEV2:
        {
            struct XHINQDEV2_args
            {
                UWORD opcode;
                UWORD drv;
                UWORD *major;
                UWORD *minor;
                ULONG *start;
                BPB *bpb;
                ULONG *blocks;
                char *partid;
            } *args = (struct XHINQDEV2_args *)stack;

            return XHInqDev2(args->drv, args->major, args->minor, args->start, args->bpb, args->blocks, args->partid);
        }

        case XHDRIVERSPECIAL:
        {
            struct XHDRIVERSPECIAL_args
            {
                UWORD opcode;
                ULONG key1;
                ULONG key2;
                UWORD subopcode;
                void *data;
            } *args = (struct XHDRIVERSPECIAL_args *)stack;

            return XHDriverSpecial(args->key1, args->key2, args->subopcode, args->data);
        }

        case XHGETCAPACITY:
        {
            struct XHGETCAPACITY_args
            {
                UWORD opcode;
                UWORD major;
                UWORD minor;
                ULONG *blocks;
                ULONG *blocksize;
            } *args = (struct XHGETCAPACITY_args *)stack;

            return XHGetCapacity(args->major, args->minor, args->blocks, args->blocksize);
        }

        case XHMEDIUMCHANGED:
        {
            struct XHMEDIUMCHANGED_args
            {
                UWORD opcode;
                UWORD major;
                UWORD minor;
            } *args = (struct XHMEDIUMCHANGED_args *)stack;

            return XHMediumChanged(args->major, args->minor);
        }

/* 
       MiNT places itself at the beginning of the chain,
       We don't need to pass this call.

        case XHMINTINFO:
        {
            return XHMiNTInfo();
        }
*/
        case XHDOSLIMITS:
        {
            struct XHDOSLIMITS_args
            {
                UWORD opcode;
                UWORD which;
                ULONG limit;
            } *args = (struct XHDOSLIMITS_args *)stack;

            return XHDOSLimits(args->which, args->limit);
        }

        case XHLASTACCESS:
        {
            struct XHLASTACCESS_args
            {
                UWORD opcode;
                UWORD major;
                UWORD minor;
                ULONG *ms;
            } *args = (struct XHLASTACCESS_args *)stack;

            return XHLastAccess(args->major, args->minor, args->ms);
        }

        case XHREACCESS:
        {
            struct XHREACCESS_args
            {
                UWORD opcode;
                UWORD major;
                UWORD minor;
            } *args = (struct XHREACCESS_args *)stack;

            return XHReaccess(args->major, args->minor);
        }

        default:
        {
            return EINVFN;
        }
    }
}

#endif /* CONF_WITH_XHDI */

/*
vim:et:ts=4:sw=4:
*/
