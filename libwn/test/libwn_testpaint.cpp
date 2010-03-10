#include "wnworld.h"
#include "wnlib.h"
#include "libwn_testpaintrc.h"

class WNTestPaintWindow : public WNWindow
{
	public:
	
	WNTestPaintWindow() : WNWindow( "TestPaint" )
	{
		SetInitialStyle( WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL );
		SetInitialDocumentW( 2000 );
		SetInitialDocumentH( 2000 );
		SetInitialVScrollBounds( 0, 2000 );
		SetInitialHScrollBounds( 0, 2000 );
		SetInitialPixPerVLine( 100 );
		SetInitialPixPerHLine( 100 );				
	}
	
	~WNTestPaintWindow()
	{
	}

	virtual	LPARAM	PaintMsg( HDC dc )
	{
		WNPen pen( dc, PS_SOLID, 5, 0,0,0 );
		
		HPEN prev=(HPEN)SelectObject( dc, pen );
		
		for( int i=0; i<800; i+=20 )
		{
			MoveToEx( dc, 500-origin_x, i-origin_y, 0 );
			LineTo( dc, 500-i-origin_x, 0-origin_y );
		}
		
		SelectObject( dc, prev );
		return 0;
	}
	
	
	
	virtual	LPARAM	LButtonDownMsg( int key, int x, int y )
	{
		MessageBeep(0);	
		return 1;
	}
	
	
};

static WNRegister WNTestPaintWindowClass(
										 	"TestPaint",
										 	0,
										 	"ICONMAIN",
										 	IDC_ARROW,
										 	COLOR_WINDOW
										 );

bool WNInitializeApplication()
{
	return true;	
}

void WNKillApplication()
{
	
}

WNWindow *WNCreateMainWindow()
{
	WNWindow *w =  new WNTestPaintWindow;
	
	w->Create();
    w->SetText( "Test Paint!" );	
	w->Show( nCmdShow );
	
	return w;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return WNWinMain( hInstance, hPrevInstance, lpCmdLine, nCmdShow, WNInitializeApplication, WNCreateMainWindow, WNKillApplication );

}
