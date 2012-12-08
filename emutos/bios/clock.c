/*
 * clock.c - BIOS time and date routines
 *
 * Copyright (c) 2001-2011 EmuTOS development team
 *
 * Authors:
 *  MAD   Martin Doering
 *  LVL   Laurent Vogel
 *  THH   Thomas Huth
 *  PES   Petr Stehlik
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#include "config.h"
#include "portab.h"
#include "kprint.h"
#include "clock.h"
#include "ikbd.h"
#include "tosvars.h"
#include "string.h"
#include "vectors.h"
#include "nvram.h"
#include "machine.h"
#include "cookie.h"
#ifdef MACHINE_AMIGA
#include "amiga.h"
#endif

#define DBG_CLOCK 0

/* Date/Time to use when the hardware clock is not set.
 * We use the OS creation date at 00:00:00
 */ 
#define DEFAULT_DATETIME ((ULONG)os_dosdate << 16)

#if CONF_WITH_MEGARTC

/*==== MegaRTC section ====================================================*/

/* one if there is a MegaST real-time clock. */
int has_megartc;

#define CLK_BASE (0x00fffc20L)

#define CLK     struct clkreg
CLK     /*  needs name for casting      */
{
        BYTE fill0,  sec_l;     /* seconds elapsed in current minute */
        BYTE fill2,  sec_h;
        BYTE fill4,  min_l;     /* minutes elapsed in current hour   */
        BYTE fill6,  min_h;
        BYTE fill8,  hour_l;    /* hours elapsed in current day      */
        BYTE filla,  hour_h;
        BYTE fillc,  daywk;     /* day of week (1-7); sunday=1       */
        BYTE fille,  day_l;     /* day of month (1-31) */
        BYTE fill10, day_h;
        BYTE fill12, mon_l;     /* month of year (1-12) */
        BYTE fill14, mon_h;
        BYTE fill16, year_l;    /* year of century (0-99) */
        BYTE fill18, year_h;
        BYTE fill1a, rega;      /* register A */
        BYTE fill1c, regb;      /* register B */
        BYTE fill1e, regc;      /* register C */
};

#define clk (*(volatile CLK*)CLK_BASE)

struct myclkreg {
        BYTE sec_l;     /* seconds elapsed in current minute */
        BYTE sec_h;
        BYTE min_l;     /* minutes elapsed in current hour   */
        BYTE min_h;
        BYTE hour_l;    /* hours elapsed in current day      */
        BYTE hour_h;
        BYTE daywk;     /* day of week (1-7); sunday=1       */
        BYTE day_l;     /* day of month (1-31) */
        BYTE day_h;
        BYTE mon_l;     /* month of year (1-12) */
        BYTE mon_h;
        BYTE year_l;    /* year of century (0-99) */
        BYTE year_h;
};

/* buffers to hols the megartc regs */
static struct myclkreg clkregs1, clkregs2;


void detect_megartc(void)
{
  has_megartc = 0;

  /* first check if the address is valid */
  if (check_read_byte(CLK_BASE+1)) {
    if ((UBYTE)clk.sec_l != 0xff && (UBYTE)clk.sec_h != 0xff) {
      has_megartc = 1;
    }
  }
  
#if NEEDED
  /* 
   * Then this is what TOS 1.2 seems to do. However emulators like
   * STonX do not accept writing to the RTC registers, even in the
   * 'second bank' (if this is what setting rega := 9 means).
   * For this reason it is commented out.
   */
  if (has_megartc) {
    /* use 'second bank' (?), write some test (?) values in registers */
    clk.rega = 9;
    clk.min_l = 10;
    clk.min_h = 5;
    if((clk.min_l != 10) || (clk.min_h != 5)) {
      has_megartc = 0;
    } else {
      /* set back to 'first bank' (?), do some initialisation (?) */
      has_megartc = 1;
      clk.sec_l = 1;
      clk.rega = 8;
      clk.regb = 0;
    }
  }
#endif /* NEEDED */
}

