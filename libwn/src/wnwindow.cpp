#include "wnworld.h"


#include "wnwindow.h"
#include "wnmain.h"
#include "wndc.h"

#ifndef DEBUG_WNWINDOW
# define DEBUG_WNWINDOW 1
#endif

#if !DEBUG_WNWINDOW
# undef DBG
# define DBG(a)
#endif


WNWList WNWindow::all_windows;
WNWindow * WNWindow::cached=0;



LPARAM	CALLBACK WnWindowProc(
							  HWND hWnd,
							  UINT m,
							  WPARAM w,
							  LPARAM l
							  )
{
	//ENTER( "WNWindow::WnWindowProc()" );
	
	if( WNWindow::cached && hWnd == WNWindow::cached->GetHWND() )
	{
		return WNWindow::cached->DispatchMsg( m,w,l );
	}
	else
	{
		WNWindow::cached = WNWindow::all_windows.Find( hWnd );
		
		if( WNWindow::cached )
		{
			return WNWindow::cached->DispatchMsg( m,w,l );
        }
	}
	
	// cant find the window in our list. call DefWindowProc()
	
	return DefWindowProc(hWnd,m,w,l);
}


WNWindow::WNWindow( LPCSTR wn_class_name_ )
{
	ENTER( "WNWindow::WNWindow()" );
	
	all_windows.Add( this, 0 );
	
	left_but_down=false;
	right_but_down=false;
	wn_class_name=wn_class_name_;
	initial_x=CW_USEDEFAULT;
	initial_y=CW_USEDEFAULT;
	initial_w=CW_USEDEFAULT;
	initial_h=CW_USEDEFAULT;
	parent=0;
	initial_windows_parent=HWND_DESKTOP;
	initial_style=WS_OVERLAPPED;
	accelerators=(HACCEL)0;
	hWnd=0;
	
	hscroll_min=0;
	hscroll_max=4096;
	hscroll_pos=0;
	vscroll_min=0;
	vscroll_max=4096;
	vscroll_pos=0;
	
	pix_per_hline = 20;
	pix_per_vline = 20;
	document_w=1024;
	document_h=1024;
	lines_per_hpage = 10;
	lines_per_vpage = 10;
	
	origin_x=0;
	origin_y=0;
	
}

WNWindow::~WNWindow()
{
	ENTER( "WNWindow::~WNWindow()" );
	
	
	all_windows.Remove( this );
}


bool WNWindow::ProcessAllAccelerators( MSG *m )
{
	WNWindow *w = all_windows.Find( m->hwnd );
	HWND evw = m->hwnd;
	while( !w && evw )
	{		
		evw=::GetParent(evw);
		w = all_windows.Find( evw );
	}
	
	if( w &&  w->ProcessAccelerator(m) )
	{
		return true;
	}
	
	return false;
}

void	WNWindow::SetInitialAccelerators( const char *accel_name )
{
	accelerators = LoadAccelerators( hInstance, accel_name );
}


bool WNWindow::ProcessAccelerator( MSG *m )
{
	if( accelerators )
	{
		return TranslateAccelerator( GetHWND(), accelerators, m )!=0;
	}
	else
	{
		return false;
	}
}

bool 	WNWindow::Create()
{
	ENTER( "WNWindow::Create()" );
	
	
	hWnd=CreateWindow(
					  wn_class_name,
					  "",
					  initial_style,
					  initial_x,
					  initial_y,
					  initial_w,
					  initial_h,
					  initial_windows_parent,
					  0,
					  hInstance,
					  0
					  );
	
	RECT client;
	
	GetClientRect( hWnd, &client );
	
	if( initial_style & WS_HSCROLL )
	{
		SetScrollRange( GetHWND(), SB_HORZ, hscroll_min, hscroll_max, false );
		SetScrollPos( GetHWND(), SB_HORZ, hscroll_pos, false );
		lines_per_hpage = client.right/pix_per_hline;
		origin_x = hscroll_pos * pix_per_hline;
	}
	
	if( initial_style & WS_VSCROLL )
	{
		SetScrollRange( GetHWND(), SB_VERT, vscroll_min, vscroll_max, false );
		SetScrollPos( GetHWND(), SB_VERT, vscroll_pos, false );
		lines_per_vpage = client.bottom/pix_per_vline;
		origin_y = vscroll_pos * pix_per_vline;
	}
	
	return hWnd!=0;
}



