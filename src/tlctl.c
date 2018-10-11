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
  INT         lines;
  INT         cols;
  CHAR        caption[TUI_MAX_WNDTEXT+1];
  DWORD       attrs;
  VOID*       data;
  
  struct _TLISTCELLSTRUCT *prev;
  struct _TLISTCELLSTRUCT *next;
};
typedef struct _TLISTCELLSTRUCT tlistcell_t;

struct _THEADERSTRUCT
{
  INT             id;
  CHAR            caption[TUI_MAX_WNDTEXT+1];
  INT             cols;      /* width */  
  INT             align;     /* left is a default */
  DWORD           attrs;
  DWORD           editstyle;
  INT             decwidth;
  
  tlistcell_t*    firstcell;
  tlistcell_t*    lastcell;

  struct _THEADERSTRUCT *prev;
  struct _THEADERSTRUCT *next;
};
typedef struct _THEADERSTRUCT theader_t;


struct _TLISTCTRLSTRUCT
{
  theader_t*    firsthdr;
  theader_t*    lasthdr;
  theader_t*    firstvisiblehdr;
  theader_t*    lastvisiblehdr;

  INT           nheaders;
  INT           nitems;
  INT           hdrids;      /* header id */
  INT           hdrallwidths;
  /* item control */
  INT           curselrow;
  INT           curselcol;
  INT           firstvisiblerow;
  
  TWND          editbox;
  INT           state;
  tlistcell_t*  editingcell;
};
typedef struct _TLISTCTRLSTRUCT _TLISTCTRL;
typedef struct _TLISTCTRLSTRUCT *TLISTCTRL;

theader_t*   _TLCTL_FindHeaderByIndex(TLISTCTRL lctl, INT col);
tlistcell_t* _TLCTL_FindCellByIndex(TLISTCTRL lctl, INT col, INT idx);
tlistcell_t* _TLCTL_FindCellByHeader(TLISTCTRL lctl, theader_t* header, INT idx);
LONG         _TLCTL_GetCellRect(tlistcell_t* cell, RECT* rect);
INT          _TLCTL_FindHeaderIndex(TLISTCTRL lctl, theader_t* header);

VOID _TLCTL_DrawItem(
  TDC dc, RECT* rccell, LPCSTR caption, DWORD attrs, INT align, INT isheader);

LONG LISTCTRLPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);
VOID TLCTL_OnKeyDown(TWND wnd, LONG ch);
VOID TLCTL_OnChar(TWND wnd, LONG ch);
VOID TLCTL_OnPaint(TWND wnd, TDC dc);
LONG TLCTL_OnAddItem(TWND wnd, LPSTR text, INT nitems);
VOID TLCTL_OnDeleteAllItems(TWND wnd);
VOID TLCTL_OnDeleteColumn(TWND wnd, INT col);
LONG TLCTL_OnAddColumn(TWND wnd, HEADERITEM* hdritem);
VOID TLCTL_OnDestroy(TWND wnd);
LONG TLCTL_OnCreate(TWND wnd);
LONG TLCTL_OnGetItem(TWND wnd, UINT flags, SUBITEM* subitem);
LONG TLCTL_OnSetItem(TWND wnd, UINT flags, SUBITEM* subitem);
VOID TLCTL_OnSetFocus(TWND wnd);
LONG TLCTL_OnKillFocus(TWND wnd);
VOID TLCTL_OnInvalidateItem(TWND wnd, UINT col, UINT idx);
VOID TLCTL_OnSelChanged(TWND wnd);
VOID TLCTL_OnBeginMoving(TWND wnd);
VOID TLCTL_OnMovingCursor(TWND wnd, LONG ch);
VOID TLCTL_OnEndMoving(TWND wnd);
VOID TLCTL_OnBeginEdit(TWND wnd);
VOID TLCTL_OnEndEdit(TWND wnd, INT ok);
DWORD TLCTL_OnGetEditStyle(TWND wnd, INT col);
VOID TLCTL_OnSetEditStyle(TWND wnd, INT col, DWORD editstyle);
/*VOID TLCTL_OnEditItem(TWND wnd, INT col, INT idx);*/
INT TLCTL_OnGetCurSelRow(TWND wnd);
VOID TLCTL_OnSetCurSelRow(TWND wnd, INT idx);


