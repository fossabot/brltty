/*
 * BRLTTY - A background process providing access to the Linux console (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2001 by The BRLTTY Team. All rights reserved.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation.  Please see the file COPYING for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */
#define VERSION \
"BRLTTY driver for Tieman Voyager, version 0.4 (October 2001)"
#define COPYRIGHT \
"   Copyright (C) 2001 by St�phane Doyon  <s.doyon@videotron.ca>\n" \
"                     and St�phane Dalton <sdalton@videotron.ca>"
/* Voyager/brl.c - Braille display driver for Tieman Voyager displays.
 *
 * Written by:
 *   St�phane Doyon  <s.doyon@videotron.ca>
 *   St�phane Dalton <sdalton@videotron.ca>
 *
 * It is being tested on Voyager 44, should also support Voyager 70.
 * It is designed to be compiled in BRLTTY version 3.0.
 *
 * This is the first release. Should be usable; key bindings might
 * benefit from some fine-tuning.
 */

#define BRL_C 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#include "../brl.h"
#include "../misc.h"
#include "../scr.h"
#include "../inskey.h"

#define BRLNAME "Voyager"
#define PREFSTYLE ST_VoyagerStyle

typedef enum {
  PARM_REPEAT_INIT_DELAY=0,
  PARM_REPEAT_INTER_DELAY
} DriverParameter;
#define BRLPARMS "repeat_init_delay", "repeat_inter_delay"

#include "../brl_driver.h"

#include "brlconf.h"

/* Kernel driver interface */
#include "kernel/voyager.h"

/* This defines the mapping from brltty coding to Voyager's dot coding. */
static unsigned char brl2voyDotsTable[256] =
{
  0x00, 0x01, 0x08, 0x09, 0x02, 0x03, 0x0A, 0x0B,
  0x10, 0x11, 0x18, 0x19, 0x12, 0x13, 0x1A, 0x1B,
  0x04, 0x05, 0x0C, 0x0D, 0x06, 0x07, 0x0E, 0x0F,
  0x14, 0x15, 0x1C, 0x1D, 0x16, 0x17, 0x1E, 0x1F,
  0x20, 0x21, 0x28, 0x29, 0x22, 0x23, 0x2A, 0x2B,
  0x30, 0x31, 0x38, 0x39, 0x32, 0x33, 0x3A, 0x3B,
  0x24, 0x25, 0x2C, 0x2D, 0x26, 0x27, 0x2E, 0x2F,
  0x34, 0x35, 0x3C, 0x3D, 0x36, 0x37, 0x3E, 0x3F,
  0x40, 0x41, 0x48, 0x49, 0x42, 0x43, 0x4A, 0x4B,
  0x50, 0x51, 0x58, 0x59, 0x52, 0x53, 0x5A, 0x5B,
  0x44, 0x45, 0x4C, 0x4D, 0x46, 0x47, 0x4E, 0x4F,
  0x54, 0x55, 0x5C, 0x5D, 0x56, 0x57, 0x5E, 0x5F,
  0x60, 0x61, 0x68, 0x69, 0x62, 0x63, 0x6A, 0x6B,
  0x70, 0x71, 0x78, 0x79, 0x72, 0x73, 0x7A, 0x7B,
  0x64, 0x65, 0x6C, 0x6D, 0x66, 0x67, 0x6E, 0x6F,
  0x74, 0x75, 0x7C, 0x7D, 0x76, 0x77, 0x7E, 0x7F,
  0x80, 0x81, 0x88, 0x89, 0x82, 0x83, 0x8A, 0x8B,
  0x90, 0x91, 0x98, 0x99, 0x92, 0x93, 0x9A, 0x9B,
  0x84, 0x85, 0x8C, 0x8D, 0x86, 0x87, 0x8E, 0x8F,
  0x94, 0x95, 0x9C, 0x9D, 0x96, 0x97, 0x9E, 0x9F,
  0xA0, 0xA1, 0xA8, 0xA9, 0xA2, 0xA3, 0xAA, 0xAB,
  0xB0, 0xB1, 0xB8, 0xB9, 0xB2, 0xB3, 0xBA, 0xBB,
  0xA4, 0xA5, 0xAC, 0xAD, 0xA6, 0xA7, 0xAE, 0xAF,
  0xB4, 0xB5, 0xBC, 0xBD, 0xB6, 0xB7, 0xBE, 0xBF,
  0xC0, 0xC1, 0xC8, 0xC9, 0xC2, 0xC3, 0xCA, 0xCB,
  0xD0, 0xD1, 0xD8, 0xD9, 0xD2, 0xD3, 0xDA, 0xDB,
  0xC4, 0xC5, 0xCC, 0xCD, 0xC6, 0xC7, 0xCE, 0xCF,
  0xD4, 0xD5, 0xDC, 0xDD, 0xD6, 0xD7, 0xDE, 0xDF,
  0xE0, 0xE1, 0xE8, 0xE9, 0xE2, 0xE3, 0xEA, 0xEB,
  0xF0, 0xF1, 0xF8, 0xF9, 0xF2, 0xF3, 0xFA, 0xFB,
  0xE4, 0xE5, 0xEC, 0xED, 0xE6, 0xE7, 0xEE, 0xEF,
  0xF4, 0xF5, 0xFC, 0xFD, 0xF6, 0xF7, 0xFE, 0xFF
};