LPARAM 	WNWindow::DispatchMsg( UINT m, WPARAM w, LPARAM l )
{
	//ENTER( "WNWindow::DispatchMsg()" );
	cur_msg=m;
	cur_wParam=w;
	cur_lParam=l;
	
	switch( m )
	{
	case WM_COMMAND:
		return CommandMsg( w, (HWND) (DWORD)LOWORD(l) );
	case WM_CHAR:
		return CharMsg( w,l );
	case WM_NEXTDLGCTL:
		return NextDlgCtlMsg(w,l);
	case WM_CLOSE:
		return CloseMsg();
	case WM_DESTROY:
		return DestroyMsg();
	case WM_DRAWITEM:
		return DrawItemMsg( (int)w, (const DRAWITEMSTRUCT FAR *)l );
	case WM_DROPFILES:
		return DropFilesMsg( (HANDLE)w );
	case WM_HSCROLL:
		return HScrollMsg( w, LOWORD(l), (HWND)(DWORD)HIWORD(l) );
	case WM_VSCROLL:
		return VScrollMsg( w, LOWORD(l), (HWND)(DWORD)HIWORD(l) );
	case WM_QUERYENDSESSION:
		return QueryEndSession();
	case WM_PAINT:
		return HandlePaint();
	case WM_KEYDOWN:
		return KeyDownMsg( w, l );
	case WM_KEYUP:
		return KeyUpMsg( w, l );
	case WM_MOUSEMOVE:
		return MouseMoveMsg(w,LOWORD(l),HIWORD(l));
	case WM_LBUTTONDOWN:
		return LButtonDownMsg(w,LOWORD(l),HIWORD(l));
	case WM_LBUTTONUP:
		return LButtonUpMsg(w,LOWORD(l),HIWORD(l));
	case WM_LBUTTONDBLCLK:
		return LButtonDblClkMsg(w,LOWORD(l),HIWORD(l));
	case WM_RBUTTONDOWN:
		return RButtonDownMsg(w,LOWORD(l),HIWORD(l));
	case WM_RBUTTONUP:
		return RButtonUpMsg(w,LOWORD(l),HIWORD(l));
	case WM_RBUTTONDBLCLK:
		return RButtonDblClkMsg(w,LOWORD(l),HIWORD(l));
	case WM_SIZE:
		return SizeMsg(w,LOWORD(l),HIWORD(l));
	case WM_MOVE:
		return MoveMsg(LOWORD(l),HIWORD(l));
	case WM_SETFOCUS:
		return SetFocusMsg( (HWND)w );
	case WM_TIMER:
		return TimerMsg(w);
	case WMJK_CHILDDESTROYED:
		return ChildDestroyedMsg( (WNWindow *) l );
	case WM_SHOWWINDOW:
        return ShowWindowMsg( (w!=0), (int)l );
	case BN_CLICKED:
        return BnClickedMsg( LOWORD(w), HWND(l) ); 
		
	}

	if( m>WMJK_USER )
	  return UserMsg(m,w,l);
	else
	  return DefaultHandler();
}

LPARAM	WNWindow::DefaultHandler()
{
	return DefWindowProc(GetHWND(),cur_msg,cur_wParam,cur_lParam);
}

LPARAM WNWindow::HandlePaint()
{
	ENTER( "WNWindow::HandlePaint()" );
	
	LPARAM ret;
	
	RECT r;
	
	if( !GetUpdateRect( GetHWND(), &r, true ) )
	{
		WNDC dc(this);
		ret=PaintMsg(dc);
	}
	else
	{
		PAINTSTRUCT p;
		BeginPaint( GetHWND(), &p );
		
		ret= PaintMsg(p.hdc);
		EndPaint( GetHWND(), &p );
	} 
	
	return ret;
}


LPARAM	WNWindow::DrawItemMsg(                               	// WM_DRAWITEM
							  int idCtl,
							  const DRAWITEMSTRUCT FAR * draw)
{
	
	ENTER( "WNWindow::DrawItemMsg()" );
	
 	return DefaultHandler();
}


LPARAM	WNWindow::CommandMsg( WPARAM cmd, HWND hCtl )		// WM_COMMAND
{
	
	ENTER( "WNWindow::CommandMsg()" );
	
	return DefaultHandler();
}

