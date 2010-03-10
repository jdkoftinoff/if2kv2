#include "wnworld.h"


#include "wnwindow.h"
#include "wnmain.h"
#include "wnreg.h"
#include "wnextdll.h"

#ifndef DEBUG_WNMAIN
# define DEBUG_WNMAIN 0
#endif

#if !DEBUG_WNMAIN
# undef DBG
# define DBG(a)
#endif


HINSTANCE hInstance;
HINSTANCE hPrevInstance;
LPCSTR lpCmdLine;
int nCmdShow;




int WNWinMain(
				   HINSTANCE hInstance_,          	// current instance
				   HINSTANCE hPrevInstance_,        // previous instance
				   LPSTR lpCmdLine_,                // command line
				   int nCmdShow_,                    // show-window type (open/icon)
           bool (*WNInitializeApplication)(),
           WNWindow* (*WNCreateMainWindow)(),          
           void (*WNKillApplication)()
				   )
{
	hInstance=hInstance_;
	hPrevInstance=hPrevInstance_;
	lpCmdLine=lpCmdLine_;
	nCmdShow=nCmdShow_;
	
	if( WNExternalDLL::LoadAllDLLs() )
	{
		if( !WNInitializeApplication || WNInitializeApplication() )
		{
			if( WNRegister::RegisterAll() )
			{
				
				WNWindow *w=WNCreateMainWindow();
				MSG m;
				
				while( GetMessage(&m,0,0,0) )
				{
					if( WNWindow::ProcessAllAccelerators(&m)==0 )
					{
						TranslateMessage(&m);
						DispatchMessage(&m);
					}
				}
				delete w;
			}
			
			
			if( !WNKillApplication)
        WNKillApplication();
		}
		
		WNExternalDLL::FreeAllDLLs();
	}
	else
	{
		MessageBox( 0, "Error loading required DLL's", "ERROR", MB_OK );
	}
	
	return 0;
}

