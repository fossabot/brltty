/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2005 by The BRLTTY Team. All rights reserved.
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

#ifndef BRLTTY_INCLUDED_SCR_DRIVER
#define BRLTTY_INCLUDED_SCR_DRIVER

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* this header file is used to create the driver structure
 * for a dynamically loadable screen driver.
 * SCRNAME, SCRCODE, and SCRCOMMENT must be defined - see driver make file
 */

#include "scr.h"
#include "scr_real.h"

/* Routines provided by this screen driver. */
static void scr_initialize (MainScreen *main);

#ifdef SCRPARMS
  static const char *const scr_parameters[] = {SCRPARMS, NULL};
#endif /* SCRPARMS */

#ifndef SCRSYMBOL
#  define SCRSYMBOL CONCATENATE(scr_driver_,SCRCODE)
#endif /* SCRSYMBOL */

#ifndef SCRCONST
#  define SCRCONST const
#endif /* SCRCONST */

extern SCRCONST ScreenDriver SCRSYMBOL;
SCRCONST ScreenDriver SCRSYMBOL = {
  STRINGIFY(SCRNAME),
  STRINGIFY(SCRCODE),
  SCRCOMMENT,
  __DATE__,
  __TIME__,

#ifdef SCRPARMS
  scr_parameters,
#else /* SCRPARMS */
  NULL,
#endif /* SCRPARMS */

  scr_initialize
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BRLTTY_INCLUDED_SCR_DRIVER */
