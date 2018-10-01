/*-------------------------------------------------------------------
 * File name: tui.h
 * Author: Seree Rakwong
 * Date: 28-SEP-18
 *-----------------------------------------------------------------*/
#ifndef __TEXTUSERINTERFACE_H__
#define __TEXTUSERINTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------
 * defines
 *-----------------------------------------------------------------*/
#define TUI_CONTINUE     1
#define TUI_OK           0
#define TUI_ERROR       -1
#define TUI_MEM         -2

/* miscellaneous */
#define TUI_MAX_WNDTEXT    80

#define TVK_BACK           0x08
#define TVK_TAB            0x09
#define TVK_ENTER          0x0A

#define TVK_SPACE          0x20
#define TVK_PRIOR          0x21
#define TVK_NEXT           0x22
#define TVK_END            0x23
#define TVK_HOME           0x24
#define TVK_LEFT           0x25
#define TVK_UP             0x26
#define TVK_RIGHT          0x27
#define TVK_DOWN           0x28
#define TVK_INSERT         0x2D
#define TVK_DELETE         0x2E
#define TVK_HELP           0x2F


#define TVK_ESCAPE         0x1B
#define TVK_NUMPAD0        0x60
#define TVK_NUMPAD1        0x61
#define TVK_NUMPAD2        0x62
#define TVK_NUMPAD3        0x63
#define TVK_NUMPAD4        0x64
#define TVK_NUMPAD5        0x65
#define TVK_NUMPAD6        0x66
#define TVK_NUMPAD7        0x67
#define TVK_NUMPAD8        0x68
#define TVK_NUMPAD9        0x69
#define TVK_MULTIPLY       0x6A
#define TVK_ADD            0x6B
#define TVK_SEPARATOR      0x6C
#define TVK_SUBTRACT       0x6D
#define TVK_DECIMAL        0x6E
#define TVK_DIVIDE         0x6F
#define TVK_F1             0x70
#define TVK_F2             0x71
#define TVK_F3             0x72
#define TVK_F4             0x73
#define TVK_F5             0x74
#define TVK_F6             0x75
#define TVK_F7             0x76
#define TVK_F8             0x77
#define TVK_F9             0x78
#define TVK_F10            0x79
#define TVK_F11            0x7A
#define TVK_F12            0x7B
#define TVK_F13            0x7C
#define TVK_F14            0x7D
#define TVK_F15            0x7E
#define TVK_F16            0x7F
#define TVK_F17            0x80
#define TVK_F18            0x81
#define TVK_F19            0x82
#define TVK_F20            0x83
#define TVK_F21            0x84
#define TVK_F22            0x85
#define TVK_F23            0x86
#define TVK_F24            0x87

/*
 * 0x88 - 0x8F : unassigned
 */

#define TVK_NUMLOCK        0x90
#define TVK_SCROLL         0x91

/*-------------------------------------------------------------------
 * types
 *-----------------------------------------------------------------*/
typedef long                WPARAM;
typedef long                LPARAM;
typedef long                LONG;
typedef int                 INT;
typedef void                VOID;
typedef void*               LPVOID;
typedef const char*         LPCSTR;
typedef char*               LPSTR;
typedef char                CHAR;
typedef double              DOUBLE;
  
typedef unsigned long       DWORD;
typedef unsigned int        UINT;
  
typedef char                INT8;
typedef unsigned char       UINT8;
typedef short               INT16;
typedef unsigned short      UINT16;
typedef int                 INT32;
typedef unsigned int        UINT32;
typedef long long           INT64;
typedef unsigned long long  UINT64;

struct _TUIDEVICECONTEXSTRUCT;
typedef struct _TUIDEVICECONTEXSTRUCT _TDC;
typedef struct _TUIDEVICECONTEXSTRUCT *TDC;

struct _TUIWINDOWSTRUCT;
typedef struct _TUIWINDOWSTRUCT _TWND;
typedef struct _TUIWINDOWSTRUCT *TWND;

typedef long (*TWNDPROC)(TWND, UINT, WPARAM, LPARAM);

