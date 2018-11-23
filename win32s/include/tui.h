/*-------------------------------------------------------------------
 * File name: tui.h
 * Author: Seree Rakwong
 * Date: 28-SEP-18
 *-----------------------------------------------------------------*/
#ifndef __TEXTUSERTINTERFACE_H__
#define __TEXTUSERTINTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------
 * defines
 *-----------------------------------------------------------------*/
#define TUI_CONTINUE        1
#define TUI_OK              0
#define TUI_ERROR          -1
#define TUI_MEM            -2

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

typedef long                TWPARAM;
typedef long                TLPARAM;
typedef long                TLONG;
typedef int                 TINT;
typedef void                TVOID;
typedef void*               TLPVOID;
typedef const char*         TLPCSTR;
typedef char*               TLPSTR;
typedef char                TTCHAR;
typedef double              TDOUBLE;
  
typedef unsigned long       TDWORD;
typedef unsigned int        TUINT;
  
typedef char                TINT8;
typedef unsigned char       TUINT8;
typedef short               TINT16;
typedef unsigned short      TUINT16;
typedef int                 TINT32;
typedef unsigned int        TUINT32;
typedef long long           TINT64;
typedef unsigned long long  TUINT64;

typedef char                TBOOL;


#define _TRUE_              (TBOOL)0
#define _FALSE_             (TBOOL)1


/* device context */
struct _TUIDEVICECONTEXSTRUCT;
typedef struct _TUIDEVICECONTEXSTRUCT _TDC;
typedef struct _TUIDEVICECONTEXSTRUCT *TDC;

/* window handle */
struct _TUIWINDOWSTRUCT;
typedef struct _TUIWINDOWSTRUCT _TWND;
typedef struct _TUIWINDOWSTRUCT *TWND;

/* window procedure */
typedef TLONG (*TWNDPROC)(TWND, TUINT, TWPARAM, TLPARAM);
/* procedure type */
typedef TLONG (*VALIDATEPROC)(TWND, TLPCSTR);

/* environment */
struct _TUIENVSTRUCT;
typedef struct _TUIENVSTRUCT _TENV;
typedef struct _TUIENVSTRUCT *TENV;

/* message structure */
struct _TMSGSTRUCT
{
  TWND    wnd;         /* window handle  */
  TUINT   msg;         /* window message */
  TWPARAM wparam;      /* parameter      */
  TLPARAM lparam;      /* parameter      */
};
typedef struct _TMSGSTRUCT TMSG;

/* notify message structure */
struct _NMHDRSTRUCT
{
  TUINT id;            /* control id     */
  TUINT code;          /* notify code    */
  TWND  ctl;           /* control handle */
};
typedef struct _NMHDRSTRUCT TNMHDR;

/* window rectangle */
struct _TRECTSTRUCT
{
  TINT y;
  TINT x;
  TINT lines;
  TINT cols;
};
typedef struct _TRECTSTRUCT TRECT;

/* y,x position */
struct _POSSTRUCT
{
  TINT y;
  TINT x;
};
typedef struct _POSSTRUCT TPOS;

struct _DRAWITEMSTRUCT
{
  TINT  idx;
  TRECT rcitem;
};
typedef struct _DRAWITEMSTRUCT TDRAWITEM;

/* colors */
enum
{
  BLACK_RED,
  BLACK_GREEN,
  BLACK_YELLOW,
  BLACK_BLUE,
  BLACK_MAGENTA,
  BLACK_CYAN,
  BLACK_WHITE,
  RED_BLACK,
  RED_RED,
  RED_GREEN,
  RED_YELLOW,
  RED_BLUE,
  RED_MAGENTA,
  RED_CYAN,
  RED_WHITE,
  GREEN_BLACK,
  GREEN_RED,
  GREEN_GREEN,
  GREEN_YELLOW,
  GREEN_BLUE,
  GREEN_MAGENTA,
  GREEN_CYAN,
  GREEN_WHITE,
  YELLOW_BLACK,
  YELLOW_RED,
  YELLOW_GREEN,
  YELLOW_YELLOW,
  YELLOW_BLUE,
  YELLOW_MAGENTA,
  YELLOW_CYAN,
  YELLOW_WHITE,
  BLUE_BLACK,
  BLUE_RED,
  BLUE_GREEN,
  BLUE_YELLOW,
  BLUE_MAGENTA,
  BLUE_CYAN,
  BLUE_WHITE,
  MAGENTA_BLACK,
  MAGENTA_RED,
  MAGENTA_GREEN,
  MAGENTA_YELLOW,
  MAGENTA_BLUE,
  MAGENTA_CYAN,
  MAGENTA_WHITE,
  CYAN_BLACK,
  CYAN_RED,
  CYAN_GREEN,
  CYAN_YELLOW,
  CYAN_BLUE,
  CYAN_MAGENTA,
  CYAN_WHITE,
  WHITE_BLACK,
  WHITE_RED,
  WHITE_GREEN,
  WHITE_YELLOW,
  WHITE_BLUE,
  WHITE_MAGENTA,
  WHITE_CYAN,
  /* no color */
  COLOR_MAX
};

enum
{
  THEME_STANDARD = 0,
  THEME_LHS,
  THEME_ASP,
  THEME_CNS,
  THEME_KTZ,
  THEME_YUANTA,
  THEME_KSS,
  THEME_PST,
  THEME_MBKET,
  THEME_AIRA,
  THEME_ASL,
  THEME_MERCHANT,
  /* standard */
  THEME_LAST
};

enum /*THEME_STANDARD*/
{
  /* simple theme */
  COLOR_WNDTEXT = 0,
  COLOR_BTNENABLED,
  COLOR_BTNDISABLED,
  COLOR_BTNFOCUSED,
  COLOR_HDRTEXT,
  COLOR_EDTTEXT,
  COLOR_LBXTEXT,
  COLOR_WNDTITLE,
  COLOR_HIGHLIGHTED,
  /* last color */
  COLOR_LAST
};


#define TALIGN_LEFT              0
#define TALIGN_CENTER            1
#define TALIGN_RIGHT             2

#define TW_HIDE                 0
#define TW_SHOW                 1
#define TW_DISABLE              0
#define TW_ENABLE               1

/* window styles */
#define TWS_WINDOW              0x00000001
#define TWS_CHILD               0x00000002
#define TWS_VISIBLE             0x00000004
#define TWS_DISABLED            0x00000008
#define TWS_BORDER              0x00000010
#define TWS_LEFT                0x00000000             /* shared controls style */
#define TWS_CENTER              0x00010000
#define TWS_RIGHT               0x00020000

