/*-------------------------------------------------------------------
 * File name: tfrmwnd.c
 * Author: Seree Rakwong
 * Date: 08-OCT-18
 *-----------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#ifdef __USE_CURSES__
#include <curses.h>
#endif

#include "tui.h"
/*-------------------------------------------------------------------
 * Frame window functions
 *-----------------------------------------------------------------*/
#define FRMWND_BTNWIDTH      12

struct _SHOWMSGBOXSTRUCT
{
  UINT   id;
  INT    y;
  INT    x;
  LPCSTR text;
  LPCSTR caption;
  UINT   flags; /* MB_OK, MB_YES, MB_NO, MB_CANCEL */
  INT    limit;
  LPCSTR deftext;
  DWORD  edtstyle;
  DWORD  wndattrs;
  LPCSTR validch;
  INT    align;
};
typedef struct _SHOWMSGBOXSTRUCT SHOWMSGBOX;

struct _WNDENABLEDSTRUCT
{
  TWND    child;
  struct _WNDENABLEDSTRUCT*      prev;
  struct _WNDENABLEDSTRUCT*      next;
};
typedef struct _WNDENABLEDSTRUCT twndenabled_t;

struct _TFRAMEWNDSTRUCT
{
  TWND            edtinput;  /* always last child and hidden */
  TWND            btnyes;    /* always last child and hidden */
  TWND            btnno;     /* always last child and hidden */
  TWND            btnok;     /* always last child and hidden */
  TWND            btncancel; /* always last child and hidden */
  VOID*           param;
  TWND            lastfocus;

  twndenabled_t*  firstwndenabled;
  twndenabled_t*  lastwndenabled;
  
  SHOWMSGBOX      msgbox;
  LONG            nextmove;
  UINT            msg;
};
typedef struct _TFRAMEWNDSTRUCT _TFRAMEWND;
typedef struct _TFRAMEWNDSTRUCT *TFRAMEWND;

VOID _TFRAMEWND_RestoreAndEnableAllWnds(TWND wnd);
VOID _TFRAMEWND_SaveAndDisableAllWnds(TWND wnd);

INT  TFRAMEWND_OnInitDailog(TWND wnd, LPARAM lparam);
VOID TFRAMEWND_OnCommand(TWND wnd, UINT cmd);
VOID TFRAMEWND_OnShowMsgBox(TWND wnd, SHOWMSGBOX* param);
VOID TFRAMEWND_OnHideMsgBox(TWND wnd);
VOID TFRAMEWND_OnShowInputBox(TWND wnd, SHOWMSGBOX* param);
VOID TFRAMEWND_OnHideInputBox(TWND wnd);
VOID TFRAMEWND_OnShowLineInputBox(TWND wnd, SHOWMSGBOX* param);
VOID TFRAMEWND_OnHideLineInputBox(TWND wnd);
VOID TFRAMEWND_OnNotify(TWND wnd, NMHDR* nmhdr);