INT  _TLCTL_FindHeaderIndex(TLISTCTRL lctl, theader_t* header)
{
  INT col = -1;
  theader_t* ctlhdr = lctl->firsthdr;
  
  if (header && ctlhdr)
  {
    while (ctlhdr)
    {
      ++col;
      if (header == ctlhdr)
      {
        break;
      }
      ctlhdr = ctlhdr->next;
    }
  }
  return col;
}
theader_t* _TLCTL_FindHeaderByIndex(TLISTCTRL lctl, INT col)
{
  theader_t* header = 0;
  INT i = 0;
  
  if (col < 0 || col >= lctl->nheaders)
  {
    return 0; /* no header deleted */
  }
  
  header = lctl->firsthdr;
  while (header)
  {
    if (i == col)
    {
      break;
    }
    header = header->next;
    ++i;
  }
  return header;
}

tlistcell_t* _TLCTL_FindCellByIndex(TLISTCTRL lctl, INT col, INT idx)
{
  tlistcell_t* cell = 0;
  theader_t* header = 0;
  INT i = 0;
  
  if (idx < 0 || idx >= lctl->nitems)
  {
    return 0;
  }
  header = _TLCTL_FindHeaderByIndex(lctl, col);
  if (header)
  {
    cell = header->firstcell;
    while (cell)
    {
      if (i == idx)
      {
        break;
      }
      cell = cell->next;
      ++i;
    }
  }
  return cell;
}

tlistcell_t* _TLCTL_FindCellByHeader(TLISTCTRL lctl, theader_t* header, INT idx)
{
  tlistcell_t* cell = 0;
  INT i = 0;
  
  if (idx < 0 || idx >= lctl->nitems)
  {
    return 0;
  }

  if (header)
  {
    cell = header->firstcell;
    while (cell)
    {
      if (i == idx)
      {
        break;
      }
      cell = cell->next;
      ++i;
    }
  }
  return cell;
}
LONG _TLCTL_GetCellRect(tlistcell_t* cell, RECT* rect)
{
  rect->y = cell->y;
  rect->x = cell->x;
  rect->lines = cell->lines;
  rect->cols  = cell->cols;
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
  lctl->editbox = TuiCreateWnd(EDITBOX,
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
  lctl->firstvisiblerow = -1;
  /* increment child ids */
  ++lctl->hdrids;
  
  /* save memory */
  TuiSetWndParam(wnd, (LPVOID)lctl);
  return TUI_CONTINUE;
}

VOID TLCTL_OnSelChanged(TWND wnd)
{
  NMHDR nmhdr;
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_SELCHANGED;

  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}

VOID TLCTL_OnSetFocus(TWND wnd)
{
  NMHDR nmhdr;
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_SETFOCUS;
  
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}

LONG TLCTL_OnKillFocus(TWND wnd)
{
  NMHDR nmhdr;
  TLISTCTRL lctl = 0;
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->state == LCS_MOVINGCURSOR)
  {
    TLCTL_OnEndMoving(wnd);
    lctl->state = LCS_VIEW;
  }
  else if (lctl->state == LCS_EDITING)
  {
    TLCTL_OnEndEdit(wnd, 0);
    lctl->state = LCS_VIEW;
  }
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_KILLFOCUS;
  
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);

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
  strncpy(header->caption,
    hdritem->caption,
    MIN(TUI_MAX_WNDTEXT, strlen(hdritem->caption)));
  header->cols    = hdritem->cols;
  header->align   = hdritem->align;
  header->attrs   = hdritem->attrs;
  header->editstyle = hdritem->editstyle;
  header->decwidth  = hdritem->decwidth;
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

        lctl->firstvisiblerow   = 0;
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

