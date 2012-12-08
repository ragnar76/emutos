/*
 * mem.h - header file for memory and process management routines  
 *
 * Copyright (c) 2001 Lineo, Inc. and 
 *
 * Authors:
 *  xxx <xxx@xxx>
 *  LVL Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef MEM_H
#define MEM_H


/*
 *  conditional compile switches
 */

#define OSMPANIC        FALSE
#define OSMLIST         FALSE


/*
 *  Data Structures
 */

/*
 *  MD - Memory Descriptor
 */

typedef struct _md MD;

struct _md
{
    MD      *m_link;
    LONG    m_start;
    LONG    m_length;
    PD      *m_own;
};


/*
 *  MPB - Memory Partition Block
 */

typedef struct _mpb MPB;

struct _mpb
{
    MD      *mp_mfl;
    MD      *mp_mal;
    MD      *mp_rover;
};


/*
 *  externals
 */

extern  FTAB    sft[];
extern  MPB     pmd;    /* the mem pool for the main user ST ram */
#if CONF_WITH_ALT_RAM
extern  MPB     pmdalt;  /* the memory pool for the alternative ram (FastRAM or other) */
extern  int     has_alt_ram; /* 1 if alternative RAM has been declared to BDOS */
#endif

/*
 * these should be internal
 */

extern long start_stram;
extern long end_stram;

/* 
 * in osmem.c
 */

/*  xmgetblk - get a block of memory from the o/s pool. */
void *xmgetblk(int i);

/*  MGET - wrapper around xmgetblk */
#define MGET(x) ((x *) xmgetblk((sizeof(x) + 15)>>4))

/*  xmfreblk - free up memory allocated through mgetblk */
void xmfreblk(void *m);

/* init os memory */
void osmem_init(void);

/*
 * in umem.c 
 */

/* allocate memory */
long xmalloc(long amount);
/* mfree */
long xmfree(long addr);
/* mshrink */
long xsetblk(int n, void *blk, long len);
/* mxalloc */
long xmxalloc(long amount, int mode);

/* allowed values for Mxalloc mode: */
#define MX_STRAM 0
#define MX_TTRAM 1
#define MX_PREFSTRAM 2
#define MX_PREFTTRAM 3

#if CONF_WITH_ALT_RAM
/* declare additional memory */
long xmaddalt(long start, long size);
#endif /* CONF_WITH_ALT_RAM */

/* init user memory */
void umem_init(void);

/*
 * in iumem.c
 */ 

/* find first fit for requested memory in ospool */
MD *ffit(long amount, MPB *mp);
/* Free up a memory descriptor */
void freeit(MD *m, MPB *mp);


#endif /* MEM_H */
