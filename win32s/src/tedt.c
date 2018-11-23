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
  TTCHAR             editbuf[TUI_MAX_WNDTEXT+1];
  TINT              firstvisit;
  TTCHAR             passchar;
  TINT              showpass;
  TINT              firstchar;
  TINT              limitchars;
  TINT              editing;
  TINT              decwidth;
  TTCHAR             validstr[TUI_MAX_WNDTEXT+1];
  TINT              min;
  TINT              max;
  TINT              onminmax;
  TVOID*            exparam;
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
TLONG _TEDT_AddDecimalFormat(TEDIT edit);
TLONG _TEDT_RemoveDecimalFormat(TEDIT edit);
TLONG _TEDT_AddSuffix(TEDIT edit, TINT cols);
TLONG _TEDT_RemoveSuffix(TEDIT edit);

TVOID _TEDT_OnSetValidMinMax(TWND wnd, TINT on, VALIDMINMAX* vmm);
TLONG _TEDT_OnCreate(TWND wnd);
TVOID _TEDT_OnDestroy(TWND wnd);
TVOID _TEDT_OnSetFocus(TWND wnd);
TLONG _TEDT_OnKillFocus(TWND wnd);
TVOID _TEDT_OnChar(TWND wnd, TLONG ch);
TVOID _TEDT_OnPaint(TWND wnd, TDC dc);
TVOID _TEDT_OnLimitText(TWND wnd, TINT limit);
TVOID _TEDT_OnSetPasswdChar(TWND wnd, TTCHAR passchar);
TVOID _TEDT_OnShowPasswdChar(TWND wnd, TINT show);
TVOID _TEDT_OnSetDecWidth(TWND wnd, TINT width);
TLONG _TEDT_ValidateNumberStyle(TWND wnd, TEDIT edit, TLONG ch);
TLONG _TEDT_ValidateDecimalStyle(TWND wnd, TEDIT edit, TLONG ch);
TLONG _TEDT_ValidateA2ZStyle(TWND wnd, TEDIT edit, TLONG ch);
TVOID _TEDT_OnKeyDown(TWND wnd, TLONG ch);

TLONG EDITBOXPROC(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam);

TLONG _TEDT_AddDecimalFormat(TEDIT edit)
{
  TTCHAR buf[TUI_MAX_WNDTEXT+1];
  TINT cnt = 1;
  TTCHAR* pbuf;
  TLONG len = strlen(edit->editbuf) - 1;
  TTCHAR* psz = &edit->editbuf[len];
  
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

TLONG _TEDT_RemoveDecimalFormat(TEDIT edit)
{
  TTCHAR buf[TUI_MAX_WNDTEXT+1];
  TTCHAR* psz = edit->editbuf;
  TTCHAR* pbuf = buf;
  
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

TLONG _TEDT_AddSuffix(TEDIT edit, TINT cols)
{
  TDOUBLE dbl = atof(edit->editbuf);
  TLONG len = strlen(edit->editbuf);
  TTCHAR suffix[3] = { ' ', 0, 0 };
  TINT cnt = 0;

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

TLONG _TEDT_RemoveSuffix(TEDIT edit)
{
  TDOUBLE dbl = atof(edit->editbuf);
  TTCHAR suffix = edit->editbuf[strlen(edit->editbuf) - 1];
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

TLONG _TEDT_OnCreate(TWND wnd)
{
  TEDIT edit = 0;
  TDWORD style = TuiGetWndStyle(wnd);
  TRECT rc;
  
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
  
  TuiSetWndParam(wnd, (TLPVOID)edit);
  TuiSetWndTextAttrs(wnd, TuiGetSysColor(COLOR_EDTTEXT));
 
  return TUI_CONTINUE;
}

TVOID _TEDT_OnDestroy(TWND wnd)
{
  TEDIT edit = 0;
  /* release memory of edit control */
  edit = (TEDIT)TuiGetWndParam(wnd);
  free(edit);
}

TVOID _TEDT_OnSetFocus(TWND wnd)
{
  TEDIT edit = 0;
  TNMHDR nmhdr;
  TDWORD style = TuiGetWndStyle(wnd);
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
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (TLPARAM)&nmhdr);
}

TLONG _TEDT_OnKillFocus(TWND wnd)
{
  TEDIT edit = 0;
  TNMHDR nmhdr;
  TLONG rc = TUI_CONTINUE;
  TTCHAR buf[TUI_MAX_WNDTEXT + 1];
  TDOUBLE decimal = 0.0;
  TDWORD style = TuiGetWndStyle(wnd);
  TLONG rcminmax = TUI_CONTINUE;
  TINT number = 0;
  TRECT rcwnd;
  TuiGetWndRect(wnd, &rcwnd);
  TPOS pos;
  
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
      if (decimal < (TDOUBLE)edit->min &&
          decimal > (TDOUBLE)edit->max)
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
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (TLPARAM)&nmhdr);

  pos.y = rcwnd.y;
  pos.x = rcwnd.x;
  TuiPostMsg(TuiGetParent(wnd), TWM_SETCURSOR, 0, (TLPARAM)&pos);
  
  TuiInvalidateWnd(wnd);
  return rc;
}

TLONG _TEDT_ValidateA2ZStyle(TWND wnd, TEDIT edit, TLONG ch)
{
  TLONG rc = TUI_ERROR;
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

TLONG _TEDT_ValidateNumberStyle(TWND wnd, TEDIT edit, TLONG ch)
{
  TLONG rc = TUI_CONTINUE;
  if (ch < '0' || ch > '9')
  {
    rc = TUI_ERROR;
  }
  return rc;
}

TLONG _TEDT_ValidateDecimalStyle(TWND wnd, TEDIT edit, TLONG ch)
{
  TLONG rc = TUI_CONTINUE;
  TTCHAR* decimal = strchr(edit->editbuf, '.');
  TLONG len = strlen(edit->editbuf);
  
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

TVOID _TEDT_OnKeyDown(TWND wnd, TLONG ch)
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
      TLONG rc = _TEDT_OnKillFocus(wnd);
      if (TUI_CONTINUE == rc)
      {
        TuiPostMsg(TuiGetParent(wnd), TWM_KEYDOWN, (TWPARAM)ch, 0);
      }
      break;
    }
  }
*/
}

TVOID _TEDT_OnChar(TWND wnd, TLONG ch)
{
  TEDIT edit = 0;
  TNMHDR nmhdr;
  TINT changed = 0;
  TDC dc = TuiGetDC(wnd);
  TLONG len = 0;
  TTCHAR buf[TUI_MAX_WNDTEXT+1];
  TDWORD attrs = TuiGetWndTextAttrs(wnd);
  TTCHAR text[TUI_MAX_WNDTEXT+1];
  TDWORD style = TuiGetWndStyle(wnd);
  TRECT rc;
  TTCHAR* psz;
  TLONG  ret = TUI_CONTINUE;
  
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
  else if (0x7f == ch || 
#ifdef __USE_CURSES__
  KEY_BACKSPACE == ch) /* delete char */
#elif defined __USE_QIO__
  KEY_BS == ch)
#elif defined __USE_WIN32__
      TVK_BACK == ch)
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
    TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (TLPARAM)&nmhdr);
  }
}

