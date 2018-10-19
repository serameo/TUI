/*-------------------------------------------------------------------
 * File name: tfrmwnd.c
 * Author: Seree Rakwong
 * Date: 08-OCT-18
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
 * Frame window functions
 *-----------------------------------------------------------------*/
#define FRMWND_BTNWIDTH      12

struct _SHOWMSGBOXSTRUCT
{
  UINT      id;
  INT       y;
  INT       x;
  LPCSTR    text;
  LPCSTR    caption;
  UINT      flags; /* MB_OK, MB_YES, MB_NO, MB_CANCEL */
  INT       limit;
  LPCSTR    deftext;
  DWORD     edtstyle;
  DWORD     wndattrs;
  LPCSTR    validch;
  INT       align;
};
typedef struct _SHOWMSGBOXSTRUCT SHOWMSGBOX;

struct _WNDENABLEDSTRUCT
{
  TWND      child;
  struct _WNDENABLEDSTRUCT*     prev;
  struct _WNDENABLEDSTRUCT*     next;
};
typedef struct _WNDENABLEDSTRUCT twndenabled_t;

struct _WNDVISIBLESTRUCT
{
  TWND      child;
  struct _WNDVISIBLESTRUCT*     prev;
  struct _WNDVISIBLESTRUCT*     next;
};
typedef struct _WNDVISIBLESTRUCT twndvisible_t;

struct _WNDPAGESTRUCT
{
  INT     id;
  TWND    firstchild;
  TWND    lastchild;
  VOID*   param;

  twndvisible_t*  firstvisible;
  twndvisible_t*  lastvisible;
  
  twndenabled_t*  firstenabled;
  twndenabled_t*  lastenabled;
  
  struct _WNDPAGESTRUCT*        prev;
  struct _WNDPAGESTRUCT*        next;
};
typedef struct _WNDPAGESTRUCT twndpage_t;

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
  
  twndpage_t*     firstpage;
  twndpage_t*     lastpage;
  twndpage_t*     activepage;
  INT             npages;
  
  SHOWMSGBOX      msgbox;
  UINT            msg;
  VOID*           exparam;
};
typedef struct _TFRAMEWNDSTRUCT _TFRAMEWND;
typedef struct _TFRAMEWNDSTRUCT *TFRAMEWND;

VOID _TFRMWND_RestoreEnableAllWnds(TWND wnd);
VOID _TFRMWND_SaveAndDisableAllWnds(TWND wnd);
VOID _TFRMWND_RestoreAndShowAllWndsPage(TWND wnd);
VOID _TFRMWND_SaveAndHideAllWndsPage(TWND wnd);
twndpage_t* _TFRMWND_FindPage(TFRAMEWND frmwnd, INT npage);
VOID _TFRMWND_ShowPage(twndpage_t* page, INT show);

INT  _TFRMWND_OnInitDailog(TWND wnd, LPARAM lparam);
VOID _TFRMWND_OnCommand(TWND wnd, UINT cmd);
VOID _TFRMWND_OnShowMsgBox(TWND wnd, SHOWMSGBOX* param);
VOID _TFRMWND_OnHideMsgBox(TWND wnd);
VOID _TFRMWND_OnShowInputBox(TWND wnd, SHOWMSGBOX* param);
VOID _TFRMWND_OnHideInputBox(TWND wnd);
VOID _TFRMWND_OnShowLineInputBox(TWND wnd, SHOWMSGBOX* param);
VOID _TFRMWND_OnHideLineInputBox(TWND wnd);
VOID _TFRMWND_OnNotify(TWND wnd, NMHDR* nmhdr);
INT  _TFRMWND_OnInitPage(TWND wnd, TINITPAGE initpage, LPVOID param);
VOID _TFRMWND_OnShowPage(TWND wnd, INT npage);
VOID _TFRMWND_OnHidePage(TWND wnd, INT npage);
VOID _TFRMWND_OnSetCurPage(TWND wnd, INT npage);
INT  _TFRMWND_OnGetCurPage(TWND wnd);
VOID _TFRMWND_OnKeyDown(TWND wnd, LONG ch);

