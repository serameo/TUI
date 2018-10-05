/*-------------------------------------------------------------------
 * File name: tedt.c
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

#include "m_tui.h"

/*-------------------------------------------------------------------
 * EDIT functions
 *-----------------------------------------------------------------*/

struct _TEDITSTRUCT
{
  CHAR             editbuf[TUI_MAX_WNDTEXT+1];
  INT              firstvisit;
  CHAR             passchar;
  INT              showpass;
  INT              firstchar;
  INT              limitchars;
  INT              editing;
  INT              decwidth;
};
typedef struct _TEDITSTRUCT _TEDIT;
typedef struct _TEDITSTRUCT *TEDIT;

LONG TEDT_OnCreate(TWND wnd);
VOID TEDT_OnDestroy(TWND wnd);
VOID TEDT_OnSetFocus(TWND wnd);
LONG TEDT_OnKillFocus(TWND wnd);
LONG TEDT_ValidateNumberStyle(TWND wnd, TEDIT edit, LONG ch);
LONG TEDT_ValidateDecimalStyle(TWND wnd, TEDIT edit, LONG ch);
VOID TEDT_OnChar(TWND wnd, LONG ch);
VOID TEDT_OnPaint(TWND wnd, TDC dc);
VOID TEDT_OnLimitText(TWND wnd, INT limit);
VOID TEDT_OnSetPasswdChar(TWND wnd, CHAR passchar);
VOID TEDT_OnShowPasswdChar(TWND wnd, INT show);
VOID TEDT_OnSetDecWidth(TWND wnd, INT width);
LONG EDITPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);


LONG TEDT_OnCreate(TWND wnd)
{
  TEDIT edit = 0;
  DWORD style = TuiGetWndStyle(wnd);
  RECT rc;
  
  /* initial memory for edit control */
  edit = (TEDIT)malloc(sizeof(_TEDIT));
  if (!edit)
  {
    return TUI_MEM;
  }
  
  TuiGetWndRect(wnd, &rc);
  TuiGetWndText(wnd, edit->editbuf, TUI_MAX_WNDTEXT);
  
  edit->firstvisit = 1;
  edit->firstchar  = 0;
  edit->passchar   = '*';
  edit->showpass   = TW_SHOW;
  edit->editing    = 0;
  edit->decwidth   = 6;
  edit->limitchars = (TES_AUTOHSCROLL & style ? TUI_MAX_WNDTEXT : rc.cols);
  
  TuiSetWndParam(wnd, (LPVOID)edit);
  TuiSetWndTextAttrs(wnd, TuiGetSysColor(COLOR_EDTTEXT));
 
  return TUI_CONTINUE;
}

VOID TEDT_OnDestroy(TWND wnd)
{
  TEDIT edit = 0;
  /* release memory of edit control */
  edit = (TEDIT)TuiGetWndParam(wnd);
  free(edit);
}

VOID TEDT_OnSetFocus(TWND wnd)
{
  TEDIT edit = 0;
  NMHDR nmhdr;
  /* save edited buffer */
  edit = (TEDIT)TuiGetWndParam(wnd);
  TuiGetWndText(wnd, edit->editbuf, TUI_MAX_WNDTEXT);
  edit->firstvisit = 1;
  edit->firstchar  = 0;
  edit->editing    = 0;
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TEN_SETFOCUS;
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}

LONG TEDT_OnKillFocus(TWND wnd)
{
  TEDIT edit = 0;
  NMHDR nmhdr;
  LONG rc = TUI_CONTINUE;
  CHAR buf[TUI_MAX_WNDTEXT + 1];
  DOUBLE decimal = 0.0;
  DWORD style = TuiGetWndStyle(wnd);
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  
  rc = TuiIsWndValidate(wnd, buf);
  if (rc != TUI_CONTINUE)
  {
    return rc;
  }  
  /* check if style is TES_DECIMAL */
  if (style & TES_DECIMAL)
  {
    decimal = atof(edit->editbuf);
    sprintf(buf, "%.*f", edit->decwidth, decimal);
    strcpy(edit->editbuf, buf);
  }
  /* update text */
  edit->firstchar = 0;
  edit->editing   = 0;
  TuiSetWndText(wnd, edit->editbuf);
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TEN_KILLFOCUS;
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
  
  TuiInvalidateWnd(wnd);
  return rc;
}

