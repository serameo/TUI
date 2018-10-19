/*-------------------------------------------------------------------
 * File name: tlctl.c
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
 * LISTCTRL functions
 *-----------------------------------------------------------------*/
/* listctrl procedure is a based class of listpagectrl */
LONG LISTCTRLPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);
VOID _TLPC_OnKeyDown(TWND wnd, LONG ch);
VOID _TLPC_OnPaint(TWND wnd, TDC dc);

VOID _TLPC_OnKeyDown(TWND wnd, LONG ch)
{
  TLISTCTRL lctl = 0;
  INT repaint = 0;
  RECT rc;
  INT lines = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  TuiGetWndRect(wnd, &rc);
  
  if (lctl->nheaders <= 0)
  {
    return;
  }
  
  switch (ch)
  {
    case TVK_SPACE:
    {
      /* toggle begin/end moving */
      _TLPC_OnBeginMoving(wnd);
      break;
    }
    
#if defined __USE_CURSES__
    case KEY_F(6):
#elif defined __USE_QIO__
    case KEY_F6:
#endif
    {
      /* end moving */
      _TLPC_OnBeginEdit(wnd);
      /* begin edit */
      break;
    }
    
    case TVK_ENTER:
    case KEY_ENTER:
    {
      if (lctl->state == LCS_EDITING)
      {
        _TLPC_OnEndEdit(wnd, 1); /* edit ok */
      }
      break;
    }

#if defined __USE_CURSES__
    case KEY_F(7):
#elif defined __USE_QIO__
    case KEY_F7:
#endif
    {
      if (lctl->state == LCS_EDITING)
      {
        _TLPC_OnEndEdit(wnd, 0); /* edit cancel */
      }
      break;
    }

    case KEY_RIGHT:
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        _TLPC_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else if (lctl->lastvisiblehdr != lctl->lasthdr)
      {
        lctl->firstvisiblehdr = lctl->firstvisiblehdr->next;
        ++repaint;
      }
      break;
    }
    case KEY_LEFT:
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        _TLPC_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else if (lctl->firstvisiblehdr != lctl->firsthdr)
      {
        lctl->firstvisiblehdr = lctl->firstvisiblehdr->prev;
        ++repaint;
      }
      break;
    }
    case KEY_DOWN:
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        _TLPC_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else
      {
        ++lines;
        ++repaint;
      }
      break;
    }
  
    case KEY_UP:
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        _TLPC_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else
      {
        --lines;
        ++repaint;
      }
      break;
    }
      
    case TVK_PRIOR:
#ifdef __USE_CURSES__
    case KEY_PPAGE:
#elif (defined __USE_QIO__ && defined __VMS__)
    case KEY_PREV:
#endif
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        _TLPC_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else
      {
        lines -= rc.lines - 1;
        ++repaint;
      }
      break;
    }
    
    case TVK_NEXT:
#ifdef __USE_CURSES__
    case KEY_NPAGE:
#elif (defined __USE_QIO__ && defined __VMS__)
    case KEY_NEXT:
#endif
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        _TLPC_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else
      {
        lines += rc.lines - 1;
        ++repaint;
      }
      break;
    }
  }
  if (repaint)
  {
    lctl->curselrow += lines;
    if (lctl->curselrow >= lctl->nitems)
    {
      lctl->curselrow = lctl->nitems - 1;
    }
    else if (lctl->curselrow < 0)
    {
      lctl->curselrow = 0;
    }
    
    if (lctl->curselrow >= lctl->firstvisiblerow + rc.lines - 1)
    {
      lctl->firstvisiblerow += lines;
      if (lctl->firstvisiblerow - 1 > lctl->nitems - rc.lines)
      {
        lctl->firstvisiblerow = lctl->nitems - rc.lines;
      }
    }
    else if (lctl->firstvisiblerow > lctl->curselrow)
    {
      lctl->firstvisiblerow += lines;
      if (lctl->firstvisiblerow < 0)
      {
        lctl->firstvisiblerow = 0;
      }
    }
    TuiInvalidateWnd(wnd);
    /* send notification */
    _TLPC_OnSelChanged(wnd);
  }
}

LONG LISTPAGECTRLPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_KEYDOWN:
    {
      _TLPC_OnKeyDown(wnd, (LONG)wparam);
      break;
    }
    case TWM_PAINT:
    {
      _TLPC_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
    case TLPCM_ADDITEM:
    {
      return LISTCTRLPROC(wnd, TLCM_ADDITEM, (WPARAM)3, lparam);
    }
    case TLPCM_DELETEITEM:
    {
      LISTCTRLPROC(wnd, TLCM_DELETEITEM, wparam, lparam);
      return 0;
    }
    case TLPCM_DELETEALLITEMS:
    {
      LISTCTRLPROC(wnd, TLCM_DELETEALLITEMS, wparam, lparam);
      return 0;
    }
    case TLPCM_SETITEMTEXT:
    {
      return LISTCTRLPROC(wnd, TLCM_SETITEM, wparam, lparam);
    }
    case TLPCM_GETITEMTEXT:
    {
      return LISTCTRLPROC(wnd, TLCM_GETITEM, wparam, lparam);
    }
    case TLPCM_GETITEMCOUNT:
    {
      return LISTCTRLPROC(wnd, TLCM_GETITEMCOUNT, wparam, lparam);
    }
    case TLPCM_SETEDITSTYLE:
    {
      _TLPC_OnSetEditStyle(wnd, (INT)wparam, (DWORD)lparam);
      return 0;
    }
    case TLPCM_GETEDITSTYLE:
    {
      return _TLPC_OnGetEditStyle(wnd, (INT)wparam);
    }
    case TLPCM_GETCURSELROW:
    {
      return LISTCTRLPROC(wnd, TLCM_GETCURSELROW, wparam, lparam);
    }
    case TLPCM_SETCURSELROW:
    {
      LISTCTRLPROC(wnd, TLCM_SETCURSELROW, wparam, lparam);
      return 0;
    }
  }
  return LISTCTRLPROC(wnd, msg, wparam, lparam);
}

