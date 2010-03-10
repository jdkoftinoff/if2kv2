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


class CBlockingDlg :
  public CJdkDialog<CBlockingDlg,IDD_DIALOG_BLOCKING>
{
public:
    CBlockingDlg( if2k_mini_client &if2k_, if2k_mini_client_collection &if2k_collection_ )
      : if2k(if2k_), if2k_collection(if2k_collection_)
    {
    }

  BEGIN_MSG_MAP(CBlockingDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog )
    COMMAND_ID_HANDLER( IDOK, OnOk )
    COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
  END_MSG_MAP()

  BEGIN_DLGRESIZE_MAP(CBlockingDlg)
  END_DLGRESIZE_MAP()


  BEGIN_JDK_DDX_MAP(CBlockingDlg)
    JDK_DDX_TEXT( IDC_EDIT_BLOCK_PAGE_TITLE, if2k_collection.additional_settings, "blockpage.title" )
    JDK_DDX_TEXT( IDC_EDIT_BLOCK_PAGE_IMAGE, if2k_collection.additional_settings, "blockpage.image.url" )
    JDK_DDX_TEXT( IDC_EDIT_BLOCK_PAGE_IMAGE_LINK, if2k_collection.additional_settings, "blockpage.image.link" )
    JDK_DDX_TEXT( IDC_EDIT_BLOCK_PAGE_TEXT, if2k_collection.additional_settings, "blockpage.blockedmessage" )
    JDK_DDX_CHECK( IDC_CHECK_REFERER, if2k_collection.additional_settings, "kernel.blocking.referer.enable" )
  END_JDK_DDX_MAP()

  if2k_mini_client &if2k;
  if2k_mini_client_collection &if2k_collection;

};


class CAdvancedDlg :
  public CJdkDialog<CAdvancedDlg,IDD_DIALOG_ADVANCED>
{
public:
  void Setup(
    if2k_mini_client *if2k_,
    if2k_mini_client_collection *if2k_collection_
    )
  {
    if2k=if2k_;
    if2k_collection=if2k_collection_;
  }

  BEGIN_MSG_MAP(CAdvancedDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog )
    COMMAND_ID_HANDLER( IDOK, OnOk )
    COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
    COMMAND_ID_HANDLER( IDC_BUTTON_ADVANCED_EXPORT, OnExport )
  END_MSG_MAP()


	LRESULT OnExport(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    JdkDDXReadGui();

    CFileDialog dlg(FALSE,"Text Files (*.txt)\0*.txt\0","settings.txt" );

    if( dlg.DoModal() )
    {
      jdk_string_filename f( dlg.m_szFileName );

      jdk_settings_text merged;
      merged.merge( if2k_collection->install_settings );
      merged.merge( if2k_collection->additional_settings );
      merged.merge( if2k_collection->license_settings );

      if( !merged.save_file( f ) )
      {
        MessageBox( "Error Reading file" );
      }
    }
		return 0;
	}


  if2k_mini_client *if2k;
  if2k_mini_client_collection *if2k_collection;

  BEGIN_JDK_DDX_MAP(CAdvancedDlg)
    JDK_DDX_TEXT( IDC_EDIT_ADVANCED_PROXY, if2k_collection->additional_settings, "http.proxy")
    JDK_DDX_TEXT( IDC_EDIT_ADVANCED_REMOTE_DB, if2k_collection->additional_settings, "kernel.db.user.remote")
    JDK_DDX_TEXT( IDC_EDIT_ADVANCED_REMOTE_SETTINGS, if2k_collection->additional_settings, "kernel.settings.additional.remote")
    JDK_DDX_TEXT( IDC_EDIT_ADVANCED_UPDATE_RATE, if2k_collection->additional_settings, "kernel.settings.update.rate" )
  END_JDK_DDX_MAP()

  BEGIN_DLGRESIZE_MAP(CAdvancedDlg)
  END_DLGRESIZE_MAP()


};


