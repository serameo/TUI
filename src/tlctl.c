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
struct _TLISTCELLSTRUCT
{
  INT         y;
  INT         x;
  CHAR        caption[TUI_MAX_WNDTEXT+1];
  DWORD       attrs;
  VOID*       data;
  
  struct _TLISTCELLSTRUCT *prev;
  struct _TLISTCELLSTRUCT *next;
};
typedef struct _TLISTCELLSTRUCT tlistcell_t;

struct _THEADERSTRUCT
{
  INT        id;
  CHAR       caption[TUI_MAX_WNDTEXT+1];
  INT        cols;      /* width */  
  INT        align;     /* left is a default */
  DWORD      attrs;
  
  tlistcell_t*    firstcell;
  tlistcell_t*    lastcell;

  struct _THEADERSTRUCT *prev;
  struct _THEADERSTRUCT *next;
};
typedef struct _THEADERSTRUCT theader_t;

struct _TLISTCTRLSTRUCT
{
  theader_t* firsthdr;
  theader_t* lasthdr;
  theader_t* firstvisiblehdr;
  theader_t* lastvisiblehdr;

  INT        nheaders;
  INT        nitems;
  INT        hdrids;      /* header id */
  INT        hdrallwidths;
  /* item control */
  INT        curselrow;
  INT        curselcol;
  INT        firstvisibleitem;
  
  TWND       editbox;
};
typedef struct _TLISTCTRLSTRUCT _TLISTCTRL;
typedef struct _TLISTCTRLSTRUCT *TLISTCTRL;

theader_t*   _TLCTL_FindHeaderByIndex(TLISTCTRL lctl, INT col);
tlistcell_t* _TLCTL_FindCellByIndex(TLISTCTRL lctl, INT col, INT idx);
tlistcell_t* _TLCTL_FindCellByHeader(TLISTCTRL lctl, theader_t* header, INT idx);
LONG         _TLCTL_GetCellRect(TLISTCTRL lctl, INT col, INT idx, RECT* rect);


LONG LISTCTRLPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);
VOID TLCTL_OnKeyDown(TWND wnd, LONG ch);
VOID TLCTL_OnPaint(TWND wnd, TDC dc);
LONG TLCTL_OnAddItem(TWND wnd, LPSTR text, INT nitems);
VOID TLCTL_OnDeleteAllItems(TWND wnd);
VOID TLCTL_OnDeleteColumn(TWND wnd, INT col);
LONG TLCTL_OnAddColumn(TWND wnd, HEADERITEM* hdritem);
VOID TLCTL_OnDestroy(TWND wnd);
LONG TLCTL_OnCreate(TWND wnd);
LONG TLCTL_OnGetItem(TWND wnd, UINT flags, SUBITEM* subitem);
LONG TLCTL_OnSetItem(TWND wnd, UINT flags, SUBITEM* subitem);


theader_t* _TLCTL_FindHeaderByIndex(TLISTCTRL lctl, INT col)
{
  theader_t* header = 0;
  INT i;
  
  if (col < 0 || col >= lctl->nheaders)
  {
    return 0; /* no header deleted */
  }
  
  header = lctl->firsthdr;
  for (i = 0; i == col && i < lctl->nheaders; ++i)
  {
    header = header->next;
  }
  return header;
}

tlistcell_t* _TLCTL_FindCellByIndex(TLISTCTRL lctl, INT col, INT idx)
{
  tlistcell_t* cell = 0;
  theader_t* header = 0;
  INT i;
  
  if (idx < 0 || idx >= lctl->nitems)
  {
    return 0;
  }
  header = _TLCTL_FindHeaderByIndex(lctl, col);
  if (header)
  {
    cell = header->firstcell;
    for (i = 0; i == idx && i < lctl->nitems; ++i)
    {
      cell = cell->next;
    }
  }
  return cell;
}

