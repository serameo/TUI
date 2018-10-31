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

#include "tui.h"

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
  CHAR             validstr[TUI_MAX_WNDTEXT+1];
  INT              min;
  INT              max;
  INT              onminmax;
  VOID*            exparam;
};
typedef struct _TEDITSTRUCT _TEDIT;
typedef struct _TEDITSTRUCT *TEDIT;

#define KILO      1000
#define MEGA      1000000
#define GIGA      1000000000
#define TERA      1000000000000
#define PETA      1000000000000000
#define EXA       1000000000000000000
/*
#define ZETA      1000000000000000000000
#define YOTTA     1000000000000000000000000
*/
LONG _TEDT_AddDecimalFormat(TEDIT edit);
LONG _TEDT_RemoveDecimalFormat(TEDIT edit);
LONG _TEDT_AddSuffix(TEDIT edit, INT cols);
LONG _TEDT_RemoveSuffix(TEDIT edit);

VOID _TEDT_OnSetValidMinMax(TWND wnd, INT on, VALIDMINMAX* vmm);
LONG _TEDT_OnCreate(TWND wnd);
VOID _TEDT_OnDestroy(TWND wnd);
VOID _TEDT_OnSetFocus(TWND wnd);
LONG _TEDT_OnKillFocus(TWND wnd);
VOID _TEDT_OnChar(TWND wnd, LONG ch);
VOID _TEDT_OnPaint(TWND wnd, TDC dc);
VOID _TEDT_OnLimitText(TWND wnd, INT limit);
VOID _TEDT_OnSetPasswdChar(TWND wnd, CHAR passchar);
VOID _TEDT_OnShowPasswdChar(TWND wnd, INT show);
VOID _TEDT_OnSetDecWidth(TWND wnd, INT width);
LONG _TEDT_ValidateNumberStyle(TWND wnd, TEDIT edit, LONG ch);
LONG _TEDT_ValidateDecimalStyle(TWND wnd, TEDIT edit, LONG ch);
LONG _TEDT_ValidateA2ZStyle(TWND wnd, TEDIT edit, LONG ch);
VOID _TEDT_OnKeyDown(TWND wnd, LONG ch);

LONG EDITBOXPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

LONG _TEDT_AddDecimalFormat(TEDIT edit)
{
  CHAR buf[TUI_MAX_WNDTEXT+1];
  INT cnt = 1;
  CHAR* pbuf;
  LONG len = strlen(edit->editbuf) - 1;
  CHAR* psz = &edit->editbuf[len];
  
  memset(buf, 0, sizeof(buf));
  pbuf = buf;
  
  while (*psz != 0 && *psz != '.')
  {
    *pbuf = *psz;
    ++pbuf;
    --psz;
    --len;
  }
  if ('.' == *psz)
  {
    *pbuf = *psz;
    ++pbuf;
    --psz;
    --len;
  }
  
  while (len >= 0)
  {
    if (cnt % 4 == 0 && *psz != '-')
    {
      *pbuf = ',';
      ++pbuf;
      ++cnt;
    }
    *pbuf = *psz;
    ++pbuf;
    --psz;
    --len;
    ++cnt;
  }
  /* save */
  memset(edit->editbuf, 0, sizeof(edit->editbuf));
  /* reverse copy */
  len  = strlen(buf) - 1;
  pbuf = &buf[len];
  psz  = edit->editbuf;
  while (len >= 0)
  {
    *psz = *pbuf;
    ++psz;
    --pbuf;
    --len;
  }
  return TUI_OK;
}

LONG _TEDT_RemoveDecimalFormat(TEDIT edit)
{
  CHAR buf[TUI_MAX_WNDTEXT+1];
  CHAR* psz = edit->editbuf;
  CHAR* pbuf = buf;
  
  if (strchr(edit->editbuf, ','))
  {
    memset(buf, 0, sizeof(buf));
    while (*psz != 0 && *psz != '.')
    {
      if (*psz != ',')
      {
        *pbuf = *psz;
        ++pbuf;
      }
      ++psz;
    }
    while (*psz != 0)
    {
      *pbuf = *psz;
      ++pbuf;
      ++psz;
    }
    /* save */
    strcpy(edit->editbuf, buf);
  }
  return TUI_OK;
}