VOID _TLCTL_DrawItem(
  TDC dc, RECT* rccell, LPCSTR caption, DWORD attrs, INT align, INT isheader)
{
  LONG len = 0;
  CHAR buf[TUI_MAX_WNDTEXT+1];
  len = TuiPrintTextAlignment(buf, 
          caption,
          rccell->cols,
          align);
  /* is is a header */
  if (isheader)
  {
    buf[0] = '[';
    buf[len - 1] = ']';
  }

  TuiDrawText(dc,
    rccell->y,
    rccell->x,
    buf,
    attrs);
}

VOID TLCTL_OnPaint(TWND wnd, TDC dc)
{
  TLISTCTRL lctl = 0;
  theader_t* header = 0;
  INT width = 0;
  RECT rcitem, rcwnd, rccell;
  DWORD attrs = 0;
  DWORD hdrattrs = 0;
  tlistcell_t* visiblecell = 0;
  INT i = 0;
  INT forcewidth = 0;
  DWORD style = TuiGetWndStyle(wnd);

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  
  /* if no headers or control is hiding */
  if (lctl->nheaders <= 0 || !TuiIsWndVisible(wnd))
  {
    return;
  }

  /* draw headers */
  TuiGetWndRect(wnd, &rcwnd);
  rcitem = rcwnd; /* copy some values */
  rcitem.lines = (style & TLCS_NOHEADER ? 0 : 1);
  
  header = lctl->firstvisiblehdr;
  width  = header->cols;
  
  /* first column width must not be over window columns */
  if (width > rcwnd.cols)
  {
    width = rcwnd.cols - 1;
    forcewidth = 1;
  }
  
  while (header && width < rcwnd.cols)
  {    
    rcitem.x    = rcwnd.x + width - header->cols;
    rcitem.cols = header->cols;
    if (rcitem.x < 0)
    {
      rcitem.x = rcwnd.x;
      if (forcewidth == 1)
      {
        rcitem.cols = width;
      }
      else
      {
        --rcitem.cols;
      }
    }

    hdrattrs = header->attrs;
    attrs = TuiReverseColor(header->attrs);
    if (!(style & TLCS_NOHEADER))
    {
      _TLCTL_DrawItem(dc, &rcitem, 
        header->caption, attrs, ALIGN_CENTER, 1); /* 1=header */
    }

    /* draw cells */
    rccell = rcitem;
    rccell.y += (style & TLCS_NOHEADER ? 0 : 1);
    rccell.lines = 1;
    visiblecell = header->firstcell;
    for (i = 0; i < lctl->nitems && visiblecell;
        ++i, visiblecell = visiblecell->next)
    {
      if (i < lctl->firstvisiblerow)
      {
        /* do nothing */
        continue;
      }
      else if (i - lctl->firstvisiblerow <= rcwnd.lines - 2)
      {

        attrs = visiblecell->attrs;
        if (i == lctl->curselrow)
        {
          attrs = TuiReverseColor(attrs);
        }
        /* draw th item that it can be seen */
        _TLCTL_DrawItem(dc, &rccell, 
          visiblecell->caption, attrs, header->align, 0);

        /* update cell rect */
        visiblecell->y = rccell.y;
        visiblecell->x = rccell.x;
        visiblecell->lines = rccell.lines;
        visiblecell->cols  = rccell.cols;
        /* next cell line */
        ++rccell.y;
      }
      else
      {
        /* no need to draw more items than client lines */
        break;
      }
    } /* for each drawing cell */
    
    /* draw next header */
    header = header->next;
    if (header)
    {
      width += header->cols;
      if (width > rcwnd.cols)
      {
        break;
      }
    }
  } /* while header */
  
  /* print arrow controls */
  attrs = TuiReverseColor(hdrattrs);
  if (lctl->firsthdr != lctl->firstvisiblehdr)
  {
    if (!(style & TLCS_NOHEADER))
    {
      TuiPutChar(dc, rcwnd.y, rcwnd.x + 1, '<', attrs);
    }
  }
  /* save the last visible */
  if (header)
  {
    lctl->lastvisiblehdr = header->prev;
    if (!(style & TLCS_NOHEADER))
    {
      TuiPutChar(dc, rcwnd.y, 
        rcwnd.x + width - header->cols - 2, '>', attrs);
    }
  }
  else
  {
    lctl->lastvisiblehdr = lctl->lasthdr;
  }

  TuiMoveYX(dc, rcitem.y,
      rcitem.x);
}