INT  TFRAMEWND_OnInitDailog(TWND wnd, LPARAM lparam)
{
  TFRAMEWND frmwnd = 0;

  frmwnd = (TFRAMEWND)malloc(sizeof(_TFRAMEWND));
  if (!frmwnd)
  {
    return TUI_MEM;
  }
  memset(frmwnd, 0, sizeof(_TFRAMEWND));
  frmwnd->param = (VOID*)lparam;

  frmwnd->edtinput = TuiCreateWnd(
                       EDITBOX,
                       "",
                       TWS_CHILD|TWS_DISABLED,
                       0,
                       0,
                       1,
                       1,
                       wnd,
                       IDINPUTBOX,
                       0
                       );
  if (!frmwnd->edtinput)
  {
    free(frmwnd);
    return TUI_ERROR;
  }

  frmwnd->btnyes = TuiCreateWnd(
                       BUTTON,
                       "Yes",
                       TWS_CHILD|TWS_DISABLED,
                       0,
                       0,
                       1,
                       10,
                       wnd,
                       IDYES,
                       0
                       );
  if (!frmwnd->btnyes)
  {
    free(frmwnd);
    return TUI_ERROR;
  }

  frmwnd->btnno = TuiCreateWnd(
                       BUTTON,
                       "No",
                       TWS_CHILD|TWS_DISABLED,
                       0,
                       0,
                       1,
                       10,
                       wnd,
                       IDNO,
                       0
                       );
  if (!frmwnd->btnno)
  {
    free(frmwnd);
    return TUI_ERROR;
  }

  frmwnd->btnok = TuiCreateWnd(
                       BUTTON,
                       "OK",
                       TWS_CHILD|TWS_DISABLED,
                       0,
                       0,
                       1,
                       10,
                       wnd,
                       IDOK,
                       0
                       );
  if (!frmwnd->btnok)
  {
    free(frmwnd);
    return TUI_ERROR;
  }

  frmwnd->btncancel = TuiCreateWnd(
                       BUTTON,
                       "Cancel",
                       TWS_CHILD|TWS_DISABLED,
                       0,
                       0,
                       1,
                       10,
                       wnd,
                       IDCANCEL,
                       0
                       );
  if (!frmwnd->btncancel)
  {
    free(frmwnd);
    return TUI_ERROR;
  }

  TuiSetWndParam(wnd, frmwnd);
  return TUI_CONTINUE;
}

VOID _TFRAMEWND_RestoreAndEnableAllWnds(TWND wnd)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  twndenabled_t* wndenabled = frmwnd->lastwndenabled;

  while (wndenabled)
  {
    TuiEnableWnd(wndenabled->child, TW_ENABLE);
    if (frmwnd->lastwndenabled)
    {
      frmwnd->lastwndenabled = frmwnd->lastwndenabled->prev;
    }
    wndenabled->next = wndenabled->prev = 0;
    free(wndenabled);

    wndenabled = frmwnd->lastwndenabled;
  }
  frmwnd->firstwndenabled = frmwnd->lastwndenabled = 0;
  
  TuiSetFocus(frmwnd->lastfocus);
}

VOID _TFRAMEWND_SaveAndDisableAllWnds(TWND wnd)
{
  TWND child = TuiGetFirstChildWnd(wnd);
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  twndenabled_t* wndenabled = 0;
  /* reset first and last window enabled */
  frmwnd->firstwndenabled = frmwnd->lastwndenabled = 0;

  /* save active children */
  for (; child; child = TuiGetNextWnd(child))
  {
    if (TuiIsWndVisible(child) && TuiIsWndEnabled(child))
    {
      wndenabled = (twndenabled_t*)malloc(sizeof(twndenabled_t));
      wndenabled->prev  = wndenabled->next = 0;
      wndenabled->child = child;
      TuiEnableWnd(child, TW_DISABLE);

      if (frmwnd->firstwndenabled)
      {
        wndenabled->prev = frmwnd->lastwndenabled;
        frmwnd->lastwndenabled->next = wndenabled;
        frmwnd->lastwndenabled = wndenabled;
      }
      else
      {
        frmwnd->firstwndenabled = frmwnd->lastwndenabled = wndenabled;
      }
    } /* visible & enabled windows */
  } /* child windows */
}