/* window messages */
#define TWM_FIRST               100
#define TWM_CREATE              (TWM_FIRST +    1)
#define TWM_DESTROY             (TWM_FIRST +    2)
#define TWM_INITDIALOG          (TWM_FIRST +    3) /* get when system initialized window frame */
#define TWM_PAINT               (TWM_FIRST +    4) /* paTINT window        */
#define TWM_SETFOCUS            (TWM_FIRST +    5) /* get focus           */
#define TWM_KILLFOCUS           (TWM_FIRST +    6) /* lost focus          */
#define TWM_KEYDOWN             (TWM_FIRST +    7) /* key down            */
#define TWM_KEYUP               (TWM_FIRST +    8) /* key up              */
#define TWM_CHAR                (TWM_FIRST +    9) /* character input     */
#define TWM_NOTIFY              (TWM_FIRST +   10) /* control sent notify to parent */
#define TWM_ERASEBK             (TWM_FIRST +   11) /* erase background    */
#define TWM_SETTEXT             (TWM_FIRST +   12) /* set window text     */
#define TWM_GETTEXT             (TWM_FIRST +   13) /* get window text     */
#define TWM_SETTEXTALIGN        (TWM_FIRST +   14) /* set text alignment  */
#define TWM_GETTEXTALIGN        (TWM_FIRST +   15) /* set text alignment  */
#define TWM_COMMAND             (TWM_FIRST +   16) /* control sent command to parent */
#define TWM_SETTEXTATTRS        (TWM_FIRST +   17) /* set text attributes */
#define TWM_GETTEXTATTRS        (TWM_FIRST +   18) /* set text attributes */
#define TWM_DRAWITEM            (TWM_FIRST +   19)
#define TWM_SETINFOTEXT         (TWM_FIRST +   20) /* set info text       */
#define TWM_GETINFOTEXT         (TWM_FIRST +   21) /* get info text       */
#define TWM_SHOW                (TWM_FIRST +   22) /* show window         */
#define TWM_ENABLE              (TWM_FIRST +   23) /* enable window       */
#define TWM_GETCURSOR           (TWM_FIRST +   24) /* get cursor          */
#define TWM_SETCURSOR           (TWM_FIRST +   25) /* set cursor          */
#define TWM_SHOWMSGBOX          (TWM_FIRST +  100) /* TuiDefFrameWndProc  */
#define TWM_SHOWINPUTBOX        (TWM_FIRST +  101) /* TuiDefFrameWndProc  */
#define TWM_SHOWLINEINPUTBOX    (TWM_FIRST +  102) /* TuiDefFrameWndProc  */
#define TWM_INITPAGE            (TWM_FIRST +  103) /* TuiDefFrameWndProc  */
#define TWM_SETCURPAGE          (TWM_FIRST +  104) /* TuiDefFrameWndProc  */
#define TWM_GETCURPAGE          (TWM_FIRST +  105) /* TuiDefFrameWndProc  */
#define TWM_SHOWPAGE            (TWM_FIRST +  106) /* TuiDefFrameWndProc  */

/* application user messages */
#define TWM_USER                1000
#define TFM_USER                2000               /* use defined notification */

/* notification control message */
#define TCN_FIRST               (TWM_USER +   50)
#define TSN_FIRST               (TWM_USER +  100)
#define TEN_FIRST               (TWM_USER +  150)
#define TLBN_FIRST              (TWM_USER +  200)
#define TBN_FIRST               (TWM_USER +  250)
#define TLCN_FIRST              (TWM_USER +  300)

/* display info */
#define TCN_DISPLAYINFO         (TCN_FIRST +   1) /* see also DISPLAYINFO */

/*-------------------------------------------------------------------
 * frame window
 *-----------------------------------------------------------------*/
struct _INITPAGESTRUCT
{
  TWND firstchild;
  TWND lastchild;
};
typedef struct _INITPAGESTRUCT _TINITPAGE;
typedef struct _INITPAGESTRUCT *TINITPAGE;

/* response from TWM_SHOWTMSGBOX, TWM_SHOWINPUTBOX, TWM_SHOWLINEINPUTBOX */
struct _RESPONSEMSGBOXSTRUCT
{
  TNMHDR   hdr;
  TTCHAR    text[TUI_MAX_WNDTEXT+1];
};
typedef struct _RESPONSEMSGBOXSTRUCT RESPONSEMSGBOX;
typedef struct _RESPONSEMSGBOXSTRUCT DISPLAYINFO;

struct _PAGECHANGESTRUCT
{
  TNMHDR   hdr;
  TINT     frompage;
  TINT     topage;
};

typedef struct _PAGECHANGESTRUCT PAGECHANGE;

#define TUI_MAX_NMHDR        sizeof(RESPONSEMSGBOX)

#define TMB_YES               0x00000001
#define TMB_NO                0x00000002
#define TMB_CANCEL            0x00000004
#define TMB_YESNOCANCEL       (TMB_YES|TMB_NO|TMB_CANCEL)
#define TMB_YESNO             (TMB_YES|TMB_NO)

#define TMB_OK                0x00000008
#define TMB_OKCANCEL          (TMB_OK|TMB_CANCEL)

/*
 * TuiShowMsgBox()
 *    Show message box
 */
TVOID TuiShowMsgBox(
  TWND    wnd,
  TUINT    id,               /* user defined TFM_USER + #no to handle this value in TWM_NOTIFY */
  TLPCSTR  caption,
  TLPCSTR  text,
  TUINT    flags,            /* combination from MB_XXX  */
  TINT     show);            /* TW_SHOW or TW_HIDE       */

/*
 * TuiShowInputBox()
 *    Show dynamically input box
 */
TVOID TuiShowInputBox(
  TWND    wnd,
  TUINT    id,
  TLPCSTR  caption,
  TLPCSTR  text,
  TUINT    flags,
  TINT     limit,            /* limit input    */
  TDWORD   edtstyle,         /* TES_XXX        */
  TLPCSTR  deftext,          /* default input  */
  TINT     show);

/*
 * TuiShowLineInputBox()
 *    Show dynamically input box but one line 1 character input
 */
TVOID TuiShowLineInputBox(
  TWND    wnd,
  TUINT    id,
  TINT     y,                /* y-position */
  TINT     x,                /* x-position */
  TLPCSTR  text,
  TLPCSTR  deftext,
  TLPCSTR  validch,          /* valid characters */
  TINT     align,            /* DT_CENTER or 0   */
  TINT     show);

#define TFN_PAGECHANGING        (TFM_USER +    1)
#define TFN_PAGECHANGED         (TFM_USER +    2)

TVOID TuiShowPage(
  TWND    wnd,
  TINT     npage,
  TINT     show
);

TVOID TuiSetCurPage(
  TWND    wnd,
  TINT     npage
);

TINT TuiGetCurPage(
  TWND    wnd
);

/*-------------------------------------------------------------------
 * static control
 *-----------------------------------------------------------------*/
#define TSTATIC                  "STATIC"
      
#define TSS_LEFT                TWS_LEFT
#define TSS_CENTER              TWS_CENTER
#define TSS_RIGHT               TWS_RIGHT
      