/* This is the reverse: mapping from Voyager to brltty dot cofing. */
static unsigned char voy2brlDotsTable[256] =
{
  0x0, 0x1, 0x4, 0x5, 0x10, 0x11, 0x14, 0x15, 
  0x2, 0x3, 0x6, 0x7, 0x12, 0x13, 0x16, 0x17, 
  0x8, 0x9, 0xc, 0xd, 0x18, 0x19, 0x1c, 0x1d, 
  0xa, 0xb, 0xe, 0xf, 0x1a, 0x1b, 0x1e, 0x1f, 
  0x20, 0x21, 0x24, 0x25, 0x30, 0x31, 0x34, 0x35, 
  0x22, 0x23, 0x26, 0x27, 0x32, 0x33, 0x36, 0x37, 
  0x28, 0x29, 0x2c, 0x2d, 0x38, 0x39, 0x3c, 0x3d, 
  0x2a, 0x2b, 0x2e, 0x2f, 0x3a, 0x3b, 0x3e, 0x3f, 
  0x40, 0x41, 0x44, 0x45, 0x50, 0x51, 0x54, 0x55, 
  0x42, 0x43, 0x46, 0x47, 0x52, 0x53, 0x56, 0x57, 
  0x48, 0x49, 0x4c, 0x4d, 0x58, 0x59, 0x5c, 0x5d, 
  0x4a, 0x4b, 0x4e, 0x4f, 0x5a, 0x5b, 0x5e, 0x5f, 
  0x60, 0x61, 0x64, 0x65, 0x70, 0x71, 0x74, 0x75, 
  0x62, 0x63, 0x66, 0x67, 0x72, 0x73, 0x76, 0x77, 
  0x68, 0x69, 0x6c, 0x6d, 0x78, 0x79, 0x7c, 0x7d, 
  0x6a, 0x6b, 0x6e, 0x6f, 0x7a, 0x7b, 0x7e, 0x7f, 
  0x80, 0x81, 0x84, 0x85, 0x90, 0x91, 0x94, 0x95, 
  0x82, 0x83, 0x86, 0x87, 0x92, 0x93, 0x96, 0x97, 
  0x88, 0x89, 0x8c, 0x8d, 0x98, 0x99, 0x9c, 0x9d, 
  0x8a, 0x8b, 0x8e, 0x8f, 0x9a, 0x9b, 0x9e, 0x9f, 
  0xa0, 0xa1, 0xa4, 0xa5, 0xb0, 0xb1, 0xb4, 0xb5, 
  0xa2, 0xa3, 0xa6, 0xa7, 0xb2, 0xb3, 0xb6, 0xb7, 
  0xa8, 0xa9, 0xac, 0xad, 0xb8, 0xb9, 0xbc, 0xbd, 
  0xaa, 0xab, 0xae, 0xaf, 0xba, 0xbb, 0xbe, 0xbf, 
  0xc0, 0xc1, 0xc4, 0xc5, 0xd0, 0xd1, 0xd4, 0xd5, 
  0xc2, 0xc3, 0xc6, 0xc7, 0xd2, 0xd3, 0xd6, 0xd7, 
  0xc8, 0xc9, 0xcc, 0xcd, 0xd8, 0xd9, 0xdc, 0xdd, 
  0xca, 0xcb, 0xce, 0xcf, 0xda, 0xdb, 0xde, 0xdf, 
  0xe0, 0xe1, 0xe4, 0xe5, 0xf0, 0xf1, 0xf4, 0xf5, 
  0xe2, 0xe3, 0xe6, 0xe7, 0xf2, 0xf3, 0xf6, 0xf7, 
  0xe8, 0xe9, 0xec, 0xed, 0xf8, 0xf9, 0xfc, 0xfd, 
  0xea, 0xeb, 0xee, 0xef, 0xfa, 0xfb, 0xfe, 0xff
};