LPARAM	WNWindow::CharMsg( WPARAM k, LPARAM l )		// WM_CHAR
{	
	ENTER( "WNWindow::CharMsg()" );
	
	return DefaultHandler();
}

LPARAM	WNWindow::SysCharMsg( WPARAM vk, LPARAM l )		// WM_SYSCHAR
{	
	ENTER( "WNWindow::SysCharMsg()" );
	
	return DefaultHandler();
}


LPARAM	WNWindow::NextDlgCtlMsg( WPARAM k, LPARAM l )		// WM_NEXTDLGCTL
{	
	ENTER( "WNWindow::NextDlgCtlMsg()" );
	
	return DefaultHandler();
}



LPARAM	WNWindow::CloseMsg()					// WM_CLOSE
{
	ENTER( "WNWindow::CloseMsg()" );
	
	if( children.CloseAll() )
	{
		DestroyWindow( hWnd );
	}
	
	return 0;
}

LPARAM	WNWindow::ChildDestroyedMsg( WNWindow *child )
{
	
	ENTER( "WNWindow::ChildDestroyedMsg()" );
	
	children.Remove( child );
	delete child;
	return 0;
}

LPARAM	WNWindow::DestroyMsg()					// WM_DESTROY
{
	
	ENTER( "WNWindow::DestroyMsg()" );
	
	if( !GetParent() )
	{
		children.DestroyAll();
		PostQuitMessage(0);
		return 0;
	}
	else
	{
		children.DestroyAll();
		SendMessage( GetParentHWND(), WMJK_CHILDDESTROYED, 0, (LPARAM)this );
		return DefaultHandler();
	}
}


LPARAM	WNWindow::DropFilesMsg( HANDLE drop )        		// WM_DROPFILES
{
	
	ENTER( "WNWindow::DropFilesMsg()" );
	
 	return DefaultHandler();
}

LPARAM	WNWindow::QueryEndSession()				// WM_QUERYENDSESSION
{
	
	ENTER( "WNWindow::QueryEndSession()" );
	
	return DefaultHandler();	
}

LPARAM	WNWindow::PaintMsg( HDC dc)				// WM_PAINT
{
	ENTER( "WNWindow::PaintMsg()" );
	
	return 0;
}

LPARAM	WNWindow::HScrollMsg( int code, int mpos, HWND hCtl )	// WM_HSCROLL
{
	ENTER( "WNWindow::HScrollMsg()" );
	
	if( ! (initial_style & WS_HSCROLL) )
	{
		return DefaultHandler();
	}
	
	int pos=hscroll_pos;
	
	switch(code)
	{
		case SB_TOP:
		pos=hscroll_min;
		break;
		case SB_BOTTOM:
		pos=hscroll_max;
		break;
		case SB_LINEDOWN:
		pos++;
		break;
		case SB_LINEUP:
		pos--;
		break;
		case SB_PAGEDOWN:
		pos+=lines_per_hpage;
		break;
		case SB_PAGEUP:
		pos-=lines_per_hpage;
		break;
		case SB_THUMBPOSITION:
		pos=mpos;
		break;
		case SB_THUMBTRACK:
		pos=mpos;
		break;
		case SB_ENDSCROLL:
		return 0;
		default:
		//JKASSERT(0);
		return 0;
	}
	
	if( pos >= hscroll_max )
	  pos=hscroll_max;
	if( pos <= hscroll_min )
	  pos=hscroll_min;
	
	if( hscroll_pos!=pos )
	{
		ENTER( "WNWindow:: Updating hscroll_pos" );
		
		int old_pos=hscroll_pos;
		
		SetScrollPos( GetHWND(), SB_HORZ, pos, true );
		origin_x=(long)pos * pix_per_hline;
		hscroll_pos=pos;
		
		ScrollWindow( GetHWND(), (old_pos-pos)* pix_per_hline, 0, 0,0 );
		UpdateWindow( GetHWND() );
		
	}
	
	return 0;
}

