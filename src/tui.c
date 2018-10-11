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
#include <qio_init.h>
#endif

#include "tui.h"

struct _TWNDPROCSTRUCT
{
  TWNDPROC     wndproc;
  LPCSTR       clsname;
  struct _TWNDPROCSTRUCT *prev;
  struct _TWNDPROCSTRUCT *next;
};
typedef struct _TWNDPROCSTRUCT twndproc_t;

struct _TUIMSGQUEUESTRUCT
{
  TWND     wnd;
  UINT     msg;
  WPARAM   wparam;
  LPARAM   lparam;

  struct _TUIMSGQUEUESTRUCT *next;
};
typedef struct _TUIMSGQUEUESTRUCT tmsgq_t;

struct _TUITHEMEITEMSTRUCT
{
  INT   id;
  /*DWORD color;*/
};
typedef struct _TUITHEMEITEMSTRUCT tthemeitem_t;

struct _TUITHEMESTRUCT
{
  tthemeitem_t* theme;
};
typedef struct _TUITHEMESTRUCT ttheme_t;

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

  LONG             quitcode;
  LONG             exitcode;

  LONG             nextmove; /* to identify the next control moved */
  LONG             prevmove; /* to identify the prev control moved */
  LONG             notifykey;
  
  _TDC             dc;
  /* last dialog returned message */
  /*UINT             dlgmsgid;*/
  INT              themeid;
  ttheme_t         themes[THEME_LAST];
};

struct _TUIWINDOWSTRUCT
{
  LPCSTR           clsname;
  CHAR             wndname[TUI_MAX_WNDTEXT+1];
  DWORD            style;
  DWORD            exstyle;
  INT              x;
  INT              y;
  INT              cols;
  INT              lines;
  TWND             parent;
  UINT             id;
  LPVOID           param;
  TWNDPROC         wndproc;
  LONG             enable;
  LONG             visible;
  LONG             (*validate)(TWND, LPCSTR);
  /* curses lib */
#ifdef __USE_CURSES__
  WINDOW*            win;
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  struct qio_fields* win;
#endif
  DWORD            attrs;
  /* last dialog returned message */
  /*UINT             dlgmsgid;*/
  
  /* links */
  TWND             prevwnd;
  TWND             nextwnd;
  TWND             firstchild;
  TWND             lastchild;
  TWND             activechild;
};

/*-------------------------------------------------------------------
 * global storages
 *-----------------------------------------------------------------*/

tthemeitem_t STANDARD_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

tthemeitem_t LHS_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

tthemeitem_t ASP_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

tthemeitem_t CNS_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

tthemeitem_t KTZ_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

tthemeitem_t YUANTA_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

tthemeitem_t KSS_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

tthemeitem_t PST_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

tthemeitem_t MBKET_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

tthemeitem_t AIRA_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

tthemeitem_t ASL_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

tthemeitem_t MERCHANT_THEME[] =
{
  { BLACK_WHITE }, /* COLOR_WNDTEXT     */
  { BLACK_CYAN  }, /* COLOR_BTNENABLED  */
  { WHITE_BLACK }, /* COLOR_BTNDISABLED */
  { BLACK_WHITE }, /* COLOR_BTNFOCUSED  */
  { BLUE_YELLOW }, /* COLOR_HDRTEXT     */
  { CYAN_BLACK  }, /* COLOR_EDTTEXT     */
  { YELLOW_BLUE }, /* COLOR_LBXTEXT     */
  { WHITE_BLUE  }, /* COLOR_WNDTITLE    */
  /* last color */
  { 0 }  
};

/* global environment variable */
TENV genvptr = 0;


/*-------------------------------------------------------------------
 * internal functions
 *-----------------------------------------------------------------*/
twndproc_t* _TuiFindWndProc(LPCSTR clsname);
TWND _TuiCreateWnd(
  LPCSTR   clsname,
  LPCSTR   wndname,
  DWORD    style,
  INT      y,
  INT      x,
  INT      lines,
  INT      cols,
  TWND     parent,
  UINT     id,
  LPVOID   param
);
VOID _TuiDestroyWnd(TWND wnd);
LONG _TuiInvalidateWnd(TWND wnd);
LONG _TuiDequeMsg();
LONG _TuiRemoveAllMsgs();
LONG _TuiAlignmentPrint(LPSTR out, LPCSTR in, LONG limit, INT align);
VOID _TuiInitColors();
VOID _TuiInitThemes();