LONG TEDT_ValidateNumberStyle(TWND wnd, TEDIT edit, LONG ch)
{
  LONG rc = TUI_CONTINUE;
  if (ch < '0' || ch > '9')
  {
    rc = TUI_ERROR;
  }
  return rc;
}

LONG TEDT_ValidateDecimalStyle(TWND wnd, TEDIT edit, LONG ch)
{
  LONG rc = TUI_CONTINUE;
  CHAR* decimal = strchr(edit->editbuf, '.');
  LONG len = strlen(edit->editbuf);
  
  /* not allowed '-' in the string */
  if ((len > 0 && (edit->firstvisit == 0)) && ch == '-')
  {
    rc = TUI_ERROR;
  }
  if ((rc == TUI_CONTINUE) && (len == 0 || (edit->firstvisit == 1)) && ch == '-')
  {
    /* ok */
  }
  else if (ch == '.')
  {
    if (decimal)
    {
      /* not allowed having '.' more than one */
      rc = TUI_ERROR;
    }
  }
  else if (ch < '0' || ch > '9')
  {
    rc = TUI_ERROR;
  }
  else
  {
    
  }
  return rc;
}

VOID TEDT_OnChar(TWND wnd, LONG ch)
{
  TEDIT edit = 0;
  NMHDR nmhdr;
  INT changed = 0;
  TDC dc = TuiGetDC(wnd);
  LONG len = 0;
  CHAR buf[TUI_MAX_WNDTEXT+1];
  DWORD attrs = TuiGetWndTextAttrs(wnd);
  CHAR text[TUI_MAX_WNDTEXT+1];
  DWORD style = TuiGetWndStyle(wnd);
  RECT rc;
  LONG  ret = TUI_CONTINUE;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  
  if (TuiIsWndEnabled(wnd))
  {
    attrs = TuiUnderlineText(attrs);
  }

  TuiGetWndText(wnd, text, TUI_MAX_WNDTEXT);
  TuiGetWndRect(wnd, &rc);
  
  if (ch == TVK_ESCAPE)
  {
    if (edit->firstvisit == 0)
    {
      edit->firstvisit = 1;
      strcpy(edit->editbuf, text);
      edit->firstchar  = 0;
      edit->editing    = 0;
      TuiInvalidateWnd(wnd);
      return;
    }
  }
  if (ch >= 0x20 && ch < 0x7f)
  {
    /* add char */
    len = strlen(edit->editbuf);
    if (len + 1 <= edit->limitchars)
    {
      if (TES_NUMBER & style)
      {
        /* require only number input */
        ret = TEDT_ValidateNumberStyle(wnd, edit, ch);
        if (ret != TUI_CONTINUE)
        {
          return;
        }
      }
      else if (TES_DECIMAL & style)
      {
        /* require only decimal input */
        ret = TEDT_ValidateDecimalStyle(wnd, edit, ch);
        if (ret != TUI_CONTINUE)
        {
          return;
        }
      }
      else if (TES_UPPERCASE & style)
      {
        /* require changing from small to CAPITAL */
        if (ch >= 'a' && ch <= 'z')
        {
          ch = ch - 'a' + 'A';
        }
      }
      else if (TES_LOWERCASE & style)
      {
        if (ch >= 'A' && ch <= 'Z')
        {
          ch = ch - 'A' + 'a';
        }
      }
      if (edit->firstvisit == 1)
      {
        edit->firstvisit = 0;
        if (!(TES_APPENDMODE & style) || (TES_PASSWORD & style)) /* replace mode */
        {
          edit->editbuf[0] = 0;
          memset(buf, ' ', rc.cols);
          buf[rc.cols] = 0;
          TuiDrawText(dc, rc.y, rc.x, buf, attrs);
          TuiMoveYX(dc, rc.y, rc.x);
        }
        else if (TES_AUTOHSCROLL & style)
        {
          /* show first char at */
          if (len > rc.cols)
          {
            edit->firstchar = len - rc.cols - 1;
          }
        }
      }
      
      /* append a new char */
      len = strlen(edit->editbuf);
      if (len < edit->limitchars)
      {
        edit->editbuf[len]   = ch;
        edit->editbuf[len+1] = 0;
        if (len < rc.cols)
        {
          if (TES_PASSWORD & style)
          {
            if (edit->showpass == TW_SHOW)
            {
              TuiPutChar(dc, rc.y, rc.x + len, 
                edit->passchar,
                attrs);
            }
          }
          else
          {
            TuiPutChar(dc, rc.y, rc.x + len, 
              ch,
              attrs);
          }
        }
        changed = 1;
      } /* len < limit */
      
      if (TES_AUTOHSCROLL & style)
      {
        len = strlen(edit->editbuf);
        if (len <= edit->limitchars && len > rc.cols && changed)
        {
          ++edit->firstchar;
          if (edit->firstchar > edit->limitchars - rc.cols)
          {
            edit->firstchar = edit->limitchars - rc.cols - 1;
          }
          TuiInvalidateWnd(wnd);
          if ((TES_PASSWORD & style) && (edit->showpass == TW_SHOW))
          {
            TuiMoveYX(dc, rc.y, rc.x);
          }
          else
          {
            TuiMoveYX(dc, rc.y, rc.x + rc.cols);
          }
        }
      }

      /* editing */      
      edit->editing    = 1;

    } /*TUI_MAX_WNDTEXT*/
  }
  else if (ch == 0x7f || ch == TVK_DELETE || 
#ifdef __USE_CURSES__
  ch == KEY_BACKSPACE) /* delete char */
#elif defined __USE_QIO__
  ch == KEY_BS)