VOID TFRAMEWND_OnShowMsgBox(TWND wnd, SHOWMSGBOX* param)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  LONG caplen = 0, textlen = 0, wndwidth = 0;
  LONG nbtns = 0;
  LONG btnwidth = 0;
  RECT rect;
  RECT rc;
  INT i;
  CHAR buf[TUI_MAX_WNDTEXT + 1];
  DWORD attrs = TuiGetWndTextAttrs(wnd);
  TDC dc = TuiGetDC(wnd);
  TWND btns[] = { frmwnd->btnyes, frmwnd->btnno, frmwnd->btnok, frmwnd->btncancel, 0 };

  memcpy(&frmwnd->msgbox, param, sizeof(SHOWMSGBOX));
  frmwnd->lastfocus = TuiGetFocus(wnd);
  frmwnd->msg       = TWM_SHOWMSGBOX;
  
  /* disable all objects */
  _TFRAMEWND_SaveAndDisableAllWnds(wnd);
  
  TuiSetWndText(frmwnd->edtinput, "");

  /* enable special objects */
  if (param->flags & MB_YES)
  {
    TuiEnableWnd(frmwnd->btnyes, TW_ENABLE);
    ++nbtns;
    btnwidth += FRMWND_BTNWIDTH + 2;
  }
  if (param->flags & MB_NO)
  {
    TuiEnableWnd(frmwnd->btnno, TW_ENABLE);
    ++nbtns;
    btnwidth += FRMWND_BTNWIDTH + 2;
  }
  if (param->flags & MB_OK)
  {
    TuiEnableWnd(frmwnd->btnok, TW_ENABLE);
    ++nbtns;
    btnwidth += FRMWND_BTNWIDTH + 2;
  }
  if (param->flags & MB_CANCEL)
  {
    TuiEnableWnd(frmwnd->btncancel, TW_ENABLE);
    ++nbtns;
    btnwidth += FRMWND_BTNWIDTH + 2;
  }

  TuiGetWndRect(wnd, &rect);
  caplen = strlen(param->caption);
  textlen = strlen(param->text);
  wndwidth = MAX(btnwidth, MAX(caplen, textlen));
  if (wndwidth % 2 == 1)
  {
    ++wndwidth;
  }
  /* paint background */
  rc.lines = 7;
  rc.y     = rect.y + (rect.lines - rc.lines)/2;
  rc.cols  = wndwidth + 10;
  rc.x     = rect.x + (rect.cols  - rc.cols)/2;
  /* caption */
  TuiPrintTextAlignment(buf, param->caption, rc.cols, ALIGN_CENTER);
  buf[0] = '[';
  buf[rc.cols-1] = ']';
  TuiDrawText(dc, rc.y, rc.x, buf, TuiGetSysColor(COLOR_WNDTITLE));
  
  memset(buf, ' ', rc.cols);
  buf[rc.cols] = 0;
  for (i = 1; i < rc.lines; ++i)
  {
    TuiDrawText(dc, rc.y + i, rc.x, buf, TuiReverseColor(attrs));
  }

  /* draw text */
  TuiPrintTextAlignment(buf, param->text, rc.cols, ALIGN_CENTER);
  TuiDrawText(dc, rc.y + 3, rc.x, buf, TuiReverseColor(attrs));
  
  if (nbtns > 0)
  {
    INT x = (rc.cols - btnwidth)/2;
    for (i = 0; nbtns > 0 && btns[i] != 0; ++i)
    {
      if (TuiIsWndEnabled(btns[i]))
      {
        /* move buttons */
        TuiMoveWnd(btns[i], rc.y + 5, rc.x + x + 2, 1, FRMWND_BTNWIDTH);
        TuiVisibleWnd(btns[i], TW_SHOW);
        /* next button moved */
        x += FRMWND_BTNWIDTH + 1;
        --nbtns;
      }
    }
  }
}

VOID TFRAMEWND_OnHideMsgBox(TWND wnd)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  memset(&frmwnd->msgbox, 0, sizeof(SHOWMSGBOX));
  frmwnd->msg = 0;
  /* enable all objects */
  _TFRAMEWND_RestoreAndEnableAllWnds(wnd);
  
  TuiEnableWnd(frmwnd->btnyes, TW_DISABLE);
  TuiVisibleWnd(frmwnd->btnyes, TW_HIDE);
  
  TuiEnableWnd(frmwnd->btnno, TW_DISABLE);
  TuiVisibleWnd(frmwnd->btnno, TW_HIDE);

  TuiEnableWnd(frmwnd->btnok, TW_DISABLE);
  TuiVisibleWnd(frmwnd->btnok, TW_HIDE);

  TuiEnableWnd(frmwnd->btncancel, TW_DISABLE);
  TuiVisibleWnd(frmwnd->btncancel, TW_HIDE);

  TuiEnableWnd(frmwnd->edtinput, TW_DISABLE);
  TuiVisibleWnd(frmwnd->edtinput, TW_HIDE);

  TuiInvalidateWnd(wnd);
}

