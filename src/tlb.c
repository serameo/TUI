/*-------------------------------------------------------------------
 * File name: tlb.c
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
 * LISTBOX functions
 *-----------------------------------------------------------------*/

struct _TLISTBOXITEMSTRUCT
{
  CHAR      itemtext[TUI_MAX_WNDTEXT+1];
  LPVOID    data;
  INT       checked;
  struct _TLISTBOXITEMSTRUCT *prev;
  struct _TLISTBOXITEMSTRUCT *next;
};
typedef struct _TLISTBOXITEMSTRUCT tlistbox_t;

struct _TUILISTBOXSTRUCT
{
  INT           firstvisible;
  INT           nitems;        /* item counts */
  INT           cursel;       /* highlight item */

  tlistbox_t*   firstitem;    /* always be item id = 0 if it is existing */
  tlistbox_t*   lastitem;

  tlistbox_t*   selitem;      /* must be the same cursel item */
  tlistbox_t*   firstvisibleitem;

  INT           checkeditems;    /* count item checked */
  tlistbox_t*   lastitemchecked; /* to identify the last item checked */
};
typedef struct _TUILISTBOXSTRUCT _TLISTBOX;
typedef struct _TUILISTBOXSTRUCT *TLISTBOX;

tlistbox_t* _TLB_FindItemByIndex(TWND wnd, INT idx);
VOID _TLB_OnSelChanged(TWND wnd);

INT TLB_OnCountItemCheck(TWND wnd);
INT TLB_OnGetItemChecked(TWND wnd, INT idx);
INT TLB_OnSetItemChecked(TWND wnd, INT idx, INT check);
LPVOID TLB_OnGetItemData(TWND wnd, INT idx);
VOID TLB_OnSetItemData(TWND wnd, INT idx, LPVOID data);
VOID TLB_OnSetCurSel(TWND wnd, INT idx);
LONG TLB_OnGetItemCount(TWND wnd);
VOID TLB_OnSetItemText(TWND wnd, INT idx, LPSTR text);
LONG TLB_OnGetItemText(TWND wnd, INT idx, LPSTR text);
INT TLB_OnGetCurSel(TWND wnd);
VOID TLB_OnDeleteAllItems(TWND wnd);
VOID TLB_OnDeleteItem(TWND wnd, LONG idx);
LONG TLB_OnAddItem(TWND wnd, LPCSTR text);
VOID TLB_OnPaint(TWND wnd, TDC dc);
VOID TLB_OnKeyDown(TWND wnd, LONG ch);
LONG TLB_OnKillFocus(TWND wnd);
VOID TLB_OnSetFocus(TWND wnd);
VOID TLB_OnDestroy(TWND wnd);
LONG TLB_OnCreate(TWND wnd);
LONG LISTBOXPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);


tlistbox_t* _TLB_FindItemByIndex(TWND wnd, INT idx)
{
  TLISTBOX lb = 0;
  INT i = 0;
  tlistbox_t* item = 0;
  
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  if (idx < 0 || idx >= lb->nitems)
  {
    return 0;
  }
  item = lb->firstitem;
  
  for (i = 0; i < lb->nitems && item; ++i, item = item->next)
  {
    if (i == idx)
    {
      return item;
    }
  }
  return 0;
}

LONG TLB_OnCreate(TWND wnd)
{
  TLISTBOX lb = 0;
  /* initial memory for static control */
  lb = (TLISTBOX)malloc(sizeof(_TLISTBOX));
  if (!lb)
  {
    return TUI_ERROR;
  }
  lb->firstvisible      = -1;
  lb->cursel            = -1;
  lb->nitems            = 0;
  lb->firstitem         = 0;
  lb->lastitem          = 0;
  lb->selitem           = 0;
  lb->firstvisibleitem  = 0;
  lb->checkeditems      = 0;
  lb->lastitemchecked   = 0;
  
  TuiSetWndParam(wnd, (LPVOID)lb);
  TuiSetWndTextAttrs(wnd, TuiGetSysColor(COLOR_LBXTEXT));
  
  return TUI_CONTINUE;
}

VOID TLB_OnDestroy(TWND wnd)
{
  TLISTBOX lb = 0;
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  
  /* release memory of static control */
  TLB_DeleteAllItems(wnd);
  
  free(lb);
}

VOID TLB_OnSetFocus(TWND wnd)
{
  TLISTBOX lb = 0;
  NMHDR nmhdr;
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  if (lb->cursel >= lb->nitems)
  {
    lb->cursel = 0;
  }
  else if (lb->cursel < 0)
  {
    lb->cursel = 0;
  }
  TuiInvalidateWnd(wnd);
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLBN_SETFOCUS;
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}

