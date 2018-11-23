/*-------------------------------------------------------------------
 * File name: tui.c
 * Author: Seree Rakwong
 * Date: 17-SEP-18
 *-----------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#ifdef __USE_CURSES__
#include <curses.h>
#elif (defined __USE_QIO__ && defined __VMS__)
#include <descrip.h>
#include <iodef.h>
#include <lib$routines.h>
#include <ssdef.h>
#include <starlet.h>
#include <stdio.h>
#include <string.h>
#include <stsdef.h>

#include <qio_init.h>
#elif defined __USE_WIN32__
#include <windows.h>
#endif

#include "tui.h"

struct _TWNDPROCSTRUCT
{
  TWNDPROC     wndproc;
  TLPCSTR       clsname;
  struct _TWNDPROCSTRUCT *prev;
  struct _TWNDPROCSTRUCT *next;
};
typedef struct _TWNDPROCSTRUCT twndproc_t;

struct _TUIMSGQUEUESTRUCT
{
  TWND     wnd;
  TUINT     msg;
  TWPARAM   wparam;
  TLPARAM   lparam;

  struct _TUIMSGQUEUESTRUCT *next;
};
typedef struct _TUIMSGQUEUESTRUCT tmsgq_t;

struct _TUITHEMEITEMSTRUCT
{
  TINT   id;
  /*TDWORD color;*/
};
typedef struct _TUITHEMEITEMSTRUCT tthemeitem_t;

struct _TUITHEMESTRUCT
{
  tthemeitem_t* theme;
};
typedef struct _TUITHEMESTRUCT ttheme_t;

#ifdef __USE_QIO__
/* I/O status block */
/*
struct _TIOSBSTRUCT
{
  TUINT16 iostat;
  TUINT16 ttiolen;
  TUINT32 dev_info;
};
typedef struct _TIOSBSTRUCT TIOSB;
*/
TIOSB   ttiosb;
TUINT16 ttchan;
TQIO    ttqio;
#endif

struct _TUIENVSTRUCT
{
  /* doubly linked-list */
  twndproc_t*      firstproc;
  twndproc_t*      lastproc;

  /* que */
  tmsgq_t*         headq;
  tmsgq_t*         tailq;

  /* doubly linked-list */
  TWND             firstwnd;
  TWND             lastwnd;
  TWND             activewnd;

  TLONG             quitcode;
  TLONG             exitcode;

  TLONG             nextmove; /* to identify the next control moved */
  TLONG             prevmove; /* to identify the prev control moved */
  TLONG             notifykey;
  
  _TDC             dc;
  /* last dialog returned message */
  TINT              themeid;
  ttheme_t         themes[THEME_LAST];
};

struct _TUIWINDOWSTRUCT
{
  TLPCSTR            clsname;
  TTCHAR              wndname[TUI_MAX_WNDTEXT+1];
  TDWORD             style;
  TDWORD             exstyle;
  TINT               x;
  TINT               y;
  TINT               cols;
  TINT               lines;
  TWND              parent;
  TUINT              id;
  TLPVOID            param;
  TWNDPROC          wndproc;
  TLONG              enable;
  TLONG              visible;
  VALIDATEPROC      validateproc;
  TTCHAR              infotext[TUI_MAX_WNDTEXT+1];
  /* curses lib */
#ifdef __USE_CURSES__
  WINDOW*           win;
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  TQIO              win;
#elif defined __USE_WIN32__
  HANDLE            win; /* input */
  HANDLE            wout; /* output */
#endif
  TDWORD            attrs;
  /* last dialog returned message */
  /*TUINT             dlgmsgid;*/
  
  /* links */
  TWND              prevwnd;
  TWND              nextwnd;
  TWND              firstchild;
  TWND              lastchild;
  TWND              activechild;
};

/*-------------------------------------------------------------------
 * global storages
 *-----------------------------------------------------------------*/

tthemeitem_t STANDARD_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

tthemeitem_t LHS_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

tthemeitem_t ASP_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

tthemeitem_t CNS_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

tthemeitem_t KTZ_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

tthemeitem_t YUANTA_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

tthemeitem_t KSS_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

tthemeitem_t PST_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

tthemeitem_t MBKET_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

tthemeitem_t AIRA_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

tthemeitem_t ASL_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

tthemeitem_t MERCHANT_THEME[] =
{
  { WHITE_BLACK }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  { BLUE_YELLOW }, /* COLOR_HIGHLIGHTED */
  /* last color */
  { 0 }  
};

/* global environment variable */
TENV genvptr = 0;


/*-------------------------------------------------------------------
 * TINTernal functions
 *-----------------------------------------------------------------*/
twndproc_t* _TuiFindWndProc(TLPCSTR clsname);
TWND _TuiCreateWnd(
  TLPCSTR   clsname,
  TLPCSTR   wndname,
  TDWORD    style,
  TINT      y,
  TINT      x,
  TINT      lines,
  TINT      cols,
  TWND     parent,
  TUINT     id,
  TLPVOID   param
);
TWND _TuiCreateWndEx(
  TLPCSTR   clsname,
  TLPCSTR   wndname,
  TDWORD    style,
  TDWORD    exstyle,
  TINT      y,
  TINT      x,
  TINT      lines,
  TINT      cols,
  TWND     parent,
  TUINT     id,
  TLPCSTR   infotext,
  TLPVOID   param
);
TVOID _TuiDestroyWnd(TWND wnd);
TLONG _TuiInvalidateWnd(TWND wnd);
TLONG _TuiDequeMsg();
TLONG _TuiRemoveAllMsgs();
TLONG _TuiAlignmentPrint(TLPSTR out, TLPCSTR in, TLONG limit, TINT align);
TVOID _TuiInitColors();
TVOID _TuiInitThemes();
TVOID _TuiInitPair(TINT idx, TDWORD fg, TDWORD bg, TDWORD rfg, TDWORD rbg);
TDWORD _TuiColorPair(TINT idx);
TDWORD _TuiReverseColorPair(TINT idx);
TVOID _TuiClearScreen(TDC dc);


TLONG STATICPROC(TWND, TUINT, TWPARAM, TLPARAM);
TLONG EDITBOXPROC(TWND, TUINT, TWPARAM, TLPARAM);
TLONG LISTBOXPROC(TWND, TUINT, TWPARAM, TLPARAM);
TLONG BUTTONPROC(TWND, TUINT, TWPARAM, TLPARAM);
TLONG LISTCTRLPROC(TWND, TUINT, TWPARAM, TLPARAM);
TLONG LISTPAGECTRLPROC(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam);
/*-------------------------------------------------------------------
 * functions
 *-----------------------------------------------------------------*/


TINT   TuiGetTheme()
{
  TENV env = genvptr;
  return env->themeid;
}

TINT   TuiSetTheme(TINT themeid)
{
  TENV env = genvptr;
  TINT oldtheme = env->themeid;
  if (themeid >= THEME_STANDARD && themeid < THEME_LAST)
  {
    env->themeid = themeid;
  }
  return oldtheme;
}

TVOID _TuiInitThemes()
{
  TENV env = genvptr;
  env->themes[THEME_STANDARD].theme = STANDARD_THEME;
  env->themes[THEME_LHS].theme      = &LHS_THEME[0];
  env->themes[THEME_ASP].theme      = &ASP_THEME[0];
  env->themes[THEME_CNS].theme      = &CNS_THEME[0];
  env->themes[THEME_KTZ].theme      = &KTZ_THEME[0];
  env->themes[THEME_YUANTA].theme   = &YUANTA_THEME[0];
  env->themes[THEME_KSS].theme      = &KSS_THEME[0];
  env->themes[THEME_PST].theme      = &PST_THEME[0];
  env->themes[THEME_MBKET].theme    = &MBKET_THEME[0];
  env->themes[THEME_AIRA].theme     = &AIRA_THEME[0];
  env->themes[THEME_ASL].theme      = &ASL_THEME[0];
  env->themes[THEME_MERCHANT].theme = &MERCHANT_THEME[0];
}

struct _COLORPAIRSTRUCT
{
    TDWORD   fg;
    TDWORD   bg;
    TDWORD   rfg;
    TDWORD   rbg;
};
typedef struct _COLORPAIRSTRUCT COLORPAIR;
COLORPAIR gcolors[COLOR_MAX];
TVOID _TuiInitPair(TINT idx, TDWORD fg, TDWORD bg, TDWORD rfg, TDWORD rbg)
{
    gcolors[idx].fg = fg;
    gcolors[idx].bg = bg;
    gcolors[idx].rfg = rfg;
    gcolors[idx].rbg = rbg;
}
TDWORD _TuiColorPair(TINT idx)
{
    if (idx < 0 || idx >= COLOR_MAX)
    {
        idx = 0;
    }
    return (gcolors[idx].fg | gcolors[idx].bg);
}

TDWORD _TuiReverseColorPair(TINT idx)
{
    if (idx < 0 || idx >= COLOR_MAX)
    {
        idx = 0;
    }
    return (gcolors[idx].rfg | gcolors[idx].rbg);
}

