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
  DWORD       editstyle;
  
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
  
  VOID*         exparam;
};
typedef struct _TLISTCTRLSTRUCT _TLISTCTRL;
typedef struct _TLISTCTRLSTRUCT *TLISTCTRL;

theader_t*   _TLC_FindHeaderByIndex(TLISTCTRL lctl, INT col);
tlistcell_t* _TLC_FindCellByIndex(TLISTCTRL lctl, INT col, INT idx);
tlistcell_t* _TLC_FindCellByHeader(TLISTCTRL lctl, theader_t* header, INT idx);
LONG         _TLC_GetCellRect(tlistcell_t* cell, RECT* rect);
INT          _TLC_FindHeaderIndex(TLISTCTRL lctl, theader_t* header);

VOID _TLC_DrawItem(
  TDC dc, RECT* rccell, LPCSTR caption, DWORD attrs, INT align, INT isheader);

LONG  LISTCTRLPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);
VOID  _TLC_OnKeyDown(TWND wnd, LONG ch);
VOID  _TLC_OnChar(TWND wnd, LONG ch);
VOID  _TLC_OnPaint(TWND wnd, TDC dc);
LONG  _TLC_OnAddItem(TWND wnd, LPSTR text, INT nitems);
VOID  _TLC_OnDeleteAllItems(TWND wnd);
VOID  _TLC_OnDeleteColumn(TWND wnd, INT col);
LONG  _TLC_OnAddColumn(TWND wnd, HEADERITEM* hdritem);
VOID  _TLC_OnDestroy(TWND wnd);
LONG  _TLC_OnCreate(TWND wnd);
LONG  _TLC_OnGetItem(TWND wnd, UINT flags, SUBITEM* subitem);
LONG  _TLC_OnSetItem(TWND wnd, UINT flags, SUBITEM* subitem);
VOID  _TLC_OnSetFocus(TWND wnd);
LONG  _TLC_OnKillFocus(TWND wnd);
VOID  _TLC_OnInvalidateItem(TWND wnd, UINT col, UINT idx);
VOID  _TLC_OnSelChanged(TWND wnd);
VOID  _TLC_OnBeginMoving(TWND wnd);
VOID  _TLC_OnMovingCursor(TWND wnd, LONG ch);
VOID  _TLC_OnEndMoving(TWND wnd);
VOID  _TLC_OnBeginEdit(TWND wnd);
VOID  _TLC_OnEndEdit(TWND wnd, INT ok);
DWORD _TLC_OnGetEditStyle(TWND wnd, INT col);
VOID  _TLC_OnSetEditStyle(TWND wnd, INT col, DWORD editstyle);
INT   _TLC_OnGetCurRow(TWND wnd);
VOID  _TLC_OnSetCurRow(TWND wnd, INT idx);
VOID  _TLC_OnSetColWidth(TWND wnd, INT col, INT width);
TWND  _TLC_OnGetEditBox(TWND wnd);
VOID  _TLC_OnSetCurPage(TWND wnd, INT npage);
INT   _TLC_OnGetCurPage(TWND wnd);
INT   _TLC_OnGetItemsPerPage(TWND wnd);



INT  _TLC_FindHeaderIndex(TLISTCTRL lctl, theader_t* header)
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
theader_t* _TLC_FindHeaderByIndex(TLISTCTRL lctl, INT col)
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

