#include "wnworld.h"

#if 0 // THis file is for reference only and doesnt work
#include "wnmain.h"
#include "wndialog.h"
#include "wnextdll.h"

#ifndef DEBUG_WNDIALOG
# define DEBUG_WNDIALOG	0
#endif

#if !DEBUG_WNDIALOG
# undef DBG
# define DBG(a)
#endif


#define BUTTONCLASS 	(0x80)
#define EDITCLASS       (0x81)
#define STATICCLASS 	(0x82)
#define LISTBOXCLASS    (0x83)
#define SCROLLBARCLASS  (0x84)
#define COMBOBOXCLASS   (0x85)

static char *builtin_class_names[] =
{
	"Button",
	"Edit",
	"Static",
	"ListBox",
	"ScrollBar",
	"ComboBox"
};


struct DLGTEMPLATE16
{
	LONG Style;
	BYTE ItemCount;
	int  X,
	Y,
	CX,
	CY;
};


struct DLGHDR
{
	LONG Style;
	BYTE ItemCount;
	int  X,
	Y,
	CX,
	CY;
	char MenuName[256],
	ClassName[256],
	CaptionText[256];
};


struct DLGFONTHDR
{
	short     PointSize;
	char szTypeFace[LF_FACESIZE];
};


struct DLGITEMTEMPLATE16
{
	int  X,
	Y,
	CX,
	CY,
	ID;
	LONG Style;
	char Class,
	Text;
	BYTE Info;
};

struct DLGITEM
{
	int  X,
	Y,
	CX,
	CY,
	ID;
	LONG Style;
	char Class[256],
	Text[256];
	BYTE Info;
	DWORD     Data;
};


static LPSTR GetDlgInfo(DLGTEMPLATE16 FAR *, DLGHDR * );
static LPSTR GetFontInfo(LPSTR, DLGFONTHDR * );
static LPSTR GetDlgItem(DLGITEMTEMPLATE16 FAR *, DLGITEM *);
static LPSTR MoveString(LPSTR, LPSTR, int);
static void SetControlFont(HWND hCtl, void *);

#define EXTRACT(a,c,d)	* (a *)(((char *)c)+d)

static LPSTR GetDlgInfo(DLGTEMPLATE16 FAR * DlgTemp, DLGHDR * pDlgHdr)
{
   	ENTER( "GetDlgInfo()" );
	
	LPSTR lpScan;
	
	pDlgHdr->Style = EXTRACT( LONG, DlgTemp, 0 );
	pDlgHdr->ItemCount = EXTRACT( BYTE, DlgTemp, 4 );
	pDlgHdr->X = EXTRACT( WORD, DlgTemp, 5 );
	pDlgHdr->Y = EXTRACT( WORD, DlgTemp, 7 );
	pDlgHdr->CX = EXTRACT( WORD, DlgTemp, 9 );
	pDlgHdr->CY = EXTRACT( WORD, DlgTemp, 11 );
	
	lpScan = LPSTR(DlgTemp) + 13;
	
	lpScan = MoveString(pDlgHdr->MenuName, lpScan,
						sizeof(pDlgHdr->MenuName));
	lpScan = MoveString(pDlgHdr->ClassName, lpScan,
						sizeof(pDlgHdr->ClassName));
	lpScan = MoveString(pDlgHdr->CaptionText, lpScan,
						sizeof(pDlgHdr->CaptionText));
	
	return lpScan;
	
} 

static LPSTR GetFontInfo( LPSTR DlgTemp, DLGFONTHDR * pFontHdr)
{
	ENTER( "GetFontInfo()" );
	
	LPSTR lpScan;
	pFontHdr->PointSize = short(*DlgTemp);
	lpScan = LPSTR(DlgTemp + sizeof(short));
	lpScan = MoveString(pFontHdr->szTypeFace, lpScan,
						sizeof(pFontHdr->szTypeFace));
	return lpScan;
} 

static LPSTR GetDlgItem(DLGITEMTEMPLATE16 FAR *lpItem, DLGITEM * pDlgItem)
{
	ENTER( "GetDlgItem()" );
	
	LPSTR lpScan;
	
	pDlgItem->X = lpItem->X;
	pDlgItem->Y = lpItem->Y;
	pDlgItem->CX = lpItem->CX;
	pDlgItem->CY = lpItem->CY;
	pDlgItem->ID = lpItem->ID;
	pDlgItem->Style = lpItem->Style;
	lpScan = LPSTR(&lpItem->Class);
	
	if(unsigned(*lpScan) & 0x80)
	{
		strcpy( pDlgItem->Class, builtin_class_names[unsigned(*lpScan) & 0x7f ] );
		
		lpScan++;
	}
	else
	  lpScan = MoveString(pDlgItem->Class, lpScan,
						  sizeof(pDlgItem->Class));
	
	lpScan = MoveString(pDlgItem->Text, lpScan,
						sizeof(pDlgItem->Text));
	pDlgItem->Info = BYTE(*lpScan);
	lpScan += sizeof(pDlgItem->Info);
	if(pDlgItem->Info)
	{
		pDlgItem->Data = DWORD(lpScan);
		lpScan += pDlgItem->Info;
	}
	
	return lpScan;
} 