struct _TUIENVSTRUCT;
typedef struct _TUIENVSTRUCT _TENV;
typedef struct _TUIENVSTRUCT *TENV;

struct _WNDTEMPLSTRUCT
{
  LPCSTR clsname;
  LPCSTR text;
  INT    id;
  INT    y;
  INT    x;
  INT    lines;
  INT    cols;
  DWORD  style;
  INT    (*validate)(TWND, LPCSTR);
};
typedef struct _WNDTEMPLSTRUCT WNDTEMPL;

struct _MSGSTRUCT
{
  TWND   wnd;
  UINT   msg;
  WPARAM wparam;
  LPARAM lparam;
};
typedef struct _MSGSTRUCT MSG;

struct _NMHDRSTRUCT
{
  INT  id;
  TWND ctl;
  UINT code;
};
typedef struct _NMHDRSTRUCT NMHDR;

struct _RECTSTRUCT
{
  INT y;
  INT x;
  INT lines;
  INT cols;
};
typedef struct _RECTSTRUCT RECT;

struct _DRAWITEMSTRUCT
{
  INT  idx;
  RECT rcitem;
};
typedef struct _DRAWITEMSTRUCT DRAWITEM;

/* colors */
/*
 216 #define COLOR_BLACK     0
 217 #define COLOR_RED       1
 218 #define COLOR_GREEN     2
 219 #define COLOR_YELLOW    3
 220 #define COLOR_BLUE      4
 221 #define COLOR_MAGENTA   5
 222 #define COLOR_CYAN      6
 223 #define COLOR_WHITE     7

*/
#define CYAN_BLACK              1
#define BLACK_CYAN              2
#define GREEN_BLACK             3
#define BLACK_GREEN             4
#define YELLOW_BLACK            5
#define BLACK_YELLOW            6
#define BLUE_YELLOW             7
#define YELLOW_BLUE             8

#define ALIGN_LEFT              0
#define ALIGN_CENTER            1
#define ALIGN_RIGHT             2

#define TW_HIDE                 0
#define TW_SHOW                 1

/* window styles */
#define TWS_WINDOW              0x00000001
#define TWS_CHILD               0x00000002
#define TWS_VISIBLE             0x00000004
#define TWS_ENABLE              0x00000008
#define TWS_DISABLED            0x00000010
#define TWS_LEFT                0x00000000             /* shared controls style */
#define TWS_CENTER              0x00010000
#define TWS_RIGHT               0x00020000

/* window messages */
#define TWM_FIRST               100
#define TWM_CREATE              (TWM_FIRST +    1)
#define TWM_DESTROY             (TWM_FIRST +    2)
#define TWM_INITDIALOG          (TWM_FIRST +    3)
#define TWM_PAINT               (TWM_FIRST +    4)
#define TWM_SETFOCUS            (TWM_FIRST +    5)
#define TWM_KILLFOCUS           (TWM_FIRST +    6)
#define TWM_KEYDOWN             (TWM_FIRST +    7)
#define TWM_KEYUP               (TWM_FIRST +    8)
#define TWM_CHAR                (TWM_FIRST +    9)
#define TWM_NOTIFY              (TWM_FIRST +   10)
#define TWM_ERASEBK             (TWM_FIRST +   11)
#define TWM_SETTEXT             (TWM_FIRST +   12)
#define TWM_GETTEXT             (TWM_FIRST +   13)
#define TWM_SETTEXTALIGN        (TWM_FIRST +   14)
#define TWM_COMMAND             (TWM_FIRST +   15)
#define TWM_SETTEXTATTRS        (TWM_FIRST +   16)
#define TWM_DRAWITEM            (TWM_FIRST +   17)
#define TWM_DLGMSGID            (TWM_FIRST +   18)

/* application user messages */
#define TWM_USER          10000

/* notification control message */
#define TSN_FIRST               (TWM_USER +  100)
#define TEN_FIRST               (TWM_USER +  150)
#define TLBN_FIRST              (TWM_USER +  200)
#define TBN_FIRST               (TWM_USER +  250)
#define TLCN_FIRST              (TWM_USER +  300)

