// if2k_cleanup.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
 
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>

#include "resource.h"

#include "jdk_world.h"

#include "if2k_mini_config.h"

#include "jdk_settings.h"
#include "jdk_win32wtl.h"

extern jdk_setting_description if2k_mini_kernel_defaults[];

bool if2k_is_deleted=false;

void WNInstall_RemoveDirectories(const char *dirs, bool recurse)
{
    char buf[1024];

    strcpy( buf, dirs );

    if (recurse)
    {
        HANDLE h;
        WIN32_FIND_DATA fd;

        char *p = buf;
        while( *p )
            ++p;

        strcat(buf,"\\*.*" );

        h = FindFirstFile(buf,&fd);
        if (h != INVALID_HANDLE_VALUE)
        {
          do
          {
            if (fd.cFileName[0] != '.' ||
                (fd.cFileName[1] != '.' && fd.cFileName[1]))
            {
                strcpy(p+1,fd.cFileName);
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                    SetFileAttributes(buf,fd.dwFileAttributes^FILE_ATTRIBUTE_READONLY);
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    WNInstall_RemoveDirectories(buf,recurse);
                }
                else
                {
                    DeleteFile(buf);
                }
            }
          } while (FindNextFile(h,&fd));

          FindClose(h);
        }
    }

    RemoveDirectory(buf);
}


int WNInstall_RegDeleteKeys(HKEY thiskey, const char * lpSubKey)
{
	HKEY key;
	int retval=RegOpenKey(thiskey,lpSubKey,&key);
	if (retval==ERROR_SUCCESS)
	{
		char buffer[1024];
		while (RegEnumKey(key,0,buffer,1024)==ERROR_SUCCESS)
            if ((retval=WNInstall_RegDeleteKeys(key,buffer)) != ERROR_SUCCESS) break;

		RegCloseKey(key);
		retval=RegDeleteKey(thiskey,lpSubKey);
	}
	return retval;
}

void delete_if2k()
{
  char path[4096];
  GetSystemDirectory(path,sizeof(path) );
  strcat( path, "\\if2k" );

  char if2kd_path[4096];

  strcpy( if2kd_path, path );
  strcat( if2kd_path, "\\bin\\if2kd.exe -u" );
  
  WinExec( if2kd_path, SW_HIDE );
  Sleep(1000);

  WNInstall_RemoveDirectories( path, true );
  GetSystemDirectory(path,sizeof(path) );
  strcat( path, "\\if2k_redir.dll" );
  DeleteFile( path );
  
  WNInstall_RegDeleteKeys( HKEY_LOCAL_MACHINE, IF2K_MINI_REGISTRY_LOCATION );

  if2k_is_deleted=true;
}





// if2k_only.cpp : main source file for if2k_only.exe
//


CAppModule _Module;

jdk_settings_win32registry *settings=0;



class CMainDlg : 
  public CJdkDialog<CMainDlg, IDD_DIALOG1>
{
public:
  CMainDlg()
  {
    m_password = "password";
  }

  CString m_password;

  BEGIN_JDK_DDX_MAP(CMainDlg)
    DDX_TEXT( IDC_EDIT1,m_password)
  END_JDK_DDX_MAP()


  BEGIN_JDK_DLGITEM_LIST(CMainDlg)
    JDK_DLGITEM( IDOK )
    JDK_DLGITEM( IDCANCEL )
  END_JDK_DLGITEM_LIST()


  BEGIN_DLGRESIZE_MAP(CMainDlg)
  END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    CHAIN_MSG_MAP(CDialogResize<CMainDlg>)
  END_MSG_MAP()




	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
    JdkSetupIcon();
      
    JdkDlgItemEnable(TRUE);
    JdkDDXUpdateGui();
		return TRUE;
	}



	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    JdkDDXReadGui();
    jdk_str<512> p;
    jdk_str<512> raw( m_password );
    if2k_hash_for_password( p, raw );
 
    jdk_str<1024> actual = settings->get("kernel.remote.update.password");
    jdk_str<1024> m;
    //m.form( "raw=%s p=%s actual=%s", raw.c_str(), p.c_str(), actual.c_str() );
    //MessageBox( m.c_str(), "DEBUG", MB_OK );

    if( p == actual )
    {
      delete_if2k();
		  EndDialog(wID);
    }
    else
    {
      int id=MessageBox( "Incorrect password", "Internet Filter Uninstall", MB_RETRYCANCEL );
      if( id==IDCANCEL )
      {
        EndDialog(wID);
      }
    }

	  return 0;
	}



	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{   
		EndDialog(wID);

		return 0;
	}
};



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
  jdk_settings_text default_settings(if2k_mini_kernel_defaults,__argc,__argv);
  jdk_settings_win32registry main_settings;

  main_settings.merge( default_settings );
  jdk_string_filename home( main_settings.get("home").c_str() );

  jdk_set_app_name( "if2k_cleanup" );
  main_settings.load_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_INSTALL_REGISTRY_LOCATION );
	main_settings.load_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_LICENSE_REGISTRY_LOCATION );
  main_settings.load_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_ADDITIONAL_REGISTRY_LOCATION );

  settings = &main_settings;

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
		nRet = dlgMain.DoModal();
	}

	_Module.Term();

	::CoUninitialize();



  if( if2k_is_deleted )
  {
    return 0;
  }
  else
  {
    return 1;
  }

}