/*-------------------------------------------------------------------
 * edit control
 *-----------------------------------------------------------------*/
#define TEDITBOX                 "EDITBOX"

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
#define TES_AUTODECIMALCOMMA    0x02000000
#define TES_UNDERLINE           0x04000000
#define TES_A2Z                 0x08000000
#define TES_AUTOSUFFIX          0x10000000

#define TEM_LIMITTEXT           (TWM_USER  +    1)
#define TEM_SETPASSWDCHAR       (TWM_USER  +    2)
#define TEM_SHOWPASSWDCHAR      (TWM_USER  +    3)
#define TEM_SETDECWIDTH         (TWM_USER  +    4)
#define TEM_SETVALIDSTRING      (TWM_USER  +    5)
#define TEM_SETVALIDMINMAX      (TWM_USER  +    6)

/* edit notified message */
#define TEN_CHANGED             (TEN_FIRST  +   0)
#define TEN_SETFOCUS            (TEN_FIRST  +   1)
#define TEN_KILLFOCUS           (TEN_FIRST  +   2)

/* edit macros */
#define TEDT_LimitText(edt, lim)    \
  TuiSendMsg(edt, TEM_LIMITTEXT, (TWPARAM)lim, (TLPARAM)0)
#define TEDT_SetPasswdChar(edt, ch)    \
  TuiSendMsg(edt, TEM_SETPASSWDCHAR, (TWPARAM)ch, (TLPARAM)0)
#define TEDT_ShowPasswdChar(edt, show)    \
  TuiSendMsg(edt, TEM_SHOWPASSWDCHAR, (TWPARAM)show, (TLPARAM)0)
#define TEDT_SetDecimalWidth(edt, wid)    \
  TuiSendMsg(edt, TEM_SETDECWIDTH, (TWPARAM)wid, (TLPARAM)0)
#define TEDT_SetValidString(edt, val)    \
  TuiSendMsg(edt, TEM_SETVALIDSTRING, (TWPARAM)0, (TLPARAM)val)

struct _VALIDMINMAXSTRUCT
{
  TINT min;
  TINT max;
};
typedef struct _VALIDMINMAXSTRUCT VALIDMINMAX;
#define TEDT_SetValidMinMax(edt, mn, mx, on)    \
do {\
  VALIDMINMAX val; \
  val.min = mn; \
  val.max = mx; \
  TuiSendMsg(edt, TEM_SETVALIDMINMAX, \
    (TWPARAM)(on == TW_ENABLE ? TW_ENABLE : TW_DISABLE), (TLPARAM)&val); \
} while (0)



/*-------------------------------------------------------------------
 * listbox control
 *-----------------------------------------------------------------*/
#define TLISTBOX                 "LISTBOX"

#define TLB_OK                   TUI_OK
#define TLB_ERROR                TUI_ERROR
#define TLB_UNCHECKED            0
#define TLB_CHECKED              1

#define TLBS_LEFT               TWS_LEFT
#define TLBS_CENTER             TWS_CENTER
#define TLBS_RIGHT              TWS_RIGHT
#define TLBS_OWNERDRAW          0x00100000
#define TLBS_CHECKBOX           0x00200000
#define TLBS_RADIOBOX           0x00400000

#define TLBM_FIRST              (TWM_USER)
#define TLBM_ADDITEM            (TLBM_FIRST  +    1)
#define TLBM_DELETEITEM         (TLBM_FIRST  +    2)
#define TLBM_GETCURSEL          (TLBM_FIRST  +    3)
#define TLBM_SETCURSEL          (TLBM_FIRST  +    4)
#define TLBM_DELETEALLITEMS     (TLBM_FIRST  +    5)
#define TLBM_GETITEMCOUNT       (TLBM_FIRST  +    6)
#define TLBM_SETITEMDATA        (TLBM_FIRST  +    7)
#define TLBM_GETITEMDATA        (TLBM_FIRST  +    8)
#define TLBM_SETITEMTEXT        (TLBM_FIRST  +    9)
#define TLBM_GETITEMTEXT        (TLBM_FIRST  +   10)
#define TLBM_SETITEMCHECKED     (TLBM_FIRST  +   11)
#define TLBM_GETITEMCHECKED     (TLBM_FIRST  +   12)
#define TLBM_COUNTITEMCHECKED   (TLBM_FIRST  +   13)
#define TLBM_LAST               (TLBM_FIRST  +   50)

/* listbox notified message */
#define TLBN_SETFOCUS           (TLBN_FIRST  +    0)
#define TLBN_KILLFOCUS          (TLBN_FIRST  +    1)
#define TLBN_SELCHANGED         (TLBN_FIRST  +    2)

/* listbox macros */
#define TLB_AddItem(lbx, text)    \
  TuiSendMsg(lbx, TLBM_ADDITEM, (TWPARAM)0, (TLPARAM)text)
#define TLB_DeleteItem(lbx, idx)  \
  TuiSendMsg(lbx, TLBM_DELETEITEM, (TWPARAM)idx, (TLPARAM)0)
#define TLB_GetCurSel(lbx)        \
  TuiSendMsg(lbx, TLBM_GETCURSEL, (TWPARAM)0, (TLPARAM)0)
#define TLB_SetCurSel(lbx, idx)   \
  TuiSendMsg(lbx, TLBM_SETCURSEL, (TWPARAM)idx, (TLPARAM)0)
#define TLB_DeleteAllItems(lbx)   \
  TuiSendMsg(lbx, TLBM_DELETEALLITEMS, (TWPARAM)0, (TLPARAM)0)
#define TLB_GetItemCount(lbx)   \
  TuiSendMsg(lbx, TLBM_GETITEMCOUNT, (TWPARAM)0, (TLPARAM)0)
#define TLB_SetItemData(lbx, idx, data)   \
  TuiSendMsg(lbx, TLBM_SETITEMDATA, (TWPARAM)idx, (TLPARAM)data)
#define TLB_GetItemData(lbx, idx)   \
  (TLPVOID)TuiSendMsg(lbx, TLBM_GETITEMDATA, (TWPARAM)idx, (TLPARAM)0)
#define TLB_SetItemText(lbx, idx, text)   \
  TuiSendMsg(lbx, TLBM_SETITEMTEXT, (TWPARAM)idx, (TLPARAM)text)
#define TLB_GetItemText(lbx, idx, text)   \
  TuiSendMsg(lbx, TLBM_GETITEMTEXT, (TWPARAM)idx, (TLPARAM)text)
#define TLB_SetItemChecked(lbx, idx, check)   \
  TuiSendMsg(lbx, TLBM_SETITEMCHECKED, (TWPARAM)idx, (TLPARAM)check)
#define TLB_GetItemChecked(lbx, idx)   \
  TuiSendMsg(lbx, TLBM_GETITEMCHECKED, (TWPARAM)idx, (TLPARAM)0)
