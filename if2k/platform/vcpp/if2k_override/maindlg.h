// maindlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////



#if !defined(AFX_MAINDLG_H__1920D3BC_1FB2_42DA_94EE_92E0EF275C26__INCLUDED_)
#define AFX_MAINDLG_H__1920D3BC_1FB2_42DA_94EE_92E0EF275C26__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include <atlmisc.h>
#include <atlframe.h>
#include <atlddx.h>
#include <atldlgs.h>

#include "jdk_world.h"
#include "if2k_mini_client.h"

class CMainDlg : 
  public CDialogImpl<CMainDlg>,
  public CWinDataExchange<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };
  if2k_mini_client if2k;

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(IDC_CHECK_OVERRIDE, OnClickOverride)
  END_MSG_MAP()

  BEGIN_DDX_MAP(CMainDlg)
    DDX_CHECK( IDC_CHECK_OVERRIDE, m_override )
  END_DDX_MAP()


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

    bool o=false;
    if( !if2k.get_override_mode(o) )
    {
//      MessageBox( "Error getting override mode" );
    }
    m_override = o ? 1 : 0;

    DoDataExchange(FALSE);

		return TRUE;
	}



	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    DestroyWindow();
    return 0;
  }



  LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    PostQuitMessage(0);
    return 0;
  }


  LRESULT OnClickOverride(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{    
    DoDataExchange(TRUE);

    if( !if2k.post_override_mode( m_override!=0 ) )
    {
      MessageBox( "Error setting override mode" );
    }

    bool o;
    if( !if2k.get_override_mode(o) )
    {
//      MessageBox( "Error getting override mode" );
    }
    m_override = o ? 1 : 0;

    DoDataExchange(FALSE);
    
		return 0;
	}
  
  BOOL m_override;
};





/////////////////////////////////////////////////////////////////////////////



//{{AFX_INSERT_LOCATION}}

// Microsoft Visual C++ will insert additional declarations immediately before the previous line.



#endif // !defined(AFX_MAINDLG_H__1920D3BC_1FB2_42DA_94EE_92E0EF275C26__INCLUDED_)