class CPasswordDlg :
  public CJdkDialog<CPasswordDlg,IDD_PASSWORD>
{
public:
  void Setup( 
    if2k_mini_client *if2k_, 
    if2k_mini_client_collection *if2k_collection_ 
    )
  {
    if2k=if2k_;
    if2k_collection=if2k_collection_;
  }

  
  BEGIN_DLGRESIZE_MAP(CPasswordDlg)
  END_DLGRESIZE_MAP()


	BEGIN_MSG_MAP(CPasswordDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

  LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
    JdkSetupIcon();

    new_password="";
    DoDataExchange(FALSE);
		return TRUE;
	}


	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    bool r=false;
    JdkDDXReadGui();

    jdk_str<256> praw = new_password;
    jdk_str<512> p;
    if2k_hash_for_password( p, praw );
    if2k_collection->install_settings.set( "kernel.remote.update.password", p );

    //MessageBox( p.c_str(), "raw pw", MB_OK );
    //MessageBox( praw.c_str(), "sha1 pw", MB_OK );

    if( if2k->post_install_settings( if2k_collection->install_settings ) )
    {
      r=if2k->trigger_restart();
      if( r )
      {
        Sleep(1000);
      }
    }
    if( r )
    {
      MessageBox("New password has been set");
    }
    else
    {
      MessageBox("Error setting password." );
    }
		EndDialog(wID);
		return 0;
	}


	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

  if2k_mini_client *if2k;
  if2k_mini_client_collection *if2k_collection;
  jdk_str<256> new_password;


  BEGIN_JDK_DDX_MAP(CPasswordDlg)
    JDK_DDX_STR_TEXT( IDC_PASSWORD_EDIT_PASSWORD, new_password )
  END_JDK_DDX_MAP()
};

class CLicenseDlg :
  public CJdkDialog<CLicenseDlg,IDD_LICENSE>
{
public:
  void Setup( 
    if2k_mini_client *if2k_, 
    if2k_mini_client_collection *if2k_collection_ 
    )
  {
    if2k=if2k_;
    if2k_collection=if2k_collection_;
  }

	BEGIN_MSG_MAP(CLicenseDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(ID_LICENSE_OK, OnOK)
		COMMAND_ID_HANDLER(ID_LICENSE_CANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BUTTON_LOAD_LICENSE, OnLoad)
  END_MSG_MAP()

  BEGIN_DLGRESIZE_MAP(CLicenseDlg)
  END_DLGRESIZE_MAP()


	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
    JdkSetupIcon();

    license.merge( if2k_collection->license_settings );

    JdkDDXUpdateGui();

		return TRUE;
	}


	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    JdkDDXReadGui();

    bool r=false;

    if( if2k->post_license_settings( license ) )
    {
      r=if2k->trigger_restart();
      Sleep(2000);
    }
   
    if( !r )
    {
      MessageBox("Error setting license." );
    }

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

      if( if2k_collection->license_settings.load_file( f ) )
      {
        license.merge( if2k_collection->license_settings );
        JdkDDXUpdateGui();
      }
      else
      {
        MessageBox( "Error Reading file" );
      }
    }
		return 0;
	}

  if2k_mini_client *if2k;
  if2k_mini_client_collection *if2k_collection;
  jdk_settings_text license;

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



