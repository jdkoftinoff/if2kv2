// maindlg.h : interface of the CMainDlg class

//

/////////////////////////////////////////////////////////////////////////////



#if !defined(AFX_MAINDLG_H__A1DBC38E_6E1D_48BA_8BD3_4A3046858DDB__INCLUDED_)

#define AFX_MAINDLG_H__A1DBC38E_6E1D_48BA_8BD3_4A3046858DDB__INCLUDED_



#if _MSC_VER >= 1000

#pragma once

#endif // _MSC_VER >= 1000



#include "jdk_world.h"

#include "if2k_mini_compile.h"

#include "jdk_thread.h"



#include <atlmisc.h>

#include <atlddx.h>





extern jdk_setting_description if2k_mini_kernel_defaults[];



class if2k_compile_thread : public jdk_thread

{

public:

  if2k_compile_thread( jdk_settings &settings_, volatile int *progress1_, volatile int *progress2_ )

    : settings( settings_ ),

      progress1( progress1_ ),

      progress2( progress2_ )

  {

  }



protected:

  void main()

  {

    if2k_mini_compile_all( settings, progress1, progress2 );    

  }



  jdk_settings &settings;

  volatile int *progress1;

  volatile int *progress2;



};





class CMainDlg : public CDialogImpl<CMainDlg>,

  public CWinDataExchange<CMainDlg>

{

public:

  jdk_settings_text settings;

  if2k_compile_thread *compile_thread;

  volatile int progress1;

  volatile int progress2;

  volatile int progress2a;



  CMainDlg()

    :   

    settings(if2k_mini_kernel_defaults,__argc,__argv)

  {



    jdk_string_filename home( settings.get("home").c_str() );



    jdk_set_app_name( "if2k_install_lists" );



    settings.load_file( settings.get("kernel.settings.master.local") );

    settings.load_file( settings.get("kernel.settings.additional.local") );



  	jdk_set_home_dir(

       home.c_str()

  		  );



	  jdk_log_setup(

				  settings.get_long( "log.type" ),

				  settings.get( "log.file" ),

				  settings.get_long( "log.detail" )

				  );

 

    compile_thread = new if2k_compile_thread( settings, &progress1, &progress2 );

    compile_thread->run();

  }



    ~CMainDlg()

    {

      while( !compile_thread->thread_isdone() )

        Sleep(1000);

      delete compile_thread;

    }



	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP(CMainDlg)

		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		MESSAGE_HANDLER(WM_TIMER, OnTimer)

	END_MSG_MAP()



// Handler prototypes (uncomment arguments if needed):

//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)

//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)

//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)



	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)

	{

		// center the dialog on the screen

		CenterWindow();



		// set icons

		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 

			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);

		SetIcon(hIcon, TRUE);

		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 

			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

		SetIcon(hIconSmall, FALSE);



    SetTimer(

        0,            // timer identifier 

        200

        );



		return TRUE;

	}



  LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)

	{

    progress2a = progress2%1000;


    if( compile_thread->thread_isdone() )

    {

      PostQuitMessage(0);

    }

    return TRUE;

  }



};





/////////////////////////////////////////////////////////////////////////////



//{{AFX_INSERT_LOCATION}}

// Microsoft Visual C++ will insert additional declarations immediately before the previous line.



#endif // !defined(AFX_MAINDLG_H__A1DBC38E_6E1D_48BA_8BD3_4A3046858DDB__INCLUDED_)