#define TLB_CountItemChecked(lbx)   \
  TuiSendMsg(lbx, TLBM_COUNTITEMCHECKED, (TWPARAM)0, (TLPARAM)0)

/*-------------------------------------------------------------------
 * button control
 *-----------------------------------------------------------------*/
#define TBUTTON                  "BUTTON"

#define TBS_RELEASED            0x0000
#define TBS_PRESSED             0x0001
#define TBS_FOCUSED             0x0002

#define TBM_PRESS               (TWM_USER  +    1)

#define TBN_SETFOCUS            (TBN_FIRST  +   0)
#define TBN_KILLFOCUS           (TBN_FIRST  +   1)

/*-------------------------------------------------------------------
 * listctrl control
 *-----------------------------------------------------------------*/
#define TLISTCTRL                "LISTCTRL"

#define TLCS_NOHEADER           0x00010000
#define TLCS_EDITABLE           0x00020000
#define TLCS_LINEEDIT           0x00040000
#define TLCS_NOSELECTION        0x00080000

#define TLCM_FIRST              (TWM_USER     +   0)
#define TLCM_ADDCOLUMN          (TLCM_FIRST   +   1)
#define TLCM_DELETECOLUMN       (TLCM_FIRST   +   2)
#define TLCM_DELETEALLCOLUMNS   (TLCM_FIRST   +   3)
#define TLCM_ADDITEM            (TLCM_FIRST   +   4)
#define TLCM_DELETEITEM         (TLCM_FIRST   +   5)
#define TLCM_DELETEALLITEMS     (TLCM_FIRST   +   6)
#define TLCM_SETITEM            (TLCM_FIRST   +   7)
#define TLCM_GETITEM            (TLCM_FIRST   +   8)
#define TLCM_GETITEMCOUNT       (TLCM_FIRST   +   9)
#define TLCM_INVALIDATEITEM     (TLCM_FIRST   +  10)
#define TLCM_SETEDITSTYLE       (TLCM_FIRST   +  11)
#define TLCM_GETEDITSTYLE       (TLCM_FIRST   +  12)
#define TLCM_GETCURROW          (TLCM_FIRST   +  13)
#define TLCM_SETCURROW          (TLCM_FIRST   +  14)
#define TLCM_SETCOLWIDTH        (TLCM_FIRST   +  15)
#define TLCM_GETEDITBOX         (TLCM_FIRST   +  16)
#define TLCM_SETCURPAGE         (TLCM_FIRST   +  17)
#define TLCM_GETCURPAGE         (TLCM_FIRST   +  18)
#define TLCM_GETITEMSPERPAGE    (TLCM_FIRST   +  19)
#define TLCM_LAST               (TLCM_FIRST   +  50)
/*
#define TLCN_FIRST              (TWM_USER +  300)
*/
/* listctrl states */
#define LCS_VIEW                0
#define LCS_BEGINEDIT           1
#define LCS_EDITING             2
#define LCS_ENDEDIT             3
#define LCS_BEGINMOVING         4
#define LCS_MOVINGCURSOR        5
#define LCS_ENDMOVING           6

#define TLCN_SETFOCUS           (TLCN_FIRST  +    0)
#define TLCN_KILLFOCUS          (TLCN_FIRST  +    1)
#define TLCN_SELCHANGED         (TLCN_FIRST  +    2)
#define TLCN_BEGINEDIT          (TLCN_FIRST  +    3)
#define TLCN_ENDEDITOK          (TLCN_FIRST  +    4)
#define TLCN_ENDEDITCANCEL      (TLCN_FIRST  +    5)
#define TLCN_BEGINMOVING        (TLCN_FIRST  +    6)
#define TLCN_ENDMOVING          (TLCN_FIRST  +    7)

#define LC_ENDEDITOK            1
#define LC_ENDEDITCANCEL        0

#define  LCFM_TEXT              0x0001
#define  LCFM_ATTRS             0x0002
#define  LCFM_DATA              0x0004
struct _SUBITEMSTRUCT
{
  TINT       col;      /* column index, zero based */
  TINT       idx;      /* row index, zero based    */
  TTCHAR*     text;
  TDWORD     attrs;    /* text attributes          */
  TVOID*     data;     /* user data                */
};
typedef struct _SUBITEMSTRUCT TSUBITEM;

struct _HEADERITEMSTRUCT
{
  TTCHAR*     caption;
  TINT       cols;
  TINT       align;      /* column alignment         */
  TDWORD     attrs;      /* header text attributes   */
  TDWORD     editstyle;  /* edit style, see TES_XXX  */
  TINT       decwidth;   /* TES_DECIMAL or TES_AUTODECIMALCOMMA, default 6 */
};
typedef struct _HEADERITEMSTRUCT THEADERITEM;

/* listctrl macros */
#define TLC_AddColumn(lc, text, width, al)    \
do {\
  THEADERITEM hdr; \
  hdr.caption = text; \
  hdr.cols    = width; \
  hdr.align   = al; \
  hdr.attrs   = TuiGetSysColor(COLOR_HDRTEXT); \
  hdr.editstyle = 0; \
  hdr.decwidth  = 2; \
  TuiSendMsg(lc, TLCM_ADDCOLUMN, (TWPARAM)0, (TLPARAM)&hdr); \
} while (0)

#define TLC_AddColumnEx(lc, text, width, al, at, es, dec)    \
do {\
  THEADERITEM hdr; \
  hdr.caption = text; \
  hdr.cols    = width; \
  hdr.align   = al; \
  hdr.attrs   = at; \
  hdr.editstyle = es; \
  hdr.decwidth  = dec; \
  TuiSendMsg(lc, TLCM_ADDCOLUMN, (TWPARAM)0, (TLPARAM)&hdr); \
} while (0)
  
#define TLC_DeleteColumn(lc, col)    \
  TuiSendMsg(lc, TLCM_DELETECOLUMN, (TWPARAM)col, (TLPARAM)0)
#define TLC_DeleteAllColumns(lc)    \
  TuiSendMsg(lc, TLCM_DELETEALLCOLUMNS, (TWPARAM)0, (TLPARAM)0)
#define TLC_AddItem(lc, text, nitems)    \
  TuiSendMsg(lc, TLCM_ADDITEM, (TWPARAM)nitems, (TLPARAM)text)
#define TLC_DeleteItem(lc, idx)    \
  TuiSendMsg(lc, TLCM_DELETEITEM, (TWPARAM)0, (TLPARAM)idx)
#define TLC_DeleteAllItems(lc)    \
  TuiSendMsg(lc, TLCM_DELETEALLITEMS, (TWPARAM)0, (TLPARAM)0)
#define TLC_SetItem(lc, flags, item)    \
  TuiSendMsg(lc, TLCM_SETITEM, (TWPARAM)flags, (TLPARAM)item)