LPARAM	WNWindow::VScrollMsg( int code, int mpos, HWND hCtl )	// WM_VSCROLL
{
	ENTER( "WNWindow::VScrollMsg()" );
	
	
	if( ! (initial_style & WS_VSCROLL) )
	{
		return DefaultHandler();
	}
	
	int pos=vscroll_pos;
	
	switch(code)
	{
		case SB_TOP:
		pos=vscroll_min;
		break;
		case SB_BOTTOM:
		pos=vscroll_max;
		break;
		case SB_LINEDOWN:
		pos++;
		break;
		case SB_LINEUP:
		pos--;
		break;
		case SB_PAGEDOWN:
		pos+=lines_per_vpage;
		break;
		case SB_PAGEUP:
		pos-=lines_per_vpage;
		break;
		case SB_THUMBPOSITION:
		pos=mpos;
		break;
		case SB_THUMBTRACK:
		pos=mpos;
		break;
		case SB_ENDSCROLL:
		return 0;
		default:
		//JKASSERT(0);
		return 0;
	}
	
	if( pos>=vscroll_max )
	  pos=vscroll_max;
	if( pos<=vscroll_min )
	  pos=vscroll_min;
	
	if( vscroll_pos!=pos )
	{
		ENTER( "WNWindow:: Updating hscroll_pos" );
		
		int old_pos = vscroll_pos;
		
		SetScrollPos( GetHWND(), SB_VERT, pos, true );
		origin_y = (long)pos * pix_per_vline;
		vscroll_pos=pos;
		
		ScrollWindow( GetHWND(), 0, (old_pos-pos)*pix_per_vline, 0,0 );
		UpdateWindow( GetHWND() );
		
	}
	
	return 0;
}


LPARAM	WNWindow::KeyDownMsg( WPARAM vk, LPARAM kd )		// WM_KEYDOWN
{
	ENTER( "WNWindow::KeyDownMsg()" );
	
	return DefaultHandler();	
}

LPARAM	WNWindow::KeyUpMsg( WPARAM vk, LPARAM kd )		// WM_KEYUP
{
	ENTER( "WNWindow::KeyUpMsg()" );
	
	return DefaultHandler();
}

LPARAM	WNWindow::MouseMoveMsg( int key, int x, int y )		// WM_MOUSEMOVE
{
	//ENTER( "WNWindow::MouseMoveMsg()" );
	
	return DefaultHandler();
}

LPARAM	WNWindow::LButtonDownMsg( int key, int x, int y )	// WM_LBUTTONDOWN
{
	ENTER( "WNWindow::LButtonDownMsg()" );
	
	left_but_down=true;
	return DefaultHandler();
}

LPARAM	WNWindow::LButtonUpMsg( int key, WORD x, int y )	// WM_LBUTTONUP
{
	ENTER( "WNWindow::LButtonUpMsg()" );
	
	left_but_down=false;
	return DefaultHandler();
}

LPARAM	WNWindow::LButtonDblClkMsg( int key, int x, int y )	// WM_LBUTTONDBLCLK
{
	ENTER( "WNWindow::LButtonDblClkMsg()" );
	
	left_but_down=false;
	return DefaultHandler();
}


LPARAM	WNWindow::RButtonDownMsg( int key, int x, int y )	// WM_RBUTTONDOWN
{
	ENTER( "WNWindow::RButtonDownMsg()" );
	
	right_but_down=true;
	return DefaultHandler();
}

LPARAM	WNWindow::RButtonUpMsg( int key, WORD x, int y )	// WM_RBUTTONUP
{
	ENTER( "WNWindow::LButtonUpMsg()" );
	
	right_but_down=false;
	return DefaultHandler();
}

LPARAM	WNWindow::RButtonDblClkMsg( int key, int x, int y )	// WM_RBUTTONDBLCLK
{
	ENTER( "WNWindow::RButtonDblClkMsg()" );
	
	right_but_down=false;
	return DefaultHandler();
}