/*==== MegaRTC internal functions =========================================*/

/*
 * MegaRTC, TODO:
 * - leap year ?
 */

/* read the 13 non-control clock registers into clkregs1
 * read the registers twice, and returns only when the two reads 
 * returned the same value.
 * This is because the MegaRTC clock is a very slow chip (32768 kHz)
 * and presumably the carry is not reported instantly when the
 * time changes!!! (this is LVL interpretation, any other reason 
 * is welcome.)
 */

static void mgetregs(void)
{
  int i;
  BYTE *a, *b, *c;
  do {
    c = (BYTE *) &clk.sec_l;
    a = (BYTE *) &clkregs1.sec_l;
    for (i = 0 ; i < 13 ; i++) {
      *a++ = *c;
      c += 2;
    }
    c = (BYTE *) &clk.sec_l;
    b = (BYTE *) &clkregs2.sec_l;
    a = (BYTE *) &clkregs1.sec_l;
    for (i = 0 ; i < 13 ; i++) {
      *b = *c;
      if(*b++ != *a++) break;
      c += 2;
    }
  } while(i != 13);
}

static void msetregs(void)
{
  int i;
  BYTE *a, *c;
  c = (BYTE *) &clk.sec_l;
  a = (BYTE *) &clkregs1.sec_l;
  for (i = 0 ; i < 13 ; i++) {
    *c = *a++;
    c += 2;
  }
}


static void mdosettime(UWORD time)
{
  clkregs1.sec_l = ((time & 0x1F) << 1) % 10;
  clkregs1.sec_h = ((time & 0x1F) << 1) / 10;
  clkregs1.min_l = ((time >> 5) & 0x3F) % 10;
  clkregs1.min_h = ((time >> 5) & 0x3F) / 10;
  clkregs1.hour_l = ((time >> 11) & 0x1F) % 10;
  clkregs1.hour_h = ((time >> 11) & 0x1F) / 10;

#if DBG_CLOCK
  kprintf("mdosettime() %x%x:%x%x:%x%x\n",
    clkregs1.hour_h, clkregs1.hour_l,
    clkregs1.min_h, clkregs1.min_l,
    clkregs1.sec_h, clkregs1.sec_l);
#endif
}


static UWORD mdogettime(void)
{
  UWORD time;

#if DBG_CLOCK
  kprintf("mdogettime() %x%x:%x%x:%x%x\n",
    clkregs1.hour_h, clkregs1.hour_l,
    clkregs1.min_h, clkregs1.min_l,
    clkregs1.sec_h, clkregs1.sec_l);
#endif

  time = (((clkregs1.sec_l & 0xf) + 10 * (clkregs1.sec_h & 0xF)) >> 1)
    |  (((clkregs1.min_l & 0xf) + 10 * (clkregs1.min_h & 0xf)) << 5)
    |  (((clkregs1.hour_l & 0xf) + 10 * (clkregs1.hour_h & 0xf)) << 11) ;
 
  return time;
}


static void mdosetdate(UWORD date)
{
  clkregs1.day_l = (date & 0x1F) % 10;
  clkregs1.day_h = (date & 0x1F) / 10;
  clkregs1.mon_l = ((date >> 5) & 0xF) % 10;
  clkregs1.mon_h = ((date >> 5) & 0xF) / 10;
  clkregs1.year_l = (date >> 9) % 10;
  clkregs1.year_h = (date >> 9) / 10;  

#if DBG_CLOCK
  kprintf("mdosetdate() %x%x/%x%x/%x%x\n",
    clkregs1.year_h, clkregs1.year_l,
    clkregs1.mon_h, clkregs1.mon_l,
    clkregs1.day_h, clkregs1.day_l);
#endif
}