LONG TLB_OnKillFocus(TWND wnd)
{
  NMHDR nmhdr;
  LONG rc = TUI_CONTINUE;
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLBN_KILLFOCUS;
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
  return rc;
}

VOID _TLB_OnSelChanged(TWND wnd)
{
  NMHDR nmhdr;
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLBN_SELCHANGED;
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}

VOID TLB_OnKeyDown(TWND wnd, LONG ch)
{
  TLISTBOX lb = 0;
  INT repaint = 0;
  DWORD style = TuiGetWndStyle(wnd);
  INT lines = 0;
  RECT rc;
  
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  TuiGetWndRect(wnd, &rc);
  switch (ch)
  {
    case TVK_SPACE:
    {
      if ((style & TLBS_CHECKBOX) ||
          (style & TLBS_RADIOBOX))
      {
        /* get current and check it */
        TLB_SetItemChecked(wnd, lb->cursel, LB_CHECKED);
      }
      ++repaint;
      break;
    }

    case KEY_DOWN:
    case KEY_RIGHT:
    {
      ++lines;
      ++repaint;
      break;
    }
  
    case KEY_UP:
    case KEY_LEFT:
    {
      --lines;
      ++repaint;
      break;
    }
      
    case TVK_PRIOR:
#ifdef __USE_CURSES__
    case KEY_PPAGE:
#elif (defined __USE_QIO__ && defined __VMS__)
    case KEY_PREV:
#endif
    {
      lines -= rc.lines;
      ++repaint;
      break;
    }
    
    case TVK_NEXT:
#ifdef __USE_CURSES__
    case KEY_NPAGE:
#elif (defined __USE_QIO__ && defined __VMS__)
    case KEY_NEXT:
#endif
    {
      lines += rc.lines;
      ++repaint;
      break;
    }
  }
  if (repaint)
  {
    lb->cursel += lines;
    if (lb->cursel >= lb->nitems)
    {
      lb->cursel = lb->nitems - 1;
    }
    else if (lb->cursel < 0)
    {
      lb->cursel = 0;
    }
    /* find the new first visible */
    if (lb->cursel >= lb->firstvisible + rc.lines)
    {
      lb->firstvisible += lines;
      if (lb->firstvisible > lb->nitems - rc.lines)
      {
        lb->firstvisible = lb->nitems - rc.lines;
      }
    }
    else if (lb->firstvisible > lb->cursel)
    {
      lb->firstvisible += lines;
      if (lb->firstvisible < 0)
      {
        lb->firstvisible = 0;
      }
    }
    
    TuiInvalidateWnd(wnd);
    /* send notification */
    _TLB_OnSelChanged(wnd);
  }
  lb->selitem = _TLB_FindItemByIndex(wnd, lb->cursel);
  lb->firstvisibleitem = _TLB_FindItemByIndex(wnd, lb->firstvisible);
}

VOID TLB_OnPaint(TWND wnd, TDC dc)
{
  TLISTBOX lb = 0;
  INT i = 0;
  tlistbox_t* item = 0;
  CHAR  buf[TUI_MAX_WNDTEXT+1];
  CHAR  text[TUI_MAX_WNDTEXT+1];
  DWORD attrs = TuiGetWndTextAttrs(wnd);
  DRAWITEM di;
  RECT rc;
  DWORD style = TuiGetWndStyle(wnd);
  INT lines = 0;
  
  if (!TuiIsWndVisible(wnd))
  {
    return;
  }
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  
  /* draw */
  if (lb->nitems > 0)
  {
    item = lb->firstitem;
    TuiGetWndRect(wnd, &rc);
    lines = rc.lines;
/*
    if (style & TWS_BORDER)
    {
      lines -= 2;
      rc.y++;
      rc.x++;
      rc.cols--;
    }
*/

    for (i = 0; i < lb->nitems && item; ++i, item = item->next)
    {
      if (i < lb->firstvisible)
      {
        /* do nothing */
        continue;
      }
      else if (i - lb->firstvisible < lines)
      {
        if (style & TLBS_OWNERDRAW)
        {
          memset(&di, 0, sizeof(DRAWITEM));
          di.rcitem.y = rc.y + (i - lb->firstvisible);
          di.rcitem.x = rc.x;
          di.rcitem.lines = 1;
          di.rcitem.cols  = rc.cols;
          di.idx          = i;
          
          TuiSendMsg(TuiGetParent(wnd), TWM_DRAWITEM, (WPARAM)i, (LPARAM)&di);
        }
        else
        {
          strcpy(text, "");
          if (style & TLBS_CHECKBOX)
          {
            if (item->checked)
            {
              strcpy(text, "[X] ");
            }
            else
            {
              strcpy(text, "[ ] ");
            }
          }
          else if (style & TLBS_RADIOBOX)
          {
            if (item->checked)
            {
              strcpy(text, "(*) ");
            }
            else
            {
              strcpy(text, "( ) ");
            }
          }          
          
          /* copy from item text */
          strcat(text, item->itemtext);
          TuiPrintTextAlignment(buf, text, rc.cols, style);
          
          TuiDrawText(dc, 
            rc.y+(i-lb->firstvisible), 
            rc.x, 
            buf, 
            (i == lb->cursel ? TuiReverseColor(attrs) : attrs));
        }
      }/* not owner draw */
    } /* for each item */
  } /* items are valid */
}