VOID TFRAMEWND_OnShowInputBox(TWND wnd, SHOWMSGBOX* param)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  LONG caplen = 0, textlen = 0, wndwidth = 0;
  LONG nbtns = 0;
  LONG btnwidth = 0;
  RECT rect;
  RECT rc;
  INT i;
  CHAR buf[TUI_MAX_WNDTEXT + 1];
  DWORD attrs = TuiGetWndTextAttrs(wnd);
  TDC dc = TuiGetDC(wnd);
  TWND btns[] = { frmwnd->btnok, frmwnd->btncancel, 0 };

  memcpy(&frmwnd->msgbox, param, sizeof(SHOWMSGBOX));
  frmwnd->lastfocus = TuiGetFocus(wnd);
  frmwnd->msg       = TWM_SHOWINPUTBOX;
  
  /* disable all objects */
  _TFRAMEWND_SaveAndDisableAllWnds(wnd);
  
  /* enable special objects */
  if (param->flags & MB_OK)
  {
    TuiEnableWnd(frmwnd->btnok, TW_ENABLE);
    ++nbtns;
    btnwidth += FRMWND_BTNWIDTH + 2;
  }
  if (param->flags & MB_CANCEL)
  {
    TuiEnableWnd(frmwnd->btncancel, TW_ENABLE);
    ++nbtns;
    btnwidth += FRMWND_BTNWIDTH + 2;
  }

  TuiGetWndRect(wnd, &rect);
  caplen = strlen(param->caption);
  textlen = strlen(param->text);
  wndwidth = MAX(btnwidth, MAX(caplen, textlen));
  if (wndwidth % 2 == 1)
  {
    ++wndwidth;
  }
  /* paint background */
  rc.lines = 7;
  rc.y     = rect.y + (rect.lines - rc.lines)/2;
  rc.cols  = wndwidth + 10;
  rc.x     = rect.x + (rect.cols  - rc.cols)/2;
  /* caption */
  TuiPrintTextAlignment(buf, param->caption, rc.cols, ALIGN_CENTER);
  buf[0] = '[';
  buf[rc.cols-1] = ']';
  TuiDrawText(dc, rc.y, rc.x, buf, TuiGetSysColor(COLOR_WNDTITLE));

  memset(buf, ' ', rc.cols);
  buf[rc.cols] = 0;
  for (i = 1; i < rc.lines; ++i)
  {
    TuiDrawText(dc, rc.y + i, rc.x, buf, TuiReverseColor(attrs));
  }

  /* draw text */
  TuiPrintTextAlignment(buf, param->text, rc.cols - 2, ALIGN_LEFT);
  TuiDrawText(dc, rc.y + 2, rc.x + 1, buf, TuiReverseColor(attrs));

  /* edit box */
  TuiSetWndStyle(frmwnd->edtinput, param->edtstyle);
  TuiSetWndText(frmwnd->edtinput, param->deftext);
  TEDT_LimitText(frmwnd->edtinput, param->limit);
  TuiMoveWnd(frmwnd->edtinput,
    rc.y + 3,
    rc.x + 1,
    1,
    (param->limit < rc.cols - 2 ? param->limit : rc.cols - 2));
  TuiEnableWnd(frmwnd->edtinput, TW_ENABLE);
  TuiVisibleWnd(frmwnd->edtinput, TW_SHOW);
  
  if (nbtns > 0)
  {
    INT x = (rc.cols - btnwidth)/2;
    for (i = 0; nbtns > 0 && btns[i] != 0; ++i)
    {
      if (TuiIsWndEnabled(btns[i]))
      {
        /* move buttons */
        TuiMoveWnd(btns[i], rc.y + 5, rc.x + x + 2, 1, FRMWND_BTNWIDTH);
        TuiVisibleWnd(btns[i], TW_SHOW);
        /* next button moved */
        x += FRMWND_BTNWIDTH + 1;
        --nbtns;
      }
    }
  }
  
  TuiSetFocus(frmwnd->edtinput);
}