static UWORD mdogetdate(void)
{
  UWORD date;

#if DBG_CLOCK
  kprintf("mdogetdate() %x%x/%x%x/%x%x\n",
    clkregs1.year_h, clkregs1.year_l,
    clkregs1.mon_h, clkregs1.mon_l,
    clkregs1.day_h, clkregs1.day_l);
#endif

  /* The MegaRTC stores the year as the offset from 1980.
     Fortunately, this is exactly the same as in the BIOS format. */

  date = ((clkregs1.day_l & 0xf) + 10 * (clkregs1.day_h & 0xf))
    |  (((clkregs1.mon_l & 0xf) + 10 * (clkregs1.mon_h & 0xf)) << 5) 
    |  (((clkregs1.year_l & 0xf) + 10 * (clkregs1.year_h & 0xf)) << 9) ;
 
  return date;
}

/*==== MegaRTC high-level functions ========================================*/

static ULONG mgetdt(void)
{
  mgetregs();
  return (((ULONG) mdogetdate()) << 16) | mdogettime();
}

static void msetdt(ULONG dt)
{
  mdosetdate(dt>>16);
  mdosettime(dt);
  msetregs();
}

#endif /* CONF_WITH_MEGARTC */

#if CONF_WITH_NVRAM

/*==== NVRAM RTC internal functions =======================================*/

/*
 * The MC146818 was used as the RTC and NVRAM in MegaSTE, TT and Falcon.
 * You can find a header file in /usr/src/linux/include/linux/mc146818rtc.h
 * Proper implementation of RTC functions is in linux/arch/m68k/atari/time.c.
 * The code below is just my quick hack. It works but it could not be used
 * for updating real RTC because it doesn't handle the control registers
 * and also doesn't provide proper timing (32kHz device needs proper timing).
 * Reading of RTC should be OK on real machines.
 * (PES)
 */
#define NVRAM_RTC_SECONDS 0
#define NVRAM_RTC_MINUTES 2
#define NVRAM_RTC_HOURS   4
#define NVRAM_RTC_DAYS    7
#define NVRAM_RTC_MONTHS  8
#define NVRAM_RTC_YEARS   9

/*
 * this is an interesting moment: we should detect the year offset in the RTC
 * but it depends on the TOS version that wrote the value there.
 * It seems that MegaSTE (regardless of TOS version) does have the offset 1970.
 * But in the TT030 case it's unsure - with old TOS (3.01, 3.05) it's 1970, but
 * with latest TT TOS (3.06) it's 1968. This is completely crazy and only Atari
 * engineers could do something like that.
 * With Falcon TOS 4.0x the situation is clear - the offset is always 1968.
 * So we try to figure out the machine we're running on in clock_init and set
 * the year offset accordingly to what we've detected.
 */
static int nvram_rtc_year_offset;

static void ndosettime(UWORD time)
{
  int seconds = (time & 0x1F) << 1;
  int minutes = (time >> 5) & 0x3F;
  int hours = (time >> 11) & 0x1F;

#if DBG_CLOCK
  kprintf("ndosettime() %02d:%02d:%02d\n", hours, minutes, seconds);
#endif

  set_nvram_rtc(NVRAM_RTC_SECONDS, seconds);
  set_nvram_rtc(NVRAM_RTC_MINUTES, minutes);
  set_nvram_rtc(NVRAM_RTC_HOURS, hours);
}

static UWORD ndogettime(void)
{
  UWORD seconds = get_nvram_rtc(NVRAM_RTC_SECONDS);
  UWORD minutes = get_nvram_rtc(NVRAM_RTC_MINUTES);
  UWORD hours = get_nvram_rtc(NVRAM_RTC_HOURS);
  UWORD time;

#if DBG_CLOCK
  kprintf("ndogettime() %02d:%02d:%02d\n", hours, minutes, seconds);
#endif

  time = (seconds >> 1)
       | (minutes << 5)
       | (hours << 11);
 
  return time;
}

static void ndosetdate(UWORD date)
{
  int days = date & 0x1F;
  int months = (date >> 5) & 0xF;
  int years = (date >> 9) - nvram_rtc_year_offset;

#if DBG_CLOCK
  kprintf("ndosetdate() %02d/%02d/%02d\n", years, months, days);
#endif

  set_nvram_rtc(NVRAM_RTC_DAYS, days);
  set_nvram_rtc(NVRAM_RTC_MONTHS, months);
  set_nvram_rtc(NVRAM_RTC_YEARS, years);
}

