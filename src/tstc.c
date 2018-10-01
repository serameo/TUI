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
VOID TSTC_OnPaint(TWND wnd, TDC dc);
LONG TSTC_OnCreate(TWND wnd);
LONG STATICPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

VOID TSTC_OnPaint(TWND wnd, TDC dc)
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

LONG TSTC_OnCreate(TWND wnd)
{
#ifdef __USE_CURSES__
  TuiSetWndTextAttrs(wnd, COLOR_PAIR(GREEN_BLACK));
#elif defined __USE_QIO__
  TuiSetWndTextAttrs(wnd, GREEN_BLACK);
#endif
  
  return TUI_CONTINUE;
}

LONG STATICPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_CREATE:
    {
      /* initial memory for static control */
      return TSTC_OnCreate(wnd);
    }
    case TWM_DESTROY:
    {
      /* release memory of static control */
      return 0;
    }
    case TWM_PAINT:
    {
      TSTC_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}