TVOID _TuiClearScreen(TDC dc)
{
#if defined __USE_WIN32__

    COORD coordScreen = { 0, 0 };    /* home for the cursor */
    TDWORD cTTCHARsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    TDWORD dwConSize;

    /*Get the number of TTCHARacter cells in the current buffer. */

    if( !GetConsoleScreenBufferInfo( dc->wout, &csbi ))
    {
        return;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    /* Fill the entire screen with blanks.*/

    if (!FillConsoleOutputCharacter(dc->wout,        /* Handle to console screen buffer */
        (TTCHAR) ' ',      /* TTCHARacter to write to the buffer */
        dwConSize,       /* Number of cells to write */
        coordScreen,     /* Coordinates of first cell */
        &cTTCHARsWritten ))/* Receive number of TTCHARacters written */
    {
        return;
    }

    /* Get the current text attribute.*/

    if (!GetConsoleScreenBufferInfo(dc->wout, &csbi))
    {
        return;
    }

    /* Set the buffer's attributes accordingly. */

    if (!FillConsoleOutputAttribute(dc->wout,         /* Handle to console screen buffer */
        csbi.wAttributes, /* TTCHARacter attributes to use */
        dwConSize,        /* Number of cells to set attribute */
        coordScreen,      /* Coordinates of first cell */
        &cTTCHARsWritten )) /* Receive number of TTCHARacters written */
    {
        return;
    }

    /* Put the cursor at its home coordinates.*/

    SetConsoleCursorPosition(dc->wout, coordScreen);
#endif
}

#if defined __USE_WIN32__
#define FOREGROUND_BLACK        (0)
#define FOREGROUND_YELLOW       (FOREGROUND_GREEN | FOREGROUND_RED)
#define FOREGROUND_MAGENTA      (FOREGROUND_BLUE  | FOREGROUND_RED)
#define FOREGROUND_CYAN         (FOREGROUND_GREEN | FOREGROUND_BLUE)
#define FOREGROUND_WHITE        (FOREGROUND_GREEN | FOREGROUND_RED | BACKGROUND_BLUE)
#define BACKGROUND_BLACK        (0)
#define BACKGROUND_YELLOW       (BACKGROUND_GREEN | BACKGROUND_RED)
#define BACKGROUND_MAGENTA      (BACKGROUND_BLUE  | BACKGROUND_RED)
#define BACKGROUND_CYAN         (BACKGROUND_GREEN | BACKGROUND_BLUE)
#define BACKGROUND_WHITE        (BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_BLUE)
#endif

TVOID _TuiInitColors()
{
#ifdef __USE_CURSES__
  init_pair(BLACK_RED,      COLOR_BLACK,   COLOR_RED);
  init_pair(BLACK_GREEN,    COLOR_BLACK,   COLOR_GREEN);
  init_pair(BLACK_YELLOW,   COLOR_BLACK,   COLOR_YELLOW);
  init_pair(BLACK_BLUE,     COLOR_BLACK,   COLOR_BLUE);
  init_pair(BLACK_MAGENTA,  COLOR_BLACK,   COLOR_MAGENTA);
  init_pair(BLACK_CYAN,     COLOR_BLACK,   COLOR_CYAN); 
  init_pair(BLACK_WHITE,    COLOR_BLACK,   COLOR_WHITE);
  init_pair(RED_BLACK,      COLOR_RED,     COLOR_BLACK);
  init_pair(RED_GREEN,      COLOR_RED,     COLOR_GREEN);
  init_pair(RED_YELLOW,     COLOR_RED,     COLOR_YELLOW);
  init_pair(RED_BLUE,       COLOR_RED,     COLOR_BLUE);
  init_pair(RED_MAGENTA,    COLOR_RED,     COLOR_MAGENTA);
  init_pair(RED_CYAN,       COLOR_RED,     COLOR_CYAN); 
  init_pair(RED_WHITE,      COLOR_RED,     COLOR_WHITE);
  init_pair(GREEN_BLACK,    COLOR_GREEN,   COLOR_BLACK);
  init_pair(GREEN_RED,      COLOR_GREEN,   COLOR_RED);
  init_pair(GREEN_YELLOW,   COLOR_GREEN,   COLOR_YELLOW);
  init_pair(GREEN_BLUE,     COLOR_GREEN,   COLOR_BLUE);
  init_pair(GREEN_MAGENTA,  COLOR_GREEN,   COLOR_MAGENTA);
  init_pair(GREEN_CYAN,     COLOR_GREEN,   COLOR_CYAN); 
  init_pair(GREEN_WHITE,    COLOR_GREEN,   COLOR_WHITE);
  init_pair(YELLOW_BLACK,   COLOR_YELLOW,  COLOR_BLACK);
  init_pair(YELLOW_RED,     COLOR_YELLOW,  COLOR_RED);
  init_pair(YELLOW_GREEN,   COLOR_YELLOW,  COLOR_GREEN);
  init_pair(YELLOW_BLUE,    COLOR_YELLOW,  COLOR_BLUE);
  init_pair(YELLOW_MAGENTA, COLOR_YELLOW,  COLOR_MAGENTA);
  init_pair(YELLOW_CYAN,    COLOR_YELLOW,  COLOR_CYAN); 
  init_pair(YELLOW_WHITE,   COLOR_YELLOW,  COLOR_WHITE);
  init_pair(BLUE_BLACK,     COLOR_BLUE,    COLOR_BLACK);
  init_pair(BLUE_RED,       COLOR_BLUE,    COLOR_RED);
  init_pair(BLUE_GREEN,     COLOR_BLUE,    COLOR_GREEN);
  init_pair(BLUE_YELLOW,    COLOR_BLUE,    COLOR_YELLOW);
  init_pair(BLUE_MAGENTA,   COLOR_BLUE,    COLOR_MAGENTA);
  init_pair(BLUE_CYAN,      COLOR_BLUE,    COLOR_CYAN); 
  init_pair(BLUE_WHITE,     COLOR_BLUE,    COLOR_WHITE);
  init_pair(MAGENTA_BLACK,  COLOR_MAGENTA, COLOR_BLACK);
  init_pair(MAGENTA_RED,    COLOR_MAGENTA, COLOR_RED);
  init_pair(MAGENTA_GREEN,  COLOR_MAGENTA, COLOR_GREEN);
  init_pair(MAGENTA_YELLOW, COLOR_MAGENTA, COLOR_YELLOW);
  init_pair(MAGENTA_BLUE,   COLOR_MAGENTA, COLOR_BLUE);
  init_pair(MAGENTA_CYAN,   COLOR_MAGENTA, COLOR_CYAN); 
  init_pair(MAGENTA_WHITE,  COLOR_MAGENTA, COLOR_WHITE);
  init_pair(CYAN_BLACK,     COLOR_CYAN,    COLOR_BLACK);
  init_pair(CYAN_RED,       COLOR_CYAN,    COLOR_RED);
  init_pair(CYAN_GREEN,     COLOR_CYAN,    COLOR_GREEN);
  init_pair(CYAN_YELLOW,    COLOR_CYAN,    COLOR_YELLOW);
  init_pair(CYAN_BLUE,      COLOR_CYAN,    COLOR_BLUE);
  init_pair(CYAN_MAGENTA,   COLOR_CYAN,    COLOR_MAGENTA);
  init_pair(CYAN_WHITE,     COLOR_CYAN,    COLOR_WHITE);
  init_pair(WHITE_BLACK,    COLOR_WHITE,   COLOR_BLACK);
  init_pair(WHITE_RED,      COLOR_WHITE,   COLOR_RED);
  init_pair(WHITE_GREEN,    COLOR_WHITE,   COLOR_GREEN);
  init_pair(WHITE_YELLOW,   COLOR_WHITE,   COLOR_YELLOW);
  init_pair(WHITE_BLUE,     COLOR_WHITE,   COLOR_BLUE);
  init_pair(WHITE_MAGENTA,  COLOR_WHITE,   COLOR_MAGENTA);
  init_pair(WHITE_CYAN,     COLOR_WHITE,   COLOR_CYAN);
#elif defined __USE_WIN32__
  _TuiInitPair(BLACK_RED,       0, BACKGROUND_RED, FOREGROUND_RED, 0);
  _TuiInitPair(BLACK_GREEN,     0, BACKGROUND_GREEN, FOREGROUND_GREEN, 0);
  _TuiInitPair(BLACK_YELLOW,    0, BACKGROUND_YELLOW, FOREGROUND_YELLOW, 0);
  _TuiInitPair(BLACK_BLUE,      0, BACKGROUND_BLUE, FOREGROUND_BLUE, 0);
  _TuiInitPair(BLACK_MAGENTA,   0, BACKGROUND_MAGENTA, FOREGROUND_MAGENTA, 0);
  _TuiInitPair(BLACK_CYAN,      0, BACKGROUND_CYAN, FOREGROUND_CYAN, 0);
  _TuiInitPair(BLACK_WHITE,     0, BACKGROUND_WHITE, FOREGROUND_WHITE, 0);
  _TuiInitPair(RED_BLACK,       FOREGROUND_RED, 0, 0, BACKGROUND_RED);
  _TuiInitPair(RED_GREEN,       FOREGROUND_RED, BACKGROUND_GREEN, FOREGROUND_GREEN, BACKGROUND_RED);
  _TuiInitPair(RED_YELLOW,      FOREGROUND_RED, BACKGROUND_YELLOW, FOREGROUND_YELLOW, BACKGROUND_RED);
  _TuiInitPair(RED_BLUE,        FOREGROUND_RED, BACKGROUND_BLUE, FOREGROUND_BLUE, BACKGROUND_RED);
  _TuiInitPair(RED_MAGENTA,     FOREGROUND_RED, BACKGROUND_MAGENTA, FOREGROUND_MAGENTA, BACKGROUND_RED);
  _TuiInitPair(RED_CYAN,        FOREGROUND_RED, BACKGROUND_CYAN, FOREGROUND_CYAN, BACKGROUND_RED);
  _TuiInitPair(RED_WHITE,       FOREGROUND_RED, BACKGROUND_WHITE, FOREGROUND_WHITE, BACKGROUND_RED);
  _TuiInitPair(GREEN_BLACK,     FOREGROUND_GREEN, 0, 0, BACKGROUND_GREEN);
  _TuiInitPair(GREEN_RED,       FOREGROUND_GREEN, BACKGROUND_RED, FOREGROUND_RED, BACKGROUND_GREEN);
  _TuiInitPair(GREEN_YELLOW,    FOREGROUND_GREEN, BACKGROUND_YELLOW, FOREGROUND_YELLOW, BACKGROUND_GREEN);
  _TuiInitPair(GREEN_BLUE,      FOREGROUND_GREEN, BACKGROUND_BLUE, FOREGROUND_BLUE, BACKGROUND_GREEN);
  _TuiInitPair(GREEN_MAGENTA,   FOREGROUND_GREEN, BACKGROUND_MAGENTA, FOREGROUND_MAGENTA, BACKGROUND_GREEN);
  _TuiInitPair(GREEN_CYAN,      FOREGROUND_GREEN, BACKGROUND_CYAN, FOREGROUND_CYAN, BACKGROUND_GREEN);
  _TuiInitPair(GREEN_WHITE,     FOREGROUND_GREEN, BACKGROUND_WHITE, FOREGROUND_WHITE, BACKGROUND_GREEN);
  _TuiInitPair(YELLOW_BLACK,    FOREGROUND_YELLOW, 0, 0, BACKGROUND_YELLOW);
  _TuiInitPair(YELLOW_RED,      FOREGROUND_YELLOW, BACKGROUND_RED, FOREGROUND_RED, BACKGROUND_YELLOW);
  _TuiInitPair(YELLOW_GREEN,    FOREGROUND_YELLOW, BACKGROUND_GREEN, FOREGROUND_GREEN, BACKGROUND_YELLOW);
  _TuiInitPair(YELLOW_BLUE,     FOREGROUND_YELLOW, BACKGROUND_BLUE, FOREGROUND_BLUE, BACKGROUND_YELLOW);
  _TuiInitPair(YELLOW_MAGENTA,  FOREGROUND_YELLOW, BACKGROUND_MAGENTA, FOREGROUND_MAGENTA, BACKGROUND_YELLOW);
  _TuiInitPair(YELLOW_CYAN,     FOREGROUND_YELLOW, BACKGROUND_CYAN, FOREGROUND_CYAN, BACKGROUND_YELLOW);
  _TuiInitPair(YELLOW_WHITE,    FOREGROUND_YELLOW, BACKGROUND_WHITE, FOREGROUND_WHITE, BACKGROUND_YELLOW);
  _TuiInitPair(BLUE_BLACK,      FOREGROUND_BLUE, 0, 0, BACKGROUND_BLUE);
  _TuiInitPair(BLUE_RED,        FOREGROUND_BLUE, BACKGROUND_RED, FOREGROUND_RED, BACKGROUND_BLUE);
  _TuiInitPair(BLUE_GREEN,      FOREGROUND_BLUE, BACKGROUND_GREEN, FOREGROUND_GREEN, BACKGROUND_BLUE);
  _TuiInitPair(BLUE_YELLOW,     FOREGROUND_BLUE, BACKGROUND_YELLOW, FOREGROUND_YELLOW, BACKGROUND_BLUE);
  _TuiInitPair(BLUE_MAGENTA,    FOREGROUND_BLUE, BACKGROUND_MAGENTA, FOREGROUND_MAGENTA, BACKGROUND_BLUE);
  _TuiInitPair(BLUE_CYAN,       FOREGROUND_BLUE, BACKGROUND_CYAN, FOREGROUND_CYAN, BACKGROUND_BLUE);
  _TuiInitPair(BLUE_WHITE,      FOREGROUND_BLUE, BACKGROUND_WHITE, FOREGROUND_WHITE, BACKGROUND_BLUE);
  _TuiInitPair(MAGENTA_BLACK,   FOREGROUND_MAGENTA, 0, 0, BACKGROUND_MAGENTA);
  _TuiInitPair(MAGENTA_RED,     FOREGROUND_MAGENTA, BACKGROUND_RED, FOREGROUND_RED, BACKGROUND_MAGENTA);
  _TuiInitPair(MAGENTA_GREEN,   FOREGROUND_MAGENTA, BACKGROUND_GREEN, FOREGROUND_GREEN, BACKGROUND_MAGENTA);
  _TuiInitPair(MAGENTA_YELLOW,  FOREGROUND_MAGENTA, FOREGROUND_YELLOW, FOREGROUND_YELLOW, FOREGROUND_MAGENTA);
  _TuiInitPair(MAGENTA_BLUE,    FOREGROUND_MAGENTA, BACKGROUND_BLUE, FOREGROUND_BLUE, BACKGROUND_MAGENTA);
  _TuiInitPair(MAGENTA_CYAN,    FOREGROUND_MAGENTA, BACKGROUND_CYAN, FOREGROUND_CYAN, BACKGROUND_MAGENTA);
  _TuiInitPair(MAGENTA_WHITE,   FOREGROUND_MAGENTA, BACKGROUND_WHITE, FOREGROUND_WHITE, BACKGROUND_MAGENTA);
  _TuiInitPair(CYAN_BLACK,      FOREGROUND_CYAN, 0, 0, BACKGROUND_CYAN);
  _TuiInitPair(CYAN_RED,        FOREGROUND_CYAN, BACKGROUND_RED, FOREGROUND_RED, BACKGROUND_CYAN);
  _TuiInitPair(CYAN_GREEN,      FOREGROUND_CYAN, BACKGROUND_GREEN, FOREGROUND_GREEN, BACKGROUND_CYAN);
  _TuiInitPair(CYAN_YELLOW,     FOREGROUND_CYAN, FOREGROUND_YELLOW, FOREGROUND_YELLOW, FOREGROUND_CYAN);
  _TuiInitPair(CYAN_BLUE,       FOREGROUND_CYAN, BACKGROUND_BLUE, FOREGROUND_BLUE, BACKGROUND_CYAN);
  _TuiInitPair(CYAN_MAGENTA,    FOREGROUND_CYAN, BACKGROUND_MAGENTA, FOREGROUND_MAGENTA, BACKGROUND_CYAN);
  _TuiInitPair(CYAN_WHITE,      FOREGROUND_CYAN, BACKGROUND_WHITE, FOREGROUND_WHITE, BACKGROUND_CYAN);
  _TuiInitPair(WHITE_BLACK,     FOREGROUND_WHITE, 0, 0, BACKGROUND_WHITE);
  _TuiInitPair(WHITE_RED,       FOREGROUND_WHITE, BACKGROUND_RED, FOREGROUND_RED, BACKGROUND_WHITE);
  _TuiInitPair(WHITE_GREEN,     FOREGROUND_WHITE, BACKGROUND_GREEN, FOREGROUND_GREEN, BACKGROUND_WHITE);
  _TuiInitPair(WHITE_YELLOW,    FOREGROUND_WHITE, FOREGROUND_YELLOW, FOREGROUND_YELLOW, FOREGROUND_WHITE);
  _TuiInitPair(WHITE_BLUE,      FOREGROUND_WHITE, BACKGROUND_BLUE, FOREGROUND_BLUE, BACKGROUND_WHITE);
  _TuiInitPair(WHITE_MAGENTA,   FOREGROUND_WHITE, BACKGROUND_MAGENTA, FOREGROUND_MAGENTA, BACKGROUND_WHITE);
  _TuiInitPair(WHITE_CYAN,      FOREGROUND_WHITE, BACKGROUND_CYAN, FOREGROUND_CYAN, BACKGROUND_WHITE);
#endif
}

TLONG TuiStartup()
{
#ifdef __USE_QIO__ 
  TUINT32 status;
  $DESCRIPTOR(ttname,"SYS$INPUT");
#endif
  TENV env = (TENV)malloc(sizeof(_TENV));
  if (env)
  {
    genvptr = env;
    memset(env, 0, sizeof(_TENV));

    /* register standard controls */
    TuiRegisterCls(TSTATIC,   STATICPROC);
    TuiRegisterCls(TEDITBOX,  EDITBOXPROC);
    TuiRegisterCls(TLISTBOX,  LISTBOXPROC);
    TuiRegisterCls(TBUTTON,   BUTTONPROC);
    TuiRegisterCls(TLISTCTRL, LISTCTRLPROC);
    TuiRegisterCls(TLISTPAGECTRL, LISTPAGECTRLPROC);

    env->themeid = THEME_STANDARD;
    /* device context */
#ifdef __USE_CURSES__
    /* curses lib */
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    
    start_color();
    
    env->dc.win    = stdscr;
    env->nextmove  = TVK_ENTER;
    env->prevmove  = KEY_BTAB;
    env->notifykey = TVK_ENTER;
    
#elif defined __USE_QIO__
    /* QIO for VMS implement here */
    /* Assign a channel */
    status = SYS$ASSIGN(
                &ttname,      /* devnam - device number */
                &ttchan,      /* chan - channel number  */
                0, 0, 0);
    if (!$VMS_STATUS_SUCCESS(status))
    {
      LIB$SIGNAL( status );
      free(env);
      genvptr = 0;
    }
    else
    {
      ttqio.ttchan = ttchan;
      env->dc.win  = &ttqio;
    }
#elif defined __USE_WIN32__
    env->dc.win = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(env->dc.win, ENABLE_WINDOW_INPUT);

    env->dc.wout = GetStdHandle(STD_OUTPUT_HANDLE);
    _TuiClearScreen(&env->dc);
#endif
  }
    
  _TuiInitColors();
  _TuiInitThemes();

  return (genvptr ? TUI_OK : TUI_ERROR);
}

TVOID TuiShutdown()
{
  TWND wnd = genvptr->firstwnd;
  TWND temp = 0;
#ifdef __USE_QIO__ 
  TUINT32 status;
#endif
  
  while (wnd)
  {
    temp = wnd;
    if (wnd)
    {
        wnd = wnd->nextwnd;
    }
    TuiDestroyWnd(temp);
  }
  free(genvptr);
  genvptr = 0;
  
#ifdef __USE_CURSES__
  echo();
  nocbreak();
  endwin();
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  /* Deassign the channel */
  status = SYS$DASSGN( ttchan ); /* chan - channel */
  if (!$VMS_STATUS_SUCCESS( status ))
          LIB$SIGNAL( status );
#elif defined __USE_WIN32__
#endif
}

twndproc_t* _TuiFindWndProc(TLPCSTR clsname)
{
  TENV env = genvptr;
  twndproc_t* proc = env->firstproc;
  while (proc)
  {
    if (strcmp(proc->clsname, clsname) == 0)
    {
      break;
    }
    proc = proc->next;
  }
  return proc;
}

TENV TuiGetEnv()
{
  return genvptr;
}

TLONG TuiSetPrevMove(TLONG prevmove)
{
  TENV env = genvptr;
  TLONG oldmove = env->prevmove;
  env->prevmove = prevmove;
  return oldmove;
}

TLONG TuiSetNextMove(TLONG nextmove)
{
  TENV env = genvptr;
  TLONG oldmove = env->nextmove;
  env->nextmove = nextmove;
  return oldmove;
}

TDC TuiGetDC(TWND wnd)
{
  static _TDC dc;
#ifdef __USE_CURSES__
  if (wnd)
  {
    dc.win = wnd->win;
  }
  else
  {
    dc.win = stdscr;
  }
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  if (wnd)
  {
    dc.win = wnd->win;
  }
  else
  {
    dc = genvptr->dc;
  }
#elif defined __USE_WIN32__
  if (wnd)
  {
      dc.win = wnd->win;
      dc.wout = wnd->wout;
  }
  else
  {
      dc = genvptr->dc;
  }
#endif
  return (TDC)&dc;
}

TLONG TuiRegisterCls(TLPCSTR clsname, TWNDPROC wndproc)
{
  TENV env = genvptr;
  twndproc_t* proc = _TuiFindWndProc(clsname);

  if (!proc)
  {
    proc = (twndproc_t*)malloc(sizeof(twndproc_t));
    proc->clsname = clsname;
    proc->wndproc = wndproc;
    proc->prev    = 0;
    proc->next    = 0;

    if (env->firstproc)
    {
      proc->prev = env->lastproc;
      env->lastproc->next = proc;
      env->lastproc = proc;
    }
    else
    {
      env->firstproc = env->lastproc = proc;
    }
  }
  return TUI_OK;
}

TLONG TuiMoveWnd(TWND wnd, TINT y, TINT x, TINT lines, TINT cols)
{
  TLONG rc = TUI_OK;
  wnd->y = y;
  wnd->x = x;
  wnd->lines = lines;
  wnd->cols  = cols;
  return rc;
}

TWND _TuiCreateWndEx(
  TLPCSTR   clsname,
  TLPCSTR   wndname,
  TDWORD    style,
  TDWORD    exstyle,
  TINT      y,
  TINT      x,
  TINT      lines,
  TINT      cols,
  TWND     parent,
  TUINT     id,
  TLPCSTR   infotext,
  TLPVOID   param
)
{
  TENV env = genvptr;
  TWND wnd = 0;
  TLONG len = 0;
  twndproc_t* proc = _TuiFindWndProc(clsname);

  if ((style & TWS_WINDOW) &&
      (style & TWS_CHILD))
  {
    /* not allow creating both styles at a time */
    return wnd;
  }
  else if (style & TWS_CHILD && !parent)
  {
    /* not allow child witwout parent */
    return wnd;
  }

  if (proc)
  {
    wnd = (TWND)malloc(sizeof(_TWND));
    if (wnd)
    {
      memset(wnd, 0, sizeof(_TWND));
      wnd->clsname = clsname;
      len = strlen(wndname);
      if (wndname && len > 0)
      {
        if (len > TUI_MAX_WNDTEXT)
        {
          strncpy(wnd->wndname, wndname, TUI_MAX_WNDTEXT);
        }
        else
        {
          strcpy(wnd->wndname, wndname);
        }
      }
      wnd->style   = style;
      wnd->parent  = parent;
      wnd->id      = id;
      wnd->wndproc = proc->wndproc;
      wnd->enable  = (style & TWS_DISABLED ? 0 : 1);
      wnd->visible = (style & TWS_VISIBLE ? 1 : 0);
      wnd->validateproc = 0;
      wnd->exstyle = exstyle;
      /* info text */
      len = strlen(infotext);
      if (infotext && len > 0)
      {
        if (len > TUI_MAX_WNDTEXT)
        {
          strncpy(wnd->infotext, infotext, TUI_MAX_WNDTEXT);
        }
        else
        {
          strcpy(wnd->infotext, infotext);
        }
      }
      
      /* curses */
#ifdef __USE_CURSES__
      wnd->win     = stdscr;
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
      wnd->win     = &ttqio;
#elif defined __USE_WIN32__
      wnd->win = genvptr->dc.win;
      wnd->wout = genvptr->dc.wout;
#endif
      wnd->attrs   = 0;

      /* make link */
      if (style & TWS_WINDOW)
      {
        if (env->firstwnd)
        {
          wnd->prevwnd = env->lastwnd;
          env->lastwnd->nextwnd = wnd;
          env->lastwnd = wnd;
        }
        else
        {
          env->firstwnd = env->lastwnd = wnd;
        }
        env->activewnd = wnd;
        /*env->dlgmsgid  = MB_INVALID;*/
#ifdef __USE_CURSES__
        wnd->y       = (y > 0 && y < LINES ? y : 0);
        wnd->x       = (x > 0 && x < COLS  ? x : 0);
        wnd->lines   = (lines > 0 && lines < LINES ? lines : LINES);
        wnd->cols    = (cols  > 0 && cols  < COLS  ? cols  : COLS);
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
        wnd->y       = y;
        wnd->x       = x;
        wnd->lines   = lines;
        wnd->cols    = cols;
#elif defined __USE_WIN32__
        wnd->y = y;
        wnd->x = x;
        wnd->lines = lines;
        wnd->cols = cols;
#endif
        
      }
      else if (style & TWS_CHILD)
      {
        if (parent->firstchild)
        {
          wnd->prevwnd = parent->lastchild;
          parent->lastchild->nextwnd = wnd;
          parent->lastchild = wnd;
        }
        else
        {
          parent->firstchild = parent->lastchild = wnd;
          parent->activechild = wnd;
        }
        /* re-size window rectangle */
        wnd->y       = (parent->y + y);
        wnd->x       = (parent->x + x);
        wnd->lines   = (lines > parent->lines ? parent->lines : lines);
        wnd->cols    = (cols  > parent->cols  ? parent->cols  : cols);
        if (TuiGetWndStyle(parent) & TWS_BORDER)
        {
          wnd->lines--;
          wnd->cols--;
        }
        /* send message */
        TuiSendMsg(wnd, TWM_ERASEBK, (TWPARAM)TuiGetDC(wnd), 0);
        TuiSendMsg(wnd, TWM_CREATE, 0, 0);
      }
    }
  }
  return wnd;
}

TWND _TuiCreateWnd(
  TLPCSTR   clsname,
  TLPCSTR   wndname,
  TDWORD    style,
  TINT      y,
  TINT      x,
  TINT      lines,
  TINT      cols,
  TWND     parent,
  TUINT     id,
  TLPVOID   param
)
{
  TENV env = genvptr;
  TWND wnd = 0;
  TLONG len = 0;
  twndproc_t* proc = _TuiFindWndProc(clsname);

  if ((style & TWS_WINDOW) &&
      (style & TWS_CHILD))
  {
    /* not allow creating both styles at a time */
    return wnd;
  }
  else if (style & TWS_CHILD && !parent)
  {
    /* not allow child witwout parent */
    return wnd;
  }

  if (proc)
  {
    wnd = (TWND)malloc(sizeof(_TWND));
    if (wnd)
    {
      memset(wnd, 0, sizeof(_TWND));
      wnd->clsname = clsname;
      len = strlen(wndname);
      if (wndname && len > 0)
      {
        if (len > TUI_MAX_WNDTEXT)
        {
          strncpy(wnd->wndname, wndname, TUI_MAX_WNDTEXT);
        }
        else
        {
          strcpy(wnd->wndname, wndname);
        }
      }
      wnd->style   = style;
      wnd->parent  = parent;
      wnd->id      = id;
      wnd->wndproc = proc->wndproc;
      wnd->enable  = (style & TWS_DISABLED ? 0 : 1);
      wnd->visible = (style & TWS_VISIBLE ? 1 : 0);
      wnd->validateproc = 0;
      /* curses */
#ifdef __USE_CURSES__
      wnd->win     = stdscr;
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
      wnd->win     = &ttqio;
#elif defined __USE_WIN32__
      wnd->win = genvptr->dc.win;
      wnd->wout = genvptr->dc.wout;
#endif
      wnd->attrs   = 0;

      /* make link */
      if (style & TWS_WINDOW)
      {
        if (env->firstwnd)
        {
          wnd->prevwnd = env->lastwnd;
          env->lastwnd->nextwnd = wnd;
          env->lastwnd = wnd;
        }
        else
        {
          env->firstwnd = env->lastwnd = wnd;
        }
        env->activewnd = wnd;
        /*env->dlgmsgid  = MB_INVALID;*/
#ifdef __USE_CURSES__
        wnd->y       = (y > 0 && y < LINES ? y : 0);
        wnd->x       = (x > 0 && x < COLS  ? x : 0);
        wnd->lines   = (lines > 0 && lines < LINES ? lines : LINES);
        wnd->cols    = (cols  > 0 && cols  < COLS  ? cols  : COLS);
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
        wnd->y       = y;
        wnd->x       = x;
        wnd->lines   = lines;
        wnd->cols    = cols;
#elif defined __USE_WIN32__
        wnd->y = y;
        wnd->x = x;
        wnd->lines = lines;
        wnd->cols = cols;
#endif
        
      }
      else if (style & TWS_CHILD)
      {
        if (parent->firstchild)
        {
          wnd->prevwnd = parent->lastchild;
          parent->lastchild->nextwnd = wnd;
          parent->lastchild = wnd;
        }
        else
        {
          parent->firstchild = parent->lastchild = wnd;
          parent->activechild = wnd;
        }
        /* re-size window rectangle */
        wnd->y       = (parent->y + y);
        wnd->x       = (parent->x + x);
        wnd->lines   = (lines > parent->lines ? parent->lines : lines);
        wnd->cols    = (cols  > parent->cols  ? parent->cols  : cols);
        if (TuiGetWndStyle(parent) & TWS_BORDER)
        {
          wnd->lines--;
          wnd->cols--;
        }
        /* send message */
        TuiSendMsg(wnd, TWM_ERASEBK, (TWPARAM)TuiGetDC(wnd), 0);
        TuiSendMsg(wnd, TWM_CREATE, 0, 0);
      }
    }
  }
  return wnd;
}

TLONG TuiFrameWndCreatePage(
  TWND      wnd,
  WNDTEMPL* templs,
  TLPVOID    param
)
{
  TLONG rc = TUI_CONTINUE;
  TINT i;
  TWND child = 0;
  TWND firstchild = 0;
  TWND lastchild  = 0;
  TDWORD style = 0;
  _TINITPAGE page;
  
  if (wnd && templs)
  {
    for (i = 0; templs[i].clsname; ++i)
    {
      style  = templs[i].style;
      style &= ~TWS_WINDOW; /* others must be child */
      style |= TWS_CHILD;   /* others must be child */
      child  = _TuiCreateWnd(
                   templs[i].clsname,
                   templs[i].text,
                   style,
                   templs[i].y,
                   templs[i].x,
                   templs[i].lines,
                   templs[i].cols,
                   wnd,
                   templs[i].id,
                   0
                 );
      if (child)
      {
        TuiSetWndValidateProc(child, (VALIDATEPROC)templs[i].validateproc);
        if (!firstchild)
        {
          firstchild = child;
        }
        lastchild = child;
      }
      else
      {
        rc = TUI_ERROR;
        break;
      }
    } /* create children */

    if (rc != TUI_ERROR)
    {
      page.firstchild = firstchild;
      page.lastchild  = lastchild;
      
      rc = TuiSendMsg(wnd, TWM_INITPAGE, (TWPARAM)&page, (TLPARAM)param);
      if (rc != TUI_CONTINUE)
      {
        rc = TUI_ERROR;
      }
    }
    /* if errors found */
    if (TUI_ERROR == rc)
    {
      /* delete children created successfully */
      while (lastchild && lastchild != firstchild)
      {
        child = lastchild;
        _TuiDestroyWnd(child);
        lastchild = lastchild->prevwnd;
      }
      if (firstchild)
      {
        _TuiDestroyWnd(firstchild);
      }
    }
  } /* templs is allocated */
  return rc;
}

TWND TuiCreateFrameWnd(
  TLPCSTR   clsname,
  TLPCSTR   wndname,
  TDWORD    style,
  TINT      y,
  TINT      x,
  TINT      lines,
  TINT      cols,
  WNDTEMPL* templs,
  TLPVOID    param
)
{
  TLONG rc  = TUI_CONTINUE;
  TWND wnd = 0;
  TWND child = 0;
  TINT i = 0;
  TDWORD childstyle = 0;
  
  wnd = TuiCreateWnd(
          clsname,
          wndname,
          style,
          y,
          x,
          lines,
          cols,
          0,
          0,
          param);
  if (wnd && templs)
  {
    for (i = 0; templs[i].clsname; ++i)
    {
      childstyle  = templs[i].style;
      childstyle &= ~TWS_WINDOW; /* others must be child */
      childstyle |= TWS_CHILD;   /* others must be child */
      child  = _TuiCreateWnd(
                   templs[i].clsname,
                   templs[i].text,
                   childstyle,
                   templs[i].y,
                   templs[i].x,
                   templs[i].lines,
                   templs[i].cols,
                   wnd,
                   templs[i].id,
                   0
                 );
      if (!child)
      {
        _TuiDestroyWnd(wnd);
        wnd = 0;
      }
      else
      {
        TuiSetWndValidateProc(child, (VALIDATEPROC)templs[i].validateproc);
      }
    } /* create children */

    if (wnd)
    {
      TuiSendMsg(wnd, TWM_ERASEBK, (TWPARAM)TuiGetDC(wnd), 0);
      rc = TuiSendMsg(wnd, TWM_INITDIALOG, (TWPARAM)0, (TLPARAM)param);
      if (rc != TUI_CONTINUE)
      {
        _TuiDestroyWnd(wnd);
        wnd = 0;
      }
      child = TuiGetFirstActiveChildWnd(wnd);
      if (child)
      {
        wnd->activechild = child;
        TuiSetFocus(child);
      }
    }
  } /* templs is allocated */
  return wnd;
}


TLONG TuiFrameWndCreatePageEx(
  TWND      wnd,
  FRMWNDTEMPL* templs,
  TLPVOID    param
)
{
  TLONG rc = TUI_CONTINUE;
  TINT i;
  TWND child = 0;
  TWND firstchild = 0;
  TWND lastchild  = 0;
  TDWORD style = 0;
  _TINITPAGE page;
  
  if (wnd && templs)
  {
    for (i = 0; templs[i].clsname; ++i)
    {
      style  = templs[i].style;
      style &= ~TWS_WINDOW; /* others must be child */
      style |= TWS_CHILD;   /* others must be child */
      child  = _TuiCreateWndEx(
                   templs[i].clsname,
                   templs[i].text,
                   style,
                   templs[i].exstyle,
                   templs[i].y,
                   templs[i].x,
                   templs[i].lines,
                   templs[i].cols,
                   wnd,
                   templs[i].id,
                   templs[i].infotext,
                   0
                 );
      if (child)
      {
        TuiSetWndValidateProc(child, (VALIDATEPROC)templs[i].validateproc);
        if (!firstchild)
        {
          firstchild = child;
        }
        lastchild = child;
      }
      else
      {
        rc = TUI_ERROR;
        break;
      }
    } /* create children */

    if (rc != TUI_ERROR)
    {
      page.firstchild = firstchild;
      page.lastchild  = lastchild;
      
      rc = TuiSendMsg(wnd, TWM_INITPAGE, (TWPARAM)&page, (TLPARAM)param);
      if (rc != TUI_CONTINUE)
      {
        rc = TUI_ERROR;
      }
    }
    /* if errors found */
    if (TUI_ERROR == rc)
    {
      /* delete children created successfully */
      while (lastchild && lastchild != firstchild)
      {
        child = lastchild;
        _TuiDestroyWnd(child);
        lastchild = lastchild->prevwnd;
      }
      if (firstchild)
      {
        _TuiDestroyWnd(firstchild);
      }
    }
  } /* templs is allocated */
  return rc;
}

TWND TuiCreateFrameWndEx(
  TLPCSTR   clsname,
  TLPCSTR   wndname,
  TDWORD    style,
  TDWORD    exstyle,
  TINT      y,
  TINT      x,
  TINT      lines,
  TINT      cols,
  FRMWNDTEMPL* templs,
  TLPVOID    param
)
{
  TLONG rc  = TUI_CONTINUE;
  TWND wnd = 0;
  TWND child = 0;
  TINT i = 0;
  TDWORD childstyle = 0;
  
  wnd = TuiCreateWndEx(
          clsname,
          wndname,
          style,
          exstyle,
          y,
          x,
          lines,
          cols,
          0,
          0,
          "",
          param);
  if (wnd && templs)
  {
    for (i = 0; templs[i].clsname; ++i)
    {
      childstyle  = templs[i].style;
      childstyle &= ~TWS_WINDOW; /* others must be child */
      childstyle |= TWS_CHILD;   /* others must be child */
      child  = _TuiCreateWndEx(
                   templs[i].clsname,
                   templs[i].text,
                   childstyle,
                   templs[i].exstyle,
                   templs[i].y,
                   templs[i].x,
                   templs[i].lines,
                   templs[i].cols,
                   wnd,
                   templs[i].id,
                   templs[i].infotext,
                   0
                 );
      if (!child)
      {
        _TuiDestroyWnd(wnd);
        wnd = 0;
      }
      else
      {
        TuiSetWndValidateProc(child, (VALIDATEPROC)templs[i].validateproc);
      }
    } /* create children */

    if (wnd)
    {
      TuiSendMsg(wnd, TWM_ERASEBK, (TWPARAM)TuiGetDC(wnd), 0);
      rc = TuiSendMsg(wnd, TWM_INITDIALOG, (TWPARAM)0, (TLPARAM)param);
      if (rc != TUI_CONTINUE)
      {
        _TuiDestroyWnd(wnd);
        wnd = 0;
      }
      child = TuiGetFirstActiveChildWnd(wnd);
      if (child)
      {
        wnd->activechild = child;
        TuiSetFocus(child);
      }
    }
  } /* templs is allocated */
  return wnd;
}


TWND TuiCreateWndEx(
  TLPCSTR    clsname,
  TLPCSTR    wndname,
  TDWORD     style,
  TDWORD     exstyle,
  TINT       y,
  TINT       x,
  TINT       lines,
  TINT       cols,
  TWND      parent,
  TINT       id,
  TLPCSTR    infotext,
  TLPVOID    param
)
{
  TLONG rc  = TUI_CONTINUE;
  TWND child = 0;
  TWND wnd = _TuiCreateWndEx(
                clsname,
                wndname,
                style,
                exstyle,
                y,
                x,
                lines,
                cols,
                parent,
                id,
                infotext,
                param
             );
  if (wnd)
  {
    TuiSendMsg(wnd, TWM_ERASEBK, (TWPARAM)TuiGetDC(wnd), 0);
    rc = TuiSendMsg(wnd, TWM_CREATE, 0, (TLPARAM)param);
    if (rc != TUI_CONTINUE)
    {
      if (wnd)
      {
        _TuiDestroyWnd(wnd);
      }
      wnd = 0;
    }
    child = TuiGetFirstActiveChildWnd(wnd);
    if (child)
    {
      wnd->activechild = child;
      TuiSetFocus(child);
    }
  }
  return wnd;
}

TWND TuiCreateWnd(
  TLPCSTR   clsname,
  TLPCSTR   wndname,
  TDWORD    style,
  TINT      y,
  TINT      x,
  TINT      lines,
  TINT      cols,
  TWND     parent,
  TINT      id,
  TLPVOID   param
)
{
  TLONG rc  = TUI_CONTINUE;
  TWND child = 0;
  TWND wnd = _TuiCreateWnd(
               clsname,
               wndname,
               style,
               y,
               x,
               lines,
               cols,
               parent,
               id,
               param
             );
  if (wnd)
  {
    TuiSendMsg(wnd, TWM_ERASEBK, (TWPARAM)TuiGetDC(wnd), 0);
    rc = TuiSendMsg(wnd, TWM_CREATE, 0, (TLPARAM)param);
    if (rc != TUI_CONTINUE)
    {
      if (wnd)
      {
        _TuiDestroyWnd(wnd);
      }
      wnd = 0;
    }
    child = TuiGetFirstActiveChildWnd(wnd);
    if (child)
    {
      wnd->activechild = child;
      TuiSetFocus(child);
    }
  }
  return wnd;
}

TWND TuiCreateWndTempl(
  WNDTEMPL* templs,
  TLPVOID    param
)
{
  TLONG rc  = TUI_CONTINUE;
  TWND wnd = 0;
  TWND child = 0;
  TINT i = 0;
  TDWORD style = 0;

  if (templs)
  {
    style  = templs[i].style;
    style &= ~TWS_CHILD; /* first must be window */
    style |= TWS_WINDOW; /* first must be window */
    wnd = _TuiCreateWnd(
               templs[i].clsname,
               templs[i].text,
               style,
               templs[i].y,
               templs[i].x,
               templs[i].lines,
               templs[i].cols,
               0,
               0,
               param
             );
    if (wnd)
    {
      for (i = 1; templs[i].clsname; ++i)
      {
        style  = templs[i].style;
        style &= ~TWS_WINDOW; /* others must be child */
        style |= TWS_CHILD;   /* others must be child */
        child  = _TuiCreateWnd(
                   templs[i].clsname,
                   templs[i].text,
                   style,
                   templs[i].y,
                   templs[i].x,
                   templs[i].lines,
                   templs[i].cols,
                   wnd,
                   templs[i].id,
                   0
                 );
        if (!child)
        {
          _TuiDestroyWnd(wnd);
          wnd = 0;
        }
        else
        {
          TuiSetWndValidateProc(child, (VALIDATEPROC)templs[i].validateproc);
        }
      } /* create children */
    } /* wnd is created successfully */

    if (wnd)
    {
      TuiSendMsg(wnd, TWM_ERASEBK, (TWPARAM)TuiGetDC(wnd), 0);
      rc = TuiSendMsg(wnd, TWM_INITDIALOG, (TWPARAM)0, (TLPARAM)0);
      if (rc != TUI_CONTINUE)
      {
        _TuiDestroyWnd(wnd);
        wnd = 0;
      }
      child = TuiGetFirstActiveChildWnd(wnd);
      if (child)
      {
        wnd->activechild = child;
        TuiSetFocus(child);
      }
    }
  } /* templs is allocated */
  return wnd;
}

TVOID _TuiDestroyWnd(TWND wnd)
{
  TWND child = (wnd ? wnd->firstchild : 0);
  TWND temp  = child;
  while (child)
  {
    child = child->nextwnd;
    temp->nextwnd = temp->prevwnd = 0;
    free(temp);
    temp  = child;
  }
  wnd->nextwnd = wnd->prevwnd = 0;
  free(wnd);
}

TVOID TuiDestroyWnd(TWND wnd)
{
  TENV env = genvptr;
  /* remove the link if it is TWS_WINDOW */
  if (wnd->style & TWS_WINDOW)
  {
    env->lastwnd = wnd->prevwnd;
    if (env->lastwnd)
    {
      env->lastwnd->nextwnd = 0;
    }
    env->activewnd = env->lastwnd;
  }
  /* save the last message id */
  
  TuiSendMsg(wnd, TWM_DESTROY, 0, 0);
  _TuiDestroyWnd(wnd);
  
  if (env->activewnd)
  {
    TuiInvalidateWnd(env->activewnd);
  }
}

TLONG TuiPostMsg(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam)
{
  TLONG size = 0;
  if (wnd)
  {
    tmsgq_t* msgq = (tmsgq_t*)malloc(sizeof(tmsgq_t));
    if (msgq)
    {
      TENV env = genvptr;

      memset(msgq, 0, sizeof(tmsgq_t));
      msgq->wnd = wnd;
      msgq->msg = msg;
      if (TWM_NOTIFY == msg)
      {
        TNMHDR* nmhdr = (TNMHDR*)malloc(TUI_MAX_NMHDR);
        memset(nmhdr, 0, TUI_MAX_NMHDR);
        memcpy(nmhdr, (TNMHDR*)lparam, TUI_MAX_NMHDR);
        
        msgq->lparam = (TLPARAM)nmhdr;
      }
      else if (TWM_SETCURSOR == msg)
      {
        TPOS* pos = 0;
        size = sizeof(TPOS);

        pos = (TPOS*)malloc(size);
        memset(pos, 0, size);
        memcpy(pos, (TPOS*)lparam, size);

        msgq->lparam = (TLPARAM)pos;
      }
      else
      {
        msgq->lparam = lparam;
      }
      msgq->wparam = wparam;
      
      if (env->tailq)
      {
        env->tailq->next = msgq;
        env->tailq = msgq;
      }
      else
      {
        env->headq = env->tailq = msgq;
      }
      return TUI_OK;    
    }
  }
  return TUI_ERROR;
}

TLONG TuiSendMsg(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam)
{
  if (wnd)
  {
    return wnd->wndproc(wnd, msg, wparam, lparam);
  }
  return TUI_ERROR;
}

TLONG _TuiInvalidateWnd(TWND wnd)
{
  TWND child = wnd->firstchild;

  TuiSendMsg(wnd, TWM_ERASEBK, (TWPARAM)TuiGetDC(wnd), 0);
  TuiSendMsg(wnd, TWM_PAINT, 0, 0);
  while (child)
  {
    TuiSendMsg(child, TWM_ERASEBK, (TWPARAM)TuiGetDC(wnd), 0);
    TuiSendMsg(child, TWM_PAINT, 0, 0);
    child = child->nextwnd;
  }
  
  return TUI_OK;
}

TLONG TuiInvalidateWnd(TWND wnd)
{
  TLONG rc = _TuiInvalidateWnd(wnd);
  TWND activechild = TuiGetActiveChildWnd(wnd);
  if (!activechild)
  {
    activechild = TuiGetFirstActiveChildWnd(wnd);
  }
  if (activechild)
  {
    TuiSetFocus(activechild);
    TuiMoveYX(TuiGetDC(activechild), activechild->y, activechild->x);
  }
  return rc;
}

TLONG TuiShowWnd(TWND wnd, TLONG show)
{
  return TuiSendMsg(wnd, TWM_SHOW, (TWPARAM)show, 0);
}

TLONG TuiEnableWnd(TWND wnd, TLONG enable)
{
  return TuiSendMsg(wnd, TWM_ENABLE, (TWPARAM)enable, 0);
}

TWND TuiGetFirstActiveChildWnd(TWND wnd)
{
  if (wnd)
  {
    TWND activechild = wnd->activechild;
    TWND firstwnd = 0;

    wnd->activechild = wnd->lastchild;
    firstwnd = TuiGetNextActiveChildWnd(wnd);

    wnd->activechild = activechild;
    return firstwnd;
  }
  return 0;
}

TWND TuiGetLastActiveChildWnd(TWND wnd)
{
  if (wnd)
  {
    TWND activechild = wnd->activechild;
    TWND lastwnd = 0;

    wnd->activechild = wnd->firstchild;
    lastwnd = TuiGetPrevActiveChildWnd(wnd);

    wnd->activechild = activechild;
    return lastwnd;
  }
  return 0;
}

TWND TuiGetPrevActiveChildWnd(TWND wnd)
{
  TWND activechild = (wnd ? wnd->activechild : 0);
  if (!activechild && wnd)
  {
    activechild = wnd->lastchild;
  }
  else if (activechild)
  {
    activechild = activechild->prevwnd;
    if (!activechild && wnd)
    {
      activechild = wnd->lastchild;
    }
  }
  while (activechild)
  {
    if (TuiIsWndVisible(activechild))
    {
      if (TuiIsWndEnabled(activechild))
      {
        if (strcmp(activechild->clsname, TSTATIC) != 0)
        {
          break; /* found the next control */
        }
      }
    }
    activechild = activechild->prevwnd;
  }
  return activechild;
}

TWND TuiGetActiveChildWnd(TWND wnd)
{
  TWND parent = (wnd ? wnd->parent : 0);
  return (wnd ? wnd->activechild : parent);
}

TWND TuiGetNextActiveChildWnd(TWND wnd)
{
  TWND activechild = (wnd ? wnd->activechild : 0);
  if (!activechild && wnd)
  {
    activechild = wnd->firstchild;
  }
  else if (activechild)
  {
    activechild = activechild->nextwnd;
    if (!activechild && wnd)
    {
      activechild = wnd->firstchild;
    }
  }

  while (activechild)
  {
    if (TuiIsWndVisible(activechild))
    {
      if (TuiIsWndEnabled(activechild))
      {
        if (strcmp(activechild->clsname, TSTATIC) != 0)
        {
          break; /* found the next control */
        }
      }
    }
    activechild = activechild->nextwnd;
  }
  return activechild;
}

TLONG TuiIsWndEnabled(TWND wnd)
{
  if (!wnd)
  {
    return TUI_ERROR;
  }
  return wnd->enable;
}

TLONG TuiIsWndVisible(TWND wnd)
{
  if (!wnd)
  {
    return TUI_ERROR;
  }
  return wnd->visible;
}

TWND TuiGetWndItem(TWND wnd, TINT id)
{
  TWND child = wnd->firstchild;
  while (child)
  { 
    if (child->id == id)
    {
      break;
    }
    child = child->nextwnd;
  }
  return child;
}

TLONG TuiGetWndRect(TWND wnd, TRECT* rect)
{
  rect->y = wnd->y;
  rect->x = wnd->x;
  rect->lines = wnd->lines;
  rect->cols  = wnd->cols;
  return TUI_OK;
}

TDWORD TuiGetWndTextAttrs(TWND wnd)
{
  return wnd->attrs;
}

TDWORD TuiSetWndTextAttrs(TWND wnd, TDWORD newattrs)
{
  TDWORD oldattrs = wnd->attrs;
  wnd->attrs = newattrs;
  return oldattrs;
}

TLONG   TuiGetWndClsName(TWND wnd, TLPSTR clsname, TLONG cb)
{
  TLONG len = strlen(wnd->clsname);
  if (cb < 0 || !clsname)
  {
    return len;
  }

  memset(clsname, 0, cb);
  if (cb > len)
  {
    cb = len;
  }
  strncpy(clsname, wnd->clsname, cb);
  return len;
}

TDWORD TuiGetWndStyleEx(TWND wnd)
{
  return wnd->exstyle;
}

TDWORD TuiGetWndStyle(TWND wnd)
{
  return wnd->style;
}

TDWORD TuiSetWndStyleEx(TWND wnd, TDWORD newstyle)
{
  TDWORD oldstyle = wnd->exstyle;
  wnd->exstyle = newstyle;
  return oldstyle;
}

TDWORD TuiSetWndStyle(TWND wnd, TDWORD newstyle)
{
  TDWORD oldstyle = wnd->style;
  wnd->style = newstyle;
  return oldstyle;
}

TUINT TuiGetWndID(TWND wnd)
{
  return wnd->id;
}

TLPVOID TuiGetWndParam(TWND wnd)
{
  return wnd->param;
}
TLPVOID TuiSetWndParam(TWND wnd, TLPVOID newparam)
{
  TLPVOID oldparam = wnd->param;
  wnd->param = newparam;
  return oldparam;
}

TLONG   TuiIsWndValidate(TWND wnd, TLPCSTR text)
{
  TLONG rc = TUI_CONTINUE;
  if (wnd->validateproc)
  {
    rc = wnd->validateproc(wnd, text);
  }
  return rc;
}

VALIDATEPROC TuiSetWndValidateProc(TWND wnd, VALIDATEPROC validateproc)
{
  VALIDATEPROC oldproc = wnd->validateproc;
  wnd->validateproc = validateproc;
  return oldproc;
}

TLONG   TuiGetWndInfoText(TWND wnd, TLPSTR text, TLONG cb)
{
  TuiSendMsg(wnd, TWM_GETINFOTEXT, cb, (TLPARAM)text);
  return strlen(text);
}

TVOID   TuiSetWndInfoText(TWND wnd, TLPCSTR text)
{
  if (text && wnd)
  {
    TuiSendMsg(wnd, TWM_SETINFOTEXT, 0, (TLPARAM)text);
    TuiInvalidateWnd(wnd);
  }
}

TLONG TuiGetWndText(TWND wnd, TLPSTR text, TLONG cb)
{
  TuiSendMsg(wnd, TWM_GETTEXT, cb, (TLPARAM)text);
  return strlen(text);
}

TVOID TuiSetWndText(TWND wnd, TLPCSTR text)
{
  if (text && wnd)
  {
    TuiSendMsg(wnd, TWM_SETTEXT, 0, (TLPARAM)text);
    TuiInvalidateWnd(wnd);
  }
}

TWND TuiGetParent(TWND wnd)
{
  if (wnd)
  {
    return wnd->parent;
  }
  return 0;
}

/*-------------------------------------------------------------------
 * MSG functions
 *-----------------------------------------------------------------*/

TLONG  TuiGetChar()
{
#if defined __USE_CURSES__
  return (TLONG)wgetch(stdscr);
#elif defined __USE_QIO__
/* Request I/O */
  TTCHAR buffer[2] = { 0, 0 };
  TINT inlen = 1;
  TUINT32 status;
    
  status = SYS$QIOW(0,                    /* efn - event flag         */
                  ttchan,                 /* chan - channel number    */
                  IO$_READVBLK | IO$M_NOECHO | IO$_TTYREADALL,
                                          /* func - function modifier */
                  &ttiosb,                /* iosb - I/O status block  */
                  0,                      /* astadr - AST routine     */
                  0,                      /* astprm - AST parameter   */
                  buffer,                 /* p1 - buffer              */
                  inlen,                  /* p2 - length of buffer    */
                  0, 0, 0, 0);
  return (TLONG)buffer[0];
#elif defined __USE_WIN32__
    DWORD cNumRead, i;
    INPUT_RECORD irInBuf[128];
    TINT bDone = 0;
    while (!bDone)
    {
        ReadConsoleInput(
            genvptr->dc.win,      // input buffer handle 
            irInBuf,     // buffer to read into 
            128,         // size of read buffer 
            &cNumRead); // number of records read
        for (i = 0; i < cNumRead; ++i)
        {
            if (KEY_EVENT == irInBuf[i].EventType)
            {
                if (irInBuf[i].Event.KeyEvent.bKeyDown)
                {
                    return (TLONG)irInBuf[i].Event.KeyEvent.wVirtualKeyCode;
                }
            }
        }
    }
#endif
  return 0;
}

TLONG TuiGetMsg(TMSG* msg)
{
  TENV env = genvptr;
  tmsgq_t* msgq = 0;

  if (env->quitcode)
  {
    return 0;
  }
  /* deque */
  while (env->headq)
  {
    msgq = env->headq;
    msgq->wnd->wndproc(msgq->wnd,
      msgq->msg,
      msgq->wparam,
      msgq->lparam);

    if (env->headq)
    {
      env->headq = env->headq->next;
    }
    msgq->next = 0;
    if ((TWM_NOTIFY    == msgq->msg) ||
        (TWM_SETCURSOR == msgq->msg))
    {
      free((TVOID*)msgq->lparam);
    }
    free(msgq);
  }
  env->tailq = env->headq = 0; /* set to nil */

  memset(msg, 0, sizeof(TMSG));
  msg->wnd = env->activewnd;
  msg->wparam = TuiGetChar();

  return msg->wparam;
}

TLONG _TuiAlignmentPrint(TLPSTR out, TLPCSTR in, TLONG limit, TINT align)
{
  TLONG len = 0;
  TTCHAR text[TUI_MAX_WNDTEXT+1];
  TINT firstlen = 0;
  TINT lastlen = 0;
  
  len = strlen(in);
  if (len > limit)
  {
    len = limit;
  }
  memset(text, 0, TUI_MAX_WNDTEXT);
  memset(out, 0, limit+1);
  
  if ((TALIGN_CENTER == align) || (TWS_CENTER & align))
  {
    firstlen = (limit - len)/2;
    lastlen  = limit - (len + firstlen);
    if (firstlen > 0 && lastlen > 0)
    {
      sprintf(text, "%*s%s%*s",
        firstlen, " ",
        in,
        lastlen, " ");
    }
    else if (lastlen > 0)
    {
      sprintf(text, "%s%*s",
        in,
        lastlen, " ");
    }
    else
    {
      sprintf(text, "%s", in);
    }
    strcpy(out, text);
  }
  else if ((TALIGN_RIGHT == align) || (TWS_RIGHT & align))
  {
    sprintf(text, "%*s",
      (TINT)(limit),
      in);
    strcpy(out, text);
  }
  else
  { 
    sprintf(text, "%-*s",
      (TINT)(limit),
      in);
    strncpy(out, text, limit);
  }

  return strlen(out);
}

TLONG _TuiRemoveAllMsgs()
{
  TENV env = genvptr;
  tmsgq_t* msgq = 0;
  /* deque */
  while (env->headq)
  {
    msgq = env->headq;
    if (env->headq)
    {
      env->headq = env->headq->next;
    }
    msgq->next = 0;
    free(msgq);
  }
  env->tailq = env->headq = 0; 
  return TUI_OK;
}

TLONG _TuiDequeMsg()
{
  TENV env = genvptr;
  tmsgq_t* msgq = 0;
  /* deque */
  while (env->headq)
  {
    msgq = env->headq;
    TuiSendMsg(msgq->wnd,
      msgq->msg,
      msgq->wparam,
      msgq->lparam);

    if (env->headq)
    {
      env->headq = env->headq->next;
    }
    msgq->next = 0;
    free(msgq);
  }
  env->tailq = env->headq = 0;
  return TUI_OK;
}

TLONG TuiDispatchMsg(TMSG* msg)
{
  TENV env = genvptr;
  TWND wnd = env->activewnd;
  TWND child = (wnd ? wnd->activechild : 0);

  if (env->quitcode)
  {
    return TUI_OK;
  }

  if (child)
  {
    msg->wnd = child;
  }
  else if (wnd)
  {
    child = TuiGetFirstActiveChildWnd(wnd);
    if (child)
    {
      msg->wnd = child;
    }
    else
    {
      msg->wnd = wnd;
    }
  }

  /* deque message */
  _TuiDequeMsg();

  return TUI_OK;
}

TLONG TuiTranslateMsg(TMSG* msg)
{
  TENV env = genvptr;
  TINT  ch  = (TINT)msg->wparam;
  TLONG rc  = 0;
  TWND nextwnd = 0;
  TWND prevwnd = 0;
  TWND parent = 0;
  TWND grandparent = 0;
  TLONG nextmove = env->nextmove;
  TLONG prevmove = env->prevmove;
  TWND nextchild = 0;
  TRECT rcwnd;

  if (env->quitcode)
  {
    return TUI_OK;
  }
  /* send message to the current active window */
  if (!msg->wnd)
  {
    return TUI_CONTINUE;
  }

  /* convert keyboard code to message */
  switch (ch)
  {
#ifdef __USE_CURSES__
    /* not support mouse/resize events yet */
    case KEY_MOUSE:/* Mouse event has occurred */
#ifdef __LINUX__
    case KEY_RESIZE:/* Terminal resize event */
#endif

#elif defined __USE_QIO__
#endif
      return 0;
  }

  parent = TuiGetParent(msg->wnd);
  if (ch == nextmove)
  {
    /* kill focus the current active window */
    nextwnd = TuiGetNextActiveChildWnd(parent);
    nextchild = TuiGetNextActiveChildWnd(msg->wnd);
    if (nextchild)
    {
      rc = TuiSendMsg(msg->wnd, TWM_KILLFOCUS, 0, (TLPARAM)nextchild);
    }
    else
    {
      rc = TuiSendMsg(msg->wnd, TWM_KILLFOCUS, 0, (TLPARAM)nextwnd);
    }
    
    if (rc != TUI_CONTINUE)
    {
      /* move cursor */
      TuiGetWndRect(msg->wnd, &rcwnd);
      TuiMoveYX(TuiGetDC(msg->wnd), rcwnd.y, rcwnd.x);
      return rc;
    }

    /* set focus the new active window */    
    prevwnd = msg->wnd;
    if (parent)
    {
      if (msg->wnd == TuiGetLastActiveChildWnd(parent))
      {
        grandparent = TuiGetParent(parent);
        if (grandparent)
        {
          msg->wnd = TuiGetNextActiveChildWnd(grandparent);
        }
        else
        {
          msg->wnd = TuiGetFirstActiveChildWnd(parent);
        }
      }
      else
      {
        msg->wnd = nextwnd;
      }
      parent->activechild = msg->wnd;
    }
    else
    {
      msg->wnd = TuiGetPrevActiveChildWnd(env->activewnd);
      env->activewnd->activechild = msg->wnd;
    }
        
    if (msg->wnd)
    {
      rc = TuiPostMsg(msg->wnd, TWM_SETFOCUS, 0, (TLPARAM)prevwnd);
      TuiMoveYX(TuiGetDC(msg->wnd), msg->wnd->y, msg->wnd->x);
    }
    return 0;    
  }
  else if (ch == prevmove)
  {
    /* kill focus the current active window */
    prevwnd = TuiGetPrevActiveChildWnd(parent);
    rc = TuiSendMsg(msg->wnd, TWM_KILLFOCUS, 0, (TLPARAM)prevwnd);
    if (rc != TUI_CONTINUE)
    {
      return rc;
    }

    /* set focus the new active window */
    nextwnd = msg->wnd;
    if (parent)
    {
      if (msg->wnd == TuiGetFirstActiveChildWnd(parent))
      {
        grandparent = TuiGetParent(parent);
        if (grandparent)
        {
          msg->wnd = TuiGetPrevActiveChildWnd(grandparent);
        }
        else
        {
          msg->wnd = TuiGetLastActiveChildWnd(parent);
        }
      }
      else
      {
        msg->wnd = prevwnd;
      }
      parent->activechild = msg->wnd;
    }
    else
    {
      msg->wnd = TuiGetPrevActiveChildWnd(env->activewnd);
      env->activewnd->activechild = msg->wnd;
    }
    if (msg->wnd)
    {
      rc = TuiPostMsg(msg->wnd, TWM_SETFOCUS, 0, (TLPARAM)nextwnd);
      TuiMoveYX(TuiGetDC(msg->wnd), msg->wnd->y, msg->wnd->x);
    }
    return 0;    
  }
  else
  {
    /* otherwise key */
  }

  /* parent window */
  rc = TuiSendMsg(env->activewnd, TWM_KEYDOWN, msg->wparam, 0);
  rc = TuiSendMsg(env->activewnd, TWM_CHAR, msg->wparam, 0);
  rc = TuiSendMsg(env->activewnd, TWM_KEYUP, msg->wparam, 0);

  if (msg->wnd != env->activewnd)
  {
    rc = TuiSendMsg(msg->wnd, TWM_KEYDOWN, msg->wparam, 0);
    rc = TuiSendMsg(msg->wnd, TWM_CHAR, msg->wparam, 0);
    rc = TuiSendMsg(msg->wnd, TWM_KEYUP, msg->wparam, 0);
  }

  /* try to dispatch */
  TuiDispatchMsg(msg);

  return 0;
}

TLONG TuiPostQuitMsg(TLONG exitcode)
{
  TENV env = genvptr;
  env->quitcode = 1;
  env->exitcode = exitcode;

  return TUI_OK;
}

TWND TuiGetFocus(TWND wnd)
{
  return TuiGetActiveChildWnd(wnd);
}

TLONG TuiSetFocus(TWND wnd)
{
  TENV env = genvptr;
  TWND parent = (wnd ? wnd->parent : env->activewnd);
  TWND activewnd = TuiGetActiveChildWnd(wnd);
  TLONG rc = TUI_CONTINUE;

  /* kill focus at the current control */
  if (activewnd)
  {
    rc = TuiSendMsg(activewnd, TWM_KILLFOCUS, 0, (TLPARAM)wnd);
  }
  if (rc != TUI_CONTINUE)
  {
    return rc;
  }
  if (wnd)
  {
    rc = TuiSendMsg(wnd, TWM_SETFOCUS, 0, (TLPARAM)activewnd);
    TuiMoveYX(TuiGetDC(wnd), wnd->y, wnd->x);  

    if (parent)
    {
      parent->activechild = wnd;
    }
  }
  return rc;
}

TWND TuiGetActiveWnd()
{
  TENV env = genvptr;
  return env->activewnd;
}

TWND TuiGetFirstWnd()
{
  TENV env = genvptr;
  return env->firstwnd;
}

TWND TuiGetLastWnd()
{
  TENV env = genvptr;
  return env->lastwnd;
}

TWND   TuiGetFirstChildWnd(TWND wnd)
{
  return wnd->firstchild;
}

TWND   TuiGetLastChildWnd(TWND wnd)
{
  return wnd->lastchild;
}

TWND TuiGetNextWnd(TWND wnd)
{
  return wnd->nextwnd;
}

TWND TuiGetPrevWnd(TWND wnd)
{
  return wnd->prevwnd;
}

TLONG TuiPrintTextAlignment(TLPSTR out, TLPCSTR in, TLONG limit, TINT align)
{
    return _TuiAlignmentPrint(out, in, limit, align);
}

/*-------------------------------------------------------------------
 * DefWndProc functions
 *-----------------------------------------------------------------*/
TLONG  _TuiDefWndProc_OnEraseBk(TWND wnd, TDC dc);
TVOID  _TuiDefWndProc_OnSetText(TWND wnd, TLPCSTR text);
TLONG  _TuiDefWndProc_OnGetText(TWND wnd, TLPSTR text, TLONG cb);
TVOID  _TuiDefWndProc_OnSetTextAlign(TWND wnd, TINT align);
TDWORD _TuiDefWndProc_OnSetTextAttrs(TWND wnd, TDWORD attrs);
TDWORD _TuiDefWndProc_OnGetTextAttrs(TWND wnd);
TVOID  _TuiDefWndProc_OnSetFocus(TWND wnd);
TVOID  _TuiDefWndProc_OnSetInfoText(TWND wnd, TLPCSTR text);
TLONG  _TuiDefWndProc_OnGetInfoText(TWND wnd, TLPSTR text, TLONG cb);
TLONG  _TuiDefWndProc_OnShow(TWND wnd, TLONG show);
TLONG  _TuiDefWndProc_OnEnable(TWND wnd, TLONG enable);
TLONG  _TuiDefWndProc_OnGetCursor(TWND wnd, TPOS* pos);
TLONG  _TuiDefWndProc_OnSetCursor(TWND wnd, TPOS* pos);

TLONG  _TuiDefWndProc_OnGetCursor(TWND wnd, TPOS* pos)
{
  TuiGetYX(TuiGetDC(wnd), &pos->y, &pos->x);
  return TUI_OK;
}

TLONG  _TuiDefWndProc_OnSetCursor(TWND wnd, TPOS* pos)
{
  TuiMoveYX(TuiGetDC(wnd), pos->y, pos->x);
  return TUI_OK;
}

TLONG _TuiDefWndProc_OnEraseBk(TWND wnd, TDC dc)
{
  TINT i;
  TTCHAR buf[TUI_MAX_WNDTEXT + 1];
  TRECT rc;
  TDWORD attrs = TuiGetWndTextAttrs(wnd);
  
  if (TuiIsWndVisible(wnd))
  {
    TuiGetWndRect(wnd, &rc);
    memset(buf, ' ', rc.cols);
    buf[rc.cols] = 0;
    for (i = 0; i < rc.lines; ++i)
    {
      TuiDrawText(dc, rc.y + i, rc.x, buf, attrs);
    }
  }
  return TUI_OK;
}

TVOID _TuiDefWndProc_OnSetText(TWND wnd, TLPCSTR text)
{
  TLONG len = 0;
  if (text)
  {
    len = strlen(text);
    if (len > TUI_MAX_WNDTEXT)
    {
      strncpy(wnd->wndname, text, TUI_MAX_WNDTEXT);
    }
    else
    {
      strcpy(wnd->wndname, text);
    }
  }
}

TLONG _TuiDefWndProc_OnGetText(TWND wnd, TLPSTR text, TLONG cb)
{
  TLONG len = strlen(wnd->wndname);
  if (cb < 0 || !text)
  {
    return len;
  }
  
  if (cb > len + 1)
  {
    cb = len + 1;
  }
  memset(text, 0, cb);
  strncpy(text, wnd->wndname, cb);
  return strlen(text);
}

TVOID _TuiDefWndProc_OnSetInfoText(TWND wnd, TLPCSTR text)
{
  TLONG len = 0;
  if (text)
  {
    len = strlen(text);
    if (len > TUI_MAX_WNDTEXT)
    {
      strncpy(wnd->infotext, text, TUI_MAX_WNDTEXT);
    }
    else
    {
      strcpy(wnd->infotext, text);
    }
  }
}

TLONG _TuiDefWndProc_OnGetInfoText(TWND wnd, TLPSTR text, TLONG cb)
{
  TLONG len = strlen(wnd->infotext);
  if (cb < 0 || !text)
  {
    return len;
  }
  
  if (cb > len + 1)
  {
    cb = len + 1;
  }
  memset(text, 0, cb);
  strncpy(text, wnd->infotext, cb);
  return strlen(text);
}

TVOID _TuiDefWndProc_OnSetTextAlign(TWND wnd, TINT align)
{
  wnd->style &= ~(TWS_LEFT | TWS_CENTER | TWS_RIGHT);
  
  if (TALIGN_CENTER == align)
  {
    wnd->style |= TWS_CENTER;
  }
  else if (TALIGN_RIGHT == align)
  {
    wnd->style |= TWS_RIGHT;
  }
  else
  {
    wnd->style |= TWS_LEFT;
  }
}

TDWORD _TuiDefWndProc_OnSetTextAttrs(TWND wnd, TDWORD attrs)
{
  TDWORD oldattrs = wnd->attrs;
  wnd->attrs = attrs;
  return oldattrs;
}

TDWORD _TuiDefWndProc_OnGetTextAttrs(TWND wnd)
{
  return wnd->attrs;
}

TVOID _TuiDefWndProc_OnSetFocus(TWND wnd)
{
  TRECT rcwnd;
  DISPLAYINFO di;
  di.hdr.id    = TuiGetWndID(wnd);
  di.hdr.ctl   = wnd;
  di.hdr.code  = TCN_DISPLAYINFO;
  strcpy(di.text, " ");

  if (strlen(wnd->infotext) > 0)
  {
    strcpy(di.text, wnd->infotext);
  }
  TuiSendMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (TLPARAM)&di);

  TuiGetWndRect(wnd, &rcwnd);
  TuiMoveYX(TuiGetDC(wnd), rcwnd.y, rcwnd.x);
}

