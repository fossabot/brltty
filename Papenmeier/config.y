%{
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

/*
 * Support for all Papenmeier Terminal + config file
 *   Heimo.Sch�n <heimo.schoen@gmx.at>
 *   August H�randl <august.hoerandl@gmx.at>
 *
 * Papenmeier/read_config.y
 *  read (scan + interpret) the configuration file - included by brl.c
 *  this file can be used as a standalone test programm - see
 *   Makefile for details
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "../brl.h"
#include "brl-cfg.h"

static int yylex(void);
static int yyerror(char*);    
static int yyparse();

char* nameval = 0;
int numval, keyindex, cmdval; 

 int numkeys = 0;
 int keys[KEYMAX];

int linenumber = 1;

FILE* configfile = NULL;

 /* --------------------------------------------------------------- */
 /* some functions set_*(): set the big table with the values read  */
 
 /* global var: the current entry (index) */
 static int set_current = -1;
 /* the number of modifiers - last entry used */
 static int mod_current = 0;
 /* the number of defined commands - last entry use*/
 static int cmd_current = 0;

 static inline void set_assert(int cond)
   {
     if (! cond)
       yyerror("Range Check");
   } 

 // #define set_assert assert

 static inline void set_ident(int num)
  {
    set_current++;
    set_assert(set_current < num_terminals);
    mod_current = -1;
    cmd_current = -1;
    pm_terminals[set_current].ident = num;

    pm_terminals[set_current].x = 0;
    pm_terminals[set_current].y = 1;

    pm_terminals[set_current].statcells = 0;
    pm_terminals[set_current].frontkeys = 0;
    pm_terminals[set_current].haseasybar = 0;
  }

 static inline void set_name(char* nameval)
  {
    set_assert(set_current >= 0);
    strncpy(pm_terminals[set_current].name, nameval,
    	    sizeof(pm_terminals[set_current].name));
  }


 static inline void set_help(char* name)
   {
     set_assert(set_current >= 0);
     strncpy(pm_terminals[set_current].helpfile, name,
     	     sizeof(pm_terminals[set_current].helpfile));
   }

 static inline void set_size(int code)
  {
    set_assert(set_current >= 0);
    pm_terminals[set_current].x = code;
  }

 static inline void set_statcells(int code)
  {
    set_assert(set_current >= 0);
    pm_terminals[set_current].statcells = code;
  }

 static inline void set_frontkeys(int code)
  {
    set_assert(set_current >= 0);
    pm_terminals[set_current].frontkeys = code;
  }

 static inline void set_haseasybar(int code)
  {
    set_assert(set_current >= 0);
    pm_terminals[set_current].haseasybar = code;
  }

 static inline void set_showstat(int pos, int code)
  {
    set_assert(set_current >= 0);
    set_assert(0 < pos && pos <= STATMAX);
    pm_terminals[set_current].statshow[pos-1] = code;
  }

 static inline void set_modifier(int code)
  {
    set_assert(set_current >= 0);
    mod_current ++;
    set_assert(0 <= mod_current && mod_current < MODMAX);
    pm_terminals[set_current].modifiers[mod_current] = code;
  }

 static inline void set_keycode(int code, int numkeys, int keys[])
  {
    int i, j, chk;
    commands* curr;
    set_assert(set_current >= 0);
    cmd_current++;
    set_assert(0 <= cmd_current && cmd_current < CMDMAX);
    curr = &(pm_terminals[set_current].cmds[cmd_current]);
    curr->code = code;
    curr->keycode=keys[0];
    chk = 1;
    for (i=1; i < numkeys; i++)
      for(j=0; j <= mod_current; j++)
	if (keys[i] == pm_terminals[set_current].modifiers[j]) {
	  curr->modifiers |= (1 << j);
	  chk++;
	}
    set_assert(chk == numkeys);
  }

%}

%start input

%token NAME NUM STRING IDENT
%token IS HELPFILE AND
%token MODIFIER

%token CUTBEG CUTEND

