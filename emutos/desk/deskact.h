/*
 * EmuTOS desktop
 *
 * Copyright (c) 2002, 2010 EmuTOS development team
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */


/* Prototypes: */
WORD act_chg(WORD wh, LONG tree, WORD root, WORD obj, GRECT *pc, UWORD chgvalue,
             WORD dochg, WORD dodraw, WORD chkdisabled);
void act_allchg(WORD wh, LONG tree, WORD root, WORD ex_obj, GRECT *pt, GRECT *pc, 
                WORD chgvalue, WORD dochg, WORD dodraw);
void act_bsclick(WORD wh, LONG tree, WORD root, WORD mx, WORD my, WORD keystate,
                 GRECT *pc, WORD dclick);
WORD act_bdown(WORD wh, LONG tree, WORD root, WORD *in_mx, WORD *in_my,
               WORD keystate, GRECT *pc, WORD *pdobj);