/* Braille display parameters that do not change */
#define BRLROWS 1		/* only one row on braille display */

#define MAXNRCELLS 120 /* arbitrary max for allocations */

/* We'll use 4cells as status cells, both on Voyager 44 and 70. (3cells have
   content and the fourth is blank to mark the separation.)
   NB: You can't just change this constant to vary the number of status
   cells: some key bindings for cursor routing keys assign special
   functions to routing keys over status cells.
*/
#define NRSTATCELLS 4

/* Global variables */

static int brl_fd; /* to kernel driver */

static unsigned char *prevdata, /* previous pattern displayed */
                     *dispbuf; /* buffer to prepare new pattern */
static unsigned brl_cols, /* Number of cells available for text */
                ncells; /* total number of cells including status */
static char readbrl_init; /* Flag to reinitialize readbrl function state. */
static int repeat_init_delay, repeat_inter_delay; /* key repeat rate params */


static void 
identbrl (void)
{
  LogPrint(LOG_NOTICE, VERSION);
  LogPrint(LOG_INFO, COPYRIGHT);
}

static void
initbrl (char **parameters, brldim *brl, const char *dev)
{
  brldim res;			/* return result */
  struct voyager_info vi;

  /* use user parameters */
  {
    int min = 0, max = 5000;
    if(!*parameters[PARM_REPEAT_INIT_DELAY]
       || !validateInteger(&repeat_init_delay, "Delay before repeat begins",
			   parameters[PARM_REPEAT_INIT_DELAY], &min, &max))
      repeat_init_delay = DEFAULT_REPEAT_INIT_DELAY;
    if(!*parameters[PARM_REPEAT_INTER_DELAY]
       || !validateInteger(&repeat_inter_delay, 
			   "Delay between key repeatitions",
			   parameters[PARM_REPEAT_INTER_DELAY], &min, &max))
      repeat_inter_delay = DEFAULT_REPEAT_INTER_DELAY;
  }

  res.disp = dispbuf = prevdata = NULL;

  brl_fd = open (dev, O_RDWR | O_NOCTTY);
  /* Kernel driver will block until a display is connected. */
  if (brl_fd < 0){
    LogPrint(LOG_ERR, "Open failed on device %s: %s", dev, strerror(errno));
    goto failure;
  }
  LogPrint(LOG_DEBUG,"Device %s opened", dev);

  /* Get display and USB kernel driver info */
  if(ioctl(brl_fd, VOYAGER_GET_INFO, &vi) <0) {
    LogPrint(LOG_ERR, "ioctl VOYAGER_GET_INFO failed on device %s: %s",
	     dev, strerror(errno));
    goto failure;
  }
  vi.driver_version[sizeof(vi.driver_version)-1] = 0;
  vi.driver_banner[sizeof(vi.driver_banner)-1] = 0;
  LogPrint(LOG_INFO, "Kernel driver version: %s", vi.driver_version);
  LogPrint(LOG_DEBUG, "Kernel driver identification: %s", vi.driver_banner);
  vi.hwver[sizeof(vi.hwver)-1] = 0;
  vi.fwver[sizeof(vi.fwver)-1] = 0;
  vi.serialnum[sizeof(vi.serialnum)-1] = 0;
  LogPrint(LOG_DEBUG, "Display hardware version: %u.%u",
	   vi.hwver[0],vi.hwver[1]);
  LogPrint(LOG_DEBUG, "Display firmware version: %s", vi.fwver);
  LogPrint(LOG_DEBUG, "Display serial number: %s", vi.serialnum);

  ncells = vi.display_length;
  if(ncells < NRSTATCELLS +5 || ncells > MAXNRCELLS) {
    LogPrint(LOG_ERR, "Returned unlikely number of cells %u", ncells);
    goto failure;
  }
  LogPrint(LOG_INFO,"Display has %u cells", ncells);
  if(ncells == 44)
    sethlpscr(0);
  else if(ncells == 70)
    sethlpscr(1);
  else{
    LogPrint(LOG_NOTICE, "Unexpected display length, unknown model, "
	     "using Voyager 44 help file.");
    sethlpscr(0);
  }

  brl_cols = ncells -NRSTATCELLS;

  /* cause the display to beep */
  {
    __u16 duration = 200;
    if(ioctl(brl_fd, VOYAGER_BUZZ, &duration) <0) {
      LogPrint(LOG_ERR, "ioctl VOYAGER_BUZZ: %s", strerror(errno));
      goto failure;
    }
  }

  /* readbrl will want to do non-blocking reads. */
  if(fcntl(brl_fd, F_SETFL, O_NONBLOCK) <0) {
    LogPrint(LOG_ERR, "fcntl F_SETFL O_NONBLOCK: %s", strerror(errno));
    goto failure;
  }

  if(!(dispbuf = (unsigned char *)malloc(ncells))
     || !(prevdata = (unsigned char *) malloc (ncells)))
    goto failure;

  /* dispbuf will hold the 4 status cells followed by the text cells.
     We export directly to BRLTTY only the text cells. */
  res.disp = dispbuf +NRSTATCELLS;
  res.x = brl_cols;		/* initialize size of display */
  res.y = BRLROWS;		/* always 1 */

  /* Force rewrite of display on first writebrl */
  memset(prevdata, 0xFF, ncells); /* all dots */

  readbrl_init = 1; /* init state on first readbrl */

  *brl = res;
  return;

failure:;
  LogPrint(LOG_WARNING,"Voyager driver giving up");
  closebrl(&res);
  brl->x = -1;
  return;
}