#define TLC_GetItem(lc, flags, item)    \
  TuiSendMsg(lc, TLCM_GETITEM, (TWPARAM)flags, (TLPARAM)item)
#define TLC_GetItemCount(lc)    \
  TuiSendMsg(lc, TLCM_GETITEMCOUNT, (TWPARAM)0, (TLPARAM)0)
#define TLC_SetEditStyle(lc, col, es)    \
  TuiSendMsg(lc, TLCM_SETEDITSTYLE, (TWPARAM)col, (TLPARAM)es)
#define TLC_GetEditStyle(lc, col)    \
  (TDWORD)TuiSendMsg(lc, TLCM_GETEDITSTYLE, (TWPARAM)col, (TLPARAM)0)
#define TLC_GetCurRow(lc)    \
  TuiSendMsg(lc, TLCM_GETCURROW, (TWPARAM)0, (TLPARAM)0)
#define TLC_SetCurRow(lc, idx)    \
  TuiSendMsg(lc, TLCM_SETCURROW, (TWPARAM)0, (TLPARAM)idx)
#define TLC_SetCurPage(lc, pg)    \
  TuiSendMsg(lc, TLCM_SETCURPAGE, (TWPARAM)pg, (TLPARAM)0)
#define TLC_GetCurPage(lc)    \
  TuiSendMsg(lc, TLCM_GETCURPAGE, (TWPARAM)0, (TLPARAM)0)
#define TLC_GetItemsPerPage(lc)    \
  TuiSendMsg(lc, TLCM_GETITEMSPERPAGE, (TWPARAM)0, (TLPARAM)0)
#define TLC_GetEditBox(lc)    \
  (TWND)TuiSendMsg(lc, TLCM_GETEDITBOX, (TWPARAM)0, (TLPARAM)0)

/*-------------------------------------------------------------------
 * list page control
 * This class inherited from listctrl
 * but support only 3 columns
 *   - HEADER_CAPTION   : caption field (text)
 *   - HEADER_VALUE     : editable field
 *   - HEADER_SUFFIX    : optional field (text)
 *-----------------------------------------------------------------*/
#define TLISTPAGECTRL                "LISTPAGECTRL"

#define TLPCM_FIRST                 (TLCM_LAST)
#define TLPCM_ADDITEM               (TLCM_ADDITEM)
#define TLPCM_DELETEITEM            (TLCM_DELETEITEM)
#define TLPCM_DELETEALLITEMS        (TLCM_DELETEALLITEMS)
#define TLPCM_SETITEMTEXT           (TLCM_SETITEM)
#define TLPCM_GETITEMTEXT           (TLCM_GETITEM)
#define TLPCM_GETITEMCOUNT          (TLCM_GETITEMCOUNT)
#define TLPCM_GETCURROW             (TLCM_GETCURROW)
#define TLPCM_SETCURROW             (TLCM_SETCURROW)
#define TLPCM_SETEDITSTYLE          (TLCM_SETEDITSTYLE)
#define TLPCM_GETEDITSTYLE          (TLCM_GETEDITSTYLE)
#define TLPCM_SETCURPAGE            (TLCM_SETCURPAGE)
#define TLPCM_GETCURPAGE            (TLCM_GETCURPAGE)
#define TLPCM_GETITEMSPERPAGE       (TLCM_GETITEMSPERPAGE)
#define TLPCM_NEXTPAGE              (TLCM_NEXTPAGE)
#define TLPCM_ADDITEMEX             (TLPCM_FIRST    +     1)
#define TLPCM_LAST                  (TLPCM_FIRST    +    50)

#define HEADER_CAPTION  0
#define HEADER_VALUE    1
#define HEADER_SUFFIX   2

#define TLPC_AddItem(lc, text)    \
  TuiSendMsg(lc, TLPCM_ADDITEM, (TWPARAM)0, (TLPARAM)text)
#define TLPC_AddItemEx(lc, text, style)    \
  TuiSendMsg(lc, TLPCM_ADDITEMEX, (TWPARAM)style, (TLPARAM)text)
#define TLPC_DeleteItem(lc, idx)    \
  TuiSendMsg(lc, TLPCM_DELETEITEM, (TWPARAM)idx, (TLPARAM))
#define TLPC_DeleteAllItems(lc)    \
  TuiSendMsg(lc, TLPCM_DELETEALLITEMS, (TWPARAM)0, (TLPARAM)0)
#define TLPC_SetItemText(lc, indx, id, txt)  \
do {\
  SUBITEM item; \
  item.col   = id; \
  item.idx   = indx; \
  item.text  = txt;  \
  TuiSendMsg(lc, TLPCM_SETITEMTEXT, (TWPARAM)LCFM_TEXT, (TLPARAM)&item) \
} while (0)
  
#define TLPC_GetItemText(lc, indx, id, outtext)  \
do {\
  TSUBITEM item; \
  item.col   = id; \
  item.idx   = indx; \
  item.text  = outtext;  \
  TuiSendMsg(lc, TLPCM_GETITEMTEXT, (TWPARAM)LCFM_TEXT, (TLPARAM)&item) \
} while (0)

#define TLPC_GetItemCount(lc)  \
  TuiSendMsg(lc, TLPCM_GETITEMCOUNT, (TWPARAM)0, (TLPARAM)0)
#define TLPC_SetEditStyle(lc, idx, es)  \
  TuiSendMsg(lc, TLPCM_SETEDITSTYLE, (TWPARAM)idx, (TLPARAM)es)
#define TLPC_GetEditStyle(lc, idx)  \
  TuiSendMsg(lc, TLPCM_GETEDITSTYLE, (TWPARAM)idx, (TLPARAM)0)
#define TLPC_SetCurRow(lc, idx)    \
  TuiSendMsg(lc, TLPCM_SETCURROW, (TWPARAM)idx, (TLPARAM)0)
#define TLPC_GetCurRow(lc)    \
  TuiSendMsg(lc, TLPCM_GETCURROW, (TWPARAM)0, (TLPARAM)0)
#define TLPC_SetCurPage(lc, pg)    \
  TuiSendMsg(lc, TLPCM_SETCURPAGE, (TWPARAM)pg, (TLPARAM)0)
#define TLPC_GetCurPage(lc)    \
  TuiSendMsg(lc, TLPCM_GETCURPAGE, (TWPARAM)0, (TLPARAM)0)
#define TLPC_GetItemsPerPage(lc)    \
  TuiSendMsg(lc, TLPCM_GETITEMSPERPAGE, (TWPARAM)0, (TLPARAM)0)

/*-------------------------------------------------------------------
 * scroll view
 *-----------------------------------------------------------------*/
/*
#define SCROLLVIEW                 "SCROLLVIEW"

#define TSVM_INSERTBAND            (TWM_USER  +    1)
*/
/*-------------------------------------------------------------------
 * window functions
 *-----------------------------------------------------------------*/
/*
 * TuiStartup()
 *   Start TUI environment
 */
