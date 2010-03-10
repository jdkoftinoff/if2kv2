#include "wnworld.h"
#include "wnlib.h"


class WNTestHelloWindow : public WNWindow
{
	public:
	
	WNTestHelloWindow() : WNWindow( "TestHello" )
	{
		SetInitialStyle( WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL );
	}
	
	~WNTestHelloWindow()
	{
	}

	virtual	LPARAM	LButtonDownMsg( int key, int x, int y )
	{
		MessageBeep(0);	
		return 1;
	}
	
	
};

static WNRegister WNTestHelloWindowClass(
										 	"TestHello",
										 	0,
										 	0,
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
	WNWindow *w =  new WNTestHelloWindow;
	
	w->Create();
    w->SetText( "Test Hello!" );	
	w->Show( nCmdShow );
	
	return w;
}

#if JDK_IS_WINE
int main( int argc, char **argv )
{
		return WNWinMain( 0, 0, "", SW_SHOW );
}

#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return WNWinMain( hInstance, hPrevInstance, lpCmdLine, nCmdShow, WNInitializeApplication, WNCreateMainWindow, WNKillApplication );
}
#endif
