/*-------------------------------------------------------------------
 * File name: tbtn.c
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

/*-------------------------------------------------------------------
 * BUTTON functions
 *-----------------------------------------------------------------*/
 struct _TBUTTONSTRUCT
{
  INT              state;
  VOID*            exparam;
};
typedef struct _TBUTTONSTRUCT _TBUTTON;
typedef struct _TBUTTONSTRUCT *TBUTTON;

LONG _TBTN_OnCreate(TWND wnd);
VOID _TBTN_OnPaint(TWND wnd, TDC dc);
VOID _TBTN_OnSetFocus(TWND wnd);
LONG _TBTN_OnKillFocus(TWND wnd);
VOID _TBTN_OnDestroy(TWND wnd);
VOID _TBTN_OnKeyDown(TWND wnd, LONG ch);
LONG BUTTONPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

VOID _TBTN_OnPaint(TWND wnd, TDC dc)
{
  LONG len = 0;
  CHAR buf[TUI_MAX_WNDTEXT+1];
  CHAR text[TUI_MAX_WNDTEXT+1];
  DWORD attrs = TuiGetWndTextAttrs(wnd);
  RECT rc;
  TBUTTON btn = 0;
  
  btn = (TBUTTON)TuiGetWndParam(wnd);
  TuiGetWndRect(wnd, &rc);
  
  if (TuiIsWndVisible(wnd))
  {
    TuiGetWndText(wnd, text, TUI_MAX_WNDTEXT);
    len = TuiPrintTextAlignment(buf,
      text,
      rc.cols,
      ALIGN_CENTER);

    buf[0] = '[';
    buf[len-1] = ']';
    
    if (!TuiIsWndEnabled(wnd))
    {
      attrs = TuiGetSysColor(COLOR_BTNDISABLED);
    }
    else if (btn->state == TBS_RELEASED)
    {
      attrs = TuiGetSysColor(COLOR_BTNFOCUSED);
    }
    TuiDrawText(dc, rc.y, rc.x, buf, attrs);
  }
  TuiMoveYX(dc, rc.y, rc.x);  
}

LONG _TBTN_OnCreate(TWND wnd)
{
  TBUTTON btn = 0;
  /* initial memory for static control */
  btn = (TBUTTON)malloc(sizeof(_TBUTTON));
  if (!btn)
  {
    return TUI_ERROR;
  }
  memset(btn, 0, sizeof(_TBUTTON));
  btn->state = TBS_RELEASED;
  
  TuiSetWndParam(wnd, (LPVOID)btn);
  TuiSetWndTextAttrs(wnd, TuiGetSysColor(COLOR_BTNENABLED));
  
  return TUI_CONTINUE;
}

VOID _TBTN_OnSetFocus(TWND wnd)
{
  NMHDR nmhdr;
  TBUTTON btn = (TBUTTON)TuiGetWndParam(wnd);

  btn->state = TBS_FOCUSED; /* add state */
  TuiInvalidateWnd(wnd);
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TBN_SETFOCUS;

  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}

LONG _TBTN_OnKillFocus(TWND wnd)
{
  NMHDR nmhdr;
  LONG rc = TUI_CONTINUE;
  TBUTTON btn = (TBUTTON)TuiGetWndParam(wnd);

  btn->state &= ~TBS_FOCUSED; /* remove state */
  TuiInvalidateWnd(wnd);
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TBN_KILLFOCUS;
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);

  return rc;
}

VOID _TBTN_OnDestroy(TWND wnd)
{
  TBUTTON btn = (TBUTTON)TuiGetWndParam(wnd);
   
  free(btn);
}

VOID _TBTN_OnKeyDown(TWND wnd, LONG ch)
{
  TBUTTON btn = (TBUTTON)TuiGetWndParam(wnd);
  INT repaint = 0;
  
  switch (ch)
  {
    case TVK_SPACE:
      repaint = 1;
      break;
  }
  if (repaint)
  {
    btn->state = TBS_PRESSED;
    TuiInvalidateWnd(wnd);

    btn->state = TBS_RELEASED | TBS_FOCUSED;
    TuiInvalidateWnd(wnd);
    
    /* send notification */
    TuiPostMsg(TuiGetParent(wnd), 
      TWM_COMMAND,
      (WPARAM)TuiGetWndID(wnd),
      (LPARAM)wnd);
  }
}

LONG BUTTONPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_CREATE:
    {
      /* initial memory for static control */
      return _TBTN_OnCreate(wnd);
    }
    case TWM_DESTROY:
    {
      /* release memory of static control */
      _TBTN_OnDestroy(wnd);
      return 0;
    }
    case TWM_SETFOCUS:
    {
      _TBTN_OnSetFocus(wnd);
      break;
    }
    case TWM_KILLFOCUS:
    {
      return _TBTN_OnKillFocus(wnd);
    }
    case TWM_KEYDOWN:
    {
      _TBTN_OnKeyDown(wnd, (LONG)wparam);
      break;
    }
    case TWM_PAINT:
    {
      _TBTN_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
    case TBM_PRESS:
    {
      _TBTN_OnSetFocus(wnd);
      _TBTN_OnKeyDown(wnd, TVK_SPACE);
      break;
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}