LONG _TEDT_AddSuffix(TEDIT edit, INT cols)
{
  DOUBLE dbl = atof(edit->editbuf);
  LONG len = strlen(edit->editbuf);
  CHAR suffix[3] = { ' ', 0, 0 };
  INT cnt = 0;

  while (len > 0)
  {
    len -= 3;
    ++cnt;
  }
  len = strlen(edit->editbuf) + cnt;
  cnt = 0;

  while (cols < len)
  {
    len -= 3;
    ++cnt;
    dbl = dbl / KILO;
  }
  switch (cnt)
  {
    case 1:
    {
      suffix[0] = 'K';
      break;
    }
    case 2:
    {
      suffix[0] = 'M';
      break;
    }
    case 3:
    {
      suffix[0] = 'G';
      break;
    }
    case 4:
    {
      suffix[0] = 'T';
      break;
    }
    case 5:
    {
      suffix[0] = 'P';
      break;
    }
    case 6:
    {
      suffix[0] = 'E';
      break;
    }
/*
    case 7:
    {
      suffix[0] = 'Z';
      break;
    }
    case 8:
    {
      suffix[0] = 'Y';
      break;
    }
*/
  }

  if (suffix[0] != ' ')
  {
    sprintf(edit->editbuf, "%.*f", edit->decwidth, dbl);
    strcat(edit->editbuf, suffix);
  }
  else
  {
    /* not support */
    return TUI_ERROR;
  }
  return TUI_OK;
}

LONG _TEDT_RemoveSuffix(TEDIT edit)
{
  DOUBLE dbl = atof(edit->editbuf);
  CHAR suffix = edit->editbuf[strlen(edit->editbuf) - 1];
  switch (suffix)
  {
    case 'K':
    {
      dbl = dbl * KILO;
      break;
    }
    case 'M':
    {
      dbl = dbl * MEGA;
      break;
    }
    case 'G':
    {
      dbl = dbl * GIGA;
      break;
    }
    case 'T':
    {
      dbl = dbl * TERA;
      break;
    }
    case 'P':
    {
      dbl = dbl * PETA;
      break;
    }
    case 'E':
    {
      dbl = dbl * EXA;
      break;
    }
/*
    case 'Z':
    {
      dbl = dbl * ZETA;
      break;
    }
    case 'Y':
    {
      dbl = dbl * YOTTA;
      break;
    }
*/
  }
  sprintf(edit->editbuf, "%.*f", edit->decwidth, dbl);
  return TUI_OK;
}

LONG _TEDT_OnCreate(TWND wnd)
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
  
  memset(edit, 0, sizeof(_TEDIT));
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

VOID _TEDT_OnDestroy(TWND wnd)
{
  TEDIT edit = 0;
  /* release memory of edit control */
  edit = (TEDIT)TuiGetWndParam(wnd);
  free(edit);
}

VOID _TEDT_OnSetFocus(TWND wnd)
{
  TEDIT edit = 0;
  NMHDR nmhdr;
  DWORD style = TuiGetWndStyle(wnd);
  /* save edited buffer */
  edit = (TEDIT)TuiGetWndParam(wnd);
  TuiGetWndText(wnd, edit->editbuf, TUI_MAX_WNDTEXT);
  edit->firstvisit = 1;
  edit->firstchar  = 0;
  edit->editing    = 0;
  
  if (TES_AUTODECIMALCOMMA & style)
  {
    _TEDT_RemoveDecimalFormat(edit);
    if (TES_AUTOSUFFIX & style)
    {
      _TEDT_RemoveSuffix(edit);
    }
  }
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TEN_SETFOCUS;
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}