#endif
  {
    len = strlen(edit->editbuf);
    if (edit->firstvisit == 1)
    {
      edit->firstvisit = 0;
      if (!(TES_APPENDMODE & style) || TES_PASSWORD & style) /* replace mode */
      {
        edit->editbuf[0] = 0;
        sprintf(buf, "%*s", rc.cols, " ");
        TuiMoveYX(dc, rc.y, rc.x);
        TuiDrawText(dc, rc.y, rc.x, buf, attrs);
      }
      else if (TES_AUTOHSCROLL & style)
      {
        /* show first char at */
        if (len > rc.cols)
        {
          edit->firstchar = len - rc.cols - 1;
        }
      }
    }
    
    if (len > 0)
    {
      if (len > rc.cols)
      {
        TuiPutChar(dc, rc.y, rc.x + rc.cols - 1, ' ', attrs);
      }
      else
      {
        TuiPutChar(dc, rc.y, rc.x + len - 1, ' ', attrs);
      }
      TuiMoveYX(dc, rc.y, rc.x + len - 1);
      edit->editbuf[len - 1] = 0;
      changed = 1;
      
      if (TES_AUTOHSCROLL & style)
      {
        len = strlen(edit->editbuf);
        if (len >= rc.cols)
        {
          edit->firstchar = len - rc.cols;
          if (edit->firstchar < 0)
          {
            edit->firstchar = 0;
          }
          len = rc.cols;
        }
        else
        {
          edit->firstchar = 0;
        }
        TuiInvalidateWnd(wnd);
        TuiMoveYX(dc, rc.y, rc.x + len);
      }
    }      

    /* editing */      
    edit->editing    = 1;
  }
  if (changed)
  {
    nmhdr.id   = TuiGetWndID(wnd);
    nmhdr.ctl  = wnd;
    nmhdr.code = TEN_CHANGED;
    TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
  }
}

