/*-------------------------------------------------------------------
 * File name: tui_test.c
 * Author: Seree Rakwong
 * Date: 28-SEP-18
 *-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "tui.h"

#define IDC_STATUSBAR    199
#define IDC_NAME         200
#define IDC_LISTBOX1     204
#define IDC_PASSWORD     205
#define IDC_OK           206
#define IDC_CANCEL       207
#define IDC_LISTBOX2     208
#define IDC_LISTBOX3     209
#define IDC_CLOSE        210
#define IDC_SAVE         211
#define IDC_OPENDLG2     212
#define IDC_OPENDLG3     213
#define IDC_MSG          214
#define IDC_PRICE        215
#define IDC_EDITCELL     216
#define IDC_OPENDLG4     217

WNDTEMPL dlg3[] =
{
  /* 1st object is always dialog */
  { "mylistctlproc", "Dialog3", 2,  0,  0, 25, 80, TWS_WINDOW, 0 },
  /* 2nd and others are controls */
  { LISTCTRL, "",    IDC_LISTBOX1,  1,  1,  16,  79, 
    TWS_CHILD|TWS_VISIBLE|
      TLCS_EDITABLE|TLCS_LINEEDIT/*|TLCS_NOSELECTION|TLCS_NOHEADER*/, 0 },
  { BUTTON, "Edit",    IDC_EDITCELL,  20,  1,  1,  14, TWS_CHILD|TWS_VISIBLE, 0 },
  { BUTTON, "Message",    IDC_MSG,  20,  20,  1,  15, TWS_CHILD|TWS_VISIBLE, 0 },
  { BUTTON, "Close",    IDC_CLOSE,  20,  40,  1,  15, TWS_CHILD|TWS_VISIBLE, 0 },
  /* the last is the end-of-controls */
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};


