/*
 * acsi.h - Atari Simple Computer Interface (ACSI) support
 *
 * Copyright (c) 2002 EmuTOS development team
 *
 * Authors:
 *  LVL   Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef ACSI_H
#define ACSI_H

#include "portab.h"

#if CONF_WITH_ACSI

LONG acsi_rw(WORD rw, LONG sector, WORD count, LONG buf, WORD dev);

#endif /* CONF_WITH_ACSI */

#endif /* ACSI_H */