VOID TLCTL_OnBeginMoving(TWND wnd)
{
  NMHDR nmhdr;
  TLISTCTRL lctl = 0;
  RECT rccell;
  tlistcell_t* cell = 0;
  theader_t* header = 0;
    
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->state == LCS_MOVINGCURSOR)
  {
    TLCTL_OnEndMoving(wnd);
    lctl->state = LCS_VIEW;
    return;
  }
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_BEGINMOVING;

  TuiSendMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
  
  lctl->state = LCS_MOVINGCURSOR;
  /* move cursor to the current row */
  lctl->curselcol = _TLCTL_FindHeaderIndex(lctl, lctl->firstvisiblehdr);
  if (lctl->curselrow < 0)
  {
    lctl->curselrow = 0;
    TuiInvalidateWnd(wnd);
  }
  
  /* draw moving cursor */
  header = _TLCTL_FindHeaderByIndex(lctl, lctl->curselcol);
  cell   = _TLCTL_FindCellByHeader(lctl, header, lctl->curselrow);
  _TLCTL_GetCellRect(cell, &rccell);
  
  _TLCTL_DrawItem(TuiGetDC(wnd), &rccell, 
          cell->caption,
          TuiUnderlineText(cell->attrs),
          header->align, 0);

  /* move cursor */
  TuiMoveYX(TuiGetDC(wnd), rccell.y, rccell.x);
  
  /* save the editing cell */
  lctl->editingcell = cell;
}

VOID TLCTL_OnMovingCursor(TWND wnd, LONG ch)
{
  TLISTCTRL lctl = 0;
  INT firstcol = -1, lastcol = -1;
  INT newcol = -1;
  INT newrow = -1;
  RECT rccell;
  RECT rcwnd;
  tlistcell_t* cell = 0;  
  theader_t* header = 0;
  INT maxrows = 25;

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->state != LCS_MOVINGCURSOR)
  {
    return;
  }
  
  TuiGetWndRect(wnd, &rcwnd);
  firstcol = _TLCTL_FindHeaderIndex(lctl, lctl->firstvisiblehdr);
  lastcol  = _TLCTL_FindHeaderIndex(lctl, lctl->lastvisiblehdr);
  newcol   = lctl->curselcol;
  newrow   = lctl->curselrow;
  
  maxrows  = lctl->firstvisiblerow + rcwnd.lines - 2;
  
  switch (ch)
  {
    case KEY_LEFT:
    {
      --newcol;
      if (newcol < firstcol)
      {
        newcol = firstcol;
      }
      break;
    }
    
    case KEY_RIGHT:
    {
      ++newcol;
      if (newcol > lastcol)
      {
        newcol = lastcol;
      }
      break;
    }
    
    case KEY_DOWN:
    {
      ++newrow;
      if (newrow > maxrows)
      {
        newrow = maxrows;
      }
      break;
    }
    
    case KEY_UP:
    {
      --newrow;
      if (newrow < lctl->firstvisiblerow)
      {
        newrow = lctl->firstvisiblerow;
      }
      break;
    }
  }
  
  if (newcol != lctl->curselcol || newrow != lctl->curselrow)
  {
    /* redraw the previous moving cursor */
    header = _TLCTL_FindHeaderByIndex(lctl, lctl->curselcol);
    cell   = _TLCTL_FindCellByHeader(lctl, header, lctl->curselrow);
    _TLCTL_GetCellRect(cell, &rccell);
    _TLCTL_DrawItem(
      TuiGetDC(wnd),
      &rccell,
      cell->caption,
      cell->attrs,
      header->align,
      0);

    /* draw moving cursor */
    header = _TLCTL_FindHeaderByIndex(lctl, newcol);
    cell   = _TLCTL_FindCellByHeader(lctl, header, newrow);
    _TLCTL_GetCellRect(cell, &rccell);
    _TLCTL_DrawItem(TuiGetDC(wnd),
      &rccell,
      cell->caption,
      TuiUnderlineText(cell->attrs),
      header->align,
      0);
    
    TuiMoveYX(TuiGetDC(wnd), rccell.y, rccell.x);

    /* save the last current selected column */
    lctl->curselcol = newcol;
    lctl->curselrow = newrow;
    /* save the editing cell */
    lctl->editingcell = cell;
  }
}

