/*
 * bios.h - misc BIOS function prototypes
 *
 * Copyright (c) 2011 by the EmuTOS Development Team
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef BIOS_H
#define BIOS_H
 
void biosmain(void);
LONG bios_do_unimpl(WORD number);

/* misc BIOS functions */
LONG bconstat(WORD handle);
LONG bconin(WORD handle);
LONG bconout(WORD handle, WORD what);
LONG lrwabs(WORD r_w, LONG adr, WORD numb, WORD first, WORD drive, LONG lfirst);
LONG setexec(WORD num, LONG vector);
LONG tickcal(void);
LONG getbpb(WORD drive);
LONG bcostat(WORD handle);
LONG mediach(WORD drv);
LONG drvmap(void);

#endif /* BIOS_H */
