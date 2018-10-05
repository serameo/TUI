/*-------------------------------------------------------------------
 * File name: tpgctl.c
 * Author: Seree Rakwong
 * Date: 04-OCT-18
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
 * PAGECTRL functions
 *-----------------------------------------------------------------*/
struct _PAGESTRUCT
{
  INT  nchildren;
  TWND firstchild;
  TWND lastchild;
  TWND activechild;
};
typedef struct _PAGESTRUCT tpage_t;

struct _PAGECTRLSTRUCT
{
  INT      npages;
  tpage_t* firstpage;
  tpage_t* lastpage;
};
typedef struct _PAGECTRLSTRUCT _TPAGECTRL;
typedef struct _PAGECTRLSTRUCT *TPAGECTRL;

VOID TPGCTL_OnPaint(TWND wnd, TDC dc);
LONG TPGCTL_OnCreate(TWND wnd);
LONG STATICPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

VOID TPGCTL_OnPaint(TWND wnd, TDC dc)
{
  CHAR buf[TUI_MAX_WNDTEXT+1];
  CHAR text[TUI_MAX_WNDTEXT+1];
  RECT rc;

  if (!TuiIsWndVisible(wnd))
  {
    return;
  }
  TuiGetWndRect(wnd, &rc);
  
  TuiGetWndText(wnd, text, TUI_MAX_WNDTEXT);
  TuiPrintTextAlignment(buf, text, rc.cols, TuiGetWndStyle(wnd));
  TuiDrawText(dc, rc.y, rc.x, buf, TuiGetWndTextAttrs(wnd));
}

LONG PAGECTRLPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_PAINT:
    {
      TPGCTL_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}

