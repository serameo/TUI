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
  TINT              state;
  TVOID*            exparam;
};
typedef struct _TBUTTONSTRUCT _TBUTTON;
typedef struct _TBUTTONSTRUCT *PTBUTTON;

TLONG _TBTN_OnCreate(TWND wnd);
TVOID _TBTN_OnPaint(TWND wnd, TDC dc);
TVOID _TBTN_OnSetFocus(TWND wnd);
TLONG _TBTN_OnKillFocus(TWND wnd);
TVOID _TBTN_OnDestroy(TWND wnd);
TVOID _TBTN_OnKeyDown(TWND wnd, TLONG ch);
TLONG BUTTONPROC(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam);

TVOID _TBTN_OnPaint(TWND wnd, TDC dc)
{
  TLONG len = 0;
  TTCHAR buf[TUI_MAX_WNDTEXT+1];
  TTCHAR text[TUI_MAX_WNDTEXT+1];
  TDWORD attrs = TuiGetWndTextAttrs(wnd);
  TRECT rc;
  PTBUTTON btn = 0;
  
  btn = (PTBUTTON)TuiGetWndParam(wnd);
  TuiGetWndRect(wnd, &rc);
  
  if (TuiIsWndVisible(wnd))
  {
    TuiGetWndText(wnd, text, TUI_MAX_WNDTEXT);
    len = TuiPrintTextAlignment(buf,
      text,
      rc.cols,
      TALIGN_CENTER);

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
    else
    {
      attrs = TuiGetSysColor(COLOR_BTNENABLED);
    }
    TuiDrawText(dc, rc.y, rc.x, buf, attrs);
  }
  TuiMoveYX(dc, rc.y, rc.x);  
}

TLONG _TBTN_OnCreate(TWND wnd)
{
  PTBUTTON btn = 0;
  /* initial memory for static control */
  btn = (PTBUTTON)malloc(sizeof(_TBUTTON));
  if (!btn)
  {
    return TUI_ERROR;
  }
  memset(btn, 0, sizeof(_TBUTTON));
  btn->state = TBS_RELEASED;
  
  TuiSetWndParam(wnd, (TLPVOID)btn);
  /*TuiSetWndTextAttrs(wnd, TuiGetSysColor(COLOR_BTNENABLED));*/
  
  return TUI_CONTINUE;
}

TVOID _TBTN_OnSetFocus(TWND wnd)
{
  TNMHDR nmhdr;
  PTBUTTON btn = (PTBUTTON)TuiGetWndParam(wnd);

  btn->state = TBS_FOCUSED; /* add state */
  TuiInvalidateWnd(wnd);
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TBN_SETFOCUS;

  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (TLPARAM)&nmhdr);
}

TLONG _TBTN_OnKillFocus(TWND wnd)
{
  TNMHDR nmhdr;
  TLONG rc = TUI_CONTINUE;
  PTBUTTON btn = (PTBUTTON)TuiGetWndParam(wnd);

  btn->state &= ~TBS_FOCUSED; /* remove state */
  TuiInvalidateWnd(wnd);
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TBN_KILLFOCUS;
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (TLPARAM)&nmhdr);

  return rc;
}

TVOID _TBTN_OnDestroy(TWND wnd)
{
  PTBUTTON btn = (PTBUTTON)TuiGetWndParam(wnd);
   
  free(btn);
}

TVOID _TBTN_OnKeyDown(TWND wnd, TLONG ch)
{
  PTBUTTON btn = (PTBUTTON)TuiGetWndParam(wnd);
  TINT repaTINT = 0;
  
  switch (ch)
  {
    case TVK_SPACE:
      repaTINT = 1;
      break;
  }
  if (repaTINT)
  {
    btn->state = TBS_PRESSED;
    TuiInvalidateWnd(wnd);

    btn->state = TBS_RELEASED | TBS_FOCUSED;
    TuiInvalidateWnd(wnd);
    
    /* send notification */
    TuiPostMsg(TuiGetParent(wnd), 
      TWM_COMMAND,
      (TWPARAM)TuiGetWndID(wnd),
      (TLPARAM)wnd);
  }
}

TLONG BUTTONPROC(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam)
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
      _TBTN_OnKeyDown(wnd, (TLONG)wparam);
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


