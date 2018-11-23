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
  TTCHAR      itemtext[TUI_MAX_WNDTEXT+1];
  TLPVOID    data;
  TINT       checked;
  struct _TLISTBOXITEMSTRUCT *prev;
  struct _TLISTBOXITEMSTRUCT *next;
};
typedef struct _TLISTBOXITEMSTRUCT tlistbox_t;

struct _TUILISTBOXSTRUCT
{
  TINT           firstvisible;
  TINT           nitems;        /* item counts */
  TINT           cursel;       /* highlight item */

  tlistbox_t*   firstitem;    /* always be item id = 0 if it is existing */
  tlistbox_t*   lastitem;

  tlistbox_t*   selitem;      /* must be the same cursel item */
  tlistbox_t*   firstvisibleitem;

  TINT           checkeditems;    /* count item checked */
  tlistbox_t*   lastitemchecked; /* to identify the last item checked */
  
  TVOID*         exparam;
};
typedef struct _TUILISTBOXSTRUCT _TLISTBOX;
typedef struct _TUILISTBOXSTRUCT *PTLISTBOX;

tlistbox_t* _TLB_FindItemByIndex(TWND wnd, TINT idx);
TVOID   _TLB_OnSelChanged(TWND wnd);

TINT    _TLB_OnCountItemCheck(TWND wnd);
TINT    _TLB_OnGetItemChecked(TWND wnd, TINT idx);
TINT    _TLB_OnSetItemChecked(TWND wnd, TINT idx, TINT check);
TLPVOID _TLB_OnGetItemData(TWND wnd, TINT idx);
TVOID   _TLB_OnSetItemData(TWND wnd, TINT idx, TLPVOID data);
TVOID   _TLB_OnSetCurSel(TWND wnd, TINT idx);
TLONG   _TLB_OnGetItemCount(TWND wnd);
TVOID   _TLB_OnSetItemText(TWND wnd, TINT idx, TLPSTR text);
TLONG   _TLB_OnGetItemText(TWND wnd, TINT idx, TLPSTR text);
TINT    _TLB_OnGetCurSel(TWND wnd);
TVOID   _TLB_OnDeleteAllItems(TWND wnd);
TVOID   _TLB_OnDeleteItem(TWND wnd, TLONG idx);
TLONG   _TLB_OnAddItem(TWND wnd, TLPCSTR text);
TVOID   _TLB_OnPaint(TWND wnd, TDC dc);
TVOID   _TLB_OnKeyDown(TWND wnd, TLONG ch);
TLONG   _TLB_OnKillFocus(TWND wnd);
TVOID   _TLB_OnSetFocus(TWND wnd);
TVOID   _TLB_OnDestroy(TWND wnd);
TLONG   _TLB_OnCreate(TWND wnd);
TLONG   LISTBOXPROC(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam);


tlistbox_t* _TLB_FindItemByIndex(TWND wnd, TINT idx)
{
  PTLISTBOX lb = 0;
  TINT i = 0;
  tlistbox_t* item = 0;
  
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
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

TLONG _TLB_OnCreate(TWND wnd)
{
  PTLISTBOX lb = 0;
  /* initial memory for static control */
  lb = (PTLISTBOX)malloc(sizeof(_TLISTBOX));
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
  
  TuiSetWndParam(wnd, (TLPVOID)lb);
  /*TuiSetWndTextAttrs(wnd, TuiGetSysColor(COLOR_LBXTEXT));*/
  
  return TUI_CONTINUE;
}

TVOID _TLB_OnDestroy(TWND wnd)
{
  PTLISTBOX lb = 0;
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
  
  /* release memory of static control */
  TLB_DeleteAllItems(wnd);
  
  free(lb);
}

TVOID _TLB_OnSetFocus(TWND wnd)
{
  PTLISTBOX lb = 0;
  TNMHDR nmhdr;
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
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
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (TLPARAM)&nmhdr);
}