tlistcell_t* _TLC_FindCellByIndex(TLISTCTRL lctl, INT col, INT idx)
{
  tlistcell_t* cell = 0;
  theader_t* header = 0;
  INT i = 0;
  
  if (idx < 0 || idx >= lctl->nitems)
  {
    return 0;
  }
  header = _TLC_FindHeaderByIndex(lctl, col);
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

tlistcell_t* _TLC_FindCellByHeader(TLISTCTRL lctl, theader_t* header, INT idx)
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
LONG _TLC_GetCellRect(tlistcell_t* cell, RECT* rect)
{
  rect->y = cell->y;
  rect->x = cell->x;
  rect->lines = cell->lines;
  rect->cols  = cell->cols;
  return TUI_OK;
}

LONG _TLC_OnCreate(TWND wnd)
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

VOID _TLC_OnSelChanged(TWND wnd)
{
  NMHDR nmhdr;
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_SELCHANGED;

  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}

VOID _TLC_OnSetFocus(TWND wnd)
{
  NMHDR nmhdr;
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_SETFOCUS;
  
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}

LONG _TLC_OnKillFocus(TWND wnd)
{
  NMHDR nmhdr;
  TLISTCTRL lctl = 0;
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->state == LCS_MOVINGCURSOR)
  {
    _TLC_OnEndMoving(wnd);
    lctl->state = LCS_VIEW;
  }
  else if (lctl->state == LCS_EDITING)
  {
    _TLC_OnEndEdit(wnd, LC_ENDEDITCANCEL);
    lctl->state = LCS_VIEW;
  }
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_KILLFOCUS;
  
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);

  return TUI_CONTINUE;
}

VOID _TLC_OnDestroy(TWND wnd)
{
  TLISTCTRL lctl = 0;
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  
  TLC_DeleteAllColumns(wnd);
  free(lctl);
}

LONG _TLC_OnAddColumn(TWND wnd, HEADERITEM* hdritem)
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

