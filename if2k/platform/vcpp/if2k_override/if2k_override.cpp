// if2k_override.cpp : main source file for if2k_override.exe

//



#include "stdafx.h"
 


#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>

#include "resource.h"

#include "maindlg.h"



CAppModule _Module;


HWND Install_CreateInvisibleWindow(
                                    HINSTANCE instance,
                                    const char *clsname,
                                    const char *title,
                                    WNDPROC proc
                                    )
{
    HWND w=0;

    WNDCLASS *cls = new WNDCLASS;
    cls->style = 0;
    cls->lpfnWndProc = proc;
    cls->cbClsExtra = 0;
    cls->cbWndExtra = 0;
    cls->hInstance = instance;
    cls->hIcon = 0;
    cls->hCursor = 0;
    cls->hbrBackground = 0;
    cls->lpszMenuName = 0;
    cls->lpszClassName = new char[ strlen(clsname)+1 ];
    strcpy( (char *)cls->lpszClassName, clsname );

    if( RegisterClass( cls ) )
    {
        w = CreateWindow(
                        clsname,
                        title,
                        0, // invisible
                        0,0,10,10,
                        0, // no parent
                        0, // no menu
                        instance,
                        0  // no createparam
                        );
    }

    return w;
}




LRESULT Install_SendMsgToWindow(
                            const char *clsname,
                            const char *title,
                            UINT msg,
                            WPARAM wparam,
                            LPARAM lparam
                            )
{
    HWND w = FindWindow( clsname, title );
    if( w )
    {
        return SendMessage( w, msg, wparam, lparam );
    }
    return -1;
}


CMainDlg *main_dlg=0;
 
LRESULT CALLBACK InvisibleWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{  
    switch (uMsg) 
    { 
        case WM_USER+2: 
          if( main_dlg )
          {
            main_dlg->EndDialog(0);
          }

          return 0; 
        // 
        // Process other messages. 
        // 
 
        default: 
            return DefWindowProc(hwnd, uMsg, wParam, lParam); 
    } 
    return 0; 
} 



int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
  Install_SendMsgToWindow(
                            "if2k_override_marker",
                            "if2k_override_marker",
                            WM_USER+2,
                            0,
                            0
                            );
  Sleep(200);

  Install_CreateInvisibleWindow(
                                hInstance,
                                "if2k_override_marker",
                                "if2k_override_marker",
                                InvisibleWndProc
                                );


	HRESULT hRes = ::CoInitialize(NULL);

// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));


	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));
	int nRet = 0;

	// BLOCK: Run application

	{
		CMainDlg dlgMain;
    main_dlg = &dlgMain;
		nRet = dlgMain.DoModal();
    main_dlg = 0;
	}

	_Module.Term();

	::CoUninitialize();

	return nRet;

}

