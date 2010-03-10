#ifndef _JDK_WIN32WTL_H
#define _JDK_WIN32WTL_H


#include <atlmisc.h>
#include <atlframe.h>
#include <atlddx.h>
#include <atldlgs.h>

#include "jdk_settings.h"
#include "jdk_dynbuf.h"
#include "jdk_string.h"
#include "jdk_util.h"

#define BEGIN_JDK_DDX_MAP(t) \
  void JdkDDXUpdateGui() { DoDataExchange(FALSE); } \
  void JdkDDXReadGui() { DoDataExchange(TRUE); } \
  BEGIN_DDX_MAP(t)

#define END_JDK_DDX_MAP() END_DDX_MAP()

#define JDK_DDX_CHECK(nID, map, key ) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
			JDK_DDX_Check(nID, map, key, bSaveAndValidate);

#define JDK_DDX_TEXT(nID, map, key ) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
			JDK_DDX_Text(nID, map, key, bSaveAndValidate);
      
#define JDK_DDX_BUF_TEXT(nID, dynbuf ) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
			JDK_DDX_Buf_Text(nID, dynbuf, bSaveAndValidate);

#define JDK_DDX_STR_TEXT( nID, str ) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
			JDK_DDX_Str_Text(nID, str, bSaveAndValidate);
      

template <class T>
class CJdkDataExchange :
   public CWinDataExchange<T>
{
public:

  virtual void JdkDDXUpdateGui() = 0;
  virtual void JdkDDXReadGui() = 0;

	void JDK_DDX_Check(UINT nID, jdk_settings &map, const char *key, BOOL bSave)
	{
		T* pT = static_cast<T*>(this);
		HWND hWndCtrl = pT->GetDlgItem(nID);
		if(bSave)
		{
      map.set_bool( key, ::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L) == 0 ? false : true );
		}
		else
		{
			::SendMessage(hWndCtrl, BM_SETCHECK, (int)map.get_bool( key ), 0L);
		}
	}
   

	BOOL JDK_DDX_Buf_Text(UINT nID, jdk_dynbuf &buf, BOOL bSave )
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			HWND hWndCtrl = pT->GetDlgItem(nID);
         buf.clear();

         int len = ::GetWindowTextLength(hWndCtrl);
         buf.resize( len+1);

			::GetWindowText(hWndCtrl, (char *)buf.get_data(), buf.get_buf_length() );
         buf.set_data_length( len );
		}
		else
		{
         jdk_dynbuf termbuf( buf );

         unsigned char zero=0;
         termbuf.append_from_data( &zero, 1 );

			bSuccess = pT->SetDlgItemText(nID, (char *)termbuf.get_data() );
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		return bSuccess;
	}

	BOOL JDK_DDX_Str_Text(UINT nID, jdk_string &str, BOOL bSave )
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			HWND hWndCtrl = pT->GetDlgItem(nID);
         str.clear();

			::GetWindowText(hWndCtrl, str.c_str(), str.getmaxlen() );
		}
		else
		{
			bSuccess = pT->SetDlgItemText(nID, str.c_str() );
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		return bSuccess;
	}


	BOOL JDK_DDX_Text(UINT nID, jdk_settings &map, const char *key, BOOL bSave )
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			HWND hWndCtrl = pT->GetDlgItem(nID);
         jdk_setting_value s;
			 int nRetLen = ::GetWindowText(hWndCtrl, s.c_str(), int(s.getmaxlen()));
         map.set( key, s );
		}
		else
		{
         jdk_setting_value s( map.get(key) );
			bSuccess = pT->SetDlgItemText(nID, s.c_str() );
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		return bSuccess;
	}
 
};


#define BEGIN_JDK_DLGITEM_LIST(DLG) void JdkDlgItemEnable( BOOL V )  { HWND h;
#define JDK_DLGITEM( ITEM ) h=GetDlgItem( ITEM ); ::EnableWindow(h,V);
#define END_JDK_DLGITEM_LIST() }


template <class T,int ID >
class CJdkDialog : 
  public CDialogImpl<T>,
  public CJdkDataExchange<T>,
  public CDialogResize<T>
{
public:
  enum { IDD = ID };

  void JdkSetupIcon()
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

    JdkDDXUpdateGui();
    DlgResize_Init();
  }

  LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    JdkSetupIcon();
    JdkDDXUpdateGui();
		return TRUE;
	}

	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    JdkDDXReadGui();
		EndDialog(wID);
		return 0;
	}


	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
};


#endif