LONG TLB_OnAddItem(TWND wnd, LPCSTR text)
{
  TLISTBOX lb = 0;
  LONG len = 0;
  tlistbox_t* item = 0;
  
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  item = (tlistbox_t*)malloc(sizeof(tlistbox_t));
  if (item)
  {
    memset(item, 0, sizeof(tlistbox_t));
    len = strlen(text);
    if (len > TUI_MAX_WNDTEXT)
    {
      len = TUI_MAX_WNDTEXT;
    }
    strncpy(item->itemtext, text, len);
  
    if (lb->firstitem)
    {
      item->prev = lb->lastitem;
      lb->lastitem->next = item;
      lb->lastitem = item;
    }
    else
    {
      lb->firstitem = lb->lastitem = item;
      lb->firstvisible = 0;
    }
    ++lb->nitems;
    return lb->nitems;
  }
  return TUI_MEM;
}

VOID TLB_OnDeleteItem(TWND wnd, LONG idx)
{
  TLISTBOX lb = 0;
  tlistbox_t* item = 0;
  tlistbox_t* nextitem = 0;
  
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  item = _TLB_FindItemByIndex(wnd, idx);
  
  if (item)
  {
    /* remove the links */
    nextitem = item->next;
    if (nextitem)
    {
      nextitem->prev = item->prev;
    }
    if (item->prev)
    {
      item->prev->next = nextitem;
    }
    /* check if removed item is the first item or the last item */
    if (item == lb->firstitem)
    {
      lb->firstitem = nextitem;
    }
    else if (item == lb->lastitem)
    {
      lb->lastitem = item->prev;
    }
    
    /* free the memory */
    item->next = item->prev = 0;
    free(item);
    
    /* decrement items */
    --lb->nitems;
    if (lb->cursel >= lb->nitems)
    {
      lb->cursel = lb->nitems - 1;
    }
  }
}

VOID TLB_OnDeleteAllItems(TWND wnd)
{
  TLISTBOX lb = 0;
  INT nitems = 0;
  INT i = 0;
  
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  nitems = lb->nitems;
  for (i = 0; i < nitems; ++i)
  {
    TuiPostMsg(wnd, TLBM_DELETEITEM, 0, 0);
  }
}

INT TLB_OnGetCurSel(TWND wnd)
{
  TLISTBOX lb = 0;
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  return lb->cursel;
}

LONG TLB_OnGetItemText(TWND wnd, INT idx, LPSTR text)
{
  tlistbox_t* item = 0;
  
  item = _TLB_FindItemByIndex(wnd, idx);
  
  if (item)
  {
    strcpy(text, item->itemtext);
  }
  return strlen(text);
}

VOID TLB_OnSetItemText(TWND wnd, INT idx, LPSTR text)
{
  tlistbox_t* item = 0;
  
  item = _TLB_FindItemByIndex(wnd, idx);  
  if (item)
  {
    strcpy(item->itemtext, text);
  }
}

LONG TLB_OnGetItemCount(TWND wnd)
{
  TLISTBOX lb = 0;
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  return lb->nitems;
}

VOID TLB_OnSetCurSel(TWND wnd, INT idx)
{
  TLISTBOX lb = 0;
  RECT rc;
  
  lb = (TLISTBOX)TuiGetWndParam(wnd);

  if (idx < 0 || idx >= lb->nitems)
  {
    idx = -1;
  }
  lb->cursel = idx;
  
  if (lb->cursel >= 0)
  {
    lb->firstvisible = lb->cursel;
    TuiGetWndRect(wnd, &rc);
    if (lb->firstvisible + rc.lines > lb->nitems)
    {
      lb->firstvisible = lb->nitems - rc.lines;
    }
  }
  else
  {
    lb->firstvisible = 0;
  }
  
  TuiInvalidateWnd(wnd);
  /* send notification */
  _TLB_OnSelChanged(wnd);

  lb->selitem = _TLB_FindItemByIndex(wnd, lb->cursel);
  lb->firstvisibleitem = _TLB_FindItemByIndex(wnd, lb->firstvisible);
}

VOID TLB_OnSetItemData(TWND wnd, INT idx, LPVOID data)
{
  tlistbox_t* item = 0;
  
  item = _TLB_FindItemByIndex(wnd, idx);  
  if (item)
  {
    item->data = data;
  }
}