TLONG  _TuiDefWndProc_OnShow(TWND wnd, TLONG show)
{
  wnd->visible = show;
  return TuiInvalidateWnd(wnd);
}

TLONG  _TuiDefWndProc_OnEnable(TWND wnd, TLONG enable)
{
  wnd->enable = enable;
  return TUI_OK;
}

TLONG TuiDefWndProc(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam)
{
  switch (msg)
  {
    case TWM_CREATE:
    case TWM_KILLFOCUS:
      return TUI_CONTINUE;
      
    case TWM_SHOW:
    {
      return _TuiDefWndProc_OnShow(wnd,
              (TW_SHOW == (TINT)wparam ? TW_SHOW : TW_HIDE));
    }

    case TWM_ENABLE:
    {
      return _TuiDefWndProc_OnEnable(wnd,
              (TW_ENABLE == (TINT)wparam ? TW_ENABLE : TW_DISABLE));
    }

    case TWM_SETFOCUS:
    {
      _TuiDefWndProc_OnSetFocus(wnd);
      break;
    }
      
    case TWM_GETCURSOR:
    {
      return _TuiDefWndProc_OnGetCursor(wnd, (TPOS*)lparam);
    }
      
    case TWM_SETCURSOR:
    {
      return _TuiDefWndProc_OnSetCursor(wnd, (TPOS*)lparam);
    }
      
    case TWM_ERASEBK:
    {
      return _TuiDefWndProc_OnEraseBk(wnd, (TDC)wparam);
    }
    
    case TWM_SETINFOTEXT:
    {
      _TuiDefWndProc_OnSetInfoText(wnd, (TLPCSTR)lparam);
      return 0;
    }    
    case TWM_GETINFOTEXT:
    {
      return _TuiDefWndProc_OnGetInfoText(wnd, (TLPSTR)lparam, (TLONG)wparam);
    }
    
    case TWM_SETTEXT:
    {
      _TuiDefWndProc_OnSetText(wnd, (TLPCSTR)lparam);
      return 0;
    }    
    case TWM_GETTEXT:
    {
      return _TuiDefWndProc_OnGetText(wnd, (TLPSTR)lparam, (TLONG)wparam);
    }
    case TWM_SETTEXTALIGN:
    {
      _TuiDefWndProc_OnSetTextAlign(wnd, (TINT)wparam);
    }
    case TWM_SETTEXTATTRS:
    {
      return _TuiDefWndProc_OnSetTextAttrs(wnd, (TDWORD)wparam);
    }
    case TWM_GETTEXTATTRS:
    {
      _TuiDefWndProc_OnGetTextAttrs(wnd);
    }
  }
  return TUI_OK;
}