LONG STATICPROC(TWND, UINT, WPARAM, LPARAM);
LONG EDITBOXPROC(TWND, UINT, WPARAM, LPARAM);
LONG LISTBOXPROC(TWND, UINT, WPARAM, LPARAM);
LONG BUTTONPROC(TWND, UINT, WPARAM, LPARAM);
LONG LISTCTRLPROC(TWND, UINT, WPARAM, LPARAM);
LONG PAGECTRLPROC(TWND, UINT, WPARAM, LPARAM);
/*
LONG MSGBOXPROC(TWND, UINT, WPARAM, LPARAM);
*/
/*-------------------------------------------------------------------
 * functions
 *-----------------------------------------------------------------*/
 #ifdef __USE_QIO__
int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);

LONG QIO_Initialize();
VOID QIO_Get();
VOID QIO_Put();

VOID DoUpdate() /* required by QIO */
{
    return;
}
VOID CheckTerm() /* required by QIO */
{
    return;
}
#endif /*__USE_QIO__*/


INT   TuiGetTheme()
{
  TENV env = genvptr;
  return env->themeid;
}

INT   TuiSetTheme(INT themeid)
{
  TENV env = genvptr;
  INT oldtheme = env->themeid;
  if (themeid >= THEME_STANDARD && themeid < THEME_LAST)
  {
    env->themeid = themeid;
  }
  return oldtheme;
}

VOID _TuiInitThemes()
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


VOID _TuiInitColors()
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
#endif
}

LONG TuiStartup()
{
  TENV env = (TENV)malloc(sizeof(_TENV));
  if (env)
  {
    genvptr = env;
    memset(env, 0, sizeof(_TENV));

    /* register standard controls */
    TuiRegisterCls(STATIC,   STATICPROC);
    TuiRegisterCls(EDITBOX,  EDITBOXPROC);
    TuiRegisterCls(LISTBOX,  LISTBOXPROC);
    TuiRegisterCls(BUTTON,   BUTTONPROC);
    TuiRegisterCls(LISTCTRL, LISTCTRLPROC);
    /*TuiRegisterCls(PAGECTRL, PAGECTRLPROC);*/
    /*TuiRegisterCls(MSGBOX,   MSGBOXPROC);*/

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
    if (!QIO_Initialize())
    {
        free(env);
        genvptr = 0;
    }
    env->dc.win = &qio_field;
#endif
  }
    
  _TuiInitColors();
  _TuiInitThemes();

  return (genvptr ? TUI_OK : TUI_ERROR);
}

VOID TuiShutdown()
{
  TWND wnd = genvptr->firstwnd;
  TWND temp = 0;
  
  while (wnd)
  {
    temp = wnd;
    wnd = wnd->nextwnd;
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
#endif
}

twndproc_t* _TuiFindWndProc(LPCSTR clsname)
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

LONG TuiSetPrevMove(LONG prevmove)
{
  TENV env = genvptr;
  LONG oldmove = env->prevmove;
  env->prevmove = prevmove;
  return oldmove;
}

LONG TuiSetNextMove(LONG nextmove)
{
  TENV env = genvptr;
  LONG oldmove = env->nextmove;
  env->nextmove = nextmove;
  return oldmove;
}
/*
UINT TuiGetDlgMsgID()
{
  TENV env = genvptr;
  return env->dlgmsgid;
}
*/
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
#endif
  return (TDC)&dc;
}

LONG TuiRegisterCls(LPCSTR clsname, TWNDPROC wndproc)
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

LONG TuiMoveWnd(TWND wnd, INT y, INT x, INT lines, INT cols)
{
  LONG rc = TUI_OK;
  wnd->y = y;
  wnd->x = x;
  wnd->lines = lines;
  wnd->cols  = cols;
  return rc;
}