VOID _TLC_OnDeleteColumn(TWND wnd, INT col)
{
  TLISTCTRL lctl = 0;
  theader_t* next = 0;
  tlistcell_t* cell = 0;
  tlistcell_t* nextcell = 0;
  theader_t* header = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  header = _TLC_FindHeaderByIndex(lctl, col);
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

VOID _TLC_OnDeleteAllItems(TWND wnd)
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
    cell = _TLC_FindCellByHeader(lctl, header, idx);
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

LONG _TLC_OnAddItem(TWND wnd, LPSTR text, INT nitems)
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
    while (/*tok &&*/ i < nitems && i < lctl->nheaders)
    {
      newcell = (tlistcell_t*)malloc(sizeof(tlistcell_t));
      if (!newcell)
      {
        break;
      }
      memset(newcell, 0, sizeof(tlistcell_t));
      if (tok)
      {
        strncpy(newcell->caption, tok, MIN(TUI_MAX_WNDTEXT, strlen(tok)));
      }
      else
      {
        strcpy(newcell->caption, "");
      }
      
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

VOID _TLC_DrawItem(
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

VOID _TLC_OnPaint(TWND wnd, TDC dc)
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
      _TLC_DrawItem(dc, &rcitem, 
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
        if (!(TLCS_NOSELECTION & style) && i == lctl->curselrow)
        {
          attrs = TuiGetReverseSysColor(COLOR_HIGHLIGHT);
        }
        /* draw th item that it can be seen */
        _TLC_DrawItem(dc, &rccell, 
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

VOID _TLC_OnBeginMoving(TWND wnd)
{
  NMHDR nmhdr;
  TLISTCTRL lctl = 0;
  RECT rccell;
  tlistcell_t* cell = 0;
  theader_t* header = 0;
    
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->state == LCS_MOVINGCURSOR)
  {
    _TLC_OnEndMoving(wnd);
    lctl->state = LCS_VIEW;
    return;
  }
  
  lctl->state = LCS_MOVINGCURSOR;
  /* move cursor to the current row */
  lctl->curselcol = _TLC_FindHeaderIndex(lctl, lctl->firstvisiblehdr);
  if (lctl->curselrow < 0)
  {
    lctl->curselrow = 0;
    TuiInvalidateWnd(wnd);
  }
  
  /* draw moving cursor */
  header = _TLC_FindHeaderByIndex(lctl, lctl->curselcol);
  cell   = _TLC_FindCellByHeader(lctl, header, lctl->curselrow);
  _TLC_GetCellRect(cell, &rccell);
  
  _TLC_DrawItem(TuiGetDC(wnd), &rccell, 
          cell->caption,
          TuiUnderlineText(cell->attrs),
          header->align, 0);

  /* move cursor */
  TuiMoveYX(TuiGetDC(wnd), rccell.y, rccell.x);
  
  /* save the editing cell */
  lctl->editingcell = cell;
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_BEGINMOVING;

  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}

VOID _TLC_OnMovingCursor(TWND wnd, LONG ch)
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
  DWORD style = TuiGetWndStyle(wnd);

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->state != LCS_MOVINGCURSOR)
  {
    return;
  }
  
  TuiGetWndRect(wnd, &rcwnd);
  firstcol = _TLC_FindHeaderIndex(lctl, lctl->firstvisiblehdr);
  lastcol  = _TLC_FindHeaderIndex(lctl, lctl->lastvisiblehdr);
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
      if (!(TLCS_LINEEDIT & style))
      {
        /* not fixed line edit style */
        ++newrow;
        if (newrow > maxrows)
        {
          newrow = maxrows;
        }
      }
      break;
    }
    
    case KEY_UP:
    {
      if (!(TLCS_LINEEDIT & style))
      {
        /* not fixed line edit style */
        --newrow;
        if (newrow < lctl->firstvisiblerow)
        {
          newrow = lctl->firstvisiblerow;
        }
      }
      break;
    }
  }
  
  if (newcol != lctl->curselcol || newrow != lctl->curselrow)
  {
    /* redraw the previous moving cursor */
    header = _TLC_FindHeaderByIndex(lctl, lctl->curselcol);
    cell   = _TLC_FindCellByHeader(lctl, header, lctl->curselrow);
    _TLC_GetCellRect(cell, &rccell);
    _TLC_DrawItem(
      TuiGetDC(wnd),
      &rccell,
      cell->caption,
      cell->attrs,
      header->align,
      0);

    /* draw moving cursor */
    header = _TLC_FindHeaderByIndex(lctl, newcol);
    cell   = _TLC_FindCellByHeader(lctl, header, newrow);
    _TLC_GetCellRect(cell, &rccell);
    _TLC_DrawItem(TuiGetDC(wnd),
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

VOID _TLC_OnEndMoving(TWND wnd)
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
  
  /* redraw the previous moving cursor */
  header = _TLC_FindHeaderByIndex(lctl, lctl->curselcol);
  cell   = _TLC_FindCellByHeader(lctl, header, lctl->curselrow);
  _TLC_GetCellRect(cell, &rccell);

  _TLC_DrawItem(
    TuiGetDC(wnd),
    &rccell, 
    cell->caption,
    TuiReverseColor(cell->attrs),
    header->align,
    0);

  /* update state */
  lctl->state = LCS_VIEW;
  
  TuiInvalidateWnd(wnd);
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_ENDMOVING;

  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}


VOID _TLC_OnBeginEdit(TWND wnd)
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
    _TLC_OnBeginMoving(wnd);
  }
  if (lctl->state == LCS_MOVINGCURSOR)
  {
    _TLC_OnEndMoving(wnd);
  }
  lctl->state = LCS_BEGINEDIT;
  /* show edit box at the current row and column */
  _TLC_GetCellRect(lctl->editingcell, &rccell);
  TuiMoveWnd(lctl->editbox, rccell.y, rccell.x, rccell.lines, rccell.cols);

  editstyle = TLC_GetEditStyle(wnd, lctl->curselcol);

  TuiSetWndStyle(lctl->editbox, editstyle);
  if (editstyle & TES_DECIMAL)
  {
    theader_t* header = _TLC_FindHeaderByIndex(lctl, lctl->curselcol);
    TEDT_SetDecimalWidth(lctl->editbox, header->decwidth);
  }
  
  /* update text before showing */
  strcpy(buf, lctl->editingcell->caption);
  TuiSetWndText(lctl->editbox, buf);
  TuiShowWnd(lctl->editbox, TW_SHOW);
  
  TuiSetFocus(lctl->editbox);
  
  /* update state */
  lctl->state = LCS_EDITING;
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = TLCN_BEGINEDIT;

  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
}