TDWORD TuiGetColor(TINT idx)
{
#ifdef __USE_CURSES__
  return COLOR_PAIR(idx);
#elif defined __USE_WIN32__
  return _TuiColorPair(idx);
#endif
  return 0;
}

TDWORD TuiGetSysColor(TINT idx)
{
  TENV env = genvptr;
  if (idx < 0 || idx >= COLOR_LAST)
  {
    idx = 0;
  }
#ifdef __USE_CURSES__
  return COLOR_PAIR(env->themes[env->themeid].theme[idx].id);
#elif defined __USE_WIN32__
  return _TuiColorPair(env->themes[env->themeid].theme[idx].id);
#endif
  return 0;
}

TDWORD TuiGetReverseSysColor(TINT idx)
{
#ifdef __USE_CURSES__
  return TuiGetSysColor(idx)|A_REVERSE;
#elif defined __USE_WIN32__
    return _TuiReverseColorPair(idx);
#endif
  return 0;
}

TDWORD TuiReverseColor(TDWORD color)
{
#ifdef __USE_CURSES__
  return color|A_REVERSE;
#elif defined __USE_WIN32__
#endif
  return color;
}

TDWORD TuiUnderlineText(TDWORD attrs)
{
#ifdef __USE_CURSES__
  return attrs|A_UNDERLINE;
#endif
  return attrs;
}

TDWORD TuiGetSysColorTheme(TINT themeid, TINT idx)
{
  TENV env = genvptr;
  if (themeid < 0 || themeid >= THEME_LAST)
  {
    themeid = 0;
  }
  if (idx < 0 || idx >= COLOR_LAST)
  {
    idx = 0;
  }
#ifdef __USE_CURSES__
  return COLOR_PAIR(env->themes[env->themeid].theme[idx].id);
#elif defined __USE_WIN32__
  return _TuiColorPair(env->themes[env->themeid].theme[idx].id);
#endif
  return 0;
}

TDWORD TuiGetReverseSysColorTheme(TINT themeid, TINT idx)
{
#ifdef __USE_CURSES__
  return TuiGetSysColorTheme(themeid, idx)|A_REVERSE;
#elif defined __USE_WIN32__
    TENV env = genvptr;
    if (themeid < 0 || themeid >= THEME_LAST)
    {
        themeid = 0;
    }
    if (idx < 0 || idx >= COLOR_LAST)
    {
        idx = 0;
    }
    return _TuiReverseColorPair(genvptr->themes[genvptr->themeid].theme[idx].id);
#endif
  return 0;
}