TWND _TuiCreateWnd(
  LPCSTR   clsname,
  LPCSTR   wndname,
  DWORD    style,
  INT      y,
  INT      x,
  INT      lines,
  INT      cols,
  TWND     parent,
  UINT      id,
  LPVOID   param
)
{
  TENV env = genvptr;
  TWND wnd = 0;
  LONG len = 0;
  twndproc_t* proc = _TuiFindWndProc(clsname);

  if ((style & TWS_WINDOW) &&
      (style & TWS_CHILD))
  {
    /* not allow creating both styles at a time */
    return wnd;
  }
  else if (style & TWS_CHILD && !parent)
  {
    /* not allow child without parent */
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
      wnd->validate = 0;
      /* curses */
#ifdef __USE_CURSES__
      wnd->win     = stdscr;
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
      wnd->win     = &qio_field;
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
          /*wnd->y++;*/
          /*wnd->x++;*/
          wnd->lines--;
          wnd->cols--;
        }
        /* send message */
        TuiSendMsg(wnd, TWM_ERASEBK, (WPARAM)TuiGetDC(wnd), 0);
        TuiSendMsg(wnd, TWM_CREATE, 0, 0);
      }
    }
  }
  return wnd;
}

TWND TuiCreateFrameWnd(
  LPCSTR   clsname,
  LPCSTR   wndname,
  DWORD    style,
  INT      y,
  INT      x,
  INT      lines,
  INT      cols,
  WNDTEMPL* templs,
  LPVOID    param
)
{
  LONG rc  = TUI_CONTINUE;
  TWND wnd = 0;
  TWND child = 0;
  INT i = 0;

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
      style  = templs[i].style;
      style &= ~TWS_WINDOW; /* others must be child */
      style |= TWS_CHILD;   /* others must be child */
      child  = _TuiCreateWnd(
                   templs[i].clsname,
                   templs[i].text,
                   templs[i].style,
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
        TuiSetWndValidate(child, (LONG (*)(TWND, LPCSTR))templs[i].validate);
      }
    } /* create children */

    if (wnd)
    {
      TuiSendMsg(wnd, TWM_ERASEBK, (WPARAM)TuiGetDC(wnd), 0);
      rc = TuiSendMsg(wnd, TWM_INITDIALOG, (WPARAM)0, (LPARAM)param);
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

TWND TuiCreateWnd(
  LPCSTR   clsname,
  LPCSTR   wndname,
  DWORD    style,
  INT      y,
  INT      x,
  INT      lines,
  INT      cols,
  TWND     parent,
  INT      id,
  LPVOID   param
)
{
  LONG rc  = TUI_CONTINUE;
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
    TuiSendMsg(wnd, TWM_ERASEBK, (WPARAM)TuiGetDC(wnd), 0);
    rc = TuiSendMsg(wnd, TWM_CREATE, 0, (LPARAM)param);
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
  LPVOID    param
)
{
  LONG rc  = TUI_CONTINUE;
  TWND wnd = 0;
  TWND child = 0;
  INT i = 0;
  DWORD style = 0;

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
                   templs[i].style,
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
          child->validate = (LONG (*)(TWND, LPCSTR))templs[i].validate;
        }
      } /* create children */
    } /* wnd is created successfully */

    if (wnd)
    {
      TuiSendMsg(wnd, TWM_ERASEBK, (WPARAM)TuiGetDC(wnd), 0);
      rc = TuiSendMsg(wnd, TWM_INITDIALOG, (WPARAM)0, (LPARAM)0);
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

void _TuiDestroyWnd(TWND wnd)
{
  TWND child = (wnd ? wnd->firstchild : 0);
  TWND temp  = child;
  while (child)
  {
    child = child->nextwnd;
    free(temp);
    temp->nextwnd = temp->prevwnd = 0;
    temp  = child;
  }
  wnd->nextwnd = wnd->prevwnd = 0;
  free(wnd);
}

void TuiDestroyWnd(TWND wnd)
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
  /*env->dlgmsgid = wnd->dlgmsgid;*/
  
  TuiSendMsg(wnd, TWM_DESTROY, 0, 0);
  _TuiDestroyWnd(wnd);
  
  if (env->activewnd)
  {
    TuiInvalidateWnd(env->activewnd);
  }
}