LONG _TEDT_OnKillFocus(TWND wnd)
{
  TEDIT edit = 0;
  NMHDR nmhdr;
  LONG rc = TUI_CONTINUE;
  CHAR buf[TUI_MAX_WNDTEXT + 1];
  DOUBLE decimal = 0.0;
  DWORD style = TuiGetWndStyle(wnd);
  LONG rcminmax = TUI_CONTINUE;
  INT number = 0;
  RECT rcwnd;
  TuiGetWndRect(wnd, &rcwnd);
  POS pos;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->firstvisit = 1;
  
  /* check if style is TES_DECIMAL */
  if (TES_DECIMAL & style || TES_AUTODECIMALCOMMA & style)
  {
    decimal = atof(edit->editbuf);
    sprintf(buf, "%.*f", edit->decwidth, decimal);
    strcpy(edit->editbuf, buf);
    if (TW_ENABLE == edit->onminmax)
    {
      if (decimal < (DOUBLE)edit->min &&
          decimal > (DOUBLE)edit->max)
      {
        rcminmax = TUI_ERROR;
      }
    }
  }
  else
  {
    strcpy(buf, edit->editbuf);
  }
  
  if ((TES_NUMBER & style) && (TW_ENABLE == edit->onminmax))
  {
    number = atoi(edit->editbuf);
    if (number < edit->min &&
        number > edit->max)
    {
      rcminmax = TUI_ERROR;
    }
  }
  /* validate min & max */
  if (rcminmax != TUI_CONTINUE)
  {
    return rcminmax;
  }  
  
  /* validate */
  rc = TuiIsWndValidate(wnd, buf);
  if (rc != TUI_CONTINUE)
  {
    return rc;
  }  
  
  if (TES_AUTODECIMALCOMMA & style)
  {
    _TEDT_AddDecimalFormat(edit);
    if (TES_AUTOSUFFIX & style)
    {
      if (rcwnd.cols < strlen(edit->editbuf))
      {
        _TEDT_RemoveDecimalFormat(edit);
        _TEDT_AddSuffix(edit, rcwnd.cols);
        _TEDT_AddDecimalFormat(edit);
      }
    }
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

  pos.y = rcwnd.y;
  pos.x = rcwnd.x;
  TuiPostMsg(TuiGetParent(wnd), TWM_SETCURSOR, 0, (LPARAM)&pos);
  
  TuiInvalidateWnd(wnd);
  return rc;
}

LONG _TEDT_ValidateA2ZStyle(TWND wnd, TEDIT edit, LONG ch)
{
  LONG rc = TUI_ERROR;
  if (ch >= 'a' && ch <= 'z')
  {
    rc = TUI_CONTINUE;
  }
  else if (ch >= 'A' && ch <= 'Z')
  {
    rc = TUI_CONTINUE;
  }
  return rc;
}

LONG _TEDT_ValidateNumberStyle(TWND wnd, TEDIT edit, LONG ch)
{
  LONG rc = TUI_CONTINUE;
  if (ch < '0' || ch > '9')
  {
    rc = TUI_ERROR;
  }
  return rc;
}

LONG _TEDT_ValidateDecimalStyle(TWND wnd, TEDIT edit, LONG ch)
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
  return rc;
}

VOID _TEDT_OnKeyDown(TWND wnd, LONG ch)
{
/*
  switch (ch)
  {
    case KEY_RIGHT:
    case KEY_LEFT:
    case KEY_DOWN:
    case KEY_UP:
    case TVK_ENTER:
    case KEY_ENTER:
    case TVK_TAB:
    case KEY_BTAB:
    case TVK_PRIOR:
    case KEY_PPAGE:
    case TVK_NEXT:
    case KEY_NPAGE:
    {
      LONG rc = _TEDT_OnKillFocus(wnd);
      if (TUI_CONTINUE == rc)
      {
        TuiPostMsg(TuiGetParent(wnd), TWM_KEYDOWN, (WPARAM)ch, 0);
      }
      break;
    }
  }
*/
}