VOID _TLC_OnEndEdit(TWND wnd, INT ok)
{
  NMHDR nmhdr;
  TLISTCTRL lctl = 0;
  CHAR buf[TUI_MAX_WNDTEXT+1];
  INT rc = TUI_CONTINUE;
  
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

  /* hide edit box */
  rc = TuiSendMsg(lctl->editbox, TWM_KILLFOCUS, 0, 0);
  TuiShowWnd(lctl->editbox, TW_HIDE);
  
  /* send notification */
  nmhdr.id   = TuiGetWndID(wnd);
  nmhdr.ctl  = wnd;
  nmhdr.code = (LC_ENDEDITOK == ok && TUI_CONTINUE == rc ?
                  TLCN_ENDEDITOK : TLCN_ENDEDITCANCEL);
  if (ok)
  {
    if (TUI_CONTINUE == rc)
    {
      /* update */
      TuiGetWndText(lctl->editbox, buf, TUI_MAX_WNDTEXT);
      strcpy(lctl->editingcell->caption, buf);
    }
  }
  TuiPostMsg(TuiGetParent(wnd), TWM_NOTIFY, 0, (LPARAM)&nmhdr);
  
  TuiInvalidateWnd(wnd);
}

VOID _TLC_OnChar(TWND wnd, LONG ch)
{
  TLISTCTRL lctl = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (lctl->state == LCS_EDITING)
  {
    /* forward key entered to edit */
    TuiSendMsg(lctl->editbox, TWM_CHAR, (WPARAM)ch, 0);
  }
}

VOID _TLC_OnKeyDown(TWND wnd, LONG ch)
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
    /*case KEY_F(10):*/
    {
      /* toggle begin/end moving */
      _TLC_OnBeginMoving(wnd);
      break;
    }
    
#if defined __USE_CURSES__
    case KEY_F(6):
#elif defined __USE_QIO__
    case KEY_F6:
#endif
    {
      /* end moving */
      _TLC_OnBeginEdit(wnd);
      /* begin edit */
      break;
    }
    
    case TVK_ENTER:
    case KEY_ENTER:
    {
      if (lctl->state == LCS_EDITING)
      {
        _TLC_OnEndEdit(wnd, LC_ENDEDITOK); /* edit ok */
      }
      break;
    }

#if defined __USE_CURSES__
    case KEY_F(7):
#elif defined __USE_QIO__
    case KEY_F7:
#endif
    {
      if (lctl->state == LCS_EDITING)
      {
        _TLC_OnEndEdit(wnd, LC_ENDEDITCANCEL); /* edit cancel */
      }
      break;
    }

    case KEY_RIGHT:
    {
      if (lctl->state == LCS_MOVINGCURSOR)
      {
        _TLC_OnMovingCursor(wnd, ch);
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
        _TLC_OnMovingCursor(wnd, ch);
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
        _TLC_OnMovingCursor(wnd, ch);
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
        _TLC_OnMovingCursor(wnd, ch);
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
        _TLC_OnMovingCursor(wnd, ch);
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
        _TLC_OnMovingCursor(wnd, ch);
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
    _TLC_OnSelChanged(wnd);
  }
}

LONG _TLC_OnGetItemCount(TWND wnd)
{
  TLISTCTRL lctl = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  return lctl->nitems;
}

LONG _TLC_OnSetItem(TWND wnd, UINT flags, SUBITEM* subitem)
{
  TLISTCTRL lctl = 0;
  tlistcell_t* cell = 0;
  LONG rc = TUI_ERROR;

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  cell = _TLC_FindCellByIndex(lctl, subitem->col, subitem->idx);
  if (cell)
  {
    if (flags & LCFM_TEXT)  { strcpy(cell->caption, subitem->text);  }
    if (flags & LCFM_DATA)  { cell->data  = subitem->data;  }
    if (flags & LCFM_ATTRS) { cell->attrs = subitem->attrs; }
    rc = TUI_OK;
  }
  return rc;
}

LONG _TLC_OnGetItem(TWND wnd, UINT flags, SUBITEM* subitem)
{
  TLISTCTRL lctl = 0;
  tlistcell_t* cell = 0;
  LONG rc = TUI_ERROR;

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  cell = _TLC_FindCellByIndex(lctl, subitem->col, subitem->idx);
  if (cell)
  {
    if (flags & LCFM_TEXT)  { subitem->text  = cell->caption;  }
    if (flags & LCFM_DATA)  { subitem->data  = cell->data;  }
    if (flags & LCFM_ATTRS) { subitem->attrs = cell->attrs; }
    rc = TUI_OK;
  }
  return rc;
}