INT  _TFRMWND_OnInitPage(TWND wnd, TINITPAGE initpage, LPVOID param)
{
  twndpage_t* page = 0;
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  
  /* init */
  page = (twndpage_t*)malloc(sizeof(twndpage_t));
  if (!page)
  {
    return TUI_MEM;
  }
  /* make links */
  memset(page, 0, sizeof(twndpage_t));
  page->id         = frmwnd->npages;
  page->param      = param;
  page->firstchild = initpage->firstchild;
  page->lastchild  = initpage->lastchild;

  if (frmwnd->firstpage)
  {
    page->prev = frmwnd->lastpage;
    frmwnd->lastpage->next = page;
    frmwnd->lastpage = page;
  }
  else
  {
    frmwnd->firstpage = frmwnd->lastpage = page;
    frmwnd->activepage = page;
  }
  _TFRMWND_ShowPage(page, TW_HIDE);
  /* increment counter */
  ++frmwnd->npages;

  return TUI_CONTINUE;
}

twndpage_t* _TFRMWND_FindPage(TFRAMEWND frmwnd, INT npage)
{
  twndpage_t* page = frmwnd->firstpage;
  INT i = 0;
  
  if (npage < 0 || npage >= frmwnd->npages)
  {
    return 0;
  }
  while (page)
  {
    if (i == npage)
    {
      break;
    }
    page = page->next;
    ++i;
  }
  return page;
}

VOID _TFRMWND_OnSetCurPage(TWND wnd, INT npage)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  twndpage_t* page = frmwnd->activepage;
  twndpage_t* newpage = _TFRMWND_FindPage(frmwnd, npage);
  LONG rc = TUI_CONTINUE;
  PAGECHANGE pc;
  
  if (newpage)
  {
    /* notify to window */
    pc.hdr.id   = TuiGetWndID(wnd);
    pc.hdr.ctl  = wnd;
    pc.hdr.code = TFN_PAGECHANGING;
    pc.frompage = page->id;
    pc.topage   = newpage->id;
    rc = TuiSendMsg(wnd, TWM_NOTIFY, 0, (LPARAM)&pc);
    if (rc != TUI_CONTINUE)
    {
      return;
    }
    /* hide current page */
    _TFRMWND_ShowPage(page, TW_HIDE);
    /* set new active page */
    frmwnd->activepage = newpage;
    /* show the new page */
    _TFRMWND_ShowPage(newpage, TW_SHOW);
    /* notify to window */
    pc.hdr.id   = TuiGetWndID(wnd);
    pc.hdr.ctl  = wnd;
    pc.hdr.code = TFN_PAGECHANGED;
    pc.frompage = page->id;
    pc.topage   = newpage->id;
    rc = TuiPostMsg(wnd, TWM_NOTIFY, 0, (LPARAM)&pc);
  }
}

INT _TFRMWND_OnGetCurPage(TWND wnd)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  return (frmwnd->activepage ? frmwnd->activepage->id : -1);
}

VOID _TFRMWND_ShowPage(twndpage_t* page, INT show)
{
  TWND child = 0;
  
  if (page)
  {
    child = page->firstchild;
    while (child && child != page->lastchild)
    {
      TuiShowWnd(child, show);
      child = TuiGetNextWnd(child);
    }
    if (page->lastchild)
    {
      TuiShowWnd(page->lastchild, show);
    }
  }
}

VOID _TFRMWND_OnShowPage(TWND wnd, INT npage)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  twndpage_t* page = _TFRMWND_FindPage(frmwnd, npage);
  _TFRMWND_ShowPage(page, TW_SHOW);
}

VOID _TFRMWND_OnHidePage(TWND wnd, INT npage)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  twndpage_t* page = _TFRMWND_FindPage(frmwnd, npage);
  _TFRMWND_ShowPage(page, TW_HIDE);
}