/*-------------------------------------------------------------------
 * msgbox
 *-----------------------------------------------------------------*/
#define MSGBOX                  "MSGBOX"
/*-------------------------------------------------------------------
 * static control
 *-----------------------------------------------------------------*/
#define STATIC                  "STATIC"
      
#define TSS_LEFT                TWS_LEFT
#define TSS_CENTER              TWS_CENTER
#define TSS_RIGHT               TWS_RIGHT
      
/*-------------------------------------------------------------------
 * edit control
 *-----------------------------------------------------------------*/
#define EDIT                    "EDIT"

#define TES_LEFT                TWS_LEFT
#define TES_CENTER              TWS_CENTER
#define TES_RIGHT               TWS_RIGHT
#define TES_NUMBER              0x00040000
#define TES_UPPERCASE           0x00080000
#define TES_LOWERCASE           0x00100000
#define TES_PASSWORD            0x00200000
#define TES_APPENDMODE          0x00400000
#define TES_AUTOHSCROLL         0x00800000
#define TES_DECIMAL             0x01000000

#define TEM_LIMITTEXT           (TWM_USER  +    1)
#define TEM_SETPASSWDCHAR       (TWM_USER  +    2)
#define TEM_SHOWPASSWDCHAR      (TWM_USER  +    3)
#define TEM_SETDECWIDTH         (TWM_USER  +    4)


/* edit notified message */
#define TEN_CHANGED             (TEN_FIRST  +   0)
#define TEN_SETFOCUS            (TEN_FIRST  +   1)
#define TEN_KILLFOCUS           (TEN_FIRST  +   2)

/* edit macros */
#define TEDT_LimitText(edt, lim)    \
  TuiSendMsg(edt, TEM_LIMITTEXT, (WPARAM)lim, (LPARAM)0)
#define TEDT_SetPasswdChar(edt, ch)    \
  TuiSendMsg(edt, TEM_SETPASSWDCHAR, (WPARAM)ch, (LPARAM)0)
#define TEDT_ShowPasswdChar(edt, show)    \
  TuiSendMsg(edt, TEM_SHOWPASSWDCHAR, (WPARAM)show, (LPARAM)0)
#define TEDT_SetDecimalWidth(edt, wid)    \
  TuiSendMsg(edt, TEM_SETDECWIDTH, (WPARAM)wid, (LPARAM)0)

/*-------------------------------------------------------------------
 * listbox control
 *-----------------------------------------------------------------*/
#define LISTBOX                 "LISTBOX"

#define LB_OK                   TUI_OK
#define LB_ERROR                TUI_ERROR
#define LB_UNCHECKED            0
#define LB_CHECKED              1

#define TLBS_LEFT               TWS_LEFT
#define TLBS_CENTER             TWS_CENTER
#define TLBS_RIGHT              TWS_RIGHT
#define TLBS_OWNERDRAW          0x00100000
#define TLBS_CHECKBOX           0x00200000
#define TLBS_RADIOBOX           0x00400000

#define TLBM_ADDITEM            (TWM_USER  +    1)
#define TLBM_DELETEITEM         (TWM_USER  +    2)
#define TLBM_GETCURSEL          (TWM_USER  +    3)
#define TLBM_SETCURSEL          (TWM_USER  +    4)
#define TLBM_DELETEALLITEMS     (TWM_USER  +    5)
#define TLBM_GETITEMCOUNT       (TWM_USER  +    6)
#define TLBM_SETITEMDATA        (TWM_USER  +    7)
#define TLBM_GETITEMDATA        (TWM_USER  +    8)
#define TLBM_SETITEMTEXT        (TWM_USER  +    9)
#define TLBM_GETITEMTEXT        (TWM_USER  +   10)
#define TLBM_SETITEMCHECKED     (TWM_USER  +   11)
#define TLBM_GETITEMCHECKED     (TWM_USER  +   12)
#define TLBM_COUNTITEMCHECKED   (TWM_USER  +   13)