VOID TLCTL_OnEndMoving(TWND wnd)
{
  NMHDR nmhdr;
  TLISTCTRL lctl = 0;
  tlistcell_t* cell = 0;
  theader_t* header = 0;
  RECT rccell;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->state != LCS_MOVINGCURSOR)
  {
    return;
  }
  lctl->state = LCS_ENDMOVING;
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_ENDMOVING;

  TuiSendMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
  
  /* redraw the previous moving cursor */
  header = _TLCTL_FindHeaderByIndex(lctl, lctl->curselcol);
  cell   = _TLCTL_FindCellByHeader(lctl, header, lctl->curselrow);
  _TLCTL_GetCellRect(cell, &rccell);

  _TLCTL_DrawItem(
    TuiGetDC(wnd),
    &rccell, 
    cell->caption,
    TuiReverseColor(cell->attrs),
    header->align,
    0);

  /* update state */
  lctl->state = LCS_VIEW;
  
  TuiInvalidateWnd(wnd);
}


VOID TLCTL_OnBeginEdit(TWND wnd)
{
  NMHDR nmhdr;
  TLISTCTRL lctl = 0;
  RECT rccell;
  CHAR buf[TUI_MAX_WNDTEXT+1];
  DWORD editstyle = 0;
  
  if (!(TuiGetWndStyle(wnd) & TLCS_EDITABLE))
  {
    /* if it has no EDIT style */
    return;
  }
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->state != LCS_MOVINGCURSOR)
  {
    TLCTL_OnBeginMoving(wnd);
  }
  if (lctl->state == LCS_MOVINGCURSOR)
  {
    TLCTL_OnEndMoving(wnd);
  }
  lctl->state = LCS_BEGINEDIT;
  /* show edit box at the current row and column */
  _TLCTL_GetCellRect(lctl->editingcell, &rccell);
  TuiMoveWnd(lctl->editbox, rccell.y, rccell.x, rccell.lines, rccell.cols);
  editstyle = TLC_GetEditStyle(wnd, lctl->curselcol);
  TuiSetWndStyle(lctl->editbox, editstyle);
  if (editstyle & TES_DECIMAL)
  {
    theader_t* header = _TLCTL_FindHeaderByIndex(lctl, lctl->curselcol);
    TEDT_SetDecimalWidth(lctl->editbox, header->decwidth);
  }
  
  /* update text before showing */
  strcpy(buf, lctl->editingcell->caption);
  TuiSetWndText(lctl->editbox, buf);
  TuiShowWnd(lctl->editbox, TW_SHOW);
  
  TuiSetFocus(lctl->editbox);
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_BEGINEDIT;

  TuiSendMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
  
  /* update state */
  lctl->state = LCS_EDITING;
}


VOID TLCTL_OnEndEdit(TWND wnd, INT ok)
{
  NMHDR nmhdr;
  TLISTCTRL lctl = 0;
  CHAR buf[TUI_MAX_WNDTEXT+1];
  INT rc = TUI_OK;
  
  if (!(TuiGetWndStyle(wnd) & TLCS_EDITABLE))
  {
    /* if it has no EDIT style */
    return;
  }
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->state != LCS_EDITING)
  {
    return;
  }
  lctl->state = LCS_ENDEDIT;
  
  if (ok)
  {
    /* save information back to cell */
    TuiSetFocus(wnd);
    TuiGetWndText(lctl->editbox, buf, TUI_MAX_WNDTEXT);
  }
  /* hide edit box */
  TuiShowWnd(lctl->editbox, TW_HIDE);
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = (ok ? TLCN_ENDEDITOK : TLCN_ENDEDITCANCEL);
  if (ok)
  {
    rc = TuiSendMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
    if (rc == TUI_OK)
    {
      /* update */
      strcpy(lctl->editingcell->caption, buf);
    }
  }
  TuiInvalidateWnd(wnd);
}