INT  _TFRMWND_OnInitDailog(TWND wnd, LPARAM lparam)
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

VOID _TFRMWND_RestoreEnableAllWnds(TWND wnd)
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

VOID _TFRMWND_SaveAndDisableAllWnds(TWND wnd)
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

VOID _TFRMWND_OnShowMsgBox(TWND wnd, SHOWMSGBOX* param)
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
  _TFRMWND_SaveAndDisableAllWnds(wnd);
  
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
        TuiShowWnd(btns[i], TW_SHOW);
        /* next button moved */
        x += FRMWND_BTNWIDTH + 1;
        --nbtns;
      }
    }
  }
}

VOID _TFRMWND_OnHideMsgBox(TWND wnd)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  memset(&frmwnd->msgbox, 0, sizeof(SHOWMSGBOX));
  frmwnd->msg = 0;
  /* enable all objects */
  _TFRMWND_RestoreEnableAllWnds(wnd);
  
  TuiEnableWnd(frmwnd->btnyes, TW_DISABLE);
  TuiShowWnd(frmwnd->btnyes, TW_HIDE);
  
  TuiEnableWnd(frmwnd->btnno, TW_DISABLE);
  TuiShowWnd(frmwnd->btnno, TW_HIDE);

  TuiEnableWnd(frmwnd->btnok, TW_DISABLE);
  TuiShowWnd(frmwnd->btnok, TW_HIDE);

  TuiEnableWnd(frmwnd->btncancel, TW_DISABLE);
  TuiShowWnd(frmwnd->btncancel, TW_HIDE);

  TuiEnableWnd(frmwnd->edtinput, TW_DISABLE);
  TuiShowWnd(frmwnd->edtinput, TW_HIDE);

  TuiInvalidateWnd(wnd);
}

VOID _TFRMWND_OnShowInputBox(TWND wnd, SHOWMSGBOX* param)
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
  _TFRMWND_SaveAndDisableAllWnds(wnd);
  
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
  TuiShowWnd(frmwnd->edtinput, TW_SHOW);
  
  if (nbtns > 0)
  {
    INT x = (rc.cols - btnwidth)/2;
    for (i = 0; nbtns > 0 && btns[i] != 0; ++i)
    {
      if (TuiIsWndEnabled(btns[i]))
      {
        /* move buttons */
        TuiMoveWnd(btns[i], rc.y + 5, rc.x + x + 2, 1, FRMWND_BTNWIDTH);
        TuiShowWnd(btns[i], TW_SHOW);
        /* next button moved */
        x += FRMWND_BTNWIDTH + 1;
        --nbtns;
      }
    }
  }
  
  TuiSetFocus(frmwnd->edtinput);
}

VOID _TFRMWND_OnHideInputBox(TWND wnd)
{
  _TFRMWND_OnHideMsgBox(wnd);
}

VOID _TFRMWND_OnShowLineInputBox(TWND wnd, SHOWMSGBOX* param)
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
  _TFRMWND_SaveAndDisableAllWnds(wnd);
  
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

VOID _TFRMWND_OnHideLineInputBox(TWND wnd)
{
  _TFRMWND_OnHideMsgBox(wnd);
}

VOID _TFRMWND_OnCommand(TWND wnd, UINT cmd)
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
      TuiSendMsg(wnd, TWM_SHOWMSGBOX, (WPARAM)TW_HIDE, (LPARAM)0);
      TuiPostMsg(wnd, TWM_NOTIFY, (WPARAM)0, (LPARAM)&res);
      break;
    }
  }
}

VOID _TFRMWND_OnNotify(TWND wnd, NMHDR* nmhdr)
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
          break;
        }
      }      
    }
  }
}