VOID _TLC_OnInvalidateItem(TWND wnd, UINT col, UINT idx)
{
  TLISTCTRL lctl = 0;
  tlistcell_t* cell = 0;
  theader_t* header = 0;
  RECT rcwnd;
  RECT rccell;
  DWORD attrs = 0;

  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  cell = _TLC_FindCellByIndex(lctl, col, idx);
  if (cell)
  {
    /* check if the cell is available on screen */
    TuiGetWndRect(wnd, &rcwnd);
    if (idx >= lctl->firstvisiblerow && 
        idx <  lctl->firstvisiblerow + rcwnd.lines)
    {
      if (col >= _TLC_FindHeaderIndex(lctl, lctl->firstvisiblehdr) &&
          col <  _TLC_FindHeaderIndex(lctl, lctl->lastvisiblehdr))
      {
        header = _TLC_FindHeaderByIndex(lctl, col);
        _TLC_GetCellRect(cell, &rccell);

        
        attrs = cell->attrs;
        if (idx == lctl->curselrow)
        {
          attrs = TuiReverseColor(CYAN_BLACK);
        }
        /* draw th item that it can be seen */
        _TLC_DrawItem(TuiGetDC(wnd), &rccell, 
          cell->caption, attrs, header->align, 0);
      }
    }
  }
}

VOID _TLC_OnSetEditStyle(TWND wnd, INT col, DWORD editstyle)
{
  TLISTCTRL lctl = 0;
  theader_t* header = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  header = _TLC_FindHeaderByIndex(lctl, col);
  if (header)
  {
    header->editstyle = TWS_CHILD|editstyle;
  }
}

DWORD _TLC_OnGetEditStyle(TWND wnd, INT col)
{
  TLISTCTRL lctl = 0;
  theader_t* header = 0;
  DWORD editstyle = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  header = _TLC_FindHeaderByIndex(lctl, col);
  if (header)
  {
    editstyle = header->editstyle;
  }
  return editstyle;
}

VOID _TLC_OnSetCurRow(TWND wnd, INT idx)
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

INT _TLC_OnGetCurRow(TWND wnd)
{
  TLISTCTRL lctl = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  
  return lctl->curselrow;
}

VOID _TLC_OnSetColWidth(TWND wnd, INT col, INT width)
{
  TLISTCTRL lctl = 0;
  theader_t* header = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  header = _TLC_FindHeaderByIndex(lctl, col);
  if (header)
  {
    header->cols = width;
  }
}

TWND _TLC_OnGetEditBox(TWND wnd)
{
  TLISTCTRL lctl = 0;
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  return lctl->editbox;
}

VOID _TLC_OnSetCurPage(TWND wnd, INT npage)
{
  INT idx = 0;
  INT nitemspp = _TLC_OnGetItemsPerPage(wnd);
  
  idx = (npage * nitemspp);
  TuiSendMsg(wnd, TLCM_SETCURROW, (WPARAM)idx, 0);
}

INT _TLC_OnGetCurPage(TWND wnd)
{
  INT curselrow = _TLC_OnGetCurRow(wnd);
  INT nitems = _TLC_OnGetItemCount(wnd);
  INT nitemspp = _TLC_OnGetItemsPerPage(wnd);
  INT npage = -1;
  
  /* items is greater than zero */
  if (nitems > 0 && nitemspp > 0)
  {
    /* has selected row yet? */
    if (curselrow < 0)
    {
      curselrow = 0;
    }
    /* find the current page */
    npage = (curselrow / nitemspp);
  }
  return npage;
}

INT _TLC_OnGetItemsPerPage(TWND wnd)
{
  RECT rcwnd;
  TuiGetWndRect(wnd, &rcwnd);
  return (rcwnd.lines > 1 ? rcwnd.lines - 1 : 0);
}

