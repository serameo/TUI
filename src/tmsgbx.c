/*-------------------------------------------------------------------
 * File name: tmsgbx.c
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
 * Message box functions
 *-----------------------------------------------------------------*/
struct _MSGBOXSTRUCT
{
  CHAR* caption;
  CHAR* text;
  UINT  nbtns;
};

VOID TMSGB_OnPaint(TWND wnd, TDC dc);

VOID TMSGB_OnPaint(TWND wnd, TDC dc)
{
  CHAR text[TUI_MAX_WNDTEXT+1];
  CHAR buf[TUI_MAX_WNDTEXT+1];
  RECT rc;
  
  TuiGetWndRect(wnd, &rc);
  TuiGetWndText(wnd, text, TUI_MAX_WNDTEXT);
  TuiPrintTextAlignment(buf, text, rc.cols, ALIGN_CENTER);
  TuiDrawText(dc, rc.y + 3, rc.x, buf, TuiGetWndTextAttrs(wnd));
}

LONG MSGBOXPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_INITDIALOG:
    {
      TWND btn = TuiGetFirstActiveChildWnd(wnd);
      if (btn)
      {
        TuiSetFocus(btn);
      }
#ifdef __USE_CURSES__
      TuiSendMsg(wnd, TWM_SETTEXTATTRS, (WPARAM)(A_REVERSE), (LPARAM)0);
#endif
      return TUI_CONTINUE;
    }
    
    case TWM_PAINT:
    {
      TMSGB_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }

    case TWM_COMMAND:
    {
      TuiEndDlg(wnd, (UINT)wparam);
      break;
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}