/* listbox notified message */
#define TLBN_SETFOCUS           (TLBN_FIRST  +    0)
#define TLBN_KILLFOCUS          (TLBN_FIRST  +    1)
#define TLBN_SELCHANGED         (TLBN_FIRST  +    2)

/* listbox macros */
#define TLB_AddItem(lbx, text)    \
  TuiSendMsg(lbx, TLBM_ADDITEM, (WPARAM)0, (LPARAM)text)
#define TLB_DeleteItem(lbx, idx)  \
  TuiSendMsg(lbx, TLBM_DELETEITEM, (WPARAM)idx, (LPARAM)0)
#define TLB_GetCurSel(lbx)        \
  TuiSendMsg(lbx, TLBM_GETCURSEL, (WPARAM)0, (LPARAM)0)
#define TLB_SetCurSel(lbx, idx)   \
  TuiSendMsg(lbx, TLBM_SETCURSEL, (WPARAM)idx, (LPARAM)0)
#define TLB_DeleteAllItems(lbx)   \
  TuiSendMsg(lbx, TLBM_DELETEALLITEMS, (WPARAM)0, (LPARAM)0)
#define TLB_GetItemCount(lbx)   \
  TuiSendMsg(lbx, TLBM_GETITEMCOUNT, (WPARAM)0, (LPARAM)0)
#define TLB_SetItemData(lbx, idx, data)   \
  TuiSendMsg(lbx, TLBM_SETITEMDATA, (WPARAM)idx, (LPARAM)data)
#define TLB_GetItemData(lbx, idx)   \
  (LPVOID)TuiSendMsg(lbx, TLBM_GETITEMDATA, (WPARAM)idx, (LPARAM)0)
#define TLB_SetItemText(lbx, idx, text)   \
  TuiSendMsg(lbx, TLBM_SETITEMTEXT, (WPARAM)idx, (LPARAM)text)
#define TLB_GetItemText(lbx, idx, text)   \
  TuiSendMsg(lbx, TLBM_GETITEMTEXT, (WPARAM)idx, (LPARAM)text)
#define TLB_SetItemChecked(lbx, idx, check)   \
  TuiSendMsg(lbx, TLBM_SETITEMCHECKED, (WPARAM)idx, (LPARAM)check)
#define TLB_GetItemChecked(lbx, idx)   \
  TuiSendMsg(lbx, TLBM_GETITEMCHECKED, (WPARAM)idx, (LPARAM)0)
#define TLB_CountItemChecked(lbx)   \
  TuiSendMsg(lbx, TLBM_COUNTITEMCHECKED, (WPARAM)0, (LPARAM)0)

/*-------------------------------------------------------------------
 * button control
 *-----------------------------------------------------------------*/
#define BUTTON                  "BUTTON"

#define TBS_RELEASED            0x0000
#define TBS_PRESSED             0x0001
#define TBS_FOCUSED             0x0002

#define TBN_SETFOCUS            (TBN_FIRST  +   0)
#define TBN_KILLFOCUS           (TBN_FIRST  +   1)

/*-------------------------------------------------------------------
 * listctrl control
 *-----------------------------------------------------------------*/
#define LISTCTRL                "LISTCTRL"


#define TLCM_ADDCOLUMN          (TWM_USER  +    1)
#define TLCM_DELETECOLUMN       (TWM_USER  +    2)
#define TLCM_DELETEALLCOLUMNS   (TWM_USER  +    3)
#define TLCM_ADDITEM            (TWM_USER  +    4)
#define TLCM_DELETEITEM         (TWM_USER  +    5)
#define TLCM_DELETEALLITEMS     (TWM_USER  +    6)
#define TLCM_SETITEM            (TWM_USER  +    7)
#define TLCM_GETITEM            (TWM_USER  +    8)
#define TLCM_GETITEMCOUNT       (TWM_USER  +    9)

/*
#define TLCN_FIRST              (TWM_USER +  300)
*/
#define TLCN_SETFOCUS           (TLCN_FIRST  +    0)
#define TLCN_KILLFOCUS          (TLCN_FIRST  +    1)
#define TLCN_SELCHANGED         (TLCN_FIRST  +    2)