VOID TFRAMEWND_OnHideInputBox(TWND wnd)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  TFRAMEWND_OnHideMsgBox(wnd);
  TuiSetNextMove(frmwnd->nextmove);
}

VOID TFRAMEWND_OnShowLineInputBox(TWND wnd, SHOWMSGBOX* param)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  RECT rc;
  RECT rcwnd;
  CHAR buf[TUI_MAX_WNDTEXT + 1];
  DWORD attrs = TuiGetWndTextAttrs(wnd);
  TDC dc = TuiGetDC(wnd);
  LONG textlen = 0;
  LONG ch = 0;
  CHAR* psz;
  INT   x = 0;

  memcpy(&frmwnd->msgbox, param, sizeof(SHOWMSGBOX));
  frmwnd->lastfocus = TuiGetFocus(wnd);
  frmwnd->msg       = TWM_SHOWLINEINPUTBOX;
  TuiSetWndText(frmwnd->edtinput, param->deftext);
  
  /* disable all objects */
  _TFRAMEWND_SaveAndDisableAllWnds(wnd);
  
  /* enable special objects */
  textlen = strlen(param->text) + param->limit;
  /* paint background */
  rc.lines = 1;
  rc.y     = param->y;
  rc.cols  = textlen + 3;
  rc.x     = param->x;

  /* draw text */
  TuiGetWndRect(wnd, &rcwnd);
  
  strcpy(buf, param->text);
  strcat(buf, " [ ]");

  textlen = strlen(buf);
  if (ALIGN_CENTER == param->align)
  {
    x  = (rcwnd.cols - textlen)/2;
    TuiDrawText(dc, rc.y,
      x + 1, buf, TuiReverseColor(attrs));
    x += textlen - 1;
  }
  else
  {
    TuiDrawText(dc, rc.y, rc.x + 1, buf, TuiReverseColor(attrs));
    x = rc.x + textlen - 1;
  }
  TuiPutChar(dc, rc.y, x, param->deftext[0], TuiReverseColor(attrs));
  TuiMoveYX(dc, rc.y, x);
  
  buf[0] = param->deftext[0];
  buf[1] = 0;
  do
  {
    ch = TuiGetChar();
    if (TVK_ENTER == ch)
      break;
    /* transform to upper char */
    if (ch >= 'a' && ch <= 'z')
    {
      ch = ch - 'a' + 'A';
    }
    /* check valid chars */
    if (ch >= 'A' && ch <= 'Z')
    {
      psz = strchr(param->validch, (INT)ch);
      if (psz)
      {
        TuiPutChar(dc, rc.y, x, ch, TuiReverseColor(attrs));
        TuiMoveYX(dc, rc.y, x);
        buf[0] = ch;
      }
    }
  } while (1);
  
  TuiSetWndText(frmwnd->edtinput, buf);
  TuiSendMsg(wnd, TWM_COMMAND, (WPARAM)IDOK, 0);
}

VOID TFRAMEWND_OnHideLineInputBox(TWND wnd)
{
  TFRAMEWND_OnHideMsgBox(wnd);
}

VOID TFRAMEWND_OnCommand(TWND wnd, UINT cmd)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  switch (cmd)
  {
    case IDOK:
    case IDYES:
    case IDNO:
    case IDCANCEL:
    {
      RESPONSEMSGBOX res;
      res.hdr.id   = cmd;
      res.hdr.code = frmwnd->msgbox.id;
      res.hdr.ctl  = 0;
      if (IDOK == cmd || IDYES == cmd)
      {
        TuiGetWndText(frmwnd->edtinput, res.text, TUI_MAX_WNDTEXT);
      }
      else
      {
        res.text[0] = 0;
      }
      TuiSendMsg(wnd, TWM_NOTIFY, (WPARAM)0, (LPARAM)&res);
      TuiSendMsg(wnd, TWM_SHOWMSGBOX, (WPARAM)TW_HIDE, (LPARAM)0);
      break;
    }
  }
}