tlistcell_t* _TLCTL_FindCellByHeader(TLISTCTRL lctl, theader_t* header, INT idx)
{
  tlistcell_t* cell = 0;
  INT i;
  
  if (idx < 0 || idx >= lctl->nitems)
  {
    return 0;
  }

  if (header)
  {
    cell = header->firstcell;
    for (i = 0; i == idx && i < lctl->nitems; ++i)
    {
      cell = cell->next;
    }
  }
  return cell;
}

LONG _TLCTL_GetCellRect(TLISTCTRL lctl, INT col, INT idx, RECT* rect)
{
  /*
  tlistcell_t* cell = 0;
  TLISTCTRL lctl = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  cell = TLCTL_FindCellByIndex(wnd, row, col);
*/
  return TUI_OK;
}

LONG TLCTL_OnCreate(TWND wnd)
{
  TLISTCTRL lctl = (TLISTCTRL)malloc(sizeof(_TLISTCTRL));
  if (!lctl)
  {
    return TUI_MEM;
  }
  memset(lctl, 0, sizeof(_TLISTCTRL));
  lctl->hdrids    = 1;
  lctl->editbox = TuiCreateWnd(EDIT,
                    "",
                    TWS_CHILD|TES_AUTOHSCROLL,
                    0,    /* y */
                    0,    /* x */
                    1,    /* lines  */
                    1,    /* cols   */
                    wnd,  /* parent */
                    lctl->hdrids,    /* id */
                    0);   /* no parameter */
  if (!lctl->editbox)
  {
    free(lctl);
    return TUI_MEM;
  }
  lctl->curselrow = -1;
  lctl->curselcol = -1;
  lctl->firstvisibleitem = -1;
  /* increment child ids */
  ++lctl->hdrids;
  
  /* save memory */
  TuiSetWndParam(wnd, (LPVOID)lctl);
  return TUI_CONTINUE;
}

VOID TLCTL_OnDestroy(TWND wnd)
{
  TLISTCTRL lctl = 0;
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  
  TLC_DeleteAllColumns(wnd);
  free(lctl);
}

LONG TLCTL_OnAddColumn(TWND wnd, HEADERITEM* hdritem)
{
  TLISTCTRL lctl = 0;
  theader_t* header = 0;
  RECT rc;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  
  if (lctl->nitems > 0)
  {
    /* not allowed to add header after there are any items */
    return TUI_ERROR;
  }

  header = (theader_t*)malloc(sizeof(theader_t));
  if (!header)
  {
    return TUI_MEM;
  }
  memset(header, 0, sizeof(theader_t));
  
  TuiGetWndRect(TuiGetParent(wnd), &rc);
  /*header->caption = hdritem->caption;*/
  strncpy(header->caption,
    hdritem->caption,
    MIN(TUI_MAX_WNDTEXT, strlen(hdritem->caption)));
  header->cols    = hdritem->cols;
  header->align   = hdritem->align;
  header->attrs   = hdritem->attrs;
  header->id      = lctl->hdrids;
 
  /* make link */
  if (lctl->firsthdr)
  {
    header->prev = lctl->lasthdr;
    lctl->lasthdr->next = header;
    lctl->lasthdr = header;
  }
  else
  {
    lctl->firsthdr = lctl->lasthdr = header;
    lctl->firstvisiblehdr = header;
  }
  /* increment child ids */
  ++lctl->hdrids;
  ++lctl->nheaders;
  lctl->hdrallwidths += header->cols;
  
  return 0;
}

VOID TLCTL_OnDeleteColumn(TWND wnd, INT col)
{
  TLISTCTRL lctl = 0;
  theader_t* next = 0;
  tlistcell_t* cell = 0;
  tlistcell_t* nextcell = 0;
  theader_t* header = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  header = _TLCTL_FindHeaderByIndex(lctl, col);
  if (!header)
  {
    return;
  }

  /* re-link */
  next = header->next;
  if (next)
  {
    next->prev = header->prev;
  }
  if (header->prev)
  {
    header->prev->next = next;
  }
  
  if (header == lctl->firsthdr)
  {
    lctl->firsthdr = next;
  }
  else if (header == lctl->lasthdr)
  {
    lctl->lasthdr = header->prev;
  }
  /* delete */
  header->next = header->prev = 0;

  cell = header->firstcell;
  while (cell)
  {
    nextcell = cell->next;
    /*free(cell->caption);*/
    cell->next = cell->prev = 0;
    free(cell);
    cell = nextcell;
  }

  free(header);

  /* done */
  --lctl->nheaders;
}