TLONG TuiStartup();
/*
 * TuiShutdown()
 *   End TUI environment
 */
TVOID TuiShutdown();

/*
 * TuiGetEnv()
 *   Get TUI environment
 */
TENV TuiGetEnv();
/*
 * TuiSetNextMove()
 *   Set next key to move to the next control
 *   Default TVK_ENTER (ENTER)
 */
TLONG TuiSetNextMove(TLONG nextmove);
/*
 * TuiSetPrevMove()
 *   Set previous key to move to the previous control
 *   Default KEY_BTAB (Shift + TAB)
 */
TLONG TuiSetPrevMove(TLONG prevmove);
/*
 * TuiGetSysColor()
 *   Get system color
 *   see also, COLOR_XXX
 */
TDWORD TuiGetSysColor(TINT clridx);
/*
 * TuiGetReverseSysColor()
 *   Get system color and reverse it
 *   see also, COLOR_XXX
 */
TDWORD TuiGetReverseSysColor(TINT clridx);
/*
 * TuiGetColor()
 *   Directly get color from index
 *   see also, BLACK_XXX, RED_XXX, and so on.
 */
TDWORD TuiGetColor(TINT clridx);
/*
 * TuiReverseColor()
 *   Directly reverse color and reverse it
 *   see also, COLOR_XXX
 */
TDWORD TuiReverseColor(TDWORD clr);
/*
 * TuiUnderlineText()
 *   Make underline attrinute
 */
TDWORD TuiUnderlineText(TDWORD clr);
/*
 * TuiGetSysColorTheme()
 *   Get system color from theme
 *   see also, COLOR_XXX for clridx
 *             THEME_XXX for themeidx
 */
TDWORD TuiGetSysColorTheme(TINT themeidx, TINT clridx);
/*
 * TuiGetReverseSysColorTheme()
 *   Get reversed system color from theme
 *   see also, COLOR_XXX for clridx
 *             THEME_XXX for themeidx
 */
TDWORD TuiGetReverseSysColorTheme(TINT themeidx, TINT clridx);
/*
 * TuiGetTheme()
 *   Get current theme index, default is THEME_STANDARD
 */
TINT   TuiGetTheme();
/*
 * TuiSetTheme()
 *   Set theme index, return previous theme
 */
TINT   TuiSetTheme(TINT themeidx);

/* window message functions */
/*
 * TuiGetChar()
 *   Synchronously get character input from screen
 */
TLONG  TuiGetChar();
/*
 * TuiDefWndProc()
 *   All windows MUST be called at its window procedure.
 *   wnd - window handle
 *   msg - window message
 *   TWPARAM - first parameter
 *   TLPARAM - second parameter
 */
TLONG TuiDefWndProc(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam);
/*
 * TuiRegisterCls()
 *   All window class MUST be registered before creating its window instance
 *   clsname - a unique name in an application
 *   wndproc - window procedure
 */
TLONG TuiRegisterCls(TLPCSTR clsname, TWNDPROC wndproc);
/*
 * TuiGetMsg()
 *   Synchronously get message input from screen
 *   msg - output from user input
 */
TLONG TuiGetMsg(TMSG* msg);
/*
 * TuiDispatchMsg()
 *   Dispatch message from application and user input
 *   msg - input/output from user input
 */
TLONG TuiDispatchMsg(TMSG* msg);
/*
 * TuiTranslateMsg()
 *   Translate message from the application and user input
 *   msg - input/output from user input
 */
TLONG TuiTranslateMsg(TMSG* msg);
/*
 * TuiPostQuitMsg()
 *   Post message to quit an application
 */
TLONG TuiPostQuitMsg(TLONG exitcode);
/*
 * TuiSendMsg()
 *   Synchronously send message to window handle
 */
TLONG TuiSendMsg(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam);
/*
 * TuiPostMsg()
 *   Asynchronously send message to window handle
 * The message will be dispatched once the application call TuiDispatchMsg()
 *
 * This function useful when the application does not need to wait response immediately
 */
TLONG TuiPostMsg(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam);

/* frame window */
#define TIDOK           10001
#define TIDYES          10002
#define TIDNO           10003
#define TIDCANCEL       10004
#define TIDINPUTBOX     10005

/*
 * TuiDefFrameWndProc()
 *   This inherits from TuiDefWndProc() but it is used by main window
 *   wnd - window handle
 *   msg - window message
 *   TWPARAM - first parameter
 *   TLPARAM - second parameter
 */
TLONG TuiDefFrameWndProc(TWND wnd, TUINT msg, TWPARAM wparam, TLPARAM lparam);

/* window functions */
/*
 * TuiCreateWnd()
 *   Create a window object
 */
TWND TuiCreateWnd(
  TLPCSTR   clsname,       /* class name must be registered before calling this function */
  TLPCSTR   wndname,       /* window name */
  TDWORD    style,         /* window style, see TWS_XXX */
  TINT      y,             /* y-position       */
  TINT      x,             /* x-position       */
  TINT      lines,         /* window lines     */
  TINT      cols,          /* window columns   */
  TWND      parent,        /* parent of window */
  TINT      id,            /* window id        */
  TLPVOID   param          /* user parameter defined */
);
/*
 * TuiCreateWnd()
 *   Create a window object
 */
TWND TuiCreateWndEx(
  TLPCSTR    clsname,       /* class name must be registered before calling this function */
  TLPCSTR    wndname,       /* window name */
  TDWORD    style,         /* window style, see TWS_XXX */
  TDWORD    exstyle,       /* window style, see TWS_EXxxx */
  TINT      y,             /* y-position       */
  TINT      x,             /* x-position       */
  TINT      lines,         /* window lines     */
  TINT      cols,          /* window columns   */
  TWND      parent,        /* parent of window */
  TINT      id,            /* window id        */
  TLPCSTR    infotext,      /* window info */
  TLPVOID    param          /* user parameter defined */
);
/* window template */
struct _WNDTEMPLSTRUCT
{
  TLPCSTR  clsname;     /* window class   */
  TLPCSTR  text;        /* window text    */
  TINT     id;          /* window id      */
  TINT     y;           /* y-position     */
  TINT     x;           /* x-position     */
  TINT     lines;       /* window lines   */
  TINT     cols;        /* window columns */
  TDWORD   style;       /* window style   */
  VALIDATEPROC validateproc;  /* to validate value before exiting */
                              /* the edit box return TUI_CONTINUE */
                              /* if 2nd parameter is accepted     */
                              /* otherwise return TUI_ERROR       */
};
typedef struct _WNDTEMPLSTRUCT WNDTEMPL;