static void 
closebrl (brldim *brl)
{
  if (brl_fd >= 0)
    close(brl_fd);
  brl_fd = -1;
  free(dispbuf);
  free(prevdata);
}


static void
setbrlstat (const unsigned char *s)
{
  memcpy(dispbuf, s, NRSTATCELLS);
}


static void 
writebrl (brldim *brl)
{
  int i;
  int start, stop, len;

  if (brl->x != brl_cols || brl->y != BRLROWS
      || brl->disp != dispbuf+NRSTATCELLS)
    /* paranoia */
    return;
    
  /* If content hasn't changed, do nothing. */
  if(memcmp(prevdata, dispbuf, ncells) == 0)
    return;

  start = 0;
  stop = ncells-1;
  /* Whether or not to do partial updates... Not clear to me that it
     is worth the cycles. */
#define PARTIAL_UPDATE
#ifdef PARTIAL_UPDATE
  while(start <= stop && dispbuf[start] == prevdata[start])
    start++;
  while(stop >= start && dispbuf[stop] == prevdata[stop])
    stop--;
#endif

  len = stop-start+1;
  /* remember current content */
  memcpy(prevdata+start, dispbuf+start, len);

  /* translate to voyager dot pattern coding */
  for(i=start; i<=stop; i++)
    dispbuf[i] = brl2voyDotsTable[dispbuf[i]];

#ifdef PARTIAL_UPDATE
  lseek(brl_fd, start, SEEK_SET);
#endif
  write(brl_fd, dispbuf+start, len);
  /* The kernel driver currently never returns EAGAIN. If it did it would be
     wiser to select(). We don't bother to report failed writes because then
     we'd have to do rate limiting. Failures are caught in readbrl anyway. */
}

/* Names and codes for display keys */

/* Top round keys behind the routing keys, numbered assuming they are
   in a configuration to type Braille on. */