TLONG _TLB_OnKillFocus(TWND wnd)
{
  TNMHDR nmhdr;
  TLONG rc = TUI_CONTINUE;
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLBN_KILLFOCUS;
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (TLPARAM)&nmhdr);
  return rc;
}

TVOID _TLB_OnSelChanged(TWND wnd)
{
  TNMHDR nmhdr;
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLBN_SELCHANGED;
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (TLPARAM)&nmhdr);
}

TVOID _TLB_OnKeyDown(TWND wnd, TLONG ch)
{
  PTLISTBOX lb = 0;
  TINT repaint = 0;
  TDWORD style = TuiGetWndStyle(wnd);
  TINT lines = 0;
  TRECT rc;
  
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
  TuiGetWndRect(wnd, &rc);
  switch (ch)
  {
    case TVK_SPACE:
    {
      if ((style & TLBS_CHECKBOX) ||
          (style & TLBS_RADIOBOX))
      {
        /* get current and check it */
        TLB_SetItemChecked(wnd, lb->cursel, TLB_CHECKED);
      }
      ++repaint;
      break;
    }
#if defined __USE_CURSES__
    case KEY_DOWN:
#elif defined __USE_WIN32__
    case TVK_DOWN:
#endif
/*    case KEY_RIGHT:*/
    {
      ++lines;
      ++repaint;
      break;
    }
  
#if defined __USE_CURSES__
    case KEY_UP:
#elif defined __USE_WIN32__
    case TVK_UP:
#endif
/*    case KEY_LEFT:*/
    {
      --lines;
      ++repaint;
      break;
    }
      
#ifdef __USE_CURSES__
    case KEY_PPAGE:
#elif (defined __USE_QIO__ && defined __VMS__)
    case KEY_PREV:
#elif defined __USE_WIN32__
    case TVK_PRIOR:
#endif
    {
      lines -= rc.lines;
      ++repaint;
      break;
    }
    
#ifdef __USE_CURSES__
    case KEY_NPAGE:
#elif (defined __USE_QIO__ && defined __VMS__)
    case KEY_NEXT:
#elif defined __USE_WIN32__
    case TVK_NEXT:
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

TVOID _TLB_OnPaint(TWND wnd, TDC dc)
{
  PTLISTBOX lb = 0;
  TINT i = 0;
  tlistbox_t* item = 0;
  TTCHAR  buf[TUI_MAX_WNDTEXT+1];
  TTCHAR  text[TUI_MAX_WNDTEXT+1];
  TDWORD attrs = TuiGetWndTextAttrs(wnd);
  TDRAWITEM di;
  TRECT rc;
  TDWORD style = TuiGetWndStyle(wnd);
  TINT lines = 0;
  TINT y, x; /* to move cursor */
  
  if (!TuiIsWndVisible(wnd))
  {
    return;
  }
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
  
  /* draw */
  if (lb->nitems > 0)
  {
    item = lb->firstitem;
    TuiGetWndRect(wnd, &rc);
    lines = rc.lines;

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
          memset(&di, 0, sizeof(TDRAWITEM));
          di.rcitem.y = rc.y + (i - lb->firstvisible);
          di.rcitem.x = rc.x;
          di.rcitem.lines = 1;
          di.rcitem.cols  = rc.cols;
          di.idx          = i;
          
          TuiSendMsg(TuiGetParent(wnd), TWM_DRAWITEM, (TWPARAM)i, (TLPARAM)&di);
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
          
          if (i == lb->cursel)
          {
            y = rc.y + (i - lb->firstvisible);
            x = rc.x;
#if defined __USE_CURSES__
            TuiDrawText(dc, 
              rc.y+(i-lb->firstvisible), 
              rc.x, 
              buf, 
              TuiReverseColor(attrs));
#elif defined __USE_WIN32__
            TuiDrawText(dc,
                rc.y + (i - lb->firstvisible),
                rc.x,
                buf,
                TuiGetSysColor(COLOR_HIGHLIGHTED));
#endif
          }
          else
          {
#if defined __USE_CURSES__
              TuiDrawText(dc,
              rc.y+(i-lb->firstvisible), 
              rc.x, 
              buf, 
              attrs);
#elif defined __USE_WIN32__
              TuiDrawText(dc,
                  rc.y + (i - lb->firstvisible),
                  rc.x,
                  buf,
                  TuiGetSysColor(COLOR_LBXTEXT));
#endif
          }
        }
      }/* not owner draw */
    } /* for each item */
    TuiMoveYX(dc, y, x);
  } /* items are valid */
}

