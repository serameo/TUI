/*-------------------------------------------------------------------
 * File name: tdc.c
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
 * DC functions
 *-----------------------------------------------------------------*/

LONG TuiPutChar(
  TDC dc,
  INT y,
  INT x,
  CHAR   ch,
  DWORD  attrs)
{
#ifdef __USE_CURSES__
  /* set attributes on */
  attron(attrs);
  /* print now */
  mvwaddch(dc->win, y, x, ch);
  /* set attributes off */
  attroff(attrs);
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  dc->win->lType = QIO_ANY;
  dc->win->cColor = 'W';
  dc->win->lLength = 1;
  dc->win->lRow = y;
  dc->win->lColumn = x;
  strcpy( dc->win->szHelp, "");
  sprintf(dc->win->szText, "%c", ch);
  QIO_Put();
#endif
  return TUI_OK;
}

LONG TuiDrawTextEx(TDC dc, INT y, INT x, INT cols, LPCSTR text, LONG len, DWORD attrs, INT align)
{
  if (DT_CENTER == align)
  {
    if (len)
    {
      return TuiDrawText(dc, y, x + (cols - len) / 2, text, attrs);
    }
    else
    {
      return TuiDrawText(dc, y, x, text, attrs);
    }
  }
  else if (DT_RIGHT == align)
  {
    if (len < cols)
    {
      return TuiDrawText(dc, y, x + cols - len, text, attrs);
    }
    else
    {
      return TuiDrawText(dc, y, x, text, attrs);
    }
  }
  return TuiDrawText(dc, y, x, text, attrs);
}

LONG TuiDrawText(
  TDC dc,
  INT y,
  INT x,
  LPCSTR text,
  DWORD  attrs)
{
#ifdef __USE_CURSES__
  if (y >= LINES)
  {
    y = LINES - 1;
  }
  if (x >= COLS)
  {
    x = COLS - 1;
  }
  /* set attributes on */
  attron(attrs);
  /* print now */
  mvwprintw(dc->win, y, x, text);
  /* set attributes off */
  attroff(attrs);
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  dc->win->lType = QIO_ANY;/*QIO_INTEGER | QIO_UPPERCASE;*/
  dc->win->cColor = 'W';
  dc->win->lLength = strlen(text);
  dc->win->lRow = y;
  dc->win->lColumn = x;
  strcpy(dc->win->szHelp, "");
  strcpy(dc->win->szText, text);
  QIO_Put();
#endif
  return TUI_OK;
}

LONG TuiMoveYX(TDC dc, INT y, INT x)
{
#ifdef __USE_CURSES__
  wmove(dc->win, y, x);
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  dc->win->lRow    = y;
  dc->win->lColumn = x;
#endif
  return TUI_OK;
}


LONG TuiGetYX(TDC dc, INT* y, INT* x)
{
  int xx = 0, yy = 0;
#ifdef __USE_CURSES__
  getyx(dc->win, yy, xx);
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  yy = dc->win->lRow;
  xx = dc->win->lColumn;
#endif
  *y = yy;
  *x = xx;
  return TUI_OK;
}

LONG TuiDrawFrame(TDC dc, RECT* rcframe, LPCSTR caption, DWORD attrs)
{
  CHAR buf[TUI_MAX_WNDTEXT+1];
  
  TuiDrawBorder(dc, rcframe);
  if (caption)
  {
    TuiPrintTextAlignment(buf, caption, strlen(caption), DT_CENTER);
    TuiDrawText(dc, rcframe->y,
      rcframe->x + (rcframe->cols - strlen(caption))/2, buf, attrs);
  }
  return TUI_OK;
}

LONG TuiDrawMultipleFrames(TDC dc, RECT* rcframe, LPCSTR caption, DWORD attrs, INT* widths)
{
  return TUI_OK;
}

LONG TuiDrawBorder(TDC dc, RECT* rcwnd)
{
#ifdef __USE_CURSES__
  /* left vertical line */
  mvwvline(dc->win, rcwnd->y, rcwnd->x, ACS_VLINE, rcwnd->lines);
  /* right vertical line */
  mvwvline(dc->win, rcwnd->y, rcwnd->x + rcwnd->cols, ACS_VLINE, rcwnd->lines);
  /* top horizontal line */
  mvwhline(dc->win, rcwnd->y, rcwnd->x, ACS_HLINE, rcwnd->cols);
  /* bottom horizontal line */
  mvwhline(dc->win, rcwnd->y + rcwnd->lines, rcwnd->x, ACS_HLINE, rcwnd->cols);
  /* upper left */
  mvwaddch(dc->win, rcwnd->y, rcwnd->x, ACS_ULCORNER);
  /* upper right */
  mvwaddch(dc->win, rcwnd->y, rcwnd->x + rcwnd->cols, ACS_URCORNER);
  /* lower left */
  mvwaddch(dc->win, rcwnd->y + rcwnd->lines, rcwnd->x, ACS_LLCORNER);
  /* lower right */
  mvwaddch(dc->win, rcwnd->y + rcwnd->lines, rcwnd->x + rcwnd->cols, ACS_LRCORNER);
#endif
  return TUI_OK;
}