LONG LISTCTRLPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_CREATE:
    {
      /* initial memory for static control */
      return _TLC_OnCreate(wnd);
    }
    case TWM_DESTROY:
    {
      /* release memory of static control */
      _TLC_OnDestroy(wnd);
      return 0;
    }

    case TWM_SETFOCUS:
    {
      _TLC_OnSetFocus(wnd);
      break;
    }
    case TWM_KILLFOCUS:
    {
      return _TLC_OnKillFocus(wnd);
    }

    case TWM_KEYDOWN:
    {
      _TLC_OnKeyDown(wnd, (LONG)wparam);
      break;
    }
    
    case TWM_CHAR:
    {
      _TLC_OnChar(wnd, (LONG)wparam);
      break;
    }

    case TWM_PAINT:
    {
      _TLC_OnPaint(wnd, TuiGetDC(wnd));
      return 0;
    }
    
    case TLCM_ADDCOLUMN:
    {
      return _TLC_OnAddColumn(wnd, (HEADERITEM*)lparam);
    }
    case TLCM_DELETECOLUMN:
    {
      _TLC_OnDeleteColumn(wnd, (INT)wparam);
      return 0;
    }
    case TLCM_DELETEALLCOLUMNS:
    {
      _TLC_OnDeleteAllItems(wnd);
      return 0;
    }
    case TLCM_ADDITEM:
    {
      return _TLC_OnAddItem(wnd, (LPSTR)lparam, (INT)wparam);
    }
    case TLCM_DELETEITEM:
    {
      TLC_OnDeleteItem(wnd, (INT)lparam);
      return 0;
    }
    case TLCM_DELETEALLITEMS:
    {
      _TLC_OnDeleteAllItems(wnd);
      return 0;
    }
    case TLCM_SETITEM:
    {
      return _TLC_OnSetItem(wnd, (UINT)wparam, (SUBITEM*)lparam);
    }
    case TLCM_GETITEM:
    {
      return _TLC_OnGetItem(wnd, (UINT)wparam, (SUBITEM*)lparam);
    }
    case TLCM_GETITEMCOUNT:
    {
      return _TLC_OnGetItemCount(wnd);
    }
    case TLCM_INVALIDATEITEM:
    {
      _TLC_OnInvalidateItem(wnd, (UINT)wparam, (UINT)lparam);
      return 0;
    }
    case TLCM_SETEDITSTYLE:
    {
      _TLC_OnSetEditStyle(wnd, (INT)wparam, (DWORD)lparam);
      return 0;
    }
    case TLCM_GETEDITSTYLE:
    {
      return _TLC_OnGetEditStyle(wnd, (INT)wparam);
    }
    case TLCM_GETCURROW:
    {
      return _TLC_OnGetCurRow(wnd);
    }
    case TLCM_SETCURROW:
    {
      _TLC_OnSetCurRow(wnd, (INT)lparam);
      return 0;
    }
    case TLCM_SETCOLWIDTH:
    {
      _TLC_OnSetColWidth(wnd, (INT)wparam, (INT)lparam);
      return 0;
    }
    case TLCM_GETEDITBOX:
    {
      return (LONG)_TLC_OnGetEditBox(wnd);
    }
    case TLCM_SETCURPAGE:
    {
      _TLC_OnSetCurPage(wnd, (INT)wparam);
      return 0;
    }
    case TLCM_GETCURPAGE:
    {
      return _TLC_OnGetCurPage(wnd);
    }
    case TLCM_GETITEMSPERPAGE:
    {
      return _TLC_OnGetItemsPerPage(wnd);
    }
  }
  return TuiDefWndProc(wnd, msg, wparam, lparam);
}

/*-------------------------------------------------------------------
 * LISTPAGECTRL functions
 *-----------------------------------------------------------------*/
/* listctrl procedure is a based class of listpagectrl */
VOID  _TLPC_OnKeyDown(TWND wnd, LONG ch);
DWORD _TLPC_OnSetEditStyle(TWND wnd, INT idx, DWORD style);
DWORD _TLPC_OnGetEditStyle(TWND wnd, INT idx);
LONG  _TLPC_OnCreate(TWND wnd);
VOID  _TLPC_OnPaint(TWND wnd, TDC dc);
VOID  _TLPC_OnSetCurPage(TWND wnd, INT npage);
INT   _TLPC_OnGetCurPage(TWND wnd);
VOID  _TLPC_OnGetItemsPerPage(TWND wnd, INT nitems);
VOID  _TLPC_OnSetCurRow(TWND wnd, INT idx);
VOID  _TLPC_OnSetFocus(TWND wnd);
VOID  _TLPC_OpenEditBox(TWND wnd, INT row);
LONG  _TLPC_OnAddItem(TWND wnd, LPSTR text);
LONG  _TLPC_OnAddItemEx(TWND wnd, LPSTR text, DWORD editstyle);

