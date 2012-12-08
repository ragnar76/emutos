/*
 * EmuTOS aes
 *
 * Copyright (c) 2002, 2010 EmuTOS development team
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef GEMMNLIB_H
#define GEMMNLIB_H


extern LONG     gl_mntree;
extern PD       *gl_mnppd;

extern LONG     desk_acc[];
extern LONG     menu_tree[];

extern WORD     gl_dacnt;
extern WORD     gl_dabox;
extern LONG     gl_datree;

extern OBJECT   M_DESK[];


UWORD do_chg(LONG tree, WORD iitem, UWORD chgvalue,
             WORD dochg, WORD dodraw, WORD chkdisabled);
WORD mn_do(WORD *ptitle, WORD *pitem);

void mn_bar(LONG tree, WORD showit, WORD pid);
void mn_clsda(void);
WORD mn_register(WORD pid, LONG pstr);
void mn_unregister(WORD da_id);
void mn_getownid(PD **owner,WORD *id,WORD item);


#endif