%token STATCELLS FRONTKEYS EASYBAR SIZE
%token KEY STAT FRONT KEYCODE STATCODE
%token STATCODE HORIZ FLAG
%token EASY EASYCODE
%token SWITCH NUMBER
%%

input:    /* empty */
       | input inputline
       ;

inputline:  '\n'
       | error '\n'                 { yyerrok;  }
       | IDENT eq NUM '\n'          { set_ident(numval); }
       | NAME eq STRING '\n'        { set_name(nameval); }
       | HELPFILE eq STRING '\n'    { set_help(nameval); }
       | SIZE eq NUM '\n'           { set_size(numval); }
       | STATCELLS eq NUM '\n'      { set_statcells(numval); }
       | FRONTKEYS eq NUM '\n'      { set_frontkeys(numval); }
       | EASYBAR '\n'               { set_haseasybar(1); }
       | EASYBAR eq NUM '\n'        { set_haseasybar(numval != 0); }

       | statdef eq statdisp '\n';  { set_showstat(keyindex, numval);  }
       | MODIFIER eq anykey '\n';   { set_modifier(keyindex); }
       | keycode eq modifiers '\n'; { set_keycode(cmdval, numkeys, keys); }
       ;

eq:    '='
       | IS
       | /* empty */
       ;

and:   '+'
       | AND
       | /* empty */
       ;

statdef: STAT NUM { keyindex=numval;  } 
       ;

keycode: KEYCODE { cmdval=numval; numkeys = 0; }
       ;

statdisp: STATCODE            {  }
        | HORIZ STATCODE      { numval += OFFS_HORIZ; }
        | FLAG STATCODE       { numval += OFFS_FLAG; }
        | NUMBER STATCODE     { numval += OFFS_NUMBER; }
        ;

anykey:   STAT NUM     { keyindex= OFFS_STAT + numval; } 
        | FRONT NUM    { keyindex= OFFS_FRONT + numval; } 
        | EASY EASYCODE { keyindex= OFFS_EASY + numval; } 
        | SWITCH NUM   { keyindex=OFFS_SWITCH + numval; }
        ; 

modifiers: modifier
         | modifier modifiers
         ;

modifier :  /* empty */
         | and anykey { keys[numkeys++] = keyindex; } 
         ;

%%

/* --------------------------------------------------------------- */
/* all the keywords */
/* the commands CMD_* in ../brl.h are autogenerated - see Makefile */

struct init_v
{
  char*  sname;			/* symbol (lowercase) */
  int token;			/* yacc token */
  int val;			/* key value */
};

static struct init_v symbols[]= {
  { "helpfile",    HELPFILE ,0 },
  { "is",          IS, 0 },
  { "and",         AND, 0 },

  { "identification", IDENT, 0 },
  { "identity",    IDENT, 0 },
  { "terminal",    NAME, 0 },
  { "type",        NAME, 0 },
  { "typ",         NAME, 0 },
  { "key",         KEY, 0 },

  { "modifier",    MODIFIER, 0},

  { "displaysize", SIZE, 0 },
  { "statuscells", STATCELLS, 0 },
  { "frontkeys",   FRONTKEYS, 0 },
  { "haseasybar",  EASYBAR, 0 },
  { "easybar",     EASYBAR, 0 },

  { "status",      STAT, 0 },
  { "front",       FRONT, 0 },
  { "easy",        EASY, 0 },
  { "switch",      SWITCH, 0 },

  { "s",           STAT, 0 },
  { "f",           FRONT, 0 },
  { "e",           EASY, 0 },

  { "cut_begin",   KEYCODE, CMD_CUT_BEG },
  { "cut_end",     KEYCODE, CMD_CUT_END },

  { "horiz",       HORIZ, 0 },
  { "flag",        FLAG, 0 },
  { "number",      NUMBER, 0 },