LONG TuiPostMsg(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  if (wnd)
  {
    tmsgq_t* msgq = (tmsgq_t*)malloc(sizeof(tmsgq_t));
    if (msgq)
    {
      TENV env = genvptr;

      memset(msgq, 0, sizeof(tmsgq_t));
      msgq->wnd = wnd;
      msgq->msg = msg;
      msgq->wparam = wparam;
      msgq->lparam = lparam;

      if (env->tailq)
      {
        env->tailq->next = msgq;
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

LONG TuiSendMsg(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  if (wnd)
  {
    return wnd->wndproc(wnd, msg, wparam, lparam);
  }
  return TUI_ERROR;
}

LONG _TuiInvalidateWnd(TWND wnd)
{
  TWND activechild = TuiGetActiveChildWnd(wnd);
  TWND child = wnd->firstchild;

  TuiSendMsg(wnd, TWM_ERASEBK, (WPARAM)TuiGetDC(wnd), 0);
  TuiSendMsg(wnd, TWM_PAINT, 0, 0);
  while (child)
  {
    TuiSendMsg(child, TWM_ERASEBK, (WPARAM)TuiGetDC(wnd), 0);
    TuiSendMsg(child, TWM_PAINT, 0, 0);
    child = child->nextwnd;
  }
  
  if (!activechild)
  {
    activechild = TuiGetFirstActiveChildWnd(wnd);
  }
  if (activechild)
  {
    TuiMoveYX(TuiGetDC(activechild), activechild->y, activechild->x);
  }
  return TUI_OK;
}

LONG TuiInvalidateWnd(TWND wnd)
{
  return _TuiInvalidateWnd(wnd);
}

LONG TuiShowWnd(TWND wnd, LONG show)
{
  wnd->visible = show;
  return _TuiInvalidateWnd(wnd);
}

LONG TuiEnableWnd(TWND wnd, LONG enable)
{
  wnd->enable = enable;
  /*return _TuiInvalidateWnd(wnd);*/
  return TUI_OK;
}

LONG   TuiVisibleWnd(TWND wnd, LONG visible)
{
  wnd->visible = visible;
  return _TuiInvalidateWnd(wnd);
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
        if (strcmp(activechild->clsname, STATIC) != 0)
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
        if (strcmp(activechild->clsname, STATIC) != 0)
        {
          break; /* found the next control */
        }
      }
    }
    activechild = activechild->nextwnd;
  }
  return activechild;
}

LONG TuiIsWndEnabled(TWND wnd)
{
  if (!wnd)
  {
    return TUI_ERROR;
  }
  return wnd->enable;
}

LONG TuiIsWndVisible(TWND wnd)
{
  if (!wnd)
  {
    return TUI_ERROR;
  }
  return wnd->visible;
}

TWND TuiGetWndItem(TWND wnd, INT id)
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

LONG TuiGetWndRect(TWND wnd, RECT* rect)
{
  rect->y = wnd->y;
  rect->x = wnd->x;
  rect->lines = wnd->lines;
  rect->cols  = wnd->cols;
  return TUI_OK;
}

DWORD TuiGetWndTextAttrs(TWND wnd)
{
  return wnd->attrs;
}

DWORD TuiSetWndTextAttrs(TWND wnd, DWORD newattrs)
{
  DWORD oldattrs = wnd->attrs;
  wnd->attrs = newattrs;
  return oldattrs;
}

DWORD TuiGetWndStyle(TWND wnd)
{
  return wnd->style;
}

DWORD TuiSetWndStyle(TWND wnd, DWORD newstyle)
{
  DWORD oldstyle = wnd->style;
  wnd->style = newstyle;
  return oldstyle;
}

UINT TuiGetWndID(TWND wnd)
{
  return wnd->id;
}

LPVOID TuiGetWndParam(TWND wnd)
{
  return wnd->param;
}
LPVOID TuiSetWndParam(TWND wnd, LPVOID newparam)
{
  LPVOID oldparam = wnd->param;
  wnd->param = newparam;
  return oldparam;
}

LONG   TuiIsWndValidate(TWND wnd, LPCSTR text)
{
  LONG rc = TUI_CONTINUE;
  if (wnd->validate)
  {
    rc = wnd->validate(wnd, text);
  }
  return rc;
}

VOID
TuiSetWndValidate(TWND wnd, LONG (*validate)(TWND, LPCSTR))
{
  wnd->validate = validate;
}

LONG TuiGetWndText(TWND wnd, LPSTR text, LONG cb)
{
  TuiSendMsg(wnd, TWM_GETTEXT, cb, (LPARAM)text);
  return strlen(text);
}

VOID TuiSetWndText(TWND wnd, LPCSTR text)
{
  if (text && wnd)
  {
    TuiSendMsg(wnd, TWM_SETTEXT, 0, (LPARAM)text);
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
#ifdef __USE_QIO__
VOID QIO_GetCh()
{         
  QIO_InitParameter();
  while(1)
  {
    QIO_GetKeyTerminal();
    switch(bProcess)
    {
      case ENDERR:
        printf("Program abort...\n");
        exit(0);
      case BEGPROC:
        QIO_InitParameter();
        break;
      case ENDSUCC:
        CheckProcessInput();
        if(CheckValue()== TRUE)
        {
          memset(qio_field.szDefault,0,SIZE_TEXT);
          return;
        }
    }
  }

}
#endif

LONG  TuiGetChar()
{
#ifdef __USE_CURSES__
  return (LONG)wgetch(stdscr);
#endif
  return 0;
}

LONG TuiGetMsg(MSG* msg)
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
    free(msgq);
  }
  env->tailq = env->headq = 0; /* set to nil */

  memset(msg, 0, sizeof(MSG));
  msg->wnd = env->activewnd;
  msg->wparam = TuiGetChar();

  return msg->wparam;
}

LONG _TuiAlignmentPrint(LPSTR out, LPCSTR in, LONG limit, INT align)
{
  LONG len = 0;
  CHAR text[TUI_MAX_WNDTEXT+1];
  INT firstlen = 0;
  INT lastlen = 0;
  
  len = strlen(in);
  if (len > limit)
  {
    len = limit;
  }
  memset(text, 0, TUI_MAX_WNDTEXT);
  
  if ((ALIGN_CENTER == align) || (TWS_CENTER & align))
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
  }
  else if ((ALIGN_RIGHT == align) || (TWS_RIGHT & align))
  {
    sprintf(text, "%*s",
      (INT)(limit),
      in);
  }
  else
  { 
    sprintf(text, "%-*s",
      (INT)(limit),
      in);
  }

  strcpy(out, text);
  return strlen(out);
}