VOID TFRAMEWND_OnNotify(TWND wnd, NMHDR* nmhdr)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  switch (nmhdr->code)
  {
    case TEN_KILLFOCUS:
    {
      if (IDINPUTBOX == nmhdr->id)
      {
        if (TWM_SHOWLINEINPUTBOX == frmwnd->msg)
        {
          TuiSendMsg(wnd, TWM_COMMAND, (WPARAM)IDOK, 0);
        }
      }
    }
  }
}

LONG TuiDefFrameWndProc(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_INITDIALOG:
    {
      return TFRAMEWND_OnInitDailog(wnd, lparam);
    }

    case TWM_SHOWMSGBOX:
    {
      if (TW_SHOW == wparam)
      {
        TFRAMEWND_OnShowMsgBox(wnd, (SHOWMSGBOX*)lparam);
      }
      else
      {
        TFRAMEWND_OnHideMsgBox(wnd);
      }
      return 0;
    }
    
    case TWM_SHOWINPUTBOX:
    {
      if (TW_SHOW == wparam)
      {
        TFRAMEWND_OnShowInputBox(wnd, (SHOWMSGBOX*)lparam);
      }
      else
      {
        TFRAMEWND_OnHideInputBox(wnd);
      }
      return 0;
    }
    
    case TWM_SHOWLINEINPUTBOX:
    {
      if (TW_SHOW == wparam)
      {
        TFRAMEWND_OnShowLineInputBox(wnd, (SHOWMSGBOX*)lparam);
      }
      else
      {
        TFRAMEWND_OnHideLineInputBox(wnd);
      }
      return 0;
    }
    
    case TWM_COMMAND:
    {
      TFRAMEWND_OnCommand(wnd, (UINT)wparam);
      break;
    }
    
    case TWM_NOTIFY:
    {
      TFRAMEWND_OnNotify(wnd, (NMHDR*)lparam);
      break;
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}

VOID TuiShowMsgBox(
  TWND    wnd,
  UINT    id,
  LPCSTR  caption,
  LPCSTR  text,
  UINT    flags,
  INT     show)
{
  SHOWMSGBOX box;
  box.id      = id;
  box.y       = 0;
  box.x       = 0;
  box.caption = caption;
  box.text    = text;
  box.flags   = flags;
  box.limit   = 0;
  box.edtstyle = TWS_CHILD|TWS_DISABLED;
  box.align    = ALIGN_CENTER;
  
  TuiSendMsg(wnd, TWM_SHOWMSGBOX, (WPARAM)show, (LPARAM)&box);
}

VOID TuiShowInputBox(
  TWND    wnd,
  UINT    id,
  LPCSTR  caption,
  LPCSTR  text,
  UINT    flags,
  INT     limit,
  DWORD   edtstyle,
  LPCSTR  deftext,
  INT     show)
{
  SHOWMSGBOX box;
  box.id      = id;
  box.y       = 0;
  box.x       = 0;
  box.caption = caption;
  box.text    = text;
  box.flags   = flags;
  box.limit   = limit;
  box.deftext = deftext;
  box.edtstyle = TWS_CHILD|TWS_VISIBLE|edtstyle;
  box.align    = ALIGN_CENTER;
  
  TuiSendMsg(wnd, TWM_SHOWINPUTBOX, (WPARAM)show, (LPARAM)&box);
}

VOID TuiShowLineInputBox(
  TWND    wnd,
  UINT    id,
  INT     y,
  INT     x,
  LPCSTR  text,
  LPCSTR  deftext,
  LPCSTR  validch,
  INT     align,
  INT     show)
{
  SHOWMSGBOX box;
  box.id      = id;
  box.y       = y;
  box.x       = x;
  box.caption = 0;
  box.text    = text;
  box.flags   = 0;
  box.limit   = 0;
  box.deftext = deftext;
  box.validch = validch;
  box.edtstyle = 0;
  box.align    = align;
  
  TuiSendMsg(wnd, TWM_SHOWLINEINPUTBOX, (WPARAM)show, (LPARAM)&box);
}