VOID  _TLPC_OpenEditBox(TWND wnd, INT row)
{
  DWORD editstyle = _TLPC_OnGetEditStyle(wnd, row);
/*  TWND edit = _TLC_OnGetEditBox(wnd);
  TuiSetWndStyle(edit, editstyle);
*/
  TLPC_SetEditStyle(wnd, row, editstyle);
  /* end editing from the previous */
  _TLC_OnEndEdit(wnd, LC_ENDEDITOK);
  
  _TLPC_OnSetCurRow(wnd, row);
  
  /* re-fresh window to re-calculate position */
  TuiInvalidateWnd(wnd);
  
  /* move edit to the new position */
  _TLC_OnBeginMoving(wnd);
  _TLC_OnMovingCursor(wnd, KEY_RIGHT);
  
  /* open the current edit position */
  _TLC_OnBeginEdit(wnd);
}

VOID _TLPC_OnKeyDown(TWND wnd, LONG ch)
{
  TLISTCTRL lctl = 0;
  RECT rc;
  INT currow = -1;
 
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  TuiGetWndRect(wnd, &rc);
  
  if (lctl->nheaders <= 0)
  {
    return;
  }

  switch (ch)
  {
    /* ignore some keys */
    case TVK_SPACE:
#if defined __USE_CURSES__
    /*case KEY_F(10):*/
    case KEY_F(6):
#elif defined __USE_QIO__
    case KEY_F6:
#endif
#if defined __USE_CURSES__
    case KEY_F(7):
#elif defined __USE_QIO__
    case KEY_F7:
#endif
    case KEY_RIGHT:
    case KEY_LEFT:
    {
      break;
    }

    /* enter is equal to key down arrow */
    case TVK_ENTER:
    case KEY_ENTER:
    case KEY_DOWN:
    {
      currow = _TLC_OnGetCurRow(wnd);
      _TLPC_OpenEditBox(wnd, currow + 1);
      break;
    }
  
    case KEY_UP:
    {
      currow = _TLC_OnGetCurRow(wnd);
      _TLPC_OpenEditBox(wnd, currow - 1);
      break;
    }
      
    case TVK_PRIOR:
#ifdef __USE_CURSES__
    case KEY_PPAGE:
#elif (defined __USE_QIO__ && defined __VMS__)
    case KEY_PREV:
#endif
    {
      break;
    }
    
    case TVK_NEXT:
#ifdef __USE_CURSES__
    case KEY_NPAGE:
#elif (defined __USE_QIO__ && defined __VMS__)
    case KEY_NEXT:
#endif
    {
      break;
    }
/*
    case TVK_TAB:
    case KEY_BTAB:
    {
      currow = _TLC_OnGetCurRow(wnd);
      _TLPC_OpenEditBox(wnd, currow);
      
      TuiPostMsg(TuiGetParent(wnd), TWM_KEYDOWN, (WPARAM)ch, 0);
      break;
    }
*/
    default:
    {
      LISTCTRLPROC(wnd, TWM_KEYDOWN, (WPARAM)ch, (LPARAM)0);
      return;
    }
  }
}

DWORD _TLPC_OnSetEditStyle(TWND wnd, INT idx, DWORD style)
{
  TLISTCTRL lctl = TuiGetWndParam(wnd);
  DWORD oldstyle = 0;  
  tlistcell_t* cell = _TLC_FindCellByIndex(lctl, HEADER_VALUE, idx);
  if (cell)
  {
    oldstyle = cell->editstyle;
    cell->editstyle = style;
  }
  return oldstyle;
}