TLONG _TLB_OnAddItem(TWND wnd, TLPCSTR text)
{
  PTLISTBOX lb = 0;
  TLONG len = 0;
  tlistbox_t* item = 0;
  
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
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

TVOID _TLB_OnDeleteItem(TWND wnd, TLONG idx)
{
  PTLISTBOX lb = 0;
  tlistbox_t* item = 0;
  tlistbox_t* nextitem = 0;
  
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
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

TVOID _TLB_OnDeleteAllItems(TWND wnd)
{
  PTLISTBOX lb = 0;
  TINT nitems = 0;
  TINT i = 0;
  
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
  nitems = lb->nitems;
  for (i = 0; i < nitems; ++i)
  {
    TuiPostMsg(wnd, TLBM_DELETEITEM, 0, 0);
  }
}

TINT _TLB_OnGetCurSel(TWND wnd)
{
  PTLISTBOX lb = 0;
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
  return lb->cursel;
}

TLONG _TLB_OnGetItemText(TWND wnd, TINT idx, TLPSTR text)
{
  tlistbox_t* item = 0;
  
  item = _TLB_FindItemByIndex(wnd, idx);
  
  if (item)
  {
    strcpy(text, item->itemtext);
  }
  return strlen(text);
}

TVOID _TLB_OnSetItemText(TWND wnd, TINT idx, TLPSTR text)
{
  tlistbox_t* item = 0;
  
  item = _TLB_FindItemByIndex(wnd, idx);  
  if (item)
  {
    strcpy(item->itemtext, text);
  }
}

TLONG _TLB_OnGetItemCount(TWND wnd)
{
  PTLISTBOX lb = 0;
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
  return lb->nitems;
}

TVOID _TLB_OnSetCurSel(TWND wnd, TINT idx)
{
  PTLISTBOX lb = 0;
  TRECT rc;
  
  lb = (PTLISTBOX)TuiGetWndParam(wnd);

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

TVOID _TLB_OnSetItemData(TWND wnd, TINT idx, TLPVOID data)
{
  tlistbox_t* item = 0;
  
  item = _TLB_FindItemByIndex(wnd, idx);  
  if (item)
  {
    item->data = data;
  }
}

TLPVOID _TLB_OnGetItemData(TWND wnd, TINT idx)
{
  tlistbox_t* item = 0;
  
  item = _TLB_FindItemByIndex(wnd, idx);  
  if (item)
  {
    return item->data;
  }
  return 0;
}

TINT _TLB_OnSetItemChecked(TWND wnd, TINT idx, TINT check)
{
  tlistbox_t* item = 0;
  PTLISTBOX lb = 0;
  TDWORD style = TuiGetWndStyle(wnd);
  
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
  item = _TLB_FindItemByIndex(wnd, idx);  
  /* found and do the check item state */
  if (item)
  {
    if (check == TLB_UNCHECKED)
    {
      --lb->checkeditems;
      if (lb->checkeditems < 0)
      {
        lb->checkeditems    = 0;
        lb->lastitemchecked = 0;
      }
      item->checked = TLB_UNCHECKED;
    }
    else
    {
      /* unselected the last if it is radio style */
      if (style & TLBS_RADIOBOX)
      {
        if (lb->lastitemchecked)
        {
          lb->lastitemchecked->checked = TLB_UNCHECKED;
        }
      }
      else if (style & TLBS_CHECKBOX)
      {
        if (item->checked == TLB_CHECKED)
        {
          --lb->checkeditems;
          if (lb->checkeditems < 0)
          {
            lb->checkeditems    = 0;
            lb->lastitemchecked = 0;
          }
          item->checked = TLB_UNCHECKED;
          return TLB_OK;
        }
      }
      /* count checked item */
      ++lb->checkeditems;
      if (lb->checkeditems > lb->nitems)
      {
        lb->checkeditems = lb->nitems;
      }
      /* checked and save the last checked item */
      item->checked = TLB_CHECKED;
      lb->lastitemchecked = item;
    }
    return TLB_OK;
  }
  return TLB_ERROR;
}

TINT _TLB_OnGetItemChecked(TWND wnd, TINT idx)
{
  tlistbox_t* item = 0;
  
  item = _TLB_FindItemByIndex(wnd, idx);  
  if (item)
  {
    return item->checked;
  }
  return TLB_ERROR;
}

TINT _TLB_OnCountItemCheck(TWND wnd)
{
  PTLISTBOX lb = 0;
  
  lb = (PTLISTBOX)TuiGetWndParam(wnd);
  return lb->checkeditems;
}

TLONG LISTBOXPROC(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam)
{
  switch (msg)
  {
    case TLBM_COUNTITEMCHECKED:
    {
      return _TLB_OnCountItemCheck(wnd);
    }
    case TLBM_GETITEMCHECKED:
    {
      return _TLB_OnGetItemChecked(wnd, (TINT)wparam);
    }
    case TLBM_SETITEMCHECKED:
    {
      return _TLB_OnSetItemChecked(wnd, (TINT)wparam, (TINT)lparam);
    }
    case TWM_CREATE:
    {
      return _TLB_OnCreate(wnd);
    }
    case TWM_DESTROY:
    {
      _TLB_OnDestroy(wnd);
      return 0;
    }
    case TWM_SETFOCUS:
    {
      _TLB_OnSetFocus(wnd);
      break;
    }
    case TWM_KILLFOCUS:
    {
      return _TLB_OnKillFocus(wnd);
    }
    case TWM_KEYDOWN:
    {
      _TLB_OnKeyDown(wnd, (TLONG)wparam);
      break;
    }
    case TWM_PAINT:
    {
      _TLB_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
    case TLBM_ADDITEM:
    {
      return _TLB_OnAddItem(wnd, (TLPCSTR)lparam);
    }
    case TLBM_DELETEITEM:
    {
      _TLB_OnDeleteItem(wnd, (TLONG)wparam);
      return 0;
    }
    case TLBM_GETCURSEL:
    {
      return _TLB_OnGetCurSel(wnd);
    }
    case TLBM_SETCURSEL:
    {
      _TLB_OnSetCurSel(wnd, (TINT)wparam);
      return 0;
    }
    case TLBM_DELETEALLITEMS:
    {
      _TLB_OnDeleteAllItems(wnd);
      return 0;
    }
    case TLBM_GETITEMCOUNT:
    {
      return _TLB_OnGetItemCount(wnd);
    }
    case TLBM_SETITEMDATA:
    {
      _TLB_OnSetItemData(wnd, (TINT)wparam, (TLPVOID)lparam);
      return 0;
    }
    case TLBM_GETITEMDATA:
    {
      return (TLONG)_TLB_OnGetItemData(wnd, (TINT)wparam);
    }
    case TLBM_SETITEMTEXT:
    {
      _TLB_OnSetItemText(wnd, (TINT)wparam, (TLPSTR)lparam);
      return 0;
    }
    case TLBM_GETITEMTEXT:
    {
      return _TLB_OnGetItemText(wnd, (TINT)wparam, (TLPSTR)lparam);
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}

