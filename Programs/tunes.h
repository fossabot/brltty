/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2014 by The BRLTTY Developers.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. Please see the file LICENSE-GPL for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

#ifndef BRLTTY_INCLUDED_TUNES
#define BRLTTY_INCLUDED_TUNES

#include "tune_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
  unsigned char note;     /* standard MIDI values (0 means silence) */
                          /* 1 through 127 are semitones, 60 is middle C */
  unsigned char duration; /* milliseconds (0 means stop) */
} TuneElement;

#define TUNE_NOTE(duration,note) {note, duration}
#define TUNE_REST(duration) TUNE_NOTE(duration, 0)
#define TUNE_STOP() TUNE_REST(0)

typedef struct {
  const TuneElement *elements;
  const char *message;
  uint16_t tactile;
} TuneDefinition;

#define TUNE_TACTILE(duration,pattern) (((duration) << 8) | (pattern))

extern const TuneDefinition tune_braille_on;
extern const TuneDefinition tune_braille_off;
extern const TuneDefinition tune_command_done;
extern const TuneDefinition tune_command_rejected;
extern const TuneDefinition tune_mark_set;
extern const TuneDefinition tune_clipboard_begin;
extern const TuneDefinition tune_clipboard_end;
extern const TuneDefinition tune_no_change;
extern const TuneDefinition tune_toggle_on;
extern const TuneDefinition tune_toggle_off;
extern const TuneDefinition tune_cursor_linked;
extern const TuneDefinition tune_cursor_unlinked;
extern const TuneDefinition tune_screen_frozen;
extern const TuneDefinition tune_screen_unfrozen;
extern const TuneDefinition tune_wrap_down;
extern const TuneDefinition tune_wrap_up;
extern const TuneDefinition tune_skip_first;
extern const TuneDefinition tune_skip;
extern const TuneDefinition tune_skip_more;
extern const TuneDefinition tune_bounce;
extern const TuneDefinition tune_routing_started;
extern const TuneDefinition tune_routing_succeeded;
extern const TuneDefinition tune_routing_failed;
extern const TuneDefinition tune_freeze_reminder;

extern void suppressTuneDeviceOpenErrors (void);
extern int setTuneDevice (TuneDevice device);
extern void closeTunes (void);
extern void playTune (const TuneDefinition *tune);

extern int showDotPattern (unsigned char dots, unsigned char duration);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BRLTTY_INCLUDED_TUNES */