#define DOT1 0x01
#define DOT2 0x02
#define DOT3 0x04
#define DOT4 0x08
#define DOT5 0x10
#define DOT6 0x20
#define DOT7 0x40
#define DOT8 0x80

/* Front keys. Codes are shifted by 8bits so they can be combined with
   DOT key codes. */
/* Leftmost */
#define K_A     0x0100
/* The next one */
#define K_B     0x0200
/* The round key to the left of the central pad */
#define K_RL  0x0400
/* Up position of central pad */
#define K_UP    0x0800
#define K_DOWN  0x1000
#define K_RR 0x2000
/* Second from the right */
#define K_C     0x4000
/* Rightmost */
#define K_D     0x8000

/* Convenience */
#define KEY(v, rcmd) \
    case v: cmd = rcmd; break;

/* OK what follows is pretty hairy. I got tired of individually maintaining
   the sources and help files so here's might first attempt at "automatic"
   generation of help files. This is my first shot at it, so be kind with
   me. */
/* These macros include an ordering hint for the help file and the help
   text. GENHLP is not defined during compilation, so at compilation the
   macros are expanded in a way that just drops the help-related
   information. */
#ifndef GENHLP
#define HKEY(n, kc, hlptxt, cmd) \
    KEY(kc, cmd);
#define PHKEY(n, prfx, kc, hlptxt, cmd) \
    KEY(kc, cmd)
#define CKEY(n, kc, hlptxt, cmd) \
    KEY(kc, cmd)
/* For pairs of symmetric commands */
#define HKEY2(n, kc1,kc2, hlptxt, cmd1,cmd2) \
    KEY(kc1, cmd1); \
    KEY(kc2, cmd2);
#define PHKEY2(n, prfx, kc1,kc2, hlptxt, cmd1,cmd2) \
    KEY(kc1, cmd1); \
    KEY(kc2, cmd2);
/* Help text only, no code */
#define HLP0(n, hlptxt)
/* Watch out: HLP0 vanishes from code, but don't put a trailing semicolon! */

#else

/* To generate the help files we do gcc -DGENHLP -E (and heavily post-process
   the result). So these macros expand to something that is easily
   searched/grepped for and "easily" post-processed. */
/* Parameters are: ordering hint, keycode, help text, and command code. */
#define HKEY(n, kc, hlptxt, cmd) \
   <HLP> n: #kc : hlptxt </HLP>
/* Add a prefix parameter, will be prepended to the key code. */
#define PHKEY(n, prfx, kc, hlptxt, cmd) \
   <HLP> n: prfx #kc : hlptxt </HLP>
/* A special case of the above for chords. */
#define CKEY(n, kc, hlptxt, cmd) \
   <HLP> n: "Chord-" #kc : hlptxt </HLP>
/* Now for pairs of symmetric commands */
#define HKEY2(n, kc1,kc2, hlptxt, cmd1,cmd2) \
   <HLP> n: #kc1 / #kc2 : hlptxt </HLP>
#define PHKEY2(n, prfx, kc1,kc2, hlptxt, cmd1,cmd2) \
   <HLP> n: prfx #kc1 / #kc2 : hlptxt </HLP>
/* Just the text, no key code */
#define HLP0(n, hlptxt) \
   <HLP> n: : hlptxt </HLP>
#endif

static void
insert(unsigned char c)
/* Put a typed braille character as console input. */
{
  unsigned char buf[2];
  buf[0] = untexttrans[voy2brlDotsTable[c]];
  buf[1] = 0;
  inskey(buf);
}

