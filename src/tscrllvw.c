/*-------------------------------------------------------------------
 * File name: tscrllvw.c
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
 * SCROLLVIEW functions
 *-----------------------------------------------------------------*/
struct _TBANDSTRUCT
{
  INT         y;       /* always 0 */
  INT         x;
  INT         lines;
  INT         cols;    /* always = parent cols */
  INT         id;
  TWND        band;
  CHAR        name[TUI_MAX_WNDTEXT+1];
  DWORD       attrs;
  
  struct _TBANDSTRUCT *prev;
  struct _TBANDSTRUCT *next;
};
typedef struct _TBANDSTRUCT tband_t;

LONG LISTCTRLPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}