DWORD _TLPC_OnGetEditStyle(TWND wnd, INT idx)
{
  TLISTCTRL lctl = TuiGetWndParam(wnd);
  DWORD style = 0;  
  tlistcell_t* cell = 0;

  idx = lctl->curselrow;
  cell = _TLC_FindCellByIndex(lctl, HEADER_VALUE, idx);
  if (cell)
  {
    style = cell->editstyle;
  }
  return style;
}

LONG _TLPC_OnCreate(TWND wnd)
{
  DWORD style = TuiGetWndStyle(wnd);
  LONG rc = _TLC_OnCreate(wnd);
  if (TUI_CONTINUE == rc)
  {
    style |= (TLCS_EDITABLE|TLCS_LINEEDIT|TLCS_NOHEADER|TLCS_NOSELECTION);
    TuiSetWndStyle(wnd, style);
    /* add 3 columns */
    TLC_AddColumnEx(wnd, "CAPTION", 40, ALIGN_LEFT, TuiGetColor(BLUE_YELLOW), 0, 0);
    TLC_AddColumnEx(wnd, "VALUE",   20, ALIGN_LEFT, TuiGetColor(BLUE_YELLOW), TES_AUTOHSCROLL, 0);
    TLC_AddColumnEx(wnd, "SUFFIX",  10, ALIGN_LEFT, TuiGetColor(BLUE_YELLOW), 0, 0);
  }
  return rc;
}

VOID _TLPC_OnSetCurRow(TWND wnd, INT idx)
{
  TLISTCTRL lctl = 0;
  INT nitemspp = _TLC_OnGetItemsPerPage(wnd);
  INT npage = 0;
  INT curpage = _TLC_OnGetCurPage(wnd);
  
  lctl = (TLISTCTRL)TuiGetWndParam(wnd);
  if (idx < 0 || idx >= lctl->nitems)
  {
    return;
  }
  
  /* find the new appropriate first visible item */
  npage = (idx / nitemspp);
  if (npage != curpage)
  {
    /* first visible must be zero's index of each page */
    lctl->firstvisiblerow = (npage * nitemspp);
  }
  /* set current selected row */
  lctl->curselrow = idx;
}

LONG _TLPC_OnAddItem(TWND wnd, LPSTR text)
{
  LONG nitems = _TLC_OnAddItem(wnd, text, 3);
  if (1 == nitems)
  {
    _TLPC_OpenEditBox(wnd, 0);
  }
  return nitems;
}

LONG _TLPC_OnAddItemEx(TWND wnd, LPSTR text, DWORD editstyle)
{
  LONG nitems = _TLPC_OnAddItem(wnd, text);
  if (nitems > 0)
  {
    _TLPC_OnSetEditStyle(wnd, nitems-1, editstyle);
  }
  return nitems;
}

VOID _TLPC_OnSetFocus(TWND wnd)
{
  INT currow = -1;
  currow = _TLC_OnGetCurRow(wnd);
  _TLPC_OnSetCurRow(wnd, currow);
}

LONG LISTPAGECTRLPROC(TWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case TWM_CREATE:
    {
      /* initial memory for static control */
      return _TLPC_OnCreate(wnd);
    }
    case TWM_KEYDOWN:
    {
      _TLPC_OnKeyDown(wnd, (LONG)wparam);
      return 0;
    }
    case TLPCM_ADDITEMEX:
    {
      /* support 3 columns */
      return _TLPC_OnAddItemEx(wnd, (LPSTR)lparam, (DWORD)wparam);
    }
    case TLPCM_ADDITEM:
    {
      /* support 3 columns */
      return _TLPC_OnAddItem(wnd, (LPSTR)lparam);
    }
    case TLPCM_SETCURROW:
    {
      _TLPC_OnSetCurRow(wnd, (INT)wparam);
      return 0;
    }
    case TWM_SETFOCUS:
    {
      _TLPC_OnSetFocus(wnd);
      break;
    }
    case TLPCM_SETEDITSTYLE:
    {
      return _TLPC_OnSetEditStyle(wnd, (INT)wparam, (DWORD)lparam);
    }
    case TLPCM_GETEDITSTYLE:
    {
      return _TLPC_OnGetEditStyle(wnd, (INT)wparam);
    }
  }
  return LISTCTRLPROC(wnd, msg, wparam, lparam);
}