LPVOID TLB_OnGetItemData(TWND wnd, INT idx)
{
  tlistbox_t* item = 0;
  
  item = _TLB_FindItemByIndex(wnd, idx);  
  if (item)
  {
    return item->data;
  }
  return 0;
}

INT TLB_OnSetItemChecked(TWND wnd, INT idx, INT check)
{
  tlistbox_t* item = 0;
  TLISTBOX lb = 0;
  DWORD style = TuiGetWndStyle(wnd);
  
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  item = _TLB_FindItemByIndex(wnd, idx);  
  /* found and do the check item state */
  if (item)
  {
    if (check == LB_UNCHECKED)
    {
      --lb->checkeditems;
      if (lb->checkeditems < 0)
      {
        lb->checkeditems    = 0;
        lb->lastitemchecked = 0;
      }
      item->checked = LB_UNCHECKED;
    }
    else
    {
      /* unselected the last if it is radio style */
      if (style & TLBS_RADIOBOX)
      {
        if (lb->lastitemchecked)
        {
          lb->lastitemchecked->checked = LB_UNCHECKED;
        }
      }
      else if (style & TLBS_CHECKBOX)
      {
        if (item->checked == LB_CHECKED)
        {
          --lb->checkeditems;
          if (lb->checkeditems < 0)
          {
            lb->checkeditems    = 0;
            lb->lastitemchecked = 0;
          }
          item->checked = LB_UNCHECKED;
          return LB_OK;
        }
      }
      /* count checked item */
      ++lb->checkeditems;
      if (lb->checkeditems > lb->nitems)
      {
        lb->checkeditems = lb->nitems;
      }
      /* checked and save the last checked item */
      item->checked = LB_CHECKED;
      lb->lastitemchecked = item;
    }
    return LB_OK;
  }
  return LB_ERROR;
}

INT TLB_OnGetItemChecked(TWND wnd, INT idx)
{
  tlistbox_t* item = 0;
  
  item = _TLB_FindItemByIndex(wnd, idx);  
  if (item)
  {
    return item->checked;
  }
  return LB_ERROR;
}

INT TLB_OnCountItemCheck(TWND wnd)
{
  TLISTBOX lb = 0;
  
  lb = (TLISTBOX)TuiGetWndParam(wnd);
  return lb->checkeditems;
}

LONG LISTBOXPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TLBM_COUNTITEMCHECKED:
    {
      return TLB_OnCountItemCheck(wnd);
    }
    case TLBM_GETITEMCHECKED:
    {
      return TLB_OnGetItemChecked(wnd, (INT)wparam);
    }
    case TLBM_SETITEMCHECKED:
    {
      return TLB_OnSetItemChecked(wnd, (INT)wparam, (INT)lparam);
    }
    case TWM_CREATE:
    {
      return TLB_OnCreate(wnd);
    }
    case TWM_DESTROY:
    {
      TLB_OnDestroy(wnd);
      return 0;
    }
    case TWM_SETFOCUS:
    {
      TLB_OnSetFocus(wnd);
      break;
    }
    case TWM_KILLFOCUS:
    {
      return TLB_OnKillFocus(wnd);
    }
    case TWM_KEYDOWN:
    {
      TLB_OnKeyDown(wnd, (LONG)wparam);
      break;
    }
    case TWM_PAINT:
    {
      TLB_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
    case TLBM_ADDITEM:
    {
      return TLB_OnAddItem(wnd, (LPCSTR)lparam);
    }
    case TLBM_DELETEITEM:
    {
      TLB_OnDeleteItem(wnd, (LONG)wparam);
      return 0;
    }
    case TLBM_GETCURSEL:
    {
      return TLB_OnGetCurSel(wnd);
    }
    case TLBM_SETCURSEL:
    {
      TLB_OnSetCurSel(wnd, (INT)wparam);
      return 0;
    }
    case TLBM_DELETEALLITEMS:
    {
      TLB_OnDeleteAllItems(wnd);
      return 0;
    }
    case TLBM_GETITEMCOUNT:
    {
      return TLB_OnGetItemCount(wnd);
    }
    case TLBM_SETITEMDATA:
    {
      TLB_OnSetItemData(wnd, (INT)wparam, (LPVOID)lparam);
      return 0;
    }
    case TLBM_GETITEMDATA:
    {
      return (LONG)TLB_OnGetItemData(wnd, (INT)wparam);
    }
    case TLBM_SETITEMTEXT:
    {
      TLB_OnSetItemText(wnd, (INT)wparam, (LPSTR)lparam);
      return 0;
    }
    case TLBM_GETITEMTEXT:
    {
      return TLB_OnGetItemText(wnd, (INT)wparam, (LPSTR)lparam);
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}

