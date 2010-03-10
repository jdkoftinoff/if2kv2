#ifndef __WNWINDOW_H
#define __WNWINDOW_H

#include "wnwlist.h"

extern HINSTANCE hInstance;
extern HINSTANCE hPrevInstance;
extern LPCSTR 	 lpCmdLine;
extern int    	 nCmdShow;

#define WMJK_CHILDDESTROYED 	(WM_USER+20)
#define WMJK_USER 		(WM_USER+100)





class WNWindow
{
	public:
	WNWindow( LPCSTR wn_class_name_ );
	virtual ~WNWindow();
	
	void	SetInitialX( int x );
	void	SetInitialY( int y );
	void	SetInitialW( int w );
	void	SetInitialH( int h );
	void	SetInitialWindowsParent( HWND w );
	void	SetInitialParent( WNWindow *parent_ );
	void 	SetInitialStyle( long style );
	void	SetInitialAccelerators( HACCEL a );
	void	SetInitialAccelerators( const char *accel_name );
	void	SetInitialHScrollBounds( int min_, int max_ );
	void	SetInitialVScrollBounds( int min_, int hax_ );
	void	SetInitialHScrollPos( int pos );
	void	SetInitialVScrollPos( int pos );
	void	SetInitialPixPerHLine( long pix );
	void	SetInitialPixPerVLine( long pix );
	void	SetInitialDocumentW( long w );
	void	SetInitialDocumentH( long h );
	
    virtual	bool	Create();
	
	bool	Update();
	bool	Show( int nCmdShow );
	bool	SetText( LPSTR txt );
	
	static	WNDPROC	GetWNDPROC();
	
	HWND	GetHWND();
	HWND	GetParentHWND();
	WNWindow *GetParent();

    operator HWND();

	static	bool    ProcessAllAccelerators( MSG *m );

	virtual	LPARAM 	DispatchMsg( UINT, WPARAM, LPARAM );

	protected:
	virtual	bool	ProcessAccelerator( MSG *m );

    virtual LPARAM  BnClickedMsg( int id, HWND but )   // BN_CLICKED
    {
        return 0;
    }
    
	virtual	LPARAM	DrawItemMsg(                         		// WM_DRAWITEM
								int idCtl,
								const DRAWITEMSTRUCT FAR * draw );

	virtual	LPARAM	CommandMsg( WPARAM cmd, HWND hCtl );		// WM_COMMAND
    virtual LPARAM  CharMsg( WPARAM k, LPARAM l ); // WM_CHAR
    virtual LPARAM  SysCharMsg( WPARAM k, LPARAM l ); // WM_SYSCHAR
    virtual LPARAM  NextDlgCtlMsg( WPARAM w, LPARAM l ); // WM_NEXTDLGCTL   
	virtual	LPARAM	CloseMsg();					// WM_CLOSE
	virtual	LPARAM	DestroyMsg();					// WM_DESTROY
	virtual	LPARAM	DropFilesMsg( HANDLE drop );			// WM_DROPFILES
	virtual	LPARAM	QueryEndSession();				// WM_QUERYENDSESSION
	virtual	LPARAM	PaintMsg( HDC dc );				// WM_PAINT

	virtual LPARAM  Activate() { return 0; }                    // WM_ACTIVATE (WA_ACTIVE)
    virtual LPARAM  DeActivate() { return 0; }                  // WM_ACTIVATE (WA_INACTIVE)
	virtual LPARAM  QueryNewPalette() { return 0; }              // WM_QUERYNEWPALETTE
	virtual LPARAM  PaletteChanged(HWND w) { return 0; }         // WM_PALETTECHANGED

	virtual	LPARAM	HScrollMsg( int code, int pos, HWND hCtl );	// WM_HSCROLL
	virtual	LPARAM	VScrollMsg( int code, int pos, HWND hCtl );	// WM_VSCROLL

	virtual	LPARAM	KeyDownMsg( WPARAM vk, LPARAM kd );		// WM_KEYDOWN
	virtual LPARAM	KeyUpMsg( WPARAM vk, LPARAM kd );		// WM_KEYUP

	virtual	LPARAM	MouseMoveMsg( int key, int x, int y );		// WM_MOUSEMOVE

	virtual	LPARAM	LButtonDownMsg( int key, int x, int y );	// WM_LBUTTONDOWN
	virtual	LPARAM	LButtonUpMsg( int key, WORD x, int y );		// WM_LBUTTONUP
	virtual	LPARAM	LButtonDblClkMsg( int key, int x, int y );	// WM_LBUTTONDBLCLK

	virtual	LPARAM	RButtonDownMsg( int key, int x, int y );	// WM_RBUTTONDOWN
	virtual	LPARAM	RButtonUpMsg( int key, WORD x, int y );		// WM_RBUTTONUP
	virtual	LPARAM	RButtonDblClkMsg( int key, int x, int y );	// WM_RBUTTONDBLCLK

	virtual	LPARAM	SizeMsg( int flag, int w, int h );		// WM_SIZE
	virtual	LPARAM	MoveMsg( int x, int y );			// WM_MOVE
	virtual	LPARAM	SetFocusMsg( HWND prev );			// WM_SETFOCUS
	virtual	LPARAM	CommNotifyMsg( int devid, int status );		// WM_COMMNOTIFY


	virtual	LPARAM	TimerMsg( int id );				// WM_TIMER