VOID TLCTL_OnChar(TWND wnd, LONG ch)
{
  TLISTCTRL lctl = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->state == LCS_EDITING)
  {
    /* forward key entered to edit */
    TuiSendMsg(lctl->editbox, TWM_CHAR, (WPARAM)ch, 0);
  }
}

VOID TLCTL_OnKeyDown(TWND wnd, LONG ch)
{
  TLISTCTRL lctl = 0;
  INT repaint = 0;
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
      /* toggle begin/end moving */
      TLCTL_OnBeginMoving(wnd);
      break;
    }
    
    /*case TVK_F2:*/
    case KEY_F(5):
    {
      /* end moving */
      TLCTL_OnBeginEdit(wnd);
      /* begin edit */
      break;
    }
    
    case TVK_ENTER:
    case KEY_ENTER:
    {
      if (lctl->state == LCS_EDITING)
      {
        TLCTL_OnEndEdit(wnd, 1); /* edit ok */
      }
      break;
    }
    /*case TVK_ESCAPE:*/
    case KEY_F(6):
    {
      if (lctl->state == LCS_EDITING)
      {
        TLCTL_OnEndEdit(wnd, 0); /* edit cancel */
      }
      break;
    }

    case KEY_RIGHT:
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        TLCTL_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else if (lctl->lastvisiblehdr != lctl->lasthdr)
      {
        lctl->firstvisiblehdr = lctl->firstvisiblehdr->next;
        ++repaint;
      }
      break;
    }
    case KEY_LEFT:
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        TLCTL_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else if (lctl->firstvisiblehdr != lctl->firsthdr)
      {
        lctl->firstvisiblehdr = lctl->firstvisiblehdr->prev;
        ++repaint;
      }
      break;
    }
    case KEY_DOWN:
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        TLCTL_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else
      {
        ++lines;
        ++repaint;
      }
      break;
    }
  
    case KEY_UP:
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        TLCTL_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else
      {
        --lines;
        ++repaint;
      }
      break;
    }
      
    case TVK_PRIOR:
#ifdef __USE_CURSES__
    case KEY_PPAGE:
#elif (defined __USE_QIO__ && defined __VMS__)
    case KEY_PREV:
#endif
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        TLCTL_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else
      {
        lines -= rc.lines - 1;
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
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        TLCTL_OnMovingCursor(wnd, ch);
      }
      else if (lctl->state == LCS_EDITING)
      {
        
      }
      else
      {
        lines += rc.lines - 1;
        ++repaint;
      }
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
    
    if (lctl->curselrow >= lctl->firstvisiblerow + rc.lines - 1)
    {
      lctl->firstvisiblerow += lines;
      if (lctl->firstvisiblerow - 1 > lctl->nitems - rc.lines)
      {
        lctl->firstvisiblerow = lctl->nitems - rc.lines;
      }
    }
    else if (lctl->firstvisiblerow > lctl->curselrow)
    {
      lctl->firstvisiblerow += lines;
      if (lctl->firstvisiblerow < 0)
      {
        lctl->firstvisiblerow = 0;
      }
    }
    TuiInvalidateWnd(wnd);
    /* send notification */
    TLCTL_OnSelChanged(wnd);
  }
}

LONG TLCTL_OnGetItemCount(TWND wnd)
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

VOID TLCTL_OnInvalidateItem(TWND wnd, UINT col, UINT idx)
{
  TLISTCTRL lctl = 0;
  tlistcell_t* cell = 0;
  theader_t* header = 0;
  RECT rcwnd;
  RECT rccell;
  DWORD attrs = 0;

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  cell = _TLCTL_FindCellByIndex(lctl, col, idx);
  if (cell)
  {
    /* check if the cell is available on screen */
    TuiGetWndRect(wnd, &rcwnd);
    if (idx >= lctl->firstvisiblerow && 
        idx <  lctl->firstvisiblerow + rcwnd.lines)
    {
      if (col >= _TLCTL_FindHeaderIndex(lctl, lctl->firstvisiblehdr) &&
          col <  _TLCTL_FindHeaderIndex(lctl, lctl->lastvisiblehdr))
      {
        header = _TLCTL_FindHeaderByIndex(lctl, col);
        _TLCTL_GetCellRect(cell, &rccell);

        
        attrs = cell->attrs;
        if (idx == lctl->curselrow)
        {
          attrs = TuiReverseColor(CYAN_BLACK);
        }
        /* draw th item that it can be seen */
        _TLCTL_DrawItem(TuiGetDC(wnd), &rccell, 
          cell->caption, attrs, header->align, 0);
      }
    }
  }
}