  { "currentline", STATCODE, STAT_current },
  { "line",        STATCODE, STAT_current },
  { "current",     STATCODE, STAT_current },
  { "cursorrow",   STATCODE, STAT_row },
  { "row",         STATCODE, STAT_row },
  { "cursorcol",   STATCODE, STAT_col },
  { "col",         STATCODE, STAT_col },
  { "column",      STATCODE, STAT_col },
  { "tracking",    STATCODE, STAT_tracking },
  { "dispmode",    STATCODE, STAT_dispmode },
  { "mode",        STATCODE, STAT_dispmode },
  { "frz",         STATCODE, STAT_frozen },
  { "frozen",      STATCODE, STAT_frozen },
  { "visible",     STATCODE, STAT_visible },
  { "size",        STATCODE, STAT_size },
  { "blink",       STATCODE, STAT_blink },
  { "capitalblink",STATCODE, STAT_capitalblink },
  { "six",         STATCODE, STAT_dots },
  { "eight",       STATCODE, STAT_dots },
  { "dots",        STATCODE, STAT_dots },
  { "sound",       STATCODE,STAT_sound },
  { "skip",        STATCODE, STAT_skip },
  { "underline",   STATCODE,STAT_underline },
  { "blinkattr",   STATCODE, STAT_blinkattr },

  { "left",        EASYCODE, EASY_LE },
  { "left1",       EASYCODE, EASY_LE },
  { "left2",       EASYCODE, EASY_LE2 },
  { "up",          EASYCODE, EASY_UP },
  { "up1",         EASYCODE, EASY_UP },
  { "up2",         EASYCODE, EASY_UP2 },
  { "right",       EASYCODE, EASY_RI },
  { "right1",      EASYCODE, EASY_RI },
  { "right2",      EASYCODE, EASY_RI2 },
  { "down",        EASYCODE, EASY_DO },
  { "down1",       EASYCODE, EASY_DO },
  { "down2",       EASYCODE, EASY_DO2 },
#include "cmd.h"
  { NULL, 0, 0 }
};

/* --------------------------------------------------------------- */
/* all the help */
/* the commands CMD_* in ../brl.h are autogenerated - see Makefile */

struct help_v
{
  int    cmd;			/* cmd */
  char*  txt;			/* help text */
};

static struct help_v hlptxt[]= {
#include "hlp.h"
  { 0, NULL }

};


/* --------------------------------------------------------------- */

int yylex ()
{
  int c;
  static char symbuf[40] = { 0 };
  const int length = sizeof(symbuf)/sizeof(symbuf[0]);

  /* Ignore whitespace, get first nonwhite character.  */
  while ((c = getc(configfile)) == ' ' || c == '\t')
    ;

  if (c == EOF)
    return 0;

  if (c == '#') {		/* comment to end of line */
    do {
      c = getc(configfile);
    } while (c != '\n' && c != EOF);
    linenumber ++;
    return '\n';
  }

  /* Char starts a number => parse the number. */
  if (c == '.' || isdigit (c)) {
    ungetc (c, configfile);
    fscanf (configfile, "%d", &numval);
    return NUM;
  }

  if (c == '"') {		/* string */
    int i=0;
    symbuf[0] = '\0';
    c = getc(configfile);
    while(c !='"' && c != EOF) {
      /* If buffer is full */
      if (i == length)
	break;
      /* Add this character to the buffer. */
      symbuf[i++] = c;
      c = getc(configfile);
    }
    symbuf[i] = 0;
    nameval = symbuf;
    return STRING;
  }

  /* Char starts an identifier => read the name. */
  if (isalpha (c) || c=='_') {
    int i = 0;
    do {
      /* If buffer is full */
      if (i == length)
	break;
      
      /* Add this character to the buffer. */
      symbuf[i++] = c;
      c = getc(configfile);
    } while (c != EOF && (isalnum (c) || c=='_'));
     
    ungetc (c, configfile);
    symbuf[i] = 0;
    for(i=0;symbols[i].sname; i++)
      if(strcasecmp(symbuf,symbols[i].sname) == 0) {
	numval = symbols[i].val;
	return symbols[i].token;
      }
    /* unused feature */
    nameval = symbuf;
    return NAME;
  }
     
  /* Any other character is a token by itself. */
  if (c == '\n')
    linenumber++;

  return c;
}