static LPSTR MoveString(LPSTR pto, LPSTR lpfrom, int imax)
{
	ENTER( "MoveString()" );
	
	for(memset(pto, 0, imax);
		imax > 1 && *lpfrom && (*pto++ = *lpfrom++);
		imax--);
	
	while(*lpfrom++);
	return (lpfrom);
	
} 


#if 0
HFONT CGWindow::CreateFont(LPSTR lpszFaceName, int
						   nPointSize) {
	
	LOGFONT   lf;
	HDC       hDC;
	
	memset(&lf, 0, sizeof(LOGFONT));
	hDC = GetDC(NULL);
	lf.lfHeight = -MulDiv(nPointSize, GetDeviceCaps(hDC,
													LOGPIXELSY),        72);
	ReleaseDC(NULL, hDC);
	lf.lfWeight = FW_BOLD;
	lf.lfQuality = PROOF_QUALITY;
	lf.lfPitchAndFamily = FF_DONTCARE;
	strcpy(lf.lfFaceName, lpszFaceName);
	return CreateFontIndirect(&lf);
	
} // F: CGWindow::CreateFont

#endif



WNDialog::WNDialog(
				   LPCSTR rsrcname,
				   LPCSTR clsname
				   )
:
WNWindow( clsname ),
dlg_resource_name(rsrcname)
{
	initial_style=WSJK_DIALOG;
	saved_focus_item=0;
}


WNDialog::~WNDialog()
{
}


bool	WNDialog::Create()
{
	ENTER( "WNDialog::Create()" );
	
	DWORD base_units= GetDialogBaseUnits();
	WORD x_scale	= LOWORD( base_units );
	WORD y_scale 	= HIWORD( base_units );
	HRSRC r 	= FindResource( hInstance, dlg_resource_name, RT_DIALOG );
	HGLOBAL g 	= LoadResource( hInstance, r );
	LPSTR data 	= (LPSTR)LockResource(g);
	
	DLGFONTHDR * 	dlg_fonthead 	= new DLGFONTHDR;
	DLGHDR * 	dlg_head 	= new DLGHDR;
	DLGITEM * 	dlg_item 	= new DLGITEM;

	data = GetDlgInfo( (DLGTEMPLATE16 FAR *)data, dlg_head );
	
	
	if( dlg_head->Style & DS_SETFONT )
	{
		data = GetFontInfo( data, dlg_fonthead );
		
		
		// TO DO: call create font and get an HFONT (in dialog class)
	}
	
	dlg_w=dlg_head->CX*x_scale/4;
	dlg_h=dlg_head->CY*y_scale/8;
	
	if( initial_style & WS_HSCROLL )
	  dlg_w +=24;
	
	if( initial_style & WS_VSCROLL )
	  dlg_h +=24;
	
	if( initial_w == CW_USEDEFAULT )
	  initial_w = dlg_w;
	
	if( initial_h == CW_USEDEFAULT )
	  initial_h = dlg_h;
	
	SetInitialDocumentW(dlg_w);
    SetInitialDocumentH(dlg_h);
	
	int hscr=dlg_w-initial_w;
	int vscr=dlg_h-initial_h;
	
	if( hscr<0)
	  hscr=1;
	if( vscr<0)
	  vscr=1;
	
	SetInitialHScrollBounds( 0, hscr/pix_per_hline );
	SetInitialVScrollBounds( 0, vscr/pix_per_vline );
	
	WNWindow::Create();
	
	
	if( hWnd )
	{
		
		for( int i=0; i<dlg_head->ItemCount; ++i )
		{
			data = GetDlgItem( (DLGITEMTEMPLATE16 FAR *)data, dlg_item );
			
			
			HWND child=CreateWindow(
									dlg_item->Class,
									dlg_item->Text,
									dlg_item->Style,
									dlg_item->X*x_scale/4,
									dlg_item->Y*y_scale/8,
									dlg_item->CX*x_scale/4,
									dlg_item->CY*y_scale/8,
									hWnd,
									(HMENU)dlg_item->ID,
									hInstance,
									0
									);
			
			if( !child )
			{
				break;
			}
			
			dialog_items.Add( child, dlg_item->ID );
			
			
			// TO DO: Set the font for each item as well
		}		
	}
	else
	{
		DBGOUT( "Error creating window\n" );
	}
	// TO DO: position the window from parent
	// TO DO: size the window
	
	UnlockResource(g);
	FreeResource(g);
	
	delete dlg_head;
	delete dlg_item;
	delete dlg_fonthead;
	return true;
}

LPARAM	WNDialog::KeyDownMsg( WPARAM vk, LPARAM kd )		// WM_KEYDOWN
{
	ENTER( "WNDialog::KeyDownMsg()" );
	return WNWindow::KeyDownMsg(vk,kd);
}



#endif