static UWORD ndogetdate(void)
{
  UWORD days = get_nvram_rtc(NVRAM_RTC_DAYS);
  UWORD months = get_nvram_rtc(NVRAM_RTC_MONTHS);
  UWORD years = get_nvram_rtc(NVRAM_RTC_YEARS);
  UWORD date;

#if DBG_CLOCK
  kprintf("ndogetdate() %02d/%02d/%02d\n", years, months, days);
#endif

  date = (days & 0x1F)
       | ((months & 0xF) << 5) 
       | ((years + nvram_rtc_year_offset) << 9);
 
  return date;
}

/*==== NVRAM RTC high-level functions ======================================*/

static ULONG ngetdt(void)
{
  return (((ULONG) ndogetdate()) << 16) | ndogettime();
}

static void nsetdt(ULONG dt)
{
  ndosetdate(dt >> 16);
  ndosettime(dt);
}

#endif /* CONF_WITH_NVRAM */

#if CONF_WITH_IKBD_CLOCK

/*==== IKBD clock section =================================================*/

static struct ikbdregs {
  UBYTE cmd;
  UBYTE year;
  UBYTE month;
  UBYTE day;
  UBYTE hour;
  UBYTE min;
  UBYTE sec;
} iclkbuf;

static volatile WORD iclk_ready;

/* called by the ACIA interrupt */
void clockvec(BYTE *buf)
{
  BYTE *b = 1 + ((BYTE *)&iclkbuf);
  memmove(b, buf, 6);
  iclk_ready = 1;
}

static inline UBYTE int2bcd(UWORD a)
{
  return (a % 10) + ((a / 10) << 4);
}

static inline UWORD bcd2int(UBYTE a)
{
  return (a & 0xF) + ((a >> 4) * 10);
}

/*==== Ikbd clock internal functions ======================================*/


static void igetregs(void)
{
  long delay;
  iclk_ready = 0;
  iclkbuf.cmd = 0x1C;
  ikbdws(0, (PTR) &iclkbuf);

  /* wait until the interrupt receives the full packet */
  delay = 8000000;
  while(!iclk_ready && delay > 0) {
    --delay;
  }
}

#if 0  /* currently unused */
static void iresetregs(void)
{
  iclkbuf.cmd   = 0x1B;
  iclkbuf.year  = 0xFF;
  iclkbuf.month = 0xFF;
  iclkbuf.day   = 0xFF;
  iclkbuf.hour  = 0xFF;
  iclkbuf.min   = 0xFF;
  iclkbuf.sec   = 0xFF;
}
#endif

static void isetregs(void)
{
  iclkbuf.cmd = 0x1B;
  ikbdws(6, (PTR) &iclkbuf);
}

static UWORD idogetdate(void)
{
  UWORD year;
  UWORD date;

#if DBG_CLOCK
  kprintf("idogetdate() %02x/%02x/%02x\n", iclkbuf.year, iclkbuf.month, iclkbuf.day);
#endif
  /* guess the real year from IKBD data */
  year = bcd2int(iclkbuf.year);
  if (year < 80)
    year += 2000;
  else
    year += 1900;

  date = ((year-1980) << 9)
    | ( bcd2int(iclkbuf.month) << 5 ) | bcd2int(iclkbuf.day);

  return date;
}

static UWORD idogettime(void)
{
  UWORD time;

#if DBG_CLOCK
  kprintf("idogettime() %02x:%02x:%02x\n", iclkbuf.hour, iclkbuf.min, iclkbuf.sec);
#endif
  time = ( bcd2int(iclkbuf.sec) >> 1 )
    | ( bcd2int(iclkbuf.min) << 5 ) | ( bcd2int(iclkbuf.hour) << 11 ) ;

  return time;
}

