// maindlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

 
#if !defined(AFX_MAINDLG_H__604B0AFF_E7EF_4E85_8084_A2F8DC7869D2__INCLUDED_)
#define AFX_MAINDLG_H__604B0AFF_E7EF_4E85_8084_A2F8DC7869D2__INCLUDED_



#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "jdk_win32wtl.h"
#include "if2k_mini_client.h"
#include "jdk_serial_number.h"


class CLicenseDlg :
  public CJdkDialog<CLicenseDlg,IDD_LICENSE>
{
public:

	BEGIN_MSG_MAP(CLicenseDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(ID_LICENSE_OK, OnOK)
		COMMAND_ID_HANDLER(ID_LICENSE_CANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BUTTON_LOAD_LICENSE, OnLoad)
		COMMAND_ID_HANDLER(IDC_BUTTON_SAVE_LICENSE, OnSave)
  END_MSG_MAP()

  BEGIN_DLGRESIZE_MAP(CLicenseDlg)
  END_DLGRESIZE_MAP()


	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
    JdkSetupIcon();

    JdkDDXUpdateGui();

		return TRUE;
	}


	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
 
		EndDialog(wID);
		return 0;
	}


	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnLoad(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    CFileDialog dlg(TRUE,"Text Files (*.txt)\0*.txt\0","license.txt" );

    if( dlg.DoModal() )
    {
      jdk_string_filename f( dlg.m_szFileName );
      
      if( license.load_file( f ) )
      {
        serial.load( license );
        JdkDDXUpdateGui();
      }
      else
      {
        MessageBox( "Error Reading file" );
      }

    }
		return 0;
	}

	LRESULT OnSave(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    JdkDDXReadGui();
    serial.load( license );
    // get the date from the license
    {
      struct tm *t;

      time_t cur_time;
      
      cur_time = time(&cur_time);
      t = localtime( &cur_time );
      
      int month, day, year;
      sscanf( serial.expires.c_str(), "%d/%d/%d", &year, &month, &day );
      
      t->tm_mon = month-1;
      t->tm_mday = day;
      t->tm_year = year - 1900;
      
      time_t time_code = mktime( t );
      serial.expires.form( "%04d/%02d/%02d", t->tm_year + 1900, t->tm_mon+1, t->tm_mday );
      serial.expires_code.form( "%lu", (unsigned long) time_code );         
    }
    
    jdk_str<128> product_id("The Internet Filter IF-2K");
    serial.serial=serial.calculate_code( product_id );

    serial.save( license );

    JdkDDXUpdateGui();

    CFileDialog dlg(FALSE,"Text Files (*.txt)\0*.txt\0","license.txt" );

    if( dlg.DoModal() )
    {
      jdk_string_filename f( dlg.m_szFileName );
      
      if( license.save_file( f ) )
      {
        MessageBox( "Saved" );
      }
      else
      {
        MessageBox( "Error Reading file" );
      }
    }
		return 0;
	}


  jdk_settings license;
  jdk_serial_number serial;


  BEGIN_JDK_DDX_MAP(CLicenseDlg)
    JDK_DDX_TEXT( IDC_EDIT_LICENSE_NAME, license, "license.name" )
    JDK_DDX_TEXT( IDC_EDIT_LICENSE_EMAIL, license,"license.email" )
    JDK_DDX_TEXT( IDC_EDIT_LICENSE_ADDRESS, license,"license.address" )
    JDK_DDX_TEXT( IDC_EDIT_LICENSE_CITY, license,"license.city" )
    JDK_DDX_TEXT( IDC_EDIT_LICENSE_REGION, license,"license.region")
    JDK_DDX_TEXT( IDC_EDIT_LICENSE_ZIPCODE, license,"license.zipcode" )
    JDK_DDX_TEXT( IDC_EDIT_LICENSE_PHONE, license,"license.phone")
    JDK_DDX_TEXT( IDC_EDIT_LICENSE_WEBSITE, license,"license.website" )
    JDK_DDX_TEXT( IDC_EDIT_LICENSE_INFO, license,"license.info" )
    JDK_DDX_TEXT( IDC_EDIT_LICENSE_EXPIRES, license,"license.expires")   
    JDK_DDX_TEXT( IDC_EDIT_LICENSE_SERIAL, license,"license.serial" )   
  END_JDK_DDX_MAP()

};









/////////////////////////////////////////////////////////////////////////////



//{{AFX_INSERT_LOCATION}}

// Microsoft Visual C++ will insert additional declarations immediately before the previous line.



#endif // !defined(AFX_MAINDLG_H__604B0AFF_E7EF_4E85_8084_A2F8DC7869D2__INCLUDED_)