struct _FRMWNDTEMPLSTRUCT
{
  TLPCSTR  clsname;     /* window class   */
  TLPCSTR  text;        /* window text    */
  TINT     id;          /* window id      */
  TINT     y;           /* y-position     */
  TINT     x;           /* x-position     */
  TINT     lines;       /* window lines   */
  TINT     cols;        /* window columns */
  TDWORD   style;       /* window style   */
  TDWORD   exstyle;     /* window style   */
  VALIDATEPROC validateproc;  /* to validate value before exiting */
                              /* the edit box return TUI_CONTINUE */
                              /* if 2nd parameter is accepted     */
                              /* otherwise return TUI_ERROR       */
  TLPCSTR  infotext;   /* to display information in a      */
                      /* specific control                 */
};
typedef struct _FRMWNDTEMPLSTRUCT FRMWNDTEMPL;

/*
 * TuiCreateWndTempl()
 *   Create a window object from template
 * NB, the first object created must be a window frame
 *     the last object MUST be all nil values
 */
TWND TuiCreateWndTempl(
  WNDTEMPL* templs,
  TLPVOID    param
);

/*
 * TuiCreateFrameWnd()
 *   Create a window object from template
 * NB, as same as TuiCreateWndTempl(), but the first object must be
 *     its child.
 *     the last object MUST be all nil values
 */
TWND TuiCreateFrameWnd(
  TLPCSTR   clsname,
  TLPCSTR   wndname,
  TDWORD    style,
  TINT      y,
  TINT      x,
  TINT      lines,
  TINT      cols,
  WNDTEMPL* templs,
  TLPVOID    param
);

/*
 * TuiFrameWndCreatePage()
 *   Create page control object
 */
TLONG TuiFrameWndCreatePage(
  TWND      wnd,
  WNDTEMPL* templs,
  TLPVOID    param
);

/*
 * TuiCreateFrameWndEx()
 *   Create a window object from template
 * NB, as same as TuiCreateWndTempl(), but the first object must be
 *     its child.
 *     the last object MUST be all nil values
 */
TWND TuiCreateFrameWndEx(
  TLPCSTR   clsname,
  TLPCSTR   wndname,
  TDWORD    style,
  TDWORD    exstyle,
  TINT      y,
  TINT      x,
  TINT      lines,
  TINT      cols,
  FRMWNDTEMPL* templs,
  TLPVOID    param
);

/*
 * TuiFrameWndCreatePage()
 *   Create page control object
 */
TLONG TuiFrameWndCreatePageEx(
  TWND      wnd,
  FRMWNDTEMPL* templs,
  TLPVOID    param
);

/*
 * TuiDestroyWnd()
 *   Destroy window object
 * This function will destroy all children that
 * attached on wnd object, otherwise the application must be
 * destroy by self
 */
TVOID   TuiDestroyWnd(TWND wnd);
/*
 * TuiGetActiveChildWnd()
 *   Get the current active child of window
 */
TWND   TuiGetActiveChildWnd(TWND wnd);
/*
 * TuiGetFirstActiveChildWnd()
 *   Get the first child of window that could be got a focus
 */
TWND   TuiGetFirstActiveChildWnd(TWND wnd);
/*
 * TuiGetLastActiveChildWnd()
 *   Get the last child of window that could be got a focus
 */
TWND   TuiGetLastActiveChildWnd(TWND wnd);
/*
 * TuiGetNextActiveChildWnd()
 *   Get the next child of window that could be got a focus
 */
TWND   TuiGetNextActiveChildWnd(TWND wnd);
/*
 * TuiGetPrevActiveChildWnd()
 *   Get the previous child of window that could be got a focus
 */
TWND   TuiGetPrevActiveChildWnd(TWND wnd);
/*
 * TuiGetFirstChildWnd()
 *   Get the first child of window
 */
TWND   TuiGetFirstChildWnd(TWND wnd);
/*
 * TuiGetLastChildWnd()
 *   Get the last child of window
 */
TWND   TuiGetLastChildWnd(TWND wnd);
/*
 * TuiGetActiveWnd()
 *   System will return the active window frame
 */
TWND   TuiGetActiveWnd();
/*
 * TuiGetFirstWnd()
 *   System will return the first window frame
 */
TWND   TuiGetFirstWnd();
/*
 * TuiGetLastWnd()
 *   System will return the last window frame
 */
TWND   TuiGetLastWnd();
/*
 * TuiGetNextWnd()
 *   Return the next window from wnd object
 */
TWND   TuiGetNextWnd(TWND wnd);
/*
 * TuiGetPrevWnd()
 *   Return the previous window from wnd object
 */
TWND   TuiGetPrevWnd(TWND wnd);
/*
 * TuiInvalidateWnd()
 *   Call this to redraw the window object
 * NB. The application MUST NOT send TWM_PATINT directly to window object
 */
TLONG   TuiInvalidateWnd(TWND wnd);
/*
 * TuiInvalidateWndRect()
 *   Call this to redraw the window object
 * NB. The application MUST NOT send TWM_PATINT directly to window object
 */
TLONG   TuiInvalidateWndRect(TWND wnd, TRECT* rect);
/*
 * TuiShowWnd()
 *   Show window object
 *   show - TW_SHOW or TW_HIDE
 */
TLONG   TuiShowWnd(TWND wnd, TLONG show);
/*
 * TuiEnableWnd()
 *   Show window object
 *   show - TW_ENABLE or TW_DISABLE
 */
TLONG   TuiEnableWnd(TWND wnd, TLONG enable);
/*
 * TuiIsWndEnabled()
 *   Check if window object is enabled
 */
TLONG   TuiIsWndEnabled(TWND wnd);
/*
 * TuiIsWndVisible()
 *   Check if window object is visible
 */
TLONG   TuiIsWndVisible(TWND wnd);
/*
 * TuiGetWndItem()
 *   Get child control window
 */
TWND   TuiGetWndItem(TWND wnd, TINT ctlid);
/*
 * TuiGetWndText()
 *   Get window text
 *   outtext - output
 *   outlen  - maximum output
 * NB. Limitation of this library is the max window length is TUI_MAX_WNDTEXT
 */
TLONG   TuiGetWndText(TWND wnd, TLPSTR outtext, TLONG outlen);
/*
 * TuiSetWndText()
 *   Set window text
 * NB. Limitation of this library is the max window length is TUI_MAX_WNDTEXT
 */
TVOID   TuiSetWndText(TWND wnd, TLPCSTR text);
/*
 * TuiGetWndInfoText()
 *   Get window text
 *   outtext - output
 *   outlen  - maximum output
 * NB. Limitation of this library is the max window length is TUI_MAX_WNDTEXT
 */
TLONG   TuiGetWndInfoText(TWND wnd, TLPSTR outtext, TLONG outlen);
/*
 * TuiSetWndInfoText()
 *   Set window text
 * NB. Limitation of this library is the max window length is TUI_MAX_WNDTEXT
 */
TVOID   TuiSetWndInfoText(TWND wnd, TLPCSTR text);
/*
 * TuiGetParent()
 *   Get window parent
 */
