#include "wnworld.h"
#include "wnregistry.h"
#include "jdk_util.h"

// curuser is searched first
// then user key
// then machine key

// curuser key always lives in HKEY_CURRENT_USER
// user key is selectable at runtime
// machine key always lives in HKEY_LOCAL_MACHINE


WNRegistry::WNRegistry(
                       bool writable,
					   const char *machine_keyname,
					   const char *user_keyname,
					   HKEY user_key_base
					   )

{
	wnregistry_machine_key = 0;
	wnregistry_user_key = 0;
	wnregistry_curuser_key = 0;


	wnregistry_user_key_base = user_key_base;

	if( machine_keyname )
	{
		RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			machine_keyname,
			0,
			writable ? KEY_WRITE : KEY_READ,
			&wnregistry_machine_key
			);
		RegOpenKeyEx(
			user_key_base,
			user_keyname,
			0,
			writable ? KEY_WRITE : KEY_READ,
			&wnregistry_user_key 
			);
	}
	else
	{
		int e;
		if( (e=RegOpenKeyEx(
			HKEY_CURRENT_USER, 
			user_keyname, 
			0, 
			writable ? KEY_WRITE : KEY_READ,
			&wnregistry_machine_key
			))!=0 )
		{
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				e,
				MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0,
				NULL
				);
			//MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK );
			LocalFree( lpMsgBuf);
		}

	}
}

WNRegistry::~WNRegistry()
{
	RegCloseKey( wnregistry_machine_key );
	RegCloseKey( wnregistry_user_key );
}

void WNRegistry::SelectUser( const char *user_keyname )
{
	RegCloseKey( wnregistry_user_key );

	RegOpenKey( wnregistry_user_key_base, user_keyname, &wnregistry_user_key );	
}


bool WNRegistry::WriteInt(
						WNREGISTRY_AREA area,
                        const char *field,
						const char *descriptive_field,
                        long val
                        )
{
	bool r=false;
	
	switch( area )
    {
    case WNREGISTRY_MACHINE:
    	r=WriteMachineInt(0,field,descriptive_field,val);
        break;
    case WNREGISTRY_USER:
    	r=WriteUserInt(0,field,descriptive_field,val);
        break;
    }
	return r;
}

bool WNRegistry::WriteString(
								WNREGISTRY_AREA area,
                                const char *field,
							    const char *descriptive_field,
                                const char *val
                                )
{
	bool r=false;
	
	switch( area )
    {
    case WNREGISTRY_MACHINE:
    	r=WriteMachineString(0,field,descriptive_field,val);
        break;
    case WNREGISTRY_USER:
    	r=WriteUserString(0,field,descriptive_field,val);
        break;
    }
	return r;
}


long WNRegistry::GetInt(
							const char *section,
							const char *field,							
							const char *descriptive_field,
							long default_val
							)
{
	DWORD val = default_val;
	DWORD size = sizeof(val);
	DWORD type;

	{
		// try read user setting

		if( RegQueryValueEx(
				wnregistry_user_key,
				field,
				0,
				&type,
				(LPBYTE)&val,
				&size
			)!=ERROR_SUCCESS || type!=REG_DWORD )
		{
			// didnt work... try read machine key
		
			if( RegQueryValueEx(
					wnregistry_machine_key,
					field,
					0,
					&type,
					(LPBYTE)&val,
					&size
				)!=ERROR_SUCCESS || type!=REG_DWORD )
			{
				// still didnt work. Use default
				// and write default to machine key

				RegSetValueEx(
					wnregistry_machine_key,
					field,
					0,
					REG_DWORD,
					(LPBYTE)&val,
					sizeof( val )
					);
			}
		}
	}

	return val;
}

bool WNRegistry::GetString(
						   const char *section,
						   const char *field,
						   const char *descriptive_field,
						   const char *default_val,
						   char *buf,
						   int buf_size
						   )
{
	DWORD size=buf_size;
	DWORD type;

	{
		// read user key
		if( RegQueryValueEx(
				wnregistry_user_key,
				field,
				0,
				&type,
				(LPBYTE)buf,
				&size
			)!=ERROR_SUCCESS || type!=REG_SZ )
		{
			// didnt work. Try read machine key
			if( RegQueryValueEx(
				wnregistry_machine_key,
				field,
				0,
				&type,
				(LPBYTE)buf,
				&size
				)!=ERROR_SUCCESS || type!=REG_SZ )
			{
				// still didnt work. write default to machine key
				// and return default.

				jdk_strncpy( buf, default_val, buf_size-1 );
				buf[buf_size-1]=0;

				RegSetValueEx(
					wnregistry_machine_key,
					field,
					0,
					REG_SZ,
					(LPBYTE)buf,
					strlen(buf)+1
					);
				return false;
			}
		}
	}

	return true;
}



bool WNRegistry::WriteMachineInt(
									 const char *section,
									 const char *field,
									 const char *descriptive_field,
									 long val
									 )
{
	RegSetValueEx(
		wnregistry_machine_key,
		field,
		0,
		REG_DWORD,
		(LPBYTE)&val,
		sizeof( val )
		);  

	return true;
}


bool WNRegistry::WriteMachineString(
										const char *section,
										const char *field,
										const char *descriptive_field,
										const char *val
							   )
{
	RegSetValueEx(
		wnregistry_machine_key,
		field,
		0,
		REG_SZ,
		(LPBYTE)val,
		strlen(val)+1
		);	

	return true;
}

bool WNRegistry::WriteUserInt(
								  const char *section,
								  const char *field,
								  const char *descriptive_field,
								  long val
							)
{
	RegSetValueEx(
		wnregistry_user_key,
		field,
		0,
		REG_DWORD,
		(LPBYTE)&val,
		sizeof( val )
		);

	return true;
}


bool WNRegistry::WriteUserString(
									 const char *section,
									 const char *field,
									 const char *descriptive_field,
									 const char *val
							   )
{
	RegSetValueEx(
		wnregistry_user_key,
		field,
		0,
		REG_SZ,
		(LPBYTE)val,
		strlen(val)+1
		);	

	return true;
}


bool WNRegistry::DeleteValue(
							 const char *field,
							 const char *descriptive_field
							 )
{
	RegDeleteValue(
				   wnregistry_machine_key,
				   field
				   );
	return true;
}


