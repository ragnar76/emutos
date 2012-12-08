/*
 * boot.c - standalone boot.prg to load EmuTOS in RAM
 *
 * Copyright (c) 2001 by following authors
 *
 * Authors:
 *  LVL     Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#include <osbind.h>
#include <stdlib.h>

extern void bootasm(long dest, char *src, long count);
extern long getbootsize(void);

/*
 * the file TOS_FILENAME will be loaded in memory, then copied at the 
 * address indicated by its header and finally be executed by
 * jumping at that address.
 */

#define TOS_FILENAME "ramtos.img"

/* hack to ensure the file in RAM is at a higher address than the
 * final relocation address
 */
#define ADDR 0x40000

struct {
  char fill[26];
  long size;
  char name[14];
} dta;

static void putl(unsigned long u)
{
  int i;
  char c[9];
  unsigned a;
  c[8] = 0;
  for(i = 0 ; i < 8 ; i++) {
    a = u & 0xf;
    u >>= 4L;
    c[7-i] = (a < 10) ? (a + '0') : (a + 'a' - 10);
  }
  (void)Cconws(c);
}

static void fatal(const char *s)
{
  (void)Cconws("Fatal: ");
  (void)Cconws(s);
  (void)Cconws("\012\015hit any key.");
  Cconin();
  exit(1);
}

int main()
{
  int fh;
  long count;
  char *buf;
  long address;
  
  /* get the file size */
  
  Fsetdta((char *)&dta);
  if (0 != Fsfirst(TOS_FILENAME, 0)) fatal("missing file " TOS_FILENAME);
  count = dta.size;
  
  /* allocate the buffer */
  
  buf = (char *) Malloc(ADDR + count + 1 + getbootsize());
  if(buf == 0) {
    fatal("cannot allocate memory");
  }
  buf += ADDR;
  
  /* open the file and load all in memory */

  fh = Fopen(TOS_FILENAME, 0);
  if(fh < 0) fatal("cannot open file " TOS_FILENAME);
  if (count != Fread(fh, count, buf)) fatal("read error");
  Fclose(fh);
  
  /* get final address */
  
  address = *((long *)(buf + 8));
  
  (void)Cconws("\012\015address = 0x");
  putl(address);
  (void)Cconws(".\012\015");
  
  /* check that the address is not after our buffer */

  if((address <= 0x400L) || (address >= (long)buf)) {
    fatal("bad address in header");
  }

  /* hit a key to let the user remove any floppy */

  (void)Cconws("Hit RETURN to boot " TOS_FILENAME);
  Cconin();
  
  /* supervisor */
  
  Super(0);
  
  /* do the rest in assembler */
  
  bootasm(address, buf, count);
  
  return 1;
}