VOID TLCTL_OnDeleteAllItems(TWND wnd)
{
  INT nitems = 0;
  INT i = 0;
  TLISTCTRL lctl = 0;

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  nitems = lctl->nheaders;
  for (i = 0; i < nitems; ++i)
  {
    TuiSendMsg(wnd, TLCM_DELETEITEM, 0, 0);
  }
}

VOID TLC_OnDeleteItem(TWND wnd, INT idx)
{
  TLISTCTRL lctl = 0;
  tlistcell_t* cell = 0;
  tlistcell_t* nextcell = 0;
  theader_t* header = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->nitems <= 0 || idx < 0 || idx >= lctl->nitems)
  {
    return;
  }
  
  header = lctl->firsthdr;
  while (header)
  {
    cell = _TLCTL_FindCellByHeader(lctl, header, idx);
    nextcell = cell->next;
    if (nextcell)
    {
      nextcell->prev = cell->prev;
    }
    if (cell->prev)
    {
      cell->prev->next = nextcell;
    }
    if (header->firstcell == cell)
    {
      header->firstcell = cell->next;
    }
    else if (header->lastcell == cell)
    {
      header->lastcell = cell->prev;
    }
    
    cell->next = cell->prev = 0;
    /*free(cell->caption);*/
    free(cell);
    /* next header */
    header = header->next;
  }
  --lctl->nitems;
}

LONG TLCTL_OnAddItem(TWND wnd, LPSTR text, INT nitems)
{
  TLISTCTRL lctl = 0;
  INT i = 0;
  CHAR* tok;
  theader_t* header = 0;
  CHAR buf[TUI_MAX_WNDTEXT+1];
  tlistcell_t* newcell = 0;

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->nheaders > 0)
  {
    /* insert into all listboxes */
    header = lctl->firsthdr;
    
    strcpy(buf, text);
    tok = strtok(buf, "\t");
    while (tok && i < nitems && i < lctl->nheaders)
    {
      newcell = (tlistcell_t*)malloc(sizeof(tlistcell_t));
      if (!newcell)
      {
        break;
      }
      memset(newcell, 0, sizeof(tlistcell_t));
      /*newcell->caption = (CHAR*)malloc(TUI_MAX_WNDTEXT+1);*/
      strncpy(newcell->caption, tok, MIN(TUI_MAX_WNDTEXT, strlen(tok)));
      
      /* add the new item */
      if (header->firstcell)
      {
        newcell->prev  = header->lastcell;
        header->lastcell->next = newcell;
        header->lastcell = newcell;
      }
      else
      {
        header->firstcell = header->lastcell = newcell;

        lctl->firstvisibleitem   = 0;
      }
      /* insert next */
      header = header->next;
      tok = strtok(0, "\t");
      ++i;
    }
    /* all items count */
    ++lctl->nitems;

  } /* have header */
  return lctl->nitems;
}