VOID _TFRMWND_OnKeyDown(TWND wnd, LONG ch)
{
  TFRAMEWND frmwnd = TuiGetWndParam(wnd);
  twndpage_t* page = frmwnd->activepage;
  INT repaint = 0;
  
  if (page)
  {
    switch(ch)
    {
      case TVK_PRIOR:
  #ifdef __USE_CURSES__
      case KEY_PPAGE:
  #elif (defined __USE_QIO__ && defined __VMS__)
      case KEY_PREV:
  #endif
      {
        if (page->id > 0)
        {
          _TFRMWND_OnSetCurPage(wnd, page->id - 1);
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
        if (page->id < frmwnd->npages - 1)
        {
          _TFRMWND_OnSetCurPage(wnd, page->id + 1);
          ++repaint;
        }
        break;
      }
    }
    if (repaint)
    {
      TuiInvalidateWnd(wnd);
    }

  }
}

LONG TuiDefFrameWndProc(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_KEYDOWN:
    {
      _TFRMWND_OnKeyDown(wnd, (LONG)wparam);
      return 0;
    }
    
    case TWM_INITDIALOG:
    {
      return _TFRMWND_OnInitDailog(wnd, lparam);
    }

    case TWM_SHOWMSGBOX:
    {
      if (TW_SHOW == wparam)
      {
        _TFRMWND_OnShowMsgBox(wnd, (SHOWMSGBOX*)lparam);
      }
      else
      {
        _TFRMWND_OnHideMsgBox(wnd);
      }
      return 0;
    }
    
    case TWM_SHOWINPUTBOX:
    {
      if (TW_SHOW == wparam)
      {
        _TFRMWND_OnShowInputBox(wnd, (SHOWMSGBOX*)lparam);
      }
      else
      {
        _TFRMWND_OnHideInputBox(wnd);
      }
      return 0;
    }
    
    case TWM_SHOWLINEINPUTBOX:
    {
      if (TW_SHOW == wparam)
      {
        _TFRMWND_OnShowLineInputBox(wnd, (SHOWMSGBOX*)lparam);
      }
      else
      {
        _TFRMWND_OnHideLineInputBox(wnd);
      }
      return 0;
    }
    
    case TWM_COMMAND:
    {
      _TFRMWND_OnCommand(wnd, (UINT)wparam);
      break;
    }
    
    case TWM_NOTIFY:
    {
      if (TFN_PAGECHANGING == ((NMHDR*)lparam)->code)
      {
        return TUI_CONTINUE;
      }
      else
      {
        _TFRMWND_OnNotify(wnd, (NMHDR*)lparam);
      }
      break;
    }
    
    case TWM_INITPAGE:
    {
      return _TFRMWND_OnInitPage(wnd, (TINITPAGE)wparam, (LPVOID)lparam);
    }
    
    case TWM_SHOWPAGE:
    {
      if (TW_SHOW == lparam)
      {
        _TFRMWND_OnShowPage(wnd, (INT)wparam);
      }
      else
      {
        _TFRMWND_OnHidePage(wnd, (INT)wparam);
      }
      return 0;
    }

    case TWM_SETCURPAGE:
    {
      _TFRMWND_OnSetCurPage(wnd, (INT)wparam);
      return 0;
    }

    case TWM_GETCURPAGE:
    {
      return _TFRMWND_OnGetCurPage(wnd);
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}

VOID TuiShowPage(
  TWND    wnd,
  INT     npage,
  INT     show
)
{
  TuiSendMsg(wnd,
    TWM_SHOWPAGE,
    (WPARAM)npage,
    (LPARAM)(TW_SHOW == show ? show : TW_HIDE));
}

VOID TuiSetCurPage(
  TWND    wnd,
  INT     npage
)
{
  TuiSendMsg(wnd,
    TWM_SETCURPAGE,
    (WPARAM)npage,
    (LPARAM)0);
}


INT TuiGetCurPage(
  TWND    wnd
)
{
  return (INT)TuiSendMsg(wnd,
                TWM_GETCURPAGE,
                (WPARAM)0,
                (LPARAM)0);
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