static int 
readbrl (DriverCommandContext cmds)
{
  /* State: */
  /* For a key binding that triggers two cmds */
  static int pending_cmd = EOF;
  /* OR of all display keys pressed since last time all keys were released. */
  static int keystate = 0;
  /* Reference time for fastkey (typematic / key repeat) */
  static struct timeval presstime;
  /* key repeat state: 0 not a fastkey (not a key that repeats), 
     1 waiting for initial delay to begin repeating (can still be combined
     with other keys),
     2 key effect occured at least once and now waiting for next repeat,
     3 during repeat another key was pressed, so lock up and do nothing
     until keys are released. */
  static int fastkey = 0;
  /* a flag for each routing key indicating if it was pressed since last time
     all keys were released. */
  static unsigned char rtk_pressed[MAXNRCELLS];

  /* Non-static: */
  /* ordered list of pressed routing keys by number */
  unsigned char rtk_which[MAXNRCELLS];
  /* number of entries in rtk_which */
  int howmanykeys = 0;
  /* read buffer: buf[0] for keys A B C D UP DOWN RL RR, buf[1] for DOT keys,
     buf[2]-buf[7] list pressed routing keys by number, maximum 6 keys,
     list ends with 0.
     All 0s is sent when all keys released. */
  unsigned char buf[8];
  /* recognized command */
  int i, r, cmd = EOF;
  int ignore_release = 0;

  if(readbrl_init) {
    /* initialize state */
    readbrl_init = 0;
    pending_cmd = EOF;
    keystate = 0;
    fastkey = 0;
    memset(rtk_pressed, 0, sizeof(rtk_pressed));
  }

  if(pending_cmd != EOF){
    cmd = pending_cmd;
    pending_cmd = EOF;
    return cmd;
  }

  r = read(brl_fd, buf, 8);
  if(r<0) {
    if(errno == ENOLINK)
      /* Display was disconnected */
      return CMD_RESTARTBRL;
    if(errno != EAGAIN && errno != EINTR) {
      LogPrint(LOG_NOTICE,"Read error: %s", strerror(errno));
      readbrl_init = 1;
      return EOF;
      /* If some errors are discovered to occur and are fatal we should
	 return CMD_RESTARTBRL for those. For now, this shouldn't happen. */
    }
  }else if(r==0) {
    /* Should not happen */
    LogPrint(LOG_NOTICE,"Read returns EOF!");
    readbrl_init = 1;
    return EOF;
  }else if(r<8) {
    /* The driver wants and handles read requests of only and exactly 8bytes */
    LogPrint(LOG_NOTICE,"Read returns short count %d", r);
    readbrl_init = 1;
    return EOF;
  }

  if(r<0) { /* no new key */
    /* handle key repetition */
    struct timeval now;
    /* If no repeatable keys are pressed then do nothing. */
    if(!fastkey)
      return EOF;
    /* If a key repeat was interrupted by the press of another key, do
       nothing and wait for the keys to be released. */
    if(fastkey == 3)
      return EOF;
    gettimeofday(&now, NULL);
    if(!((fastkey == 1 && elapsed_msec(&presstime, &now)
	  > repeat_init_delay)
	 || (fastkey > 1 && elapsed_msec(&presstime, &now)
	     > repeat_inter_delay)))
      return EOF;
    fastkey = 2;
    memcpy(&presstime, &now, sizeof(presstime));
  }else{ /* one or more keys were pressed or released */
    /* We combine dot and front key info in keystate */
    keystate |= (buf[1]<<8) | buf[0];

    for(i=2; i<8; i++) {
      unsigned key = buf[i];
      if(!key)
	break;
      if(key < 1 || key > ncells) {
	LogPrint(LOG_NOTICE, "Invalid routing key number %u", key);
	continue;
      }
      key -= 1; /* start counting at 0 */
      rtk_pressed[key] = 1;
    }

    /* build rtk_which */
    for(howmanykeys = 0, i = 0; i < ncells; i++)
      if(rtk_pressed[i])
	rtk_which[howmanykeys++] = i;
    /* rtk_pressed[i] tells if routing key i is pressed.
       rtk_which[0] to rtk_which[howmanykeys-1] lists
       the numbers of the keys that are pressed. */

    /* Few keys trigger the repeat behavior: B, C, UP and DOWN. */
    if(howmanykeys==0 && buf[1]
       && (keystate == K_B || keystate == K_C
	   || keystate == K_UP || keystate == K_DOWN)) {
      /* Stand by to begin repeating */
      gettimeofday(&presstime, NULL);
      fastkey = 1;
      return EOF;
    }else{
      if(fastkey == 2 || fastkey == 3) {
	/* A key was repeating and its effect has occured at least once. */
	if(buf[0] || buf[1] || buf[2]) {
	  /* wait for release */
	  fastkey = 3;
	  return EOF;
	}
	/* ignore release (goto clear state) */
	ignore_release = 1;
	fastkey = 0;
      }else{
	/* If there was any key waiting to repeat it is stil within
	   repeat_init_delay timeout, so allow combination. */
	fastkey = 0;
      }
    }

    if(buf[0] || buf[1] || buf[2])
      /* wait until all keys are released to decide the effect */
      return EOF;
  }

  /* Key effect */

  if(ignore_release); /* do nothing */
  else if(howmanykeys == 0) {
    if(!(keystate & 0xFF)) {
      /* No routing keys, no dots, only front keys (or possibly a spurious
         release) */
      switch(keystate) {
	HKEY2(101, K_A,K_D, "Move backward/forward", CMD_FWINLT,CMD_FWINRT );
	HKEY2(101, K_B,K_C, "Move up/down", CMD_LNUP,CMD_LNDN );
	HKEY2(101, K_A|K_B,K_A|K_C, "Goto top-left / bottom-left", 
	      CMD_TOP_LEFT,CMD_BOT_LEFT );
	HKEY(101, K_RR, "Goto cursor", CMD_HOME );
	HKEY(101, K_RL, "Cursor tracking toggle", CMD_CSRTRK );
	HKEY2(101, K_UP,K_DOWN, "Move cursor up/down (arrow keys)",
	      VAL_PASSKEY+VPK_CURSOR_UP, VAL_PASSKEY+VPK_CURSOR_DOWN );
	HKEY(201, K_RL|K_RR, "Freeze screen (toggle)", CMD_FREEZE);
	HKEY(201, K_RL|K_UP, "Show attributes (toggle)", CMD_DISPMD);
	HKEY(201, K_RR|K_UP,
	     "Show position and status info (toggle)", CMD_INFO);
	HKEY2(501, K_RL|K_B,K_RL|K_C, 
	      "Previous/next line with different attributes",
	      CMD_ATTRUP, CMD_ATTRDN);
	HKEY2(501, K_RR|K_B,K_RR|K_C, "Previous/next different line",
	      CMD_PRDIFLN, CMD_NXDIFLN);
	HKEY2(501, K_RR|K_A,K_RR|K_D, "Previous/next non-blank window",
	      CMD_FWINLTSKIP, CMD_FWINRTSKIP);
	/* typing */
	HLP0(601, "B+C: Space (spacebar)")
	  case K_B|K_C: insert(0); cmd = EOF; break;
      };
    }else if(!(keystate &~0xFF)) {
      /* no routing keys, some dots, no front keys */
      /* This is a character typed in braille */
      insert(keystate);
      cmd = EOF;
    }else if((keystate & (K_B|K_C)) && !(keystate & 0xFF00 & ~(K_B|K_C))) {
      /* no routing keys, some dots, combined with B or C or both but
	 no other front keys. */
      /* This is a chorded character typed in braille */
      switch(keystate &0xFF) {
	CKEY(601, DOT4|DOT6, "Return", VAL_PASSKEY + VPK_RETURN );
	CKEY(601, DOT2|DOT3|DOT4|DOT5, "Tab", VAL_PASSKEY + VPK_TAB );
	CKEY(601, DOT1|DOT2, "Backspace", VAL_PASSKEY + VPK_BACKSPACE );
	CKEY(601, DOT2|DOT4|DOT6, "Escape", VAL_PASSKEY + VPK_ESCAPE );
	CKEY(601, DOT1|DOT4|DOT5, "Delete", VAL_PASSKEY + VPK_DELETE );
	CKEY(601, DOT7, "Left arrow", VAL_PASSKEY+VPK_CURSOR_LEFT);
	CKEY(601, DOT8, "Right arrow", VAL_PASSKEY+VPK_CURSOR_RIGHT);
      };
    }
  }else{ /* Some routing keys */
    if(!keystate) {
      /* routing keys, no other keys */
      if (howmanykeys == 1) {
	switch(rtk_which[0]) {
	  HLP0(201, "CRs1: Help screen (toggle)")
	    KEY( 0, CMD_HELP );
	  HLP0(201, "CRs2: Preferences menu (and again to exit)")
	    KEY( 1, CMD_PREFMENU );
	  HLP0(501, "CRs3: Go back to previously selected location "
	       "(undo cursor tracking motion).")
	    KEY( 2, CMD_BACK );
	  HLP0(301, "CRs4: Route cursor to current line")
	    KEY( 3, CMD_CSRJMP_VERT );
	default:
	  HLP0(301,"CRt#: Route cursor to cell")
	    cmd = CR_ROUTEOFFSET + rtk_which[0] -NRSTATCELLS;
	};
      }else if(howmanykeys == 3
	       && rtk_which[0] >= NRSTATCELLS
	       && rtk_which[0]+2 == rtk_which[1]){
	HLP0(405,"CRtx + CRt(x+2) + CRty : Cut text from x to y")
	  cmd = CR_BEGBLKOFFSET + rtk_which[0] -NRSTATCELLS;
           pending_cmd = CR_ENDBLKOFFSET + rtk_which[2] -NRSTATCELLS;
      }else if (howmanykeys == 2
		&& ((rtk_which[0] == 1 && rtk_which[1] == 2)
		    || (rtk_which[0] == NRSTATCELLS+1
			&& rtk_which[1] == NRSTATCELLS+2)))
	HLP0(408,"CRt2+CRt3 or CRs2+CRs3: Paste cut text")
	  cmd = CMD_PASTE;
      else if (howmanykeys == 2 && rtk_which[0] == NRSTATCELLS
	       && rtk_which[1] == NRSTATCELLS+1)
	HLP0(501,"CRt1+CRt2 / CRt<COLS-1>+CRt<COLS>: Move window left/right "
	     "one character")
	  cmd = CMD_CHRLT;
      else if (howmanykeys == 2 && rtk_which[0] == ncells-2
	       && rtk_which[1] == ncells-1)
	cmd = CMD_CHRRT;
    }
    /* routing keys and other keys */
    else if(keystate & (K_UP|K_RL|K_RR)) {
      /* Some routing keys combined with UP RL or RR (actually any key
	 combo that has at least one of those) */
      /* Treated special because we use absolute routing key numbers
	 (counting the status cell keys) */
      if(howmanykeys == 1 && keystate == K_UP)
	HLP0(201,"UP+CRa#: Switch to virtual console #")
	  cmd = CR_SWITCHVT + rtk_which[0];
      else if(howmanykeys == 2 && rtk_which[0] == 0 && rtk_which[1] == 1) {
	switch(keystate) {
	  PHKEY2(201,"CRa1+CRa2+", K_RL,K_RR, "Switch to previous/next "
		 "virtual console",
		 CMD_SWITCHVT_PREV, CMD_SWITCHVT_NEXT);
	};
      }
    }
    else if(howmanykeys == 1 && rtk_which[0] >= NRSTATCELLS) {
      /* one text routing key with some other keys */
      switch(keystate){
	PHKEY(401, "CRt#+",K_D, "Mark beginning of region to cut",
	      CR_BEGBLKOFFSET + rtk_which[0] -NRSTATCELLS);
	PHKEY(401, "CRt#+",K_A, "Mark end of region and cut", 
	      CR_ENDBLKOFFSET + rtk_which[0] -NRSTATCELLS);
	PHKEY2(501, "CRt#+",K_B,K_C, "Move to previous/next line indented "
	       "no more than #",
	       CR_PRINDENT + rtk_which[0] -NRSTATCELLS,
	       CR_NXINDENT + rtk_which[0] -NRSTATCELLS);
      };
    }else if(howmanykeys == 2 && rtk_which[0] == NRSTATCELLS
	     && rtk_which[1] == NRSTATCELLS+1) {
      /* text routing keys 1 and 2, with some other keys */
      switch(keystate){
	PHKEY2(501, "CRt1+CRt2+",K_B,K_C, "Move to previous/next "
	       "paragraph (blank line separation)",
	       CMD_PRBLNKLN, CMD_NXBLNKLN);
	PHKEY2(501, "CRt1+CRt2+",K_A,K_D, "Search screen "
	       "backward/forward for cut text",
	       CMD_PRSEARCH, CMD_NXSEARCH);
      };
    }
  }

  if(!fastkey) {
    /* keys were released, clear state */
    keystate = 0;
    fastkey = 0;
    memset(rtk_pressed, 0, ncells);
  }

  return cmd;
}