LPARAM	WNWindow::SizeMsg( int flag, int w, int h )		// WM_SIZE
{
	ENTER( "WNWindow::SizeMsg()" );
	
	bool invalid=false;
	int prevhscroll=hscroll_pos;
	int prevvscroll=vscroll_pos;
	long hmax = ((document_w - w)+(pix_per_hline/2))/pix_per_hline;
	long vmax = ((document_h - h)+(pix_per_vline/2))/pix_per_vline;
	
	if( hmax < 1 )
	{
		hmax=1;
	}
	if( vmax < 1 )
	{
		vmax=1;
	}
	if( initial_style & WS_HSCROLL )
	{
		hscroll_max=hmax;
		
		SetScrollRange( GetHWND(), SB_HORZ, hscroll_min, hmax, true );
		
		
		if( hscroll_pos > hscroll_max )
		{
			hscroll_pos=hscroll_max;
			origin_x = (long)hscroll_pos * pix_per_hline;
			SetScrollPos( GetHWND(), SB_HORZ, hscroll_pos, true );
			
			invalid=true;
		}
		
		
		
	}
	
	
	if( initial_style & WS_VSCROLL )
	{
		vscroll_max=vmax;
		
		
		SetScrollRange( GetHWND(), SB_VERT, vscroll_min, vmax, true );
		
		if( vscroll_pos > vscroll_max )
		{
			vscroll_pos=vscroll_max;
			origin_y = (long)vscroll_pos * pix_per_vline;
			SetScrollPos( GetHWND(), SB_VERT, vscroll_pos, true );
			invalid=true;
		}
		
		
		
	}
	
	if( invalid )
	{
		ScrollWindow( GetHWND(),
					 - (hscroll_pos-prevhscroll) * pix_per_hline,
					 - (vscroll_pos-prevvscroll) * pix_per_vline, 0,0 );
	}
	
	
	if( flag==SIZE_RESTORED || flag==SIZE_MAXIMIZED )
	{
		lines_per_hpage = ((long)w-32)/pix_per_hline;
		lines_per_vpage = ((long)h-32)/pix_per_vline;
		
		if( lines_per_hpage<1 )
		{
			lines_per_hpage=1;
		}
		
		if( lines_per_vpage<1 )
		{
			lines_per_vpage=1;
		}
		
		
	}
	
	
	return 0;
}

void 	WNWindow::ChangeDocumentSize( long w, long h )
{
	document_w=w;
	document_h=h;
	
	RECT r;
	GetClientRect( GetHWND(), &r );
	
	SizeMsg( SIZE_RESTORED, r.right, r.bottom );
}


LPARAM	WNWindow::MoveMsg( int x, int y )			// WM_MOVE
{
	ENTER( "WNWindow::MoveMsg()" );
	
	return DefaultHandler();
}

LPARAM	WNWindow::SetFocusMsg( HWND prev )			// WM_SETFOCUS
{
	ENTER( "WNWindow::SetFocusMsg()" );
	
	return DefaultHandler();
}


LPARAM	WNWindow::CommNotifyMsg(
								int devid,
								int status )				// WM_COMMNOTIFY
{
	ENTER( "WNWindow::CommNotifyMsg()" );
	
	return DefaultHandler();
}

LPARAM	WNWindow::TimerMsg( int id )				// WM_TIMER
{
	ENTER( "WNWindow::TimerMsg()" );
	
	return DefaultHandler();
}


LPARAM	WNWindow::UserMsg( UINT msg, WPARAM w, LPARAM l )	// WMJK_USER+x
{
	ENTER( "WNWindow::UserMsg()" );
	
	return DefaultHandler();
}



void	WNWindow::ChangeHScrollBounds( int min_, int max_ )
{
	if( initial_style & WS_HSCROLL )
	{
		hscroll_max=max_;
		hscroll_min=min_;
		
		if( min_==max_ )
		{
			max_=min_+1;
		}
		
		bool force=true;
		
		if( hscroll_pos < hscroll_min )
		{
			force=false;
		}
		else
		  if( hscroll_pos > hscroll_max )
		{
			force=false;
		}
		
		
		SetScrollRange( GetHWND(), SB_HORZ, min_, max_, force );
		
		if( hscroll_pos < hscroll_min )
		{
			HScrollMsg( SB_TOP,0,0 );
		}
		else
		  if( hscroll_pos > hscroll_max )
		{
			HScrollMsg( SB_BOTTOM,0,0 );
		}
	}
}

void	WNWindow::ChangeVScrollBounds( int min_, int max_ )
{
	if( initial_style & WS_VSCROLL )
	{
		vscroll_max=max_;
		vscroll_min=min_;
		
		
		if( min_==max_ )
		{
			max_=min_+1;
		}
		
		bool force=true;
		
		if( vscroll_pos < vscroll_min )
		{
			force=false;
		}
		else
		  if( vscroll_pos > vscroll_max )
		{
			force=false;
		}
		
		
		SetScrollRange( GetHWND(), SB_VERT, min_, max_, force );
		
		if( vscroll_pos < vscroll_min )
		{
			VScrollMsg( SB_TOP,0,0 );
		}
		else
		  if( vscroll_pos > vscroll_max )
		{
			VScrollMsg( SB_BOTTOM,0,0 );
		}
		
		
		
	}
	
}



