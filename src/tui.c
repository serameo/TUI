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
  UINT             dlgmsgid;
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
  UINT             dlgmsgid;
  
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

LONG STATICPROC(TWND, UINT, WPARAM, LPARAM);
LONG EDITPROC(TWND, UINT, WPARAM, LPARAM);
LONG LISTBOXPROC(TWND, UINT, WPARAM, LPARAM);
LONG BUTTONPROC(TWND, UINT, WPARAM, LPARAM);
LONG LISTCTRLPROC(TWND, UINT, WPARAM, LPARAM);

LONG MSGBOXPROC(TWND, UINT, WPARAM, LPARAM);

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

LONG TuiStartup()
{
  TENV env = (TENV)malloc(sizeof(_TENV));
  if (env)
  {
    genvptr = env;
    memset(env, 0, sizeof(_TENV));

    /* register standard controls */
    TuiRegisterCls(STATIC, STATICPROC);
    TuiRegisterCls(EDIT, EDITPROC);
    TuiRegisterCls(LISTBOX, LISTBOXPROC);
    TuiRegisterCls(BUTTON, BUTTONPROC);
    TuiRegisterCls(LISTCTRL, LISTCTRLPROC);
    TuiRegisterCls(MSGBOX, MSGBOXPROC);

    /* device context */
#ifdef __USE_CURSES__
    /* curses lib */
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    
    start_color();
    
    init_pair(CYAN_BLACK, COLOR_CYAN, COLOR_BLACK);
    init_pair(BLACK_CYAN, COLOR_BLACK, COLOR_GREEN);
    init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);
    init_pair(BLACK_GREEN, COLOR_BLACK, COLOR_GREEN);
    init_pair(YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
    init_pair(BLACK_YELLOW, COLOR_BLACK, COLOR_YELLOW);
    init_pair(BLUE_YELLOW, COLOR_BLUE, COLOR_YELLOW);
    init_pair(YELLOW_BLUE, COLOR_YELLOW, COLOR_BLUE);
    
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

VOID TuiSetNextMove(LONG nextmove)
{
  TENV env = genvptr;
  env->nextmove = nextmove;
}

UINT TuiGetDlgMsgID()
{
  TENV env = genvptr;
  return env->dlgmsgid;
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
        env->dlgmsgid  = MB_INVALID;
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
        }
        /* re-size window rectangle */
        wnd->y       = (parent->y + y);
        wnd->x       = (parent->x + x);
        wnd->lines   = (lines > parent->lines ? parent->lines : lines);
        wnd->cols    = (cols  > parent->cols  ? parent->cols  : cols);
        /* send message */
        TuiSendMsg(wnd, TWM_ERASEBK, (WPARAM)TuiGetDC(wnd), 0);
        TuiSendMsg(wnd, TWM_CREATE, 0, 0);
      }
    }
  }
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
  int  i = 0;

  if (templs)
  {
    wnd = _TuiCreateWnd(
               templs[i].clsname,
               templs[i].text,
               templs[i].style,
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
  env->dlgmsgid = wnd->dlgmsgid;
  
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
  LONG len = (wnd ? strlen(wnd->wndname) : 0);
  if (cb < 0)
  {
    return len;
  }
  memset(text, 0, cb);
  if (cb > len)
  {
    cb = len;
  }
  strncpy(text, wnd->wndname, cb);
  return strlen(text);
}

VOID TuiSetWndText(TWND wnd, LPCSTR text)
{
  LONG len = 0;
  if (text && wnd)
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
#ifdef __USE_CURSES__
  msg->wparam = wgetch(stdscr);
#elif defined __USE_QIO__

  qio_field.lType = QIO_ANY | QIO_NO_ECHO;
  qio_field.cColor = 'W';
  qio_field.lLength = 1;
  qio_field.lRow = 1;
  qio_field.lColumn = 1;
  qio_field.bReprint = 0;

  QIO_Get();
  
  msg->wparam = qio_field.lTerm;
#endif

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
  if (ch == nextmove)
  {
    parent = TuiGetParent(msg->wnd);
    /* kill focus the current active window */
    nextwnd = TuiGetNextActiveChildWnd(parent);
    rc = TuiSendMsg(msg->wnd, TWM_KILLFOCUS, 0, (LPARAM)nextwnd);
    if (rc != TUI_CONTINUE)
    {
      return rc;
    }

    /* set focus the new active window */
    prevwnd = msg->wnd;
    if (parent)
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
    parent = TuiGetParent(msg->wnd);
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
VOID _TuiDefWndProc_OnSetTextAttrs(TWND wnd, DWORD attrs);

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
  
  if (cb > len)
  {
    cb = len;
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

VOID _TuiDefWndProc_OnSetTextAttrs(TWND wnd, DWORD attrs)
{
  wnd->attrs = attrs;
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
      _TuiDefWndProc_OnSetTextAttrs(wnd, (DWORD)wparam);
    }
  }
  return TUI_OK;
}

UINT TuiEndDlg(TWND wnd, UINT id)
{
  wnd->dlgmsgid = id;
  TuiPostMsg(TuiGetParent(wnd), TWM_DLGMSGID, (WPARAM)id, 0);
  TuiDestroyWnd(wnd);
  return id;
}