VOID TEDT_OnPaint(TWND wnd, TDC dc)
{
  TEDIT edit = 0;
  LONG len = 0;
  CHAR buf[TUI_MAX_WNDTEXT+1];
  CHAR text[TUI_MAX_WNDTEXT+1];
  DWORD attrs = TuiGetWndTextAttrs(wnd);
  RECT rc;
  DWORD style = TuiGetWndStyle(wnd);
  
  edit = (TEDIT)TuiGetWndParam(wnd);
    
  if (TuiIsWndEnabled(wnd))
  {
    attrs = TuiUnderlineText(attrs);
  }
  
  TuiGetWndRect(wnd, &rc);
  len = rc.cols;
  if (TES_PASSWORD & style)
  {
    if (edit->showpass == TW_SHOW)
    {
      memset(buf, edit->passchar, len);
    }
    else
    {
      memset(buf, ' ', len);
    }
    buf[len] = 0;
    TuiDrawText(dc, rc.y, rc.x, buf, attrs);
    return;
  }

  if (TuiIsWndVisible(wnd))
  {
    len = MIN(strlen(edit->editbuf), rc.cols);
    memset(buf, 0, TUI_MAX_WNDTEXT);
    memcpy(buf, &edit->editbuf[edit->firstchar], len);
    
    if (edit->editing)
    {
      TuiPrintTextAlignment(text, buf, rc.cols, ALIGN_LEFT);
    }
    else
    {
      TuiPrintTextAlignment(text, buf, rc.cols, style);
    }
    TuiDrawText(dc, rc.y, rc.x, text, attrs);
  }
  TuiMoveYX(dc, rc.y, rc.x);
}

VOID TEDT_OnLimitText(TWND wnd, INT limit)
{
  TEDIT edit = 0;
  CHAR text[TUI_MAX_WNDTEXT + 1];
  
  if (limit > 0 || limit <= TUI_MAX_WNDTEXT)
  {
    edit = (TEDIT)TuiGetWndParam(wnd);
    edit->limitchars = limit;
    
    TuiGetWndText(wnd, text, TUI_MAX_WNDTEXT);
    if (strlen(text) > limit)
    {
      text[limit] = 0;
      TuiSetWndText(wnd, text);
      strcpy(edit->editbuf, text);
    }
  }
}

VOID TEDT_OnSetPasswdChar(TWND wnd, CHAR passchar)
{
  TEDIT edit = 0;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->passchar = passchar;
}

VOID TEDT_OnShowPasswdChar(TWND wnd, INT show)
{
  TEDIT edit = 0;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->showpass = (show == TW_SHOW ? TW_SHOW : TW_HIDE);
}

VOID TEDT_OnSetDecWidth(TWND wnd, INT width)
{
  TEDIT edit = 0;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->decwidth = width;
}

VOID TEDT_OnSetText(TWND wnd, LPCSTR text)
{
  TEDIT edit = 0;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  TuiGetWndText(wnd, edit->editbuf, TUI_MAX_WNDTEXT);
  edit->firstvisit = 1;
}

LONG EDITPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_CREATE:
    {
      return TEDT_OnCreate(wnd);
    }
    case TWM_DESTROY:
    {
      TEDT_OnDestroy(wnd);
      return 0;
    }
    case TWM_SETTEXT:
    {
      TuiDefWndProc(wnd, msg, wparam, lparam);
      TEDT_OnSetText(wnd, (LPCSTR)lparam);
      return 0;
    }
    case TWM_SETFOCUS:
    {
      TEDT_OnSetFocus(wnd);
      break;
    }
    case TWM_KILLFOCUS:
    {
      return TEDT_OnKillFocus(wnd);
    }
    case TWM_CHAR:
    {
      TEDT_OnChar(wnd, (LONG)wparam);
      break;
    }
    case TWM_PAINT:
    {
      TEDT_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
    case TEM_LIMITTEXT:
    {
      TEDT_OnLimitText(wnd, (INT)wparam);
      return 0;
    }
    case TEM_SETPASSWDCHAR:
    {
      TEDT_OnSetPasswdChar(wnd, (CHAR)wparam);
      return 0;
    }
    case TEM_SETDECWIDTH:
    {
      TEDT_OnSetDecWidth(wnd, (INT)wparam);
      return 0;
    }
    case TEM_SHOWPASSWDCHAR:
    {
      TEDT_OnShowPasswdChar(wnd, (INT)wparam);
      return 0;
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}