FRMWNDTEMPL dlg4[] =
{
  /* 1st object is always dialog */
  /*{ "mylistpagectlproc", "Dialog4", 2,  0,  0, 25, 80, TWS_WINDOW, 0 },*/
  /* 2nd and others are controls */
  { LISTPAGECTRL, "",    IDC_LISTBOX1,  1,  1,  11,  79, 
    TWS_CHILD|TWS_VISIBLE, 0, 0, "list page control" },
  { BUTTON, "Close",    IDC_CLOSE,  20,  40,  1,  15, TWS_CHILD|TWS_VISIBLE, 0, 0, "Close" },
  { STATIC, "",    IDC_STATUSBAR,  24,  0,  1,  80, TWS_CHILD|TWS_VISIBLE, 0, 0, "" },
  /* the last is the end-of-controls */
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

WNDTEMPL frame1[] =
{
  /* 1st object is always dialog */
  { "mywndproc", "Dialog1", 1,  0,  0, 25, 80, TWS_WINDOW|TWS_BORDER, 0 },
  /* 2nd and others are controls */
  { STATIC, "Name:",    100,  1,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "12345678901234567890",       IDC_NAME,  1, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_LEFT|TES_UPPERCASE|TES_AUTOHSCROLL, 0 },
  { STATIC, "ID:",    101,  2,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "DT66234",     201,  2, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_CENTER|TES_AUTOHSCROLL, 0 },
  { STATIC, "Tel:",    102,  3,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "1234",   202,  3, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_NUMBER|TES_RIGHT|TES_AUTOHSCROLL, 0 },
  { STATIC, "Email:",    103,  4,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "abc@abc.com", 203,  4, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|TWS_DISABLED|
      TES_AUTOHSCROLL, 0 },
  { STATIC, "Password:",    104,  5,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "welcome1!", IDC_PASSWORD,  5, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_LEFT|TES_PASSWORD|TES_AUTOHSCROLL, 0 },
  { STATIC, "Price:",    105,  6,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "132,399.50", IDC_PRICE,  6, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_RIGHT|TES_AUTODECIMALCOMMA|TES_AUTOHSCROLL, 0 },
  { LISTBOX,"",               IDC_LISTBOX1,  8,  1,  5, 16, 
    TWS_CHILD|TWS_VISIBLE|TLBS_CENTER|TWS_BORDER, 0 },
  { LISTBOX,"",               IDC_LISTBOX2,  8, 21,  5, 16, 
    TWS_CHILD|TWS_VISIBLE|TLBS_CHECKBOX|TWS_BORDER, 0 },
  { LISTBOX,"",               IDC_LISTBOX3,  8, 41,  5, 16, 
    TWS_CHILD|TWS_VISIBLE|TLBS_RADIOBOX|TLBS_RIGHT|TWS_BORDER, 0 },
  { BUTTON, "Dlg2",    IDC_OPENDLG2,  14,  1,  1,   10, TWS_CHILD|TWS_VISIBLE, 0 },
  { BUTTON, "Dlg3",    IDC_OPENDLG3,  14,  11,  1,   10, TWS_CHILD|TWS_VISIBLE, 0 },
  { BUTTON, "Dlg4",    IDC_OPENDLG4,  14,  21,  1,   10, TWS_CHILD|TWS_VISIBLE, 0 },
  { BUTTON, "Close",    IDC_CLOSE,    14,  31,  1,  11, TWS_CHILD|TWS_VISIBLE, 0 },
  { STATIC, "Esc to exit: ", IDC_STATUSBAR, 24,  1,  1, 79, TWS_CHILD|TWS_VISIBLE|TWS_DISABLED|TSS_LEFT, 0 },
  /* the last is the end-of-controls */
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

VOID mywndproc_onselchanged(TWND wnd, TWND ctl)
{
  TWND statusbar = 0;
  TWND listbox = 0;
  INT i = 0;
  CHAR buf[TUI_MAX_WNDTEXT+1];
  
  statusbar = TuiGetWndItem(wnd, IDC_STATUSBAR);
  listbox = ctl;
  i = TLB_GetCurSel(listbox);
  if (i >= 0)
  {
    TLB_GetItemText(listbox, i, buf);
    TuiSetWndText(statusbar, buf);
  }
}

VOID mywndproc_onmbxcommand(TWND wnd, UINT cmd, UINT id)
{
  TWND statusbar = 0;
  CHAR buf[TUI_MAX_WNDTEXT+1];
  
  statusbar = TuiGetWndItem(wnd, IDC_STATUSBAR);
  sprintf(buf, "cmd = %d, id = %d", cmd, id);
  TuiSetWndText(statusbar, buf);
}
LONG mywndproc(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  CHAR buf[TUI_MAX_WNDTEXT+1];
  INT i;
  TWND statusbar = 0;
  TWND listbox = 0;
  TWND edit = 0;
  NMHDR* nmhdr = 0;

  switch (msg)
  {
    case TWM_INITDIALOG:
    {
      listbox = TuiGetWndItem(wnd, IDC_LISTBOX1);
      for (i = 0; i < 20; ++i)
      {
        sprintf(buf, "Item %d", (i));
        TLB_AddItem(listbox, buf);
      }
      
      TLB_DeleteItem(listbox, 5);
      TLB_SetCurSel(listbox, 7);

      listbox = TuiGetWndItem(wnd, IDC_LISTBOX2);
      for (i = 0; i < 10; ++i)
      {
        sprintf(buf, "Item %d", (i));
        TLB_AddItem(listbox, buf);
      }
      
      listbox = TuiGetWndItem(wnd, IDC_LISTBOX3);
      for (i = 0; i < 10; ++i)
      {
        sprintf(buf, "Item %d", (i));
        TLB_AddItem(listbox, buf);
      }

      edit = TuiGetWndItem(wnd, IDC_NAME);
      TuiSendMsg(edit, TEM_LIMITTEXT, (WPARAM)20, (LPARAM)0);
      
      statusbar = TuiGetWndItem(wnd, IDC_STATUSBAR);
      TuiSendMsg(statusbar,
        TWM_SETTEXTATTRS, 
        (WPARAM)TuiGetColor(BLUE_YELLOW),
        (LPARAM)0);
      edit = TuiGetWndItem(wnd, IDC_PRICE);
      TuiSendMsg(edit, TEM_SETDECWIDTH, (WPARAM)2, (LPARAM)0);
     /* 
      edit = TuiGetWndItem(wnd, IDC_PASSWORD);
      TEDT_ShowPasswdChar(edit, TW_HIDE);*/
      return TuiDefFrameWndProc(wnd, msg, wparam, lparam);
    }

    case TWM_NOTIFY:
    {
      nmhdr = (NMHDR*)lparam;
      switch (nmhdr->code)
      {
        case TLBN_SELCHANGED:
        {
          mywndproc_onselchanged(wnd, nmhdr->ctl);
          break;
        }
        case (TWM_USER+1):
        {
          /*RESPONSEMSGBOX* res = (RESPONSEMSGBOX*)lparam;*/
          mywndproc_onmbxcommand(wnd, nmhdr->id, nmhdr->code);
          break;
        }
      }
      
      break;
    }
    /*
    case TWM_MBXCOMMAND:
    {
      RESPONSEMSGBOX* res = (RESPONSEMSGBOX*)lparam;
      mywndproc_onmbxcommand(wnd, res->cmd, res->id);
      break;
    }*/
    
    case TWM_COMMAND:
    {
      sprintf(buf, "Pressed: %s", (IDC_OK == wparam ? "OK" : "Cancel"));
      statusbar = TuiGetWndItem(wnd, IDC_STATUSBAR);
      TuiSetWndText(statusbar, buf);

      if (wparam == IDC_OPENDLG2)
      {
        TuiShowMsgBox(wnd,
          TWM_USER+1,
          "Hello World",
          "Welcome to the real world",
          MB_YESNOCANCEL,
          TW_SHOW);
      }
      else if (wparam == IDC_OPENDLG3)
      {
        TWND dlg = TuiCreateWndTempl(dlg3, 0);
        TuiShowWnd(dlg, 1);        
      }
      else if (wparam == IDC_OPENDLG4)
      {/*{ "mylistpagectlproc", "Dialog4", 2,  0,  0, 25, 80, TWS_WINDOW, 0 },*/
        TWND dlg = TuiCreateFrameWndEx(
                      "mylistpagectlproc",
                      "Dialog4",
                      TWS_WINDOW|TWS_VISIBLE,
                      0,
                      0, 0, 25, 80,
                      dlg4, 0);
        TuiShowWnd(dlg, 1);        
      }
      else if (wparam == IDC_CLOSE)
      {
        TuiPostQuitMsg(0);
      }
      
      break;
    }
  }
  return TuiDefFrameWndProc(wnd, msg, wparam, lparam);
}

VOID mylistctlproc_onnotify(TWND wnd, RESPONSEMSGBOX* res)
{

}

VOID mylistctlproc_onnotify2(TWND wnd, RESPONSEMSGBOX* res)
{

}

LONG mylistpagectlproc(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  TWND list = 0;
  NMHDR* nmhdr = 0;

  switch (msg)
  {
    case TWM_INITDIALOG:
    {
      list = TuiGetWndItem(wnd, IDC_LISTBOX1);
      
      TLPC_AddItem(list, "Item #10\t""\t \t\t");
      TLPC_AddItem(list, "Item #10\t""\t \tK Baht\t");
      TLPC_AddItem(list, "Item #10\t""\t \t%%\t");
      TLPC_AddItem(list, "Item #10\t""\t \tMins\t");
      TLPC_AddItem(list, "Item #10\t""\t \t\t");
      TLPC_AddItem(list, "Item #10\t""\t \tK Baht\t");
      TLPC_AddItem(list, "Item #10\t""\t \t%%\t");
      TLPC_AddItem(list, "Item #10\t""\t \tMins\t");
      TLPC_AddItem(list, "Item #10\t""\t \t\t");
      TLPC_AddItem(list, "Item #10\t""\t \tK Baht\t");
      TLPC_AddItem(list, "Item #20\t""\t \t%%\t");
      TLPC_AddItem(list, "Item #20\t""\t \tMins\t");
      TLPC_AddItem(list, "Item #20\t""\t \t\t");
      TLPC_AddItem(list, "Item #20\t""\t \tK Baht\t");
      TLPC_AddItem(list, "Item #20\t""\t \t%%\t");
      TLPC_AddItem(list, "Item #20\t""\t \tMins\t");
      TLPC_AddItem(list, "Item #20\t""\t \t\t");
      TLPC_AddItem(list, "Item #20\t""\t \tK Baht\t");
      TLPC_AddItem(list, "Item #20\t""\t \t%%\t");
      TLPC_AddItem(list, "Item #20\t""\t \tMins\t");
      TLPC_AddItem(list, "Item #30\t""\t \t\t");
      TLPC_AddItem(list, "Item #30\t""\t \tK Baht\t");
      TLPC_AddItem(list, "Item #30\t""\t \t%%\t");
      TLPC_AddItem(list, "Item #30\t""\t \tMins\t");
      TLPC_AddItem(list, "Item #30\t""\t \t\t");
      TLPC_AddItem(list, "Item #30\t""\t \tK Baht\t");
      TLPC_AddItem(list, "Item #30\t""\t \t%%\t");
      TLPC_AddItem(list, "Item #30\t""\t \tMins\t");
      TLPC_AddItem(list, "Item #30\t""\t \t\t");
      TLPC_AddItem(list, "Item #30\t""\t \tK Baht\t");
      TLPC_AddItem(list, "Item #40\t""\t \t%%\t");
      TLPC_AddItem(list, "Item #40\t""\t \tMins\t");
      TLPC_AddItem(list, "Item #40\t""\t \t\t");
      TLPC_AddItem(list, "Item #40\t""\t \tK Baht\t");
      TLPC_AddItem(list, "Item #40\t""\t \t%%\t");
      TLPC_AddItem(list, "Item #40\t""\t \tMins\t");
      TLPC_AddItem(list, "Item #40\t""\t \t\t");
      TLPC_AddItem(list, "Item #40\t""\t \tK Baht\t");
      TLPC_AddItem(list, "Item #40\t""\t \t%%\t");
      TLPC_AddItem(list, "Item #40\t""\t \tMins\t");
      
      return TuiDefFrameWndProc(wnd, msg, wparam, lparam);
    }

    case TWM_COMMAND:
    {
      if (wparam == IDC_CLOSE)
      {
        TuiDestroyWnd(wnd);
      }
      else if (wparam == IDC_MSG)
      {
        TuiShowInputBox(wnd,
          TWM_USER+2,
          "Input Price",
          "Enter Price:",
          MB_OKCANCEL,
          20,
          TES_DECIMAL|TES_AUTOHSCROLL|TES_RIGHT,  
          "0.0",
          TW_SHOW);
      }
      else if (wparam == IDC_EDITCELL)
      {
        TuiShowLineInputBox(wnd,
          TWM_USER+3,
          18, 30,
          "Do you want to save? (Y/N)",
          "Y",
          "YN",
          0,
          TW_SHOW);
      }
      break;
    }
    case TWM_NOTIFY:
    {
      nmhdr = (NMHDR*)lparam;
      switch (nmhdr->code)
      {
        case (TWM_USER+2):
        {
          RESPONSEMSGBOX* res = (RESPONSEMSGBOX*)lparam;
          mylistctlproc_onnotify(wnd, res);
          break;
        }
        case (TWM_USER+3):
        {
          RESPONSEMSGBOX* res = (RESPONSEMSGBOX*)lparam;
          mylistctlproc_onnotify2(wnd, res);
          break;
        }
        case TCN_DISPLAYINFO:
        {
          DISPLAYINFO* di = (DISPLAYINFO*)lparam;
          TWND status = TuiGetWndItem(wnd, IDC_STATUSBAR);
          TuiSetWndText(status, di->text);
          break;
        }
      }
      
      break;
    }

  }
  return TuiDefFrameWndProc(wnd, msg, wparam, lparam);
}

LONG mylistctlproc(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  TWND list = 0;
  /*MSGBOXPARAM param;*/
  SUBITEM item;
  NMHDR* nmhdr = 0;

  switch (msg)
  {
    case TWM_INITDIALOG:
    {
      list = TuiGetWndItem(wnd, IDC_LISTBOX1);
      
      TLC_AddColumnEx(list, "STOCK", 20, ALIGN_LEFT,   TuiGetColor(BLUE_YELLOW), TES_UPPERCASE|TES_AUTOHSCROLL, 0);
      TLC_AddColumnEx(list, "DATE",  16, ALIGN_CENTER, TuiGetColor(BLUE_YELLOW), TES_UPPERCASE|TES_AUTOHSCROLL, 0);
      TLC_AddColumnEx(list, "OPEN",  16, ALIGN_RIGHT,  TuiGetColor(BLUE_YELLOW), TES_DECIMAL|TES_AUTOHSCROLL, 2);
      TLC_AddColumnEx(list, "HIGH",  16, ALIGN_RIGHT,  TuiGetColor(BLUE_YELLOW), TES_DECIMAL|TES_AUTOHSCROLL, 2);
      TLC_AddColumnEx(list, "LOW",   16, ALIGN_RIGHT,  TuiGetColor(BLUE_YELLOW), TES_DECIMAL|TES_AUTOHSCROLL, 2);
      TLC_AddColumnEx(list, "CLOSE", 16, ALIGN_RIGHT,  TuiGetColor(BLUE_YELLOW), TES_DECIMAL|TES_AUTOHSCROLL, 2);

      TLC_AddItem(list, "SCC\t01-01-2018\t560.00\t563.50\t560.00\t562.00\t", 6);
      TLC_AddItem(list, "PTTEP\t01-01-2018\t160.00\t163.50\t160.00\t162.00\t", 6);
      TLC_AddItem(list, "PTTGC\t01-01-2018\t70.00\t73.50\t70.00\t72.00\t", 6);
      TLC_AddItem(list, "PTT\t01-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);

      TLC_AddItem(list, "PTT\t05-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t06-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t07-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t08-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t09-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t10-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t11-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t12-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t13-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t14-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t15-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t16-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t17-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t18-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t19-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t20-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t21-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t22-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t23-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t24-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t25-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t26-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t27-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t28-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t29-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      TLC_AddItem(list, "PTT\t30-01-2018\t60.00\t63.50\t60.00\t62.00\t", 6);
      
      item.col   = 0;
      item.idx   = 0;
      item.attrs = TuiGetColor(GREEN_MAGENTA); 
      TLC_SetItem(list, LCFM_ATTRS, &item);
      
      item.col   = 1;
      item.idx   = 1;
      item.attrs = TuiGetColor(YELLOW_BLUE); 
      TLC_SetItem(list, LCFM_ATTRS, &item);
      
      item.col   = 2;
      item.idx   = 2;
      item.attrs = TuiGetColor(YELLOW_RED); 
      TLC_SetItem(list, LCFM_ATTRS, &item);
      
      item.col   = 3;
      item.idx   = 3;
      item.attrs = TuiGetColor(GREEN_BLUE); 
      TLC_SetItem(list, LCFM_ATTRS, &item);
      
      item.col   = 4;
      item.idx   = 4;
      item.attrs = TuiGetColor(CYAN_MAGENTA); 
      TLC_SetItem(list, LCFM_ATTRS, &item);

      item.col   = 5;
      item.idx   = 14;
      item.attrs = TuiGetColor(CYAN_MAGENTA); 
      TLC_SetItem(list, LCFM_ATTRS, &item);
      
      return TuiDefFrameWndProc(wnd, msg, wparam, lparam);
    }

    case TWM_COMMAND:
    {
      if (wparam == IDC_CLOSE)
      {
        TuiDestroyWnd(wnd);
      }
      else if (wparam == IDC_MSG)
      {
        TuiShowInputBox(wnd,
          TWM_USER+2,
          "Input Price",
          "Enter Price:",
          MB_OKCANCEL,
          20,
          TES_DECIMAL|TES_AUTOHSCROLL|TES_RIGHT,  
          "0.0",
          TW_SHOW);
      }
      else if (wparam == IDC_EDITCELL)
      {
        TuiShowLineInputBox(wnd,
          TWM_USER+3,
          18, 30,
          "Do you want to save? (Y/N)",
          "Y",
          "YN",
          0,
          TW_SHOW);
      }
      break;
    }
    case TWM_NOTIFY:
    {
      nmhdr = (NMHDR*)lparam;
      switch (nmhdr->code)
      {
        case (TWM_USER+2):
        {
          RESPONSEMSGBOX* res = (RESPONSEMSGBOX*)lparam;
          mylistctlproc_onnotify(wnd, res);
          break;
        }
        case (TWM_USER+3):
        {
          RESPONSEMSGBOX* res = (RESPONSEMSGBOX*)lparam;
          mylistctlproc_onnotify2(wnd, res);
          break;
        }
      }
      
      break;
    }

  }
  return TuiDefFrameWndProc(wnd, msg, wparam, lparam);
}

WNDTEMPL framepage[] =
{
  { BUTTON, "Close",    IDC_CLOSE,    14,  41,  1,  11, TWS_CHILD|TWS_VISIBLE, 0 },
  { STATIC, "Esc to exit: ", IDC_STATUSBAR, 24,  1,  1, 79, TWS_CHILD|TWS_VISIBLE|TWS_DISABLED|TSS_LEFT, 0 },
  /* the last is the end-of-controls */
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

#define IDC_PAGE1         10001
#define IDC_PAGE2         10021
#define IDC_PAGE3         10041

WNDTEMPL page1[] =
{
  /* 2nd and others are controls */
  { STATIC, "Name:",    IDC_PAGE1+1,  1,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "12345678901234567890",       IDC_PAGE1+2,  1, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_LEFT|TES_UPPERCASE|TES_AUTOHSCROLL, 0 },
  { STATIC, "ID:",    IDC_PAGE1+3,  2,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "DT66234",     IDC_PAGE1+4,  2, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_CENTER|TES_AUTOHSCROLL, 0 },
  { STATIC, "Tel:",    IDC_PAGE1+5,  3,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "66892030023",   IDC_PAGE1+6,  3, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_NUMBER|TES_RIGHT|TES_AUTOHSCROLL, 0 },
  /* the last is the end-of-controls */
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};


WNDTEMPL page2[] =
{
  /* 2nd and others are controls */
  { STATIC, "Email:",    IDC_PAGE2+1,  4,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "abc@abc.com", IDC_PAGE2+2,  4, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|TWS_DISABLED|
      TES_AUTOHSCROLL, 0 },
  { STATIC, "Password:",    IDC_PAGE2+3,  5,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "welcome1!", IDC_PAGE2+4,  5, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_LEFT|TES_PASSWORD|TES_AUTOHSCROLL, 0 },
  { STATIC, "Price:",    IDC_PAGE2+5,  6,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "132,399.50", IDC_PAGE2+6,  6, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_RIGHT|TES_AUTODECIMALCOMMA|TES_AUTOHSCROLL, 0 },
  /* the last is the end-of-controls */
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};


WNDTEMPL page3[] =
{
  /* 2nd and others are controls */
  { STATIC, "ID:",    IDC_PAGE3+1,  2,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "DT66234",     IDC_PAGE3+2,  2, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_CENTER|TES_AUTOHSCROLL, 0 },
  { STATIC, "Email:",    IDC_PAGE3+3,  4,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "abc@abc.com", IDC_PAGE3+4,  4, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|TWS_DISABLED|
      TES_AUTOHSCROLL, 0 },
  { STATIC, "Password:",    IDC_PAGE3+5,  5,  1,  1,  16, TWS_CHILD|TWS_VISIBLE|TSS_RIGHT, 0 },
  { EDITBOX,   "welcome1!", IDC_PAGE3+6,  5, 18,  1, 16, 
    TWS_CHILD|TWS_VISIBLE|
      TES_LEFT|TES_PASSWORD|TES_AUTOHSCROLL, 0 },
  /* the last is the end-of-controls */
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};



LONG myframepageproc(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  LONG rc = TUI_CONTINUE;

  switch (msg)
  {
    case TWM_INITDIALOG:
    {
      rc = TuiDefFrameWndProc(wnd, msg, wparam, lparam);
      if (TUI_CONTINUE == rc)
      {
        rc = TuiFrameWndCreatePage(wnd, page1, 0);
      }
      if (TUI_CONTINUE == rc)
      {
        rc = TuiFrameWndCreatePage(wnd, page2, 0);
      }
      if (TUI_CONTINUE == rc)
      {
        rc = TuiFrameWndCreatePage(wnd, page3, 0);
      }
      return rc;
    }

    case TWM_COMMAND:
    {
      if (wparam == IDC_CLOSE)
      {
        TuiPostQuitMsg(0);
      }
      break;
    }
  }
  return TuiDefFrameWndProc(wnd, msg, wparam, lparam);
}

int main(int argc, char* argv[])
{
  MSG msg;
  TWND wnd;

  TuiStartup();

  TuiSetNextMove(TVK_TAB);
  TuiRegisterCls("mywndproc", mywndproc);
  TuiRegisterCls("mylistctlproc", mylistctlproc);
  TuiRegisterCls("myframepageproc", myframepageproc);
  TuiRegisterCls("mylistpagectlproc", mylistpagectlproc);

  wnd = TuiCreateWndTempl(frame1, 0);
  
  /*
  wnd = TuiCreateFrameWnd(
          "myframepageproc",
          "frame window",
          TWS_WINDOW|TWS_VISIBLE,
          0, 0, 25, 80,
          framepage, 0);
          */
  if (!wnd)
  {
    TuiShutdown();
    return -1;
  }
  TuiShowWnd(wnd, 1);
  TuiSendMsg(wnd, TWM_SETCURPAGE, 0, 0);

  while (TuiGetMsg(&msg))
  {
    TuiDispatchMsg(&msg);
    TuiTranslateMsg(&msg);
  }
  TuiShutdown();
  return 0;
}