	virtual	LPARAM	UserMsg( UINT msg, WPARAM w, LPARAM l );	// WMJK_USER+x (x>1)

	virtual	LPARAM	ChildDestroyedMsg( WNWindow *child );		// WM_USER (WMJK_CHILDDESTROYED)

    virtual LPARAM  ShowWindowMsg( bool f, int status )
    {
        return 0;
    }

	virtual	LPARAM	DefaultHandler();

	virtual bool    RealizePalette()
	{
		 return true;
	}


	void	ChangeDocumentSize( long w, long h );
	void	ChangeHScrollBounds( int min_, int max_ );
	void	ChangeVScrollBounds( int min_, int max_ );

    HWND AddLabel( int x, int y, int w, int h, const char *text, DWORD options=0 )
    {
        HWND win=CreateWindow(
							"Static",
							text,
							WS_CHILD | options,
                            x,	y,
							w,	h,
							hWnd,
							0,
							hInstance,
							0
						);

        ShowWindow( win, SW_SHOW );
        return win;
    }

    HWND AddEdit( int x, int y, int w, int h, const char *text, DWORD options=0 )
    {
        HWND win=CreateWindow(
							"Edit",
							text,
							WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_LEFT | WS_CHILD | options,
                            x,	y,
							w,	h,
							hWnd,
							0,
							hInstance,
							0
						);

        ShowWindow( win, SW_SHOW );
        return win;
    }


    HWND AddButton( int x, int y, int w, int h, const char *text, int id, int options=0 )
    {
        HWND win=CreateWindow(
							"Button",
							text,
							WS_TABSTOP | BS_CENTER | WS_CHILD | options,
                            x,	y,
							w,	h,
							hWnd,
							(HMENU)id,
							hInstance,
							0
						);

        ShowWindow( win, SW_SHOW );
        return win;

    }



	UINT	cur_msg;
	WPARAM	cur_wParam;
	LPARAM	cur_lParam;
	HWND 	hWnd;
	bool	left_but_down;
	bool	right_but_down;
	LPCSTR	wn_class_name;
	HACCEL 	accelerators;
	WNWindow *parent;
	
	int	initial_x;
	int	initial_y;
	int	initial_w;
	int	initial_h;
	long	initial_style;
	HWND	initial_windows_parent;
	
	int	hscroll_min;
	int	hscroll_max;
	int	hscroll_pos;
	
	int	vscroll_min;
	int	vscroll_max;
	int	vscroll_pos;
	
	long 	pix_per_hline;
	long	pix_per_vline;
	int	lines_per_hpage;
	int	lines_per_vpage;
	
	long	origin_x;
	long	origin_y;
	
	long	document_w;
	long	document_h;
	
	WNWList	children;
	
	private:
	
	LPARAM	HandlePaint();
	
	static	WNWList	all_windows;
	static	WNWindow * cached;
 	
	friend 	LPARAM	CALLBACK WnWindowProc( HWND,UINT,WPARAM,LPARAM );
};



inline	WNDPROC	WNWindow::GetWNDPROC()
{
	return WnWindowProc;
}


inline	HWND	WNWindow::GetHWND()
{
	return hWnd;
}

inline	HWND	WNWindow::GetParentHWND()
{
 	return parent->GetHWND();
}

inline	WNWindow *WNWindow::GetParent()
{
	return parent;
}


inline	bool	WNWindow::Show( int nCmdShow )
{
	return ShowWindow( hWnd, nCmdShow )!=0;
}

inline	bool 	WNWindow::SetText( LPSTR txt )
{
	SendMessage( hWnd, WM_SETTEXT, 0, (LPARAM)txt );
	return true;
}

inline 	bool	WNWindow::Update()
{
	UpdateWindow( hWnd );
	return true;
}

inline	WNWindow::operator HWND()
{
	return hWnd;
}


inline	void	WNWindow::SetInitialX( int x )
{
	initial_x=x;
}

inline	void	WNWindow::SetInitialY( int y )
{
	initial_y=y;
}

inline	void	WNWindow::SetInitialW( int w )
{
	initial_w=w;
}

inline	void	WNWindow::SetInitialH( int h )
{
	initial_h=h;
}

inline	void	WNWindow::SetInitialStyle( long style )
{
 	initial_style=style;
}

inline	void	WNWindow::SetInitialParent( WNWindow *parent_ )
{
 	parent=parent_;
}

inline	void	WNWindow::SetInitialAccelerators( HACCEL a )
{
 	accelerators = a;
}


inline	void	WNWindow::SetInitialHScrollBounds( int min_, int max_ )
{
	hscroll_min = min_;
	hscroll_max = max_;
}

inline	void	WNWindow::SetInitialVScrollBounds( int min_, int max_ )
{
	vscroll_min = min_;
	vscroll_max = max_;
}

inline	void	WNWindow::SetInitialHScrollPos( int pos )
{
	hscroll_pos=pos;
}

inline	void	WNWindow::SetInitialVScrollPos( int pos )
{
	vscroll_pos=pos;
}



inline	void	WNWindow::SetInitialPixPerHLine( long pix )
{
	pix_per_hline = pix;
}

inline 	void	WNWindow::SetInitialPixPerVLine( long pix )
{
	pix_per_vline = pix;
}



inline	void	WNWindow::SetInitialDocumentW( long w )
{
	document_w = w;
}

inline 	void	WNWindow::SetInitialDocumentH( long h )
{
	document_h = h;
}


#endif