static void idosettime(UWORD time) 
{
  iclkbuf.sec = int2bcd( (time << 1) & 0x3f );
  iclkbuf.min = int2bcd( (time >> 5) & 0x3f );
  iclkbuf.hour = int2bcd( (time >> 11) & 0x1f );
#if DBG_CLOCK
  kprintf("idosettime() %02x:%02x:%02x\n", iclkbuf.hour, iclkbuf.min, iclkbuf.sec);
#endif
}

static void idosetdate(UWORD date) 
{
  UWORD year = 1980 + ((date >> 9) & 0x7f);

  iclkbuf.year = int2bcd( year % 100 );
  iclkbuf.month = int2bcd( (date >> 5) & 0xf );
  iclkbuf.day = int2bcd( date & 0x1f );
#if DBG_CLOCK
  kprintf("idosetdate() %02x/%02x/%02x\n", iclkbuf.year, iclkbuf.month, iclkbuf.day);
#endif
}


/*==== Ikbd Clock high-level functions ====================================*/

static ULONG igetdt(void)
{
  igetregs();
  return (((ULONG)idogetdate()) << 16) | idogettime();
}

static void isetdt(ULONG dt)
{
  idosetdate(dt>>16);
  idosettime(dt);
  isetregs();
}

#endif /* CONF_WITH_IKBD_CLOCK */

/* internal init */

void clock_init(void)
{
  if(FALSE) {
    /* Dummy case for conditional compilation */
  }
#ifdef MACHINE_AMIGA
  else if(TRUE) {
    return amiga_clock_init();
  }
#endif /* MACHINE_AMIGA */
#if CONF_WITH_NVRAM
  else if(has_nvram) {
    /* On Mega-STE and early TTs the year offset in the NVRAM is different */
    if (cookie_mch < MCH_TT || (cookie_mch == MCH_TT && TOS_VERSION <= 0x0305)) {
        nvram_rtc_year_offset = 1970 - 1980;
    }
    else {
        nvram_rtc_year_offset = 1968 - 1980;
    }
  }
#endif /* CONF_WITH_NVRAM */
#if CONF_WITH_MEGARTC
  else if(has_megartc) {
    /* Nothing to initialize */
  }
#endif /* CONF_WITH_MEGARTC */
#if CONF_WITH_IKBD_CLOCK
  else {
    /* The IKBD clock is lost at power off, and has bogus values at poweron.
     * So initialize it to the default date/time at startup.
     */
    isetdt(DEFAULT_DATETIME);
  }
#endif /* CONF_WITH_IKBD_CLOCK */
}

/* xbios functions */

void settime(LONG time)
{
  if(FALSE) {
    /* Dummy case for conditional compilation */
  }
#if CONF_WITH_NVRAM
  else if(has_nvram) {
    nsetdt(time);
  }
#endif /* CONF_WITH_NVRAM */
#if CONF_WITH_MEGARTC
  else if(has_megartc) {
    msetdt(time);
  }
#endif /* CONF_WITH_MEGARTC */
  else {
#if CONF_WITH_IKBD_CLOCK
    isetdt(time);
#endif /* CONF_WITH_IKBD_CLOCK */
  }
}

LONG gettime(void)
{
  if(FALSE) {
    /* Dummy case for conditional compilation */
  }
#ifdef MACHINE_AMIGA
  else if(TRUE) {
    return amiga_getdt();
  }
#endif /* MACHINE_AMIGA */
#if CONF_WITH_NVRAM
  else if(has_nvram) {
    return ngetdt();
  }
#endif /* ! NO_NVRAM */
#if CONF_WITH_MEGARTC
  else if(has_megartc) {
    return mgetdt();
  }
#endif /* CONF_WITH_MEGARTC */
  else {
#if CONF_WITH_IKBD_CLOCK
    return igetdt();
#else
    return DEFAULT_DATETIME;
#endif /* CONF_WITH_IKBD_CLOCK */
  }
}