TVOID _TEDT_OnPaint(TWND wnd, TDC dc)
{
  TEDIT edit = 0;
  TLONG len = 0;
  TTCHAR buf[TUI_MAX_WNDTEXT+1];
  TTCHAR text[TUI_MAX_WNDTEXT+1];
  TDWORD attrs = TuiGetWndTextAttrs(wnd);
  TRECT rc;
  TDWORD style = TuiGetWndStyle(wnd);
  
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
      TuiPrintTextAlignment(text, buf, rc.cols, TALIGN_LEFT);
    }
    else
    {
      TuiPrintTextAlignment(text, buf, rc.cols, style);
    }
    TuiDrawText(dc, rc.y, rc.x, text, attrs);
    TuiMoveYX(dc, rc.y, rc.x);
  }
}

TVOID _TEDT_OnLimitText(TWND wnd, TINT limit)
{
  TEDIT edit = 0;
  TTCHAR text[TUI_MAX_WNDTEXT + 1];
  
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

TVOID _TEDT_OnSetPasswdChar(TWND wnd, TTCHAR passchar)
{
  TEDIT edit = 0;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->passchar = passchar;
}

TVOID _TEDT_OnShowPasswdChar(TWND wnd, TINT show)
{
  TEDIT edit = 0;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->showpass = (show == TW_SHOW ? TW_SHOW : TW_HIDE);
}

TVOID _TEDT_OnSetDecWidth(TWND wnd, TINT width)
{
  TEDIT edit = 0;
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->decwidth = width;
}

TVOID _TEDT_OnSetText(TWND wnd, TLPCSTR text)
{
  TEDIT edit = 0;
  TDWORD style = TuiGetWndStyle(wnd);
  
  edit = (TEDIT)TuiGetWndParam(wnd);
  TuiGetWndText(wnd, edit->editbuf, TUI_MAX_WNDTEXT);
  edit->firstvisit = 1;
  
  if (TES_AUTODECIMALCOMMA & style)
  {
    _TEDT_RemoveDecimalFormat(edit);
    _TEDT_AddDecimalFormat(edit);
    if (TES_AUTOSUFFIX & style)
    {
      TRECT rcwnd;
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

TVOID _TEDT_OnSetValidString(TWND wnd, TLPCSTR validstr)
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

TVOID _TEDT_OnSetValidMinMax(TWND wnd, TINT on, VALIDMINMAX* vmm)
{
  TEDIT edit = 0;  
  edit = (TEDIT)TuiGetWndParam(wnd);
  edit->min = vmm->min;
  edit->max = vmm->max;
  edit->onminmax = (on == TW_ENABLE ? TW_ENABLE : TW_DISABLE);
}

TLONG EDITBOXPROC(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam)
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
      _TEDT_OnSetText(wnd, (TLPCSTR)lparam);
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
      _TEDT_OnKeyDown(wnd, (TLONG)wparam);
      break;
    }
    case TWM_CHAR:
    {
      _TEDT_OnChar(wnd, (TLONG)wparam);
      break;
    }
    case TWM_PAINT:
    {
      _TEDT_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
    case TEM_LIMITTEXT:
    {
      _TEDT_OnLimitText(wnd, (TINT)wparam);
      return 0;
    }
    case TEM_SETPASSWDCHAR:
    {
      _TEDT_OnSetPasswdChar(wnd, (TTCHAR)wparam);
      return 0;
    }
    case TEM_SETDECWIDTH:
    {
      _TEDT_OnSetDecWidth(wnd, (TINT)wparam);
      return 0;
    }
    case TEM_SHOWPASSWDCHAR:
    {
      _TEDT_OnShowPasswdChar(wnd, (TINT)wparam);
      return 0;
    }
    
    case TEM_SETVALIDSTRING:
    {
      _TEDT_OnSetValidString(wnd, (TLPCSTR)lparam);
      return 0;
    }
    
    case TEM_SETVALIDMINMAX:
    {
      _TEDT_OnSetValidMinMax(wnd, (TINT)wparam, (VALIDMINMAX*)lparam);
      return 0;
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}