#define  LCFM_TEXT              0x0001
#define  LCFM_ATTRS             0x0002
#define  LCFM_DATA              0x0004
struct _SUBITEMSTRUCT
{
  INT       col;
  INT       idx;
  CHAR*     text;
  DWORD     attrs;
  VOID*     data;
};
typedef struct _SUBITEMSTRUCT SUBITEM;

struct _HEADERITEMSTRUCT
{
  CHAR*     caption;
  INT       cols;
  INT       align;
  DWORD     attrs;
};
typedef struct _HEADERITEMSTRUCT HEADERITEM;

/* listctrl macros */
#define TLC_AddColumn(lc, text, width, al, at)    \
do {\
  HEADERITEM hdr; \
  hdr.caption = text; \
  hdr.cols    = width; \
  hdr.align   = al; \
  hdr.attrs   = at; \
  TuiSendMsg(lc, TLCM_ADDCOLUMN, (WPARAM)0, (LPARAM)&hdr); \
} while (0)
  
#define TLC_DeleteColumn(lc, col)    \
  TuiSendMsg(lc, TLCM_DELETECOLUMN, (WPARAM)col, (LPARAM)0)
#define TLC_DeleteAllColumns(lc)    \
  TuiSendMsg(lc, TLCM_DELETEALLCOLUMNS, (WPARAM)0, (LPARAM)0)
#define TLC_AddItem(lc, text, nitems)    \
  TuiSendMsg(lc, TLCM_ADDITEM, (WPARAM)nitems, (LPARAM)text)
#define TLC_DeleteItem(lc, idx)    \
  TuiSendMsg(lc, TLCM_DELETEITEM, (WPARAM)idx, (LPARAM)0)
#define TLC_DeleteAllItems(lc)    \
  TuiSendMsg(lc, TLCM_DELETEALLITEMS, (WPARAM)0, (LPARAM)0)  
#define TLC_SetItem(lc, flags, item)    \
  TuiSendMsg(lc, TLCM_SETITEMTEXT, (WPARAM)flags, (LPARAM)&subitem)
#define TLC_GetItem(lc, flags, item)    \
  TuiSendMsg(lc, TLCM_GETITEMTEXT, (WPARAM)flags, (LPARAM)&subitem);
  
#define TLC_GetItemCount(lc)    \
  TuiSendMsg(lc, TLCM_GETITEMCOUNT, (WPARAM)0, (LPARAM)0)
  
/*-------------------------------------------------------------------
 * window functions
 *-----------------------------------------------------------------*/
/*
 * TuiStartup()
 *   Start TUI environment
 */
LONG TuiStartup();
/*
 * TuiShutdown()
 *   End TUI environment
 */
void TuiShutdown();

/*
 * TuiGetEnv()
 *   Get TUI environment
 */
TENV TuiGetEnv();
VOID TuiSetNextMove(LONG nextmove);
UINT TuiGetDlgMsgID();

/* window message functions */
LONG TuiDefWndProc(TWND, UINT, WPARAM, LPARAM);
LONG TuiRegisterCls(LPCSTR clsname, TWNDPROC wndproc);
LONG TuiGetMsg(MSG* msg);
LONG TuiDispatchMsg(MSG* msg);
LONG TuiTranslateMsg(MSG* msg);
LONG TuiPostQuitMsg(LONG exitcode);
LONG TuiSendMsg(TWND, UINT, WPARAM, LPARAM);
LONG TuiPostMsg(TWND, UINT, WPARAM, LPARAM);