class CMainDlg : 
  public CJdkDialog<CMainDlg, IDD_MAINDLG>
{
public:
  CMainDlg() : 
      if2k_collection( if2k ),
      m_bad_urls( if2k_collection.badurl[0] ),
      m_good_urls( if2k_collection.goodurl[0] )
  {

  }


  BEGIN_JDK_DDX_MAP(CMainDlg)
    DDX_TEXT( IDC_EDIT_PASSWORD,m_password)
    DDX_TEXT( IDC_EDIT_SERVER,m_ip_and_port)    
    JDK_DDX_BUF_TEXT( IDC_EDIT_GOOD_URLS,m_good_urls)
    JDK_DDX_TEXT( IDC_EDIT_BLOCKPAGE_URL, if2k_collection.additional_settings, "blockpage.url" )
    JDK_DDX_CHECK( IDC_CHECK_ENABLE_FILTERING, if2k_collection.additional_settings, "kernel.blocking.enable" )
    JDK_DDX_CHECK( IDC_CHECK_ALLOW_USER_OVERRIDE, if2k_collection.additional_settings, "kernel.override.allow" )
  END_JDK_DDX_MAP()


  BEGIN_JDK_DLGITEM_LIST(CMainDlg)
    JDK_DLGITEM( IDC_EDIT_GOOD_URLS )
    JDK_DLGITEM( IDC_EDIT_BLOCKPAGE_URL )
    JDK_DLGITEM( IDC_CHECK_ENABLE_FILTERING )
    JDK_DLGITEM( IDC_CHECK_ALLOW_USER_OVERRIDE )
    JDK_DLGITEM( IDOK )
    JDK_DLGITEM( IDC_BUTTON_APPLY )
    JDK_DLGITEM( IDC_BUTTON_LOAD_LICENSE )
    JDK_DLGITEM( IDC_BUTTON_NEW_PASSWORD )
    JDK_DLGITEM( IDC_BUTTON_ADVANCED )
    JDK_DLGITEM( IDC_BUTTON_BLOCKING )
  END_JDK_DLGITEM_LIST()


  BEGIN_DLGRESIZE_MAP(CMainDlg)
    DLGRESIZE_CONTROL( IDC_STATIC_BOX1, DLSZ_SIZE_X | DLSZ_REPAINT )
    DLGRESIZE_CONTROL( IDC_STATIC_BOX2, DLSZ_SIZE_X | DLSZ_REPAINT )
    DLGRESIZE_CONTROL( IDOK, DLSZ_MOVE_X  )
    DLGRESIZE_CONTROL( IDCANCEL, DLSZ_MOVE_X  )
    DLGRESIZE_CONTROL( IDC_BUTTON_APPLY, DLSZ_MOVE_X  )
    DLGRESIZE_CONTROL( IDC_STATIC_GOOD_URLS, DLSZ_SIZE_X   | DLSZ_REPAINT )

    DLGRESIZE_CONTROL( IDC_STATIC_BOX3, DLSZ_SIZE_X | DLSZ_SIZE_Y | DLSZ_REPAINT )

    DLGRESIZE_CONTROL( IDC_EDIT_GOOD_URLS, DLSZ_SIZE_X | DLSZ_SIZE_Y  )

  END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER(IDC_BUTTON_LOGIN, OnLogin )
    COMMAND_ID_HANDLER(IDC_BUTTON_APPLY, OnApply )
    COMMAND_ID_HANDLER(IDC_BUTTON_LOAD_LICENSE, OnLicense )
		COMMAND_ID_HANDLER(IDC_BUTTON_NEW_PASSWORD, OnNewPassword )
		COMMAND_ID_HANDLER(IDC_BUTTON_ADVANCED, OnAdvanced )
    COMMAND_ID_HANDLER(IDC_BUTTON_BLOCKING, OnBlocking )
    CHAIN_MSG_MAP(CDialogResize<CMainDlg>)
  END_MSG_MAP()




	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
    JdkSetupIcon();
      
    if2k_logged_in = false;

    m_password = "password";
    m_ip_and_port = "localhost:8000";

    JdkDlgItemEnable(FALSE);
    JdkDDXUpdateGui();
		return TRUE;
	}

	LRESULT OnLicense(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CLicenseDlg dlg;

    dlg.Setup( &if2k, &if2k_collection );
		if( dlg.DoModal()==ID_LICENSE_OK )
    {
      if2k_collection.get_all_settings();
    }

		return 0;
	}

	LRESULT OnAdvanced(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAdvancedDlg dlg;

    dlg.Setup( &if2k, &if2k_collection );
		if( dlg.DoModal()==IDOK )
    {
      if2k_apply_settings();
    }

		return 0;
	}

  LRESULT OnBlocking(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CBlockingDlg dlg(if2k,if2k_collection);

		if( dlg.DoModal()==IDOK )
    {
      JdkDDXReadGui();
      if2k_apply_settings();
    }

		return 0;
	}


	LRESULT OnNewPassword(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CPasswordDlg dlg;

    dlg.Setup( &if2k, &if2k_collection );
		if( dlg.DoModal() == IDOK )
    {
      if2k_logout();
    }

		return 0;
	}



	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    JdkDDXReadGui();

    if( if2k_apply_settings() )
		  EndDialog(wID);

	  return 0;
	}



	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);

		return 0;
	}



	LRESULT OnLogin(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    JdkDDXReadGui();
    
    if2k.set_password( m_password );
    if( if2k_login() )
    {
      MessageBox("Login Successful");
      JdkDlgItemEnable( TRUE );
    }
    else
    {
      MessageBox("Login Failed");
      m_password = "";
      m_good_urls.clear();
      m_bad_urls.clear();
      JdkDlgItemEnable( FALSE );
    }
    JdkDDXUpdateGui();
		return 0;
	}



	LRESULT OnLogout(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    if( if2k_logged_in )
    {
     if2k_logout();
    }

		return 0;
	}


	LRESULT OnApply(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    JdkDDXReadGui();
    if2k_apply_settings();
		return 0;
	}


  bool if2k_apply_settings()
  {
    bool r=false;
        
    if2k.set_server_address( m_ip_and_port );
    if( if2k_logged_in )
    {
      if( if2k_write_settings() )
      {
        DoDataExchange(FALSE);
        MessageBox( "The settings have been applied" );
        r=true;
      }
      else
      {
        MessageBox( "There was an error applying the settings" );
      }
    }
    return r;
  }


  BOOL if2k_login()
  {
    if2k.set_password( m_password );        
    if2k.set_server_address( m_ip_and_port );
    if2k_logged_in = if2k_collection.get_all_settings();

    return if2k_logged_in;
  }

  BOOL if2k_logout()
  {
    if2k_logged_in = FALSE;

    m_password = "";
    m_good_urls.clear();
    m_bad_urls.clear();
    JdkDDXUpdateGui();
    JdkDlgItemEnable( FALSE );

    return true;
  }

  BOOL if2k_write_settings()
  {
    bool r =if2k_collection.send_all_settings();
    if( r )
    {
       r=if2k.trigger_restart();
    }
    else
    {
      MessageBox( "Error sending settings" );
      return r;
    }

    if( r )
    {
      for( int i=0; i<3; ++i )
      {
         Sleep(3000);
         r=if2k_collection.get_all_settings();
         if( r )
            break;
      }
    }
    else
    {
      MessageBox( "Error triggering restart" );
      return r;
    }

    if( !r )
    {
      MessageBox( "Error getting settings" );
      return r;
    }

    return r;
  }

  BOOL if2k_read_settings()
  {    
    if2k.set_server_address( m_ip_and_port );
    return if2k_collection.get_all_settings();
  }

  
  if2k_mini_client if2k;
  if2k_mini_client_collection if2k_collection;


  BOOL if2k_logged_in;
  CString m_ip_and_port;

  CString m_password;
  jdk_dynbuf &m_good_urls;
  jdk_dynbuf &m_bad_urls;
};





/////////////////////////////////////////////////////////////////////////////



//{{AFX_INSERT_LOCATION}}

// Microsoft Visual C++ will insert additional declarations immediately before the previous line.



#endif // !defined(AFX_MAINDLG_H__604B0AFF_E7EF_4E85_8084_A2F8DC7869D2__INCLUDED_)

