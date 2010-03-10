#ifndef _IF2K_SVC_H
#define _IF2K_SVC_H

#include "if2k_config.h"
#include "jdk_settings.h"
#include "if2k_override_map.h"
#include "if2k_kernel.h"
#include "if2k_httpproxy.h"
#include "if2k_serial.h"
#include "jdk_thread.h"

#ifdef IF2K_SVC_FORKED
#include "jdk_server_family.h"
#endif

#if JDK_IS_WIN32


void if2k_notify_ifredir();
void WINAPI if2k_ServiceMain( DWORD argc, LPTSTR *argv );
void WINAPI if2k_Handler( DWORD fdwControl );

bool if2k_ControlService(DWORD fdwControl);
bool if2k_CreateService( const char *path);
bool if2k_DeleteService();
int if2k_RunManually( int argc, char **argv );
int if2k_svc_main( int argc, char **argv );

class if2k_main_service_thread : public jdk_thread
{
public:
  if2k_main_service_thread( DWORD argc_, LPTSTR *argv_ )
    : argc( argc_ ), argv( argv_ )	   
  {
    handle = RegisterServiceCtrlHandler( IF2K_SHORT_SERVICE_NAME, if2k_Handler );	   
  }
  
  SERVICE_STATUS_HANDLE handle;   
protected:
  void main();
  
  DWORD argc;
  LPTSTR *argv;
};


#else

int if2k_svc_main( int argc, char **argv );

#endif

// encrypt all entries in settings that contain 'password' or 'license'

extern jdk_recursivemutex *if2k_load_settings_mutex;
extern if2k_http_proxy_shared *global_shared;
extern if2k_override_map *override_map;
extern if2_kernel_standard *global_kernel;
extern PLATFORM_SETTINGS *global_settings;
extern unsigned long global_session_id;

#if !JDK_IS_WIN32
extern jdk_string_filename global_settings_filename;
#endif

template <class ENCRYPTOR>
inline void
if2k_settings_encrypt(
					unsigned long enc_key,
					const jdk_settings &in,
					jdk_settings &out
					)
{
    jdk_synchronized( in.get_mutex() );
    for( int i=0; i<in.count(); ++i )
    {
		const jdk_settings::pair_t *a = in.get(i);

		if( a )
		{
			if( a->value.get(0)!=0x01)
			{
				// does the entry title contain 'password' or 'license' and does not start with 'text_' ?
				if((a->key.str("password") || a->key.str("license")) && a->key.ncmp("text_",5)!=0 )

				{
					// then we encrypt it
					int key_sum = 0;
					int key_len = a->key.len();
					for(int j=0; j<key_len; ++j )
					{
						key_sum += (unsigned char)a->key.get(j);
					}
					
					ENCRYPTOR enc(enc_key+key_sum);
					jdk_settings::value_t enc_value;
					enc_value.set(0,0x01);
					enc.encrypt_line((const unsigned char *)a->value.c_str(),(unsigned char *)enc_value.c_str()+1,enc_value.getmaxlen()-1);
					out.set( a->key, enc_value );
				}
				else
				{
					out.set( a->key, a->value ); // non-critical fields are unencrypted
				}
			}
			else
			{
				out.set( a->key, a->value );	// it is already encrypted so copy it
			}
		}
	}
}



inline void load_global_settings()
{
	jdk_synchronized( *if2k_load_settings_mutex );
	PLATFORM_SETTINGS encrypted_settings;
	PLATFORM_SETTINGS unencrypted_settings;
	encrypted_settings.PLATFORM_LOAD_SETTINGS();
	jdk_settings_decrypt<IF2K_DECRYPTOR>(ENCRYPTION_KEY,encrypted_settings,unencrypted_settings,true);

	if(! unencrypted_settings.get("remote_settings").is_clear() 
	   && unencrypted_settings.get_long("remote_update_rate")!=0 )
	{
		jdk_remote_buf remote_settings_buf(
					unencrypted_settings.get("cached_settings"),
					unencrypted_settings.get("remote_settings"),
					unencrypted_settings.get("further_proxy" ),
					128*1024,
					false	// dont load if local license file exists
					);
					
		remote_settings_buf.check_and_grab();
		jdk_settings_text remote_settings;
		remote_settings.load_buf( remote_settings_buf.get_buf() );

		jdk_settings_text unencrypted_remote_settings;
		jdk_settings_decrypt<IF2K_DECRYPTOR>(ENCRYPTION_KEY,remote_settings,unencrypted_remote_settings,true);
		unencrypted_settings.merge( unencrypted_remote_settings );
	}
	
	if2_serial_get( unencrypted_settings, ENCRYPTION_KEY, IF2K_SERIAL_URL1, IF2K_SERIAL_URL2, 0); 
	global_settings->merge( unencrypted_settings );
	global_settings->set( "if2k_version", IF2K_VERSION_STRING );
}

inline void save_global_settings()
{
	jdk_synchronized( *if2k_load_settings_mutex );
	PLATFORM_SETTINGS encrypted_settings;
	if2k_settings_encrypt<IF2K_ENCRYPTOR>(ENCRYPTION_KEY,*global_settings,encrypted_settings);
	encrypted_settings.PLATFORM_SAVE_SETTINGS();
}


#endif
