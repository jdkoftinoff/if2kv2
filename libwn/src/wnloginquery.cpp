#include "jdk_world.h"
//#include <psapi.h>
#include <winnt.h>
#include <stdio.h>
#include "wnworld.h"
#include "jdk_string.h"
#include "jdk_log.h"
#include "wnloginquery.h"
#include "jdk_winver.h"

static jdk_win_ver wn_windows_version = jdk_win_unknown;

static HMODULE psapi_handle = 0;

typedef DWORD (*WIN_ENUMPROCESSES_PROC)( DWORD *, DWORD, DWORD *);
WIN_ENUMPROCESSES_PROC MyEnumProcesses;

typedef DWORD (*WIN_ENUMPROCESSMODULES_PROC)( HANDLE, HMODULE *, DWORD, DWORD * );
WIN_ENUMPROCESSMODULES_PROC MyEnumProcessModules;

typedef DWORD (*WIN_GETMODULEBASENAME_PROC)(HANDLE,HMODULE,const char *,DWORD);
WIN_GETMODULEBASENAME_PROC MyGetModuleBaseName;

static void wn_login_load_apis()
{
	if( !psapi_handle )
	{
		psapi_handle = LoadLibrary( "PSAPI.DLL" );

		if( psapi_handle )
		{
			MyEnumProcesses = (WIN_ENUMPROCESSES_PROC)GetProcAddress(psapi_handle, "EnumProcesses" );
			MyEnumProcessModules = (WIN_ENUMPROCESSMODULES_PROC)GetProcAddress(psapi_handle, "EnumProcessModules");
			MyGetModuleBaseName = (WIN_GETMODULEBASENAME_PROC)GetProcAddress(psapi_handle, "GetModuleBaseName" );
			if( !MyEnumProcesses || !MyEnumProcessModules || !MyGetModuleBaseName )
			{
				FreeLibrary( psapi_handle );
				psapi_handle=0;
			}
		}
	}
}

bool wn_get_logged_in_user_name_raw( jdk_string *user_name )
{
// first get the default shell process name

	char shell_name[4096];
	bool r = false;
	
	if( !psapi_handle )
	{
		user_name->cpy( "Unknown" );
		return true;
	}

	//printf( "get_raw\n" );
	if( wn_windows_version == jdk_win_unknown )
	{
		wn_windows_version = jdk_win_getver();
	}

	if( (int)wn_windows_version < (int)jdk_win_nt4 )
	{
		user_name->cpy( "unknown" );
		//printf( "bad windows version\n" );
		return false;
	}

	if( jdk_util_registry_get_string( 
			HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "Shell",
			shell_name,
			sizeof( shell_name ) - 1
			) )
	{
		//printf( "got registry shell: '%s'\n", shell_name );
		// next iterate through the process list and find the owner of this process name

		DWORD aProcesses[2048], cbNeeded, cProcesses;
		


		if ( MyEnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
		{	   		
			// Calculate how many process identifiers were returned.
			//printf( "EnumProcesses good\n" );
			cProcesses = cbNeeded / sizeof(DWORD);
						
			for ( unsigned int i = 0; i < cProcesses; i++ )
			{
				// find the process that matches the shell executable
				
				char szProcessName[MAX_PATH] = "unknown";
				
				// Get a handle to the process.
				
				HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
											   PROCESS_VM_READ,
											   FALSE, aProcesses[i] );
				if (NULL != hProcess )
				{
					//printf( "OpenProcess Good\n" );
					HMODULE hMod;
					DWORD cbNeeded;

					if ( MyEnumProcessModules( hProcess, &hMod, sizeof(hMod), 
											 &cbNeeded) )
					{
						MyGetModuleBaseName( hProcess, hMod, szProcessName, 
										   sizeof(szProcessName) );
						//printf( "process id=%ld name='%s'\n", aProcesses[i], szProcessName );
						if( jdk_stricmp( szProcessName, shell_name ) == 0 )
						{
							// found the shell process! Find out the owner!
							//printf( "shell process found\n" );

							HANDLE hToken;
							if( OpenProcessToken(
									hProcess,
									TOKEN_READ,
									&hToken
									) )
							{
								//printf( "OpenProcessToken good\n" );
								char token_info[4096];
								DWORD token_result_len=sizeof(token_info);
								TOKEN_USER *owner=(TOKEN_USER *)&token_info[0];

								if( GetTokenInformation(
										hToken,
										TokenUser,
										&token_info,
										sizeof(token_info),
										&token_result_len
										) )
								{
									//printf( "GetTokenInformation good\n" );
									char looked_up_username[4096];									
									DWORD looked_up_username_len = sizeof( looked_up_username );
									char looked_up_domain[4096];
									DWORD looked_up_domain_len = sizeof( looked_up_domain );
									SID_NAME_USE sid_use;


									if( LookupAccountSid(
											NULL, // LPCTSTR lpSystemName,
											owner->User.Sid, // PSID lpSid,
											looked_up_username, // LPTSTR lpName,
										    &looked_up_username_len,
											looked_up_domain, // LPTSTR lpReferencedDomainName,
											&looked_up_domain_len, //LPDWORD cchReferencedDomainName,
											&sid_use //PSID_NAME_USE peUse
											) )
									{
										//printf( "LookupAccountSid good\n" );
										user_name->cpy( looked_up_username );
										r=true;
									}
								}

								CloseHandle( hToken );
							}

							CloseHandle( hProcess );
							break;
						}
					}
					CloseHandle( hProcess );			   
				}
			}
		}
	}
	else
	{
		//printf( "Registry error\n" );
	}
	if( !r )
	{
		user_name->cpy( "unknown" );
	}
	return r;
}

bool wn_get_logged_in_user_name( jdk_string *user_name )
{
	static bool initted=false;
	static DWORD last_check_time=0;
	static char last_result[1024];

	//printf( "get\n" );
	if( !initted || (last_check_time+4000)>GetTickCount() )
	{
		initted = true;
		last_check_time=GetTickCount();
		wn_get_logged_in_user_name_raw( user_name );
		jdk_strcpy( last_result, user_name->c_str() );
	}
	else
	{
		user_name->cpy( last_result );
	}
	return true;
}
