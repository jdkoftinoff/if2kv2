#ifndef __MAIN_H
#define __MAIN_H

#include "wnwindow.h"


extern HINSTANCE hInstance;
extern HINSTANCE hPrevInstance;
extern LPCSTR 	 lpCmdLine;
extern int    	 nCmdShow;
int WNWinMain(
				   HINSTANCE hInstance_,          	// current instance
				   HINSTANCE hPrevInstance_,        // previous instance
				   LPSTR lpCmdLine_,                // command line
				   int nCmdShow_,                    // show-window type (open/icon)
           bool (*WNInitializeApplication)(),
           WNWindow* (*WNCreateMainWindow)(),          
           void (*WNKillApplication)()
				   );

bool WNInitializeApplication();
void WNKillApplication();
WNWindow *WNCreateMainWindow();

#endif


