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


TWND  _TuiMsgBox(TWND parent, LPCSTR caption, LPCSTR text, UINT flags);
INT  TMSGBX_OnInitDailog(TWND wnd, LPARAM lparam);
VOID TMSGBX_OnPaint(TWND wnd, TDC dc);
VOID TMSGBX_OnCommand(TWND wnd, UINT cmd);
LONG MSGBOXPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

VOID TMSGBX_OnPaint(TWND wnd, TDC dc)
{
  CHAR text[TUI_MAX_WNDTEXT+1];
  CHAR buf[TUI_MAX_WNDTEXT+1];
  RECT rc;
  
  TuiGetWndRect(wnd, &rc);
  TuiGetWndText(wnd, text, TUI_MAX_WNDTEXT);
  TuiPrintTextAlignment(buf, text, rc.cols, ALIGN_CENTER);
  TuiDrawText(dc, rc.y + 3, rc.x, buf, TuiGetWndTextAttrs(wnd));
}

INT  TMSGBX_OnInitDailog(TWND wnd, LPARAM lparam)
{
  TWND btn;
  MSGBOXPARAM *msgbx = 0;
  
  msgbx = (MSGBOXPARAM*)malloc(sizeof(MSGBOXPARAM));
  memset(msgbx, 0, sizeof(MSGBOXPARAM));
  
  if (lparam)
  {
    if (!msgbx)
    {
      return TUI_ERROR;
    }
    memcpy(msgbx, (MSGBOXPARAM*)lparam, sizeof(MSGBOXPARAM));
  }
  TuiSetWndParam(wnd, (LPVOID)msgbx);
  
  btn = TuiGetFirstActiveChildWnd(wnd);
  if (btn)
  {
    TuiSetFocus(btn);
  }
  TuiSendMsg(wnd,
    TWM_SETTEXTATTRS,
    (WPARAM)(TuiGetColor(BLACK_WHITE)),
    (LPARAM)0);
  return TUI_CONTINUE;
}

VOID TMSGBX_OnCommand(TWND wnd, UINT cmd)
{
  MSGBOXPARAM *msgbx = TuiGetWndParam(wnd);
  if (msgbx)
  {
    switch (cmd)
    {
      case MB_OK:
      {
        if (msgbx->OnOK)
        {
          msgbx->OnOK(msgbx->owner, msgbx->param);
        }
        break;
      }
      case MB_YES:
      {
        if (msgbx->OnYes)
        {
          msgbx->OnYes(msgbx->owner, msgbx->param);
        }
        break;
      }
      case MB_NO:
      {
        if (msgbx->OnNo)
        {
          msgbx->OnNo(msgbx->owner, msgbx->param);
        }
        break;
      }
      case MB_CANCEL:
      {
        if (msgbx->OnCancel)
        {
          msgbx->OnCancel(msgbx->owner, msgbx->param);
        }
        break;
      }
    }
  }
}

LONG MSGBOXPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_INITDIALOG:
    {
      return TMSGBX_OnInitDailog(wnd, lparam);
    }
    
    case TWM_PAINT:
    {
      TMSGBX_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }

    case TWM_COMMAND:
    {
      TMSGBX_OnCommand(wnd, (UINT)wparam);
      TuiEndDlg(wnd, (UINT)wparam);
      break;
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}