VOID TLCTL_OnPaint(TWND wnd, TDC dc)
{
  TLISTCTRL lctl = 0;
  theader_t* header = 0;
  INT width = 0;
  RECT rcitem, rc, rccell;
  CHAR buf[TUI_MAX_WNDTEXT+1];
  DWORD attrs = 0;
  LONG len = 0;
  tlistcell_t* visiblecell = 0;
  INT i = 0;

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  
  if (lctl->nheaders <= 0 || !TuiIsWndVisible(wnd))
  {
    return;
  }

  /* draw headers */
  TuiGetWndRect(wnd, &rc);
  rcitem = rc; /* copy some values */
  
  header = lctl->firstvisiblehdr;  
  width  = header->cols;
  
  if (width > rc.cols)
  {
    width = rc.cols;
  }
  
  while (header && width < rc.cols)
  {    
    rcitem.cols = header->cols;
    rcitem.x    = rc.x + width - rcitem.cols;

    attrs = header->attrs;
#ifdef __USE_CURSES__
    attrs |= A_REVERSE;
#elif defined __USE_QIO__
    attrs = 0;
#endif
    len = TuiPrintTextAlignment(buf, 
            header->caption,
            rcitem.cols,
            ALIGN_CENTER);
    
    buf[0] = '[';
    buf[len-1] = ']';
    
    TuiDrawText(dc,
      rcitem.y,
      rcitem.x,
      buf,
      attrs);

    /* draw cells */
    rccell = rcitem;
    rccell.y += 1;
    visiblecell = header->firstcell;
    for (i = 0; i < lctl->nitems && visiblecell; ++i, visiblecell = visiblecell->next)
    {
      if (i < lctl->firstvisibleitem)
      {
        /* do nothing */
        continue;
      }
      else if (i - lctl->firstvisibleitem <= rc.lines - 2)
      {
        len = TuiPrintTextAlignment(buf,
                visiblecell->caption,
                header->cols,
                header->align);

        attrs = visiblecell->attrs;
#ifdef __USE_CURSES__
        if (i == lctl->curselrow)
        {
          attrs |= A_REVERSE;
        }
#elif defined __USE_QIO__
        
#endif
        TuiDrawText(dc,
          rccell.y,
          rccell.x,
          buf,
          attrs);
        ++rccell.y;
      }
    } /* for each drawing cell */
    
    
    /* draw next header */
    header = header->next;
    if (header)
    {
      width += header->cols;
      if (width > rc.cols)
      {
        break;
      }
    }
  } /* while header */
  
  /* print arrow controls */
#ifdef __USE_CURSES__
  attrs |= A_REVERSE;
#elif defined __USE_QIO__
  attrs = 0;
#endif
  if (lctl->firsthdr != lctl->firstvisiblehdr)
  {
    TuiPutChar(dc, rc.y, rc.x, '<', attrs);
  }
  /* save the last visible */
  if (header)
  {
    lctl->lastvisiblehdr = header->prev;
    TuiPutChar(dc, rc.y, rc.x + width - header->cols - 1, '>', attrs);
  }
  else
  {
    lctl->lastvisiblehdr = lctl->lasthdr;
  }

  TuiMoveYX(dc, rcitem.y,
      rcitem.x);
}

VOID TLCTL_OnKeyDown(TWND wnd, LONG ch)
{
  TLISTCTRL lctl = 0;
  INT repaint = 1;
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
      /*repaint = 1;*/
      break;
    }

    case KEY_RIGHT:
    {
      if (lctl->lastvisiblehdr != lctl->lasthdr)
      {
        lctl->firstvisiblehdr = lctl->firstvisiblehdr->next;
        ++repaint;
      }
      break;
    }
    case KEY_LEFT:
    {
      if (lctl->firstvisiblehdr != lctl->firsthdr)
      {
        lctl->firstvisiblehdr = lctl->firstvisiblehdr->prev;
        ++repaint;
      }
      break;
    }
    case KEY_DOWN:
    {
      ++lines;
      ++repaint;
      break;
    }
  
    case KEY_UP:
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
      lines -= rc.lines - 1;
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
      lines += rc.lines - 1;
      ++repaint;
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
    
    if (lctl->curselrow >= lctl->firstvisibleitem + rc.lines - 1)
    {
      lctl->firstvisibleitem += lines;
      if (lctl->firstvisibleitem - 1 > lctl->nitems - rc.lines)
      {
        lctl->firstvisibleitem = lctl->nitems - rc.lines;
      }
    }
    else if (lctl->firstvisibleitem > lctl->curselrow)
    {
      lctl->firstvisibleitem += lines;
      if (lctl->firstvisibleitem < 0)
      {
        lctl->firstvisibleitem = 0;
      }
    }
    TuiInvalidateWnd(wnd);
    /* send notification */
    /*
    nmhdr.id   = TuiGetWndID(wnd);
    nmhdr.ctl  = wnd;
    nmhdr.code = TLBN_SELCHANGED;
    TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
    */
  }
}