VOID _TEDT_OnChar(TWND wnd, LONG ch)
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
  CHAR* psz;
  LONG  ret = TUI_CONTINUE;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  
  if (TuiIsWndEnabled(wnd))
  {
    if (style & TES_UNDERLINE)
    {
      attrs = TuiUnderlineText(attrs);
    }
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
    /* is the first typing? */
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
        
        len = 0;
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
    
    if (len + 1 <= edit->limitchars)
    {
      if (TES_NUMBER & style)
      {
        /* require only number input */
        ret = _TEDT_ValidateNumberStyle(wnd, edit, ch);
        if (ret != TUI_CONTINUE)
        {
          return;
        }
      }
      else if (TES_DECIMAL & style || TES_AUTODECIMALCOMMA & style)
      {
        /* require only decimal input */
        ret = _TEDT_ValidateDecimalStyle(wnd, edit, ch);
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
      
      if (TES_A2Z & style)
      {        
        /* require only A-Z input */
        ret = _TEDT_ValidateA2ZStyle(wnd, edit, ch);
        if (ret != TUI_CONTINUE)
        {
          return;
        }
      }
      /* valid char if it is in valid string */
      if (edit->validstr[0] != 0)
      {
        psz = strchr(edit->validstr, ch);
        if (!psz)
        {
          return;
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

VOID _TEDT_OnPaint(TWND wnd, TDC dc)
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
    if (style & TES_UNDERLINE)
    {
      attrs = TuiUnderlineText(attrs);
    }
  }
  
  if (TuiIsWndVisible(wnd))
  {
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

VOID _TEDT_OnLimitText(TWND wnd, INT limit)
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

VOID _TEDT_OnSetPasswdChar(TWND wnd, CHAR passchar)
{
  TEDIT edit = 0;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->passchar = passchar;
}

VOID _TEDT_OnShowPasswdChar(TWND wnd, INT show)
{
  TEDIT edit = 0;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->showpass = (show == TW_SHOW ? TW_SHOW : TW_HIDE);
}

VOID _TEDT_OnSetDecWidth(TWND wnd, INT width)
{
  TEDIT edit = 0;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->decwidth = width;
}

VOID _TEDT_OnSetText(TWND wnd, LPCSTR text)
{
  TEDIT edit = 0;
  DWORD style = TuiGetWndStyle(wnd);
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  TuiGetWndText(wnd, edit->editbuf, TUI_MAX_WNDTEXT);
  edit->firstvisit = 1;
  
  if (TES_AUTODECIMALCOMMA & style)
  {
    _TEDT_RemoveDecimalFormat(edit);
    _TEDT_AddDecimalFormat(edit);
    if (TES_AUTOSUFFIX & style)
    {
      RECT rcwnd;
      TuiGetWndRect(wnd, &rcwnd);
      if (rcwnd.cols < strlen(edit->editbuf))
      {
        _TEDT_RemoveDecimalFormat(edit);
        _TEDT_AddSuffix(edit, rcwnd.cols);
        _TEDT_AddDecimalFormat(edit);
      }
    }
  }
}

VOID _TEDT_OnSetValidString(TWND wnd, LPCSTR validstr)
{
  TEDIT edit = 0;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  if (validstr)
  {
    /* set the valid string */
    strcpy(edit->validstr, validstr);
  }
  else
  {
    /* remove valid string */
    memset(edit->validstr, 0, sizeof(edit->validstr));
  }
}

VOID _TEDT_OnSetValidMinMax(TWND wnd, INT on, VALIDMINMAX* vmm)
{
  TEDIT edit = 0;  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->min = vmm->min;
  edit->max = vmm->max;
  edit->onminmax = (on == TW_ENABLE ? TW_ENABLE : TW_DISABLE);
}

LONG EDITBOXPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_CREATE:
    {
      return _TEDT_OnCreate(wnd);
    }
    case TWM_DESTROY:
    {
      _TEDT_OnDestroy(wnd);
      return 0;
    }
    case TWM_SETTEXT:
    {
      TuiDefWndProc(wnd, msg, wparam, lparam);
      _TEDT_OnSetText(wnd, (LPCSTR)lparam);
      return 0;
    }
    case TWM_SETFOCUS:
    {
      _TEDT_OnSetFocus(wnd);
      break;
    }
    case TWM_KILLFOCUS:
    {
      return _TEDT_OnKillFocus(wnd);
    }
    case TWM_KEYDOWN:
    {
      _TEDT_OnKeyDown(wnd, (LONG)wparam);
      break;
    }
    case TWM_CHAR:
    {
      _TEDT_OnChar(wnd, (LONG)wparam);
      break;
    }
    case TWM_PAINT:
    {
      _TEDT_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
    case TEM_LIMITTEXT:
    {
      _TEDT_OnLimitText(wnd, (INT)wparam);
      return 0;
    }
    case TEM_SETPASSWDCHAR:
    {
      _TEDT_OnSetPasswdChar(wnd, (CHAR)wparam);
      return 0;
    }
    case TEM_SETDECWIDTH:
    {
      _TEDT_OnSetDecWidth(wnd, (INT)wparam);
      return 0;
    }
    case TEM_SHOWPASSWDCHAR:
    {
      _TEDT_OnShowPasswdChar(wnd, (INT)wparam);
      return 0;
    }
    
    case TEM_SETVALIDSTRING:
    {
      _TEDT_OnSetValidString(wnd, (LPCSTR)lparam);
      return 0;
    }
    
    case TEM_SETVALIDMINMAX:
    {
      _TEDT_OnSetValidMinMax(wnd, (INT)wparam, (VALIDMINMAX*)lparam);
      return 0;
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}