TWND   TuiGetParent(TWND wnd);
/*
 * TuiSetFocus()
 *   Set window object to get focused
 */
TLONG   TuiSetFocus(TWND wnd);
/*
 * TuiGetFocus()
 *   Get the current active child window
 */
TWND   TuiGetFocus(TWND wnd);
/*
 * TuiMoveWnd()
 *   Move window object
 */
TLONG   TuiMoveWnd(TWND wnd, TINT y, TINT x, TINT lines, TINT cols);

/*
 * TuiGetWndClsName()
 *   Get window class name
 */
TLONG   TuiGetWndClsName(TWND wnd, TLPSTR clsname, TLONG cb);
/*
 * TuiGetWndRect()
 *   Get window rectangle
 */
TLONG   TuiGetWndRect(TWND wnd, TRECT* rect);
/*
 * TuiGetCursorPos()
 *   Get cursor position
 */
TLONG   TuiGetCursorPos(TWND wnd, TPOS* pos);
/*
 * TuiSetCursorPos()
 *   Set cursor position
 */
TLONG   TuiSetCursorPos(TWND wnd, TPOS* pos);

/*
 * TuiGetWndStyle()
 *   Get window styles
 */
TDWORD  TuiGetWndStyle(TWND wnd);
/*
 * TuiSetWndStyle()
 *   Set window styles
 */
TDWORD  TuiSetWndStyle(TWND wnd, TDWORD styles);
/*
 * TuiGetWndStyleEx()
 *   Get window extended styles
 */
TDWORD  TuiGetWndStyleEx(TWND wnd);
/*
 * TuiSetWndStyleEx()
 *   Set window extended styles
 */
TDWORD  TuiSetWndStyleEx(TWND wnd, TDWORD exstyles);
/*
 * TuiGetWndTextAttrs()
 *   Get window text attributes
 */
TDWORD  TuiGetWndTextAttrs(TWND wnd);
/*
 * TuiSetWndTextAttrs()
 *   Set window text attributes
 */
TDWORD  TuiSetWndTextAttrs(TWND wnd, TDWORD attrs);
/*
 * TuiGetWndID()
 *   Get window id
 */
TUINT   TuiGetWndID(TWND wnd);
/*
 * TuiGetWndParam()
 *   Get window user defined object
 */
TLPVOID TuiGetWndParam(TWND wnd);
/*
 * TuiSetWndParam()
 *   Set window user defined object
 */
TLPVOID TuiSetWndParam(TWND wnd, TLPVOID);
/*
 * TuiIsWndValidate()
 *   Check if text is required validation
 * return TUI_CONTINUE if it is OK
 * otherwise return TUI_ERROR
 */
TLONG   TuiIsWndValidate(TWND wnd, TLPCSTR text);
/*
 * TuiSetWndValidateProc()
 *   Set window validate function
 */
VALIDATEPROC   TuiSetWndValidateProc(TWND wnd, VALIDATEPROC validateproc);
/*
 * TuiSetWndShowInfoProc()
 *   Set window validate function
 */
/* 
SHOWINFOPROC   TuiSetWndShowInfoProc(TWND wnd, SHOWINFOPROC showinfoproc);
*/
/* device context */
#define TDT_LEFT              0x0001
#define TDT_CENTER            0x0002
#define TDT_RIGHT             0x0003


#ifdef __USE_QIO__
/* I/O status block */
struct _TIOSBSTRUCT
{
  TUINT16 status;
  TUINT16 offset;
  TUINT16 keycode;
  TUINT16 size;
};
typedef struct _TIOSBSTRUCT TIOSB;

struct _TQIOSTRUCT
{
  TIOSB       ttiosb;
  TUINT16      ttchan;
};
typedef struct _TQIOSTRUCT _TQIO;
typedef struct _TQIOSTRUCT *TQIO;

#endif /*__USE_QIO__*/

struct _TUIDEVICECONTEXSTRUCT
{
#ifdef __USE_CURSES__
  WINDOW*     win;
#elif defined __USE_QIO__
  /* QIO for VMS implement here */
  _TQIO*      win;
#elif defined __USE_WIN32__
  TVOID*      win;
  TVOID*      wout;
#endif
};

/*
 * TuiPrintTextAlignment()
 *   Helper function to prTINT text TINTo the box
 *   align - ALIGN_CENTER, ALIGN_LEFT(default), ALIGN_RIGHT
 */
TLONG TuiPrintTextAlignment(TLPSTR out, TLPCSTR in, TLONG limit, TINT align);
/*
 * TuiGetDC()
 *   Get window device context
 */
TDC  TuiGetDC(TWND wnd);
/*
 * TuiDrawText()
 *   Draw a string on screen
 */
TLONG TuiDrawText(TDC dc, TINT y, TINT x, TLPCSTR text, TDWORD attrs);
/*
 * TuiPutChar()
 *   Draw a character on screen
 */
TLONG TuiPutChar(TDC dc, TINT y, TINT x, TTCHAR ch, TDWORD attrs);
/*
 * TuiMoveYX()
 *   Move cursor to (y, x)
 */
TLONG TuiMoveYX(TDC dc, TINT, TINT);
/*
 * TuiGetYX()
 *   Get cursor position
 */
TLONG TuiGetYX(TDC dc, TINT*, TINT*);
/*
 * TuiDrawBorder()
 *   Draw border of rectangle with caption
 */
TLONG TuiDrawBorder(TDC dc, TRECT* rcwnd);
/*
 * TuiDrawBorderEx()
 *   Draw border of rectangle with caption
 */
TLONG TuiDrawBorderEx(TDC dc, TRECT* rcwnd, TDWORD attrs);
/*
 * TuiDrawFrame()
 *   Draw border of rectangle without caption
 */
TLONG TuiDrawFrame(TDC dc, TRECT* rcframe, TLPCSTR caption, TDWORD attrs);
/*
 * TuiDrawHLine()
 *   Draw border of rectangle without caption
 */
TLONG TuiDrawHLine(TDC dc, TINT y, TINT x, TINT nchars, TDWORD attrs);
/*
 * TuiDrawVLine()
 *   Draw border of rectangle without caption
 */
TLONG TuiDrawVLine(TDC dc, TINT y, TINT x, TINT nchars, TDWORD attrs);
/*
 * TuiDrawMultipleFrames()
 *   Draw multiple borders of rectangle without caption
 */
TLONG TuiDrawMultipleFrames(TDC dc, TRECT* rcframe, TLPCSTR caption, TDWORD attrs, TINT* widths);
/* simple macros */
#define MIN(a, b)    ((a) < (b) ? (a) : (b))
#define MAX(a, b)    ((a) > (b) ? (a) : (b))

#ifdef __VMS__
TINT sprTINTf(char *str, const char *format, ...);
#endif

#ifdef __cplusplus
}
#endif
#endif /*__TEXTUSERTINTERFACE_H__*/

