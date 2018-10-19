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
#include <descrip.h>
#include <iodef.h>
#include <lib$routines.h>
#include <ssdef.h>
#include <starlet.h>
#include <stdio.h>
#include <string.h>
#include <stsdef.h>

#include <qio_init.h>
#endif

#include "tui.h"

/*-------------------------------------------------------------------
 * DC functions
 *-----------------------------------------------------------------*/
/*
 https://en.wikipedia.org/wiki/ANSI_escape_code
*/

LONG TuiPutChar(
  TDC dc,
  INT y,
  INT x,
  CHAR   ch,
  DWORD  attrs)
{
#if defined __USE_CURSES__
  /* set attributes on */
  attron(attrs);
  /* print now */
  mvwaddch(dc->win, y, x, ch);
  /* set attributes off */
  attroff(attrs);
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  CHAR prompt[TUI_MAX_WNDTEXT+1];
  INT outlen = 1;
  CHAR buffer[2] = { ch, 0 };
  LONG len;
  UINT32 status;
  
  sprintf(prompt, "[%d;%dH", y, x);
  len = strlen(prompt);
  
  status = SYS$QIOW(0, 
              dc->win->ttchan,
              IO$_WRITEVBLK | IO$M_NOFORMAT,
              &dc->win->ttiosb,
              0, 0,
              buffer,
              outlen,
              0,
              0,
              prompt,
              len);
  if (!$VMS_STATUS_SUCCESS( status ))
  {
    LIB$SIGNAL( status );
  }
#endif
  return TUI_OK;
}

LONG TuiDrawText(
  TDC dc,
  INT y,
  INT x,
  LPCSTR text,
  DWORD  attrs)
{
#if defined __USE_CURSES__
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
  CHAR prompt[TUI_MAX_WNDTEXT+1];
  INT outlen = strlen(text);
  CHAR buffer[2] = { 0, 0 };
  LONG len;
  UINT32 status;
  
  sprintf(prompt, "[%d;%dH", y, x);
  len = strlen(prompt);
  
  status = SYS$QIOW(0, 
              dc->win->ttchan,
              IO$_WRITEVBLK | IO$M_NOFORMAT,
              &dc->win->ttiosb,
              0, 0,
              text,
              outlen,
              0,
              0,
              prompt,
              len);
  if (!$VMS_STATUS_SUCCESS( status ))
  {
    LIB$SIGNAL( status );
  }
#endif
  return TUI_OK;
}

LONG TuiMoveYX(TDC dc, INT y, INT x)
{
#if defined __USE_CURSES__
  wmove(dc->win, y, x);
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  CHAR prompt[TUI_MAX_WNDTEXT+1];
  LONG len;
  UINT32 status;
  
  sprintf(prompt, "[%d;%dH", y, x);
  len = strlen(prompt);
  
  status = SYS$QIOW(0, 
              dc->win->ttchan,
              IO$_WRITEVBLK| IO$M_NOFORMAT,
              &dc->win->ttiosb,
              0, 0,
              0,
              0,
              0,
              0,
              prompt,
              len);
  if (!$VMS_STATUS_SUCCESS( status ))
  {
    LIB$SIGNAL( status );
  }
#endif
  return TUI_OK;
}


LONG TuiGetYX(TDC dc, INT* y, INT* x)
{
  int xx = 0, yy = 0;
#if defined __USE_CURSES__
  getyx(dc->win, yy, xx);
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
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
#if defined __USE_CURSES__
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
#elif defined __USE_QIO__
#endif
  return TUI_OK;
}

LONG TuiDrawBorderEx(TDC dc, RECT* rcwnd, DWORD attrs)
{
#if defined __USE_CURSES__
  attron(attrs);
#endif
  TuiDrawBorder(dc, rcwnd);
#if defined __USE_CURSES__
  attroff(attrs);
#endif
  return TUI_OK;
}

LONG TuiDrawHLine(TDC dc, INT y, INT x, INT nchars, DWORD attrs)
{
#if defined __USE_CURSES__
  attron(attrs);
  /* horizontal line */
  mvwhline(dc->win, y, x, ACS_HLINE, nchars);
  attroff(attrs);
#elif defined __USE_QIO__
#endif
  return TUI_OK;
}

LONG TuiDrawVLine(TDC dc, INT y, INT x, INT nchars, DWORD attrs)
{
#if defined __USE_CURSES__
  attron(attrs);
  /* vertical line */
  mvwvline(dc->win, y, x, ACS_VLINE, nchars);
  attroff(attrs);
#elif defined __USE_QIO__
#endif
  return TUI_OK;
}