/* window functions */
TWND TuiCreateWnd(
  LPCSTR   clsname,
  LPCSTR   wndname,
  DWORD    style,
  INT      y,
  INT      x,
  INT      lines,
  INT      cols,
  TWND     parent,
  INT      id,
  LPVOID   param
);
TWND TuiCreateWndTempl(
  WNDTEMPL* templs,
  LPVOID    param
);
void TuiDestroyWnd(TWND);
TWND TuiGetActiveChildWnd(TWND);
TWND TuiGetFirstActiveChildWnd(TWND);
TWND TuiGetLastActiveChildWnd(TWND);
TWND TuiGetNextActiveChildWnd(TWND);
TWND TuiGetPrevActiveChildWnd(TWND);
TWND TuiGetActiveWnd();
TWND TuiGetFirstWnd();
TWND TuiGetLastWnd();
TWND TuiGetNextWnd(TWND wnd);
TWND TuiGetPrevWnd(TWND wnd);
LONG TuiInvalidateWnd(TWND);
LONG TuiShowWnd(TWND, LONG);
LONG TuiEnableWnd(TWND, LONG);
LONG TuiIsWndEnabled(TWND);
LONG TuiIsWndVisible(TWND);
TWND TuiGetWndItem(TWND, INT);
LONG TuiGetWndText(TWND, LPSTR, LONG);
VOID TuiSetWndText(TWND, LPCSTR);
TWND TuiGetParent(TWND);
LONG TuiSetFocus(TWND);
LONG TuiMoveWnd(TWND, INT, INT, INT, INT);
LONG TuiGetWndRect(TWND, RECT*);
DWORD TuiGetWndStyle(TWND);
DWORD TuiSetWndStyle(TWND, DWORD);
DWORD TuiGetWndTextAttrs(TWND);
DWORD TuiSetWndTextAttrs(TWND, DWORD);
UINT TuiGetWndID(TWND);
LPVOID TuiGetWndParam(TWND);
LPVOID TuiSetWndParam(TWND, LPVOID);
LONG   TuiIsWndValidate(TWND, LPCSTR);
VOID TuiSetWndValidate(TWND, LONG (*)(TWND, LPCSTR));

#define MB_INVALID           0x00000000
#define MB_CAPTION           0x00010000
#define MB_YES               0x00000001
#define MB_NO                0x00000002
#define MB_CANCEL            0x00000004
#define MB_YESNOCANCEL       (MB_YES|MB_NO|MB_CANCEL)
#define MB_YESNO             (MB_YES|MB_NO)

#define MB_OK                0x00000008
#define MB_OKCANCEL          (MB_OK|MB_CANCEL)

struct _MSGBOXSTRUCT
{
  TWND  owner;
  VOID* param;
  VOID  (*OnOK)(TWND, VOID*);
  VOID  (*OnYes)(TWND, VOID*);
  VOID  (*OnNo)(TWND, VOID*);
  VOID  (*OnCancel)(TWND, VOID*);
};
typedef struct _MSGBOXSTRUCT MSGBOXPARAM;

UINT TuiEndDlg(TWND, UINT);
LONG TuiMsgBox(TWND, LPCSTR, LPCSTR, UINT);
LONG TuiMsgBoxParam(TWND, LPCSTR, LPCSTR, UINT, MSGBOXPARAM*);
/* device context */
#define DT_LEFT              0x0001
#define DT_CENTER            0x0002
#define DT_RIGHT             0x0003

struct _TUIDEVICECONTEXSTRUCT
{
#ifdef __USE_CURSES__
  WINDOW*             win;
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  struct qio_fields*  win;
#endif
};

TDC  TuiGetDC(TWND);
LONG TuiDrawText(TDC, INT, INT, LPCSTR, DWORD);
LONG TuiDrawTextEx(TDC, INT, INT, INT, LPCSTR, LONG, DWORD, INT);
LONG TuiPutChar(TDC, INT, INT, CHAR, DWORD);
LONG TuiMoveYX(TDC, INT, INT);
LONG TuiGetYX(TDC, INT*, INT*);
LONG TuiPrintTextAlignment(LPSTR out, LPCSTR in, LONG limit, INT align);

/* simple macros */
#define MIN(a, b)    ((a) < (b) ? (a) : (b))
#define MAX(a, b)    ((a) > (b) ? (a) : (b))

#ifdef __cplusplus
}
#endif
#endif /*__TEXTUSERINTERFACE_H__*/

