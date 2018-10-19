/*-------------------------------------------------------------------
 * File name: tstc.c
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
 * STATIC functions
 *-----------------------------------------------------------------*/
VOID _TSTC_OnPaint(TWND wnd, TDC dc);
LONG _TSTC_OnCreate(TWND wnd);
LONG STATICPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

LONG _TSTC_OnCreate(TWND wnd)
{
  TuiEnableWnd(wnd, TW_DISABLE);
  return TUI_CONTINUE;
}

VOID _TSTC_OnPaint(TWND wnd, TDC dc)
{
  CHAR buf[TUI_MAX_WNDTEXT+1];
  CHAR text[TUI_MAX_WNDTEXT+1];
  RECT rc;

  if (TuiIsWndVisible(wnd))
  {
    TuiGetWndRect(wnd, &rc);
    
    TuiGetWndText(wnd, text, TUI_MAX_WNDTEXT);
    TuiPrintTextAlignment(buf, text, rc.cols, TuiGetWndStyle(wnd));
    TuiDrawText(dc, rc.y, rc.x, buf, TuiGetWndTextAttrs(wnd));
  }
}

LONG STATICPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_PAINT:
    {
      _TSTC_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
    case TWM_CREATE:
    {
      return _TSTC_OnCreate(wnd);
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}