TWND  _TuiMsgBox(TWND parent, LPCSTR caption, LPCSTR text, UINT flags)
{
  TWND box = 0;
  RECT rect;
  LONG caplen = 0, textlen = 0, wndwidth = 0;
  LONG nbtns = 0;
  LONG btnwidth = 0;
  INT  i;
  
  enum
  {
    Msgbox  = 0,
    CaptionStatic,
    YesButton,
    NoButton,
    OKButton,
    CancelButton,
    EmptyCtl
  };
  WNDTEMPL msgbox[] =
  {
    { MSGBOX, text, 0,  0,  1,  7,  1, TWS_WINDOW, 0 },
    /* text */
    { STATIC, caption,  MB_CAPTION, 0,  0,  1,   1, TWS_CHILD|TWS_VISIBLE|TSS_CENTER, 0 },
    /* buttons */
    { BUTTON, "Yes",    MB_YES,     0,  0,  1,  13, TWS_CHILD, 0 },
    { BUTTON, "No",     MB_NO,      0,  0,  1,  12, TWS_CHILD, 0 },
    { BUTTON, "OK",     MB_OK,      0,  0,  1,  12, TWS_CHILD, 0 },
    { BUTTON, "Cancel", MB_CANCEL,  0,  0,  1,  12, TWS_CHILD, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
  };

  /* calculate window width */
  if (flags & MB_YES)
  {
    msgbox[YesButton].style |= TWS_VISIBLE;
    ++nbtns;
    btnwidth += msgbox[YesButton].cols + 2;
  }
  else
  {
    msgbox[YesButton].style &= ~TWS_VISIBLE;
  }
  if (flags & MB_NO)
  {
    msgbox[NoButton].style |= TWS_VISIBLE;
    ++nbtns;
    btnwidth += msgbox[NoButton].cols + 2;
  }
  else
  {
    msgbox[NoButton].style &= ~TWS_VISIBLE;
  }
  if (flags & MB_OK)
  {
    msgbox[OKButton].style |= TWS_VISIBLE;
    ++nbtns;
    btnwidth += msgbox[OKButton].cols + 2;
  }
  else
  {
    msgbox[OKButton].style &= ~TWS_VISIBLE;
  }
  if (flags & MB_CANCEL)
  {
    msgbox[CancelButton].style |= TWS_VISIBLE;
    btnwidth += msgbox[CancelButton].cols + 2;
    ++nbtns;
  }
  else
  {
    msgbox[CancelButton].style &= ~TWS_VISIBLE;
  }
  
  TuiGetWndRect(parent, &rect);
  caplen   = strlen(caption);
  textlen  = strlen(text);
  wndwidth = MAX(btnwidth, MAX(caplen, textlen));
  if (wndwidth % 2 == 1)
  {
    ++wndwidth;
  }
  
  /* move window to center */
  msgbox[Msgbox].lines = 7;
  msgbox[Msgbox].y     = rect.y + (rect.lines - msgbox[Msgbox].lines)/2;
  msgbox[Msgbox].cols  = wndwidth + 10;
  msgbox[Msgbox].x     = rect.x + (rect.cols  - msgbox[Msgbox].cols)/2;
  
  /* move caption to center */
  msgbox[CaptionStatic].lines = 1;
  msgbox[CaptionStatic].y     = 1;
  msgbox[CaptionStatic].x     = 1;
  msgbox[CaptionStatic].cols  = msgbox[Msgbox].cols - 2;
  
  /* move buttons to center */
  if (nbtns > 0)
  {
    INT x = (msgbox[Msgbox].cols - btnwidth)/2;
    for (i = YesButton; nbtns > 0 && i < EmptyCtl; ++i)
    {
      if (msgbox[i].style & TWS_VISIBLE)
      {
        /* move buttons */
        msgbox[i].lines = 1;
        msgbox[i].y     = 5;
        msgbox[i].x     = x + 2;
        /* next button moved */
        x += msgbox[i].cols + 1;
        --nbtns;
      }
    }
  }
  
  box = TuiCreateWndTempl(msgbox, 0);
  return box;
}

LONG TuiMsgBoxParam(
  TWND parent, LPCSTR caption, LPCSTR text, UINT flags,
  MSGBOXPARAM* param)
{
  LONG rc = TUI_ERROR;
  MSGBOXPARAM *msgbx = 0;
  TWND box = _TuiMsgBox(parent, caption, text, flags);
  if (box)
  {
    rc = TUI_OK;

    msgbx = (MSGBOXPARAM*)TuiGetWndParam(box);
    memcpy(msgbx, param, sizeof(MSGBOXPARAM));

    TuiSetWndParam(box, msgbx);
    TuiShowWnd(box, TW_SHOW);
  }
  return rc; 
}

LONG TuiMsgBox(TWND parent, LPCSTR caption, LPCSTR text, UINT flags)
{
  LONG rc = TUI_ERROR;
  TWND box = _TuiMsgBox(parent, caption, text, flags);
  if (box)
  {
    rc = TUI_OK;
    TuiShowWnd(box, TW_SHOW);
  }
  return rc; 
}