VOID TLCTL_OnSetEditStyle(TWND wnd, INT col, DWORD editstyle)
{
  TLISTCTRL lctl = 0;
  theader_t* header = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  header = _TLCTL_FindHeaderByIndex(lctl, col);
  if (header)
  {
    header->editstyle = TWS_CHILD|editstyle;
  }
}

DWORD TLCTL_OnGetEditStyle(TWND wnd, INT col)
{
  TLISTCTRL lctl = 0;
  theader_t* header = 0;
  DWORD editstyle = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  header = _TLCTL_FindHeaderByIndex(lctl, col);
  if (header)
  {
    editstyle = header->editstyle;
  }
  return editstyle;
}
/*
VOID TLCTL_OnEditItem(TWND wnd, INT col, INT idx)
{
  TLISTCTRL lctl = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);

  TLCTL_OnSetCurSelItem(wnd, col, idx);
  

  TLCTL_OnBeginEdit(wnd);
  

  TuiSetFocus(wnd);
  
  TuiSetFocus(lctl->editbox);
}
*/
VOID TLCTL_OnSetCurSelRow(TWND wnd, INT idx)
{
  TLISTCTRL lctl = 0;
  RECT rcwnd;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (idx < 0 || idx >= lctl->nitems)
  {
    return;
  }
  
  /* find the new appropriate first visible item */
  TuiGetWndRect(wnd, &rcwnd);
  if (idx >= lctl->firstvisiblerow &&
      idx <  lctl->firstvisiblerow + rcwnd.lines)
  {
    /* no moved first visible item */
  }
  else
  {
    if (idx > lctl->nitems - rcwnd.lines)
    {
      lctl->firstvisiblerow = lctl->nitems - rcwnd.lines;
    }
    else
    {      
      lctl->firstvisiblerow = idx;
    }
  }
  /* set current selected row */
  lctl->curselrow = idx;
  
  /* re-fresh window */
  TuiInvalidateWnd(wnd);
}

INT TLCTL_OnGetCurSelRow(TWND wnd)
{
  TLISTCTRL lctl = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  
  return lctl->curselrow;
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

    case TWM_SETFOCUS:
    {
      TLCTL_OnSetFocus(wnd);
      break;
    }
    case TWM_KILLFOCUS:
    {
      return TLCTL_OnKillFocus(wnd);
    }

    case TWM_KEYDOWN:
    {
      TLCTL_OnKeyDown(wnd, (LONG)wparam);
      break;
    }
    
    case TWM_CHAR:
    {
      TLCTL_OnChar(wnd, (LONG)wparam);
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
      TLC_OnDeleteItem(wnd, (INT)lparam);
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
      return TLCTL_OnGetItemCount(wnd);
    }
    case TLCM_INVALIDATEITEM:
    {
      TLCTL_OnInvalidateItem(wnd, (UINT)wparam, (UINT)lparam);
      return 0;
    }
    case TLCM_SETEDITSTYLE:
    {
      TLCTL_OnSetEditStyle(wnd, (INT)wparam, (DWORD)lparam);
      return 0;
    }
    case TLCM_GETEDITSTYLE:
    {
      return TLCTL_OnGetEditStyle(wnd, (INT)wparam);
    }
    /*
    case TLCM_EDITITEM:
    {
      TLCTL_OnEditItem(wnd, (INT)wparam, (INT)lparam);
      return 0;
    }
    */
    case TLCM_GETCURSELROW:
    {
      return TLCTL_OnGetCurSelRow(wnd);
    }
    case TLCM_SETCURSELROW:
    {
      TLCTL_OnSetCurSelRow(wnd, (INT)lparam);
      return 0;
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}