LONG _TuiRemoveAllMsgs()
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

LONG _TuiDequeMsg()
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

LONG TuiDispatchMsg(MSG* msg)
{
  TENV env = genvptr;
  TWND wnd = env->activewnd;
  TWND child = wnd->activechild;

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

LONG TuiTranslateMsg(MSG* msg)
{
  TENV env = genvptr;
  INT  ch  = (int)msg->wparam;
  LONG rc  = 0;
  TWND nextwnd = 0;
  TWND prevwnd = 0;
  TWND parent = 0;
  LONG nextmove = env->nextmove;
  LONG prevmove = env->prevmove;
  TWND nextchild = 0;

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

  /*case TVK_ENTER:*/ /*ENTER*/
  parent = TuiGetParent(msg->wnd);
  if (ch == nextmove)
  {
    /*parent = TuiGetParent(msg->wnd);*/
    /* kill focus the current active window */
    nextwnd = TuiGetNextActiveChildWnd(parent);
    nextchild = TuiGetNextActiveChildWnd(msg->wnd);
    if (nextchild)
    {
      rc = TuiSendMsg(msg->wnd, TWM_KILLFOCUS, 0, (LPARAM)nextchild);
    }
    else
    {
      rc = TuiSendMsg(msg->wnd, TWM_KILLFOCUS, 0, (LPARAM)nextwnd);
    }
    
    if (rc != TUI_CONTINUE)
    {
      return rc;
    }

    /* set focus the new active window */    
    prevwnd = msg->wnd;
    if (parent)
    {
      if (nextchild)
      {
        msg->wnd = nextchild;
        parent->activechild = nextchild;
      }
      else
      {
        if (msg->wnd == TuiGetLastActiveChildWnd(parent))
        {
          msg->wnd = TuiGetFirstActiveChildWnd(parent);
        }
        else
        {
          msg->wnd = nextwnd;
        }
        parent->activechild = msg->wnd;
      }
    }
    else
    {
      msg->wnd = TuiGetNextActiveChildWnd(env->activewnd);
      env->activewnd->activechild = msg->wnd;
    }
    if (msg->wnd)
    {
      rc = TuiPostMsg(msg->wnd, TWM_SETFOCUS, 0, (LPARAM)prevwnd);
      TuiMoveYX(TuiGetDC(msg->wnd), msg->wnd->y, msg->wnd->x);
    }
    return 0;    
  }
  else if (ch == prevmove)
  {
    /*parent = TuiGetParent(msg->wnd);*/
    /* kill focus the current active window */
    prevwnd = TuiGetPrevActiveChildWnd(parent);
    rc = TuiSendMsg(msg->wnd, TWM_KILLFOCUS, 0, (LPARAM)prevwnd);
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
        msg->wnd = TuiGetLastActiveChildWnd(parent);
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
      rc = TuiPostMsg(msg->wnd, TWM_SETFOCUS, 0, (LPARAM)nextwnd);
      TuiMoveYX(TuiGetDC(msg->wnd), msg->wnd->y, msg->wnd->x);
    }
    return 0;    
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

LONG TuiPostQuitMsg(LONG exitcode)
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

LONG TuiSetFocus(TWND wnd)
{
  TENV env = genvptr;
  TWND parent = (wnd ? wnd->parent : env->activewnd);
  TWND activewnd = TuiGetActiveChildWnd(wnd);
  LONG rc = TUI_CONTINUE;

  /* kill focus at the current control */
  rc = TuiSendMsg(activewnd, TWM_KILLFOCUS, 0, (LPARAM)wnd);
  if (rc != TUI_CONTINUE)
  {
    return rc;
  }
  if (wnd)
  {
    rc = TuiSendMsg(wnd, TWM_SETFOCUS, 0, (LPARAM)activewnd);
    TuiMoveYX(TuiGetDC(wnd), wnd->y, wnd->x);  

    parent->activechild = wnd;
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

LONG TuiPrintTextAlignment(LPSTR out, LPCSTR in, LONG limit, INT align)
{
  return _TuiAlignmentPrint(out, in, limit, align);
}

/*-------------------------------------------------------------------
 * DefWndProc functions
 *-----------------------------------------------------------------*/
LONG _TuiDefWndProc_OnEraseBk(TWND wnd, TDC dc);
VOID _TuiDefWndProc_OnSetText(TWND wnd, LPCSTR text);
LONG _TuiDefWndProc_OnGetText(TWND wnd, LPSTR text, LONG cb);
VOID _TuiDefWndProc_OnSetTextAlign(TWND wnd, INT align);
DWORD _TuiDefWndProc_OnSetTextAttrs(TWND wnd, DWORD attrs);
DWORD _TuiDefWndProc_OnGetTextAttrs(TWND wnd);

LONG _TuiDefWndProc_OnEraseBk(TWND wnd, TDC dc)
{
  INT i;
  CHAR buf[TUI_MAX_WNDTEXT + 1];
  RECT rc;
  DWORD attrs = TuiGetWndTextAttrs(wnd);
  
  if (TuiIsWndVisible(wnd))
  {
    TuiGetWndRect(wnd, &rc);
    memset(buf, ' ', rc.cols);
    buf[rc.cols] = 0;
    for (i = 0; i < rc.lines; ++i)
    {
      TuiDrawText(dc, rc.y + i, rc.x, buf, attrs);
    }
/*
    if (style & TWS_BORDER)
    {
      if (style & TWS_WINDOW)
      {
        TuiGetWndText(wnd, buf, TUI_MAX_WNDTEXT);
        TuiDrawFrame(dc, &rc, buf, TuiReverseColor(attrs));
      }
      else
      {
        TuiDrawBorder(dc, &rc);
      }
    }
*/
  }
  return TUI_OK;
}

VOID _TuiDefWndProc_OnSetText(TWND wnd, LPCSTR text)
{
  LONG len = 0;
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

LONG _TuiDefWndProc_OnGetText(TWND wnd, LPSTR text, LONG cb)
{
  LONG len = strlen(wnd->wndname);
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

VOID _TuiDefWndProc_OnSetTextAlign(TWND wnd, INT align)
{
  wnd->style &= ~(TWS_LEFT | TWS_CENTER | TWS_RIGHT);
  
  if (ALIGN_CENTER == align)
  {
    wnd->style |= TWS_CENTER;
  }
  else if (ALIGN_RIGHT == align)
  {
    wnd->style |= TWS_RIGHT;
  }
  else
  {
    wnd->style |= TWS_LEFT;
  }
}

DWORD _TuiDefWndProc_OnSetTextAttrs(TWND wnd, DWORD attrs)
{
  DWORD oldattrs = wnd->attrs;
  wnd->attrs = attrs;
  return oldattrs;
}

DWORD _TuiDefWndProc_OnGetTextAttrs(TWND wnd)
{
  return wnd->attrs;
}

LONG TuiDefWndProc(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_CREATE:
    case TWM_KILLFOCUS:
      return TUI_CONTINUE;
      
    case TWM_ERASEBK:
    {
      return _TuiDefWndProc_OnEraseBk(wnd, (TDC)wparam);
    }
    
    case TWM_SETTEXT:
    {
      _TuiDefWndProc_OnSetText(wnd, (LPCSTR)lparam);
      return 0;
    }    
    case TWM_GETTEXT:
    {
      return _TuiDefWndProc_OnGetText(wnd, (LPSTR)lparam, (LONG)wparam);
    }
    case TWM_SETTEXTALIGN:
    {
      _TuiDefWndProc_OnSetTextAlign(wnd, (INT)wparam);
    }
    case TWM_SETTEXTATTRS:
    {
      return _TuiDefWndProc_OnSetTextAttrs(wnd, (DWORD)wparam);
    }
    case TWM_GETTEXTATTRS:
    {
      _TuiDefWndProc_OnGetTextAttrs(wnd);
    }
  }
  return TUI_OK;
}
/*
UINT TuiEndDlg(TWND wnd, UINT id)
{
  wnd->dlgmsgid = id;
  TuiPostMsg(TuiGetParent(wnd), TWM_DLGMSGID, (WPARAM)id, 0);
  TuiDestroyWnd(wnd);
  return id;
}
*/
DWORD TuiGetColor(INT idx)
{
#ifdef __USE_CURSES__
  return COLOR_PAIR(idx);
#endif
  return 0;
}

DWORD TuiGetSysColor(INT idx)
{
  TENV env = genvptr;
  if (idx < 0 || idx >= COLOR_LAST)
  {
    idx = 0;
  }
#ifdef __USE_CURSES__
  return COLOR_PAIR(env->themes[env->themeid].theme[idx].id);
#endif
  return 0;
}

DWORD TuiGetReverseSysColor(INT idx)
{
#ifdef __USE_CURSES__
  return TuiGetSysColor(idx)|A_REVERSE;
#endif
  return 0;
}

DWORD TuiReverseColor(DWORD color)
{
#ifdef __USE_CURSES__
  return color|A_REVERSE;
#endif
  return color;
}

DWORD TuiUnderlineText(DWORD attrs)
{
#ifdef __USE_CURSES__
  return attrs|A_UNDERLINE;
#endif
  return attrs;
}

DWORD TuiGetSysColorTheme(INT themeid, INT idx)
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
#endif
  return 0;
}

DWORD TuiGetReverseSysColorTheme(INT themeid, INT idx)
{
#ifdef __USE_CURSES__
  return TuiGetSysColorTheme(themeid, idx)|A_REVERSE;
#endif
  return 0;
}