LONG TLCM_OnGetItemCount(TWND wnd)
{
  TLISTCTRL lctl = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  return lctl->nitems;
}

LONG TLCTL_OnSetItem(TWND wnd, UINT flags, SUBITEM* subitem)
{
  TLISTCTRL lctl = 0;
  tlistcell_t* cell = 0;
  LONG rc = TUI_ERROR;

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  cell = _TLCTL_FindCellByIndex(lctl, subitem->col, subitem->idx);
  if (cell)
  {
    if (flags & LCFM_TEXT)  { strcpy(cell->caption, subitem->text);  }
    if (flags & LCFM_DATA)  { cell->data  = subitem->data;  }
    if (flags & LCFM_ATTRS) { cell->attrs = subitem->attrs; }
    rc = TUI_OK;
  }
  return rc;
}

LONG TLCTL_OnGetItem(TWND wnd, UINT flags, SUBITEM* subitem)
{
  TLISTCTRL lctl = 0;
  tlistcell_t* cell = 0;
  LONG rc = TUI_ERROR;

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  cell = _TLCTL_FindCellByIndex(lctl, subitem->col, subitem->idx);
  if (cell)
  {
    if (flags & LCFM_TEXT)  { subitem->text  = cell->caption;  }
    if (flags & LCFM_DATA)  { subitem->data  = cell->data;  }
    if (flags & LCFM_ATTRS) { subitem->attrs = cell->attrs; }
    rc = TUI_OK;
  }
  return rc;
}

LONG LISTCTRLPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_CREATE:
    {
      /* initial memory for static control */
      return TLCTL_OnCreate(wnd);
    }
    case TWM_DESTROY:
    {
      /* release memory of static control */
      TLCTL_OnDestroy(wnd);
      return 0;
    }
/*
    case TWM_SETFOCUS:
    {
      TLCTL_OnSetFocus(wnd);
      break;
    }
    case TWM_KILLFOCUS:
    {
      return TLCTL_OnKillFocus(wnd);
    }
*/
    case TWM_KEYDOWN:
    {
      TLCTL_OnKeyDown(wnd, (LONG)wparam);
      break;
    }

    case TWM_PAINT:
    {
      TLCTL_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
    
    case TLCM_ADDCOLUMN:
    {
      return TLCTL_OnAddColumn(wnd, (HEADERITEM*)lparam);
    }
    case TLCM_DELETECOLUMN:
    {
      TLCTL_OnDeleteColumn(wnd, (INT)wparam);
      return 0;
    }
    case TLCM_DELETEALLCOLUMNS:
    {
      TLCTL_OnDeleteAllItems(wnd);
      return 0;
    }
    case TLCM_ADDITEM:
    {
      return TLCTL_OnAddItem(wnd, (LPSTR)lparam, (INT)wparam);
    }
    case TLCM_DELETEITEM:
    {
      TLC_OnDeleteItem(wnd, (INT)wparam);
      return 0;
    }
    case TLCM_DELETEALLITEMS:
    {
      TLCTL_OnDeleteAllItems(wnd);
      return 0;
    }
    case TLCM_SETITEM:
    {
      return TLCTL_OnSetItem(wnd, (UINT)wparam, (SUBITEM*)lparam);
    }
    case TLCM_GETITEM:
    {
      return TLCTL_OnGetItem(wnd, (UINT)wparam, (SUBITEM*)lparam);
    }
    case TLCM_GETITEMCOUNT:
    {
      return TLCM_OnGetItemCount(wnd);
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}
