
#include "jdk_world.h"
#include "jdk_remote_buf.h"
#include "jdk_log.h"
#include "jdk_bindir.h"
#include "jdk_lz.h"

static time_t last_remote_check_time = 0;

bool if2_serial_license_download( 
                                 jdk_settings &settings, 
                                 unsigned long crypt_key, 
                                 const char *base_url, 
                                 const char *serial,
                                 const jdk_string_filename &local_file,
                                 jdk_settings_text &license_settings
                                 )
{
  jdk_string_url remote_url;
  remote_url.cat( base_url );
  remote_url.cat( serial ); 
  
  jdk_remote_buf license_buf( local_file, remote_url, settings.get( "further_proxy" ), 64*1024, true );
  
  // force a remote load if the local file doesn't exist or if it was more than one week since the last time
  
  if( license_buf.get_buf().get_data_length()==0 || (time(0) - last_remote_check_time > 3600 * 24 * 7) )
  {
    license_buf.check_and_grab();
    last_remote_check_time=time(0);
  }
  
  if( license_buf.get_buf().get_data_length()>0 )
  {
    jdk_settings_text license_settings_encrypted;
    license_settings_encrypted.load_buf( license_buf.get_buf() );
    
    jdk_settings_decrypt<jdk_decryptor_complex>(
      crypt_key,
      license_settings_encrypted,
      license_settings,
      false
      );      
    settings.merge( license_settings );   
    return true;
  }
  
  return false;
}

void if2_serial_get( jdk_settings &settings, unsigned long crypt_key, const char *base_url1, const char *base_url2, const char *serial )
{
  jdk_settings_text license_settings;	
  jdk_string_filename local_file;	
  
  jdk_str<1024> orig_serial;
  if( serial )
  {
    orig_serial.cpy(serial);
  }	
  else
  {
    serial=settings.get("serial").c_str();
    orig_serial.cpy(serial);
  }
  orig_serial.strip_endws();
  orig_serial.strip_begws();
  
  bool true_number=true;
  // disallow any non demo serial numbers that are not just plain numbers!
  int orig_serial_len = orig_serial.len();
  for( int i=0; i<orig_serial_len; ++i )
  {
    if( orig_serial.get(i)<'0' || orig_serial.get(i)>'9' )
    {
      true_number=false;
      break;
    }
  }
  
  if( !true_number )
  {
    // use internal compressed license file
    
    local_file.cpy( orig_serial );
    local_file.cat( ".txt.jz" );
    
    jdk_dynbuf license_buf;
    if( jdk_lz_decompress( "licenses", local_file.c_str(), &license_buf ) )
    {
      license_settings.load_buf( license_buf );
      settings.merge( license_settings );				
    }
    else
    {
      // couldnt find it so default to 'demo'
      if( jdk_lz_decompress( "licenses", "demo", &license_buf ) )
      {
        license_settings.load_buf( license_buf );
        settings.merge( license_settings );			
      }
      else
      {
        // whoa! no internal demo license. abort program now!
        jdk_log( JDK_LOG_ERROR, "Internal license error, serial #%s", "demo" );
        abort();
      }
    }
  }
  else if( settings.get("license_validated_serial").cmp( settings.get("serial"))!=0 )
  {
    // load remote encrypted buffer
    jdk_string_filename local_filename_raw;
    local_filename_raw.cat("$/");
    local_filename_raw.cat(orig_serial);
    jdk_process_path( local_filename_raw.c_str(), local_file.c_str(), local_file.getmaxlen() );
    

    if( !if2_serial_license_download( settings, crypt_key, base_url1, serial, local_file, license_settings ) 
      && !if2_serial_license_download( settings, crypt_key, base_url2, serial, local_file, license_settings ) )
    {
      // load internal demo license, cant find it locally or on server			
      jdk_dynbuf license_buf1;      
      if( jdk_lz_decompress( "licenses", "demo.txt.jz", &license_buf1 ) )
      {
        license_settings.load_buf( license_buf1 );
        settings.merge( license_settings );			
      }
      else
      {
        // whoa! no internal demo license. abort program now!
        jdk_log( JDK_LOG_ERROR, "Internal license error, serial #%s", "demo" );
        abort();
      }			
    }
  }
  else
  {
    license_settings.merge( settings );
  }
  
  
  jdk_log( JDK_LOG_INFO, "Serial # %s", orig_serial.c_str() );		
  jdk_log( JDK_LOG_INFO, "Licensed to: %s %s", 
           settings.get("license_name").c_str(),
           settings.get("license_email").c_str() );
  
  jdk_log( JDK_LOG_INFO, "%ld client version",
           settings.get_long("license_clients") );
  
  if( license_settings.find("license_valid")!=-1 
      && license_settings.find("serial")!=-1 )
  {	
    time_t license_expires=0;
    if( license_settings.find("license_expires")!=-1 )
    {
      license_expires = license_settings.get_ulong( "license_expires" );
    }
    long license_valid=license_settings.get_long("license_valid");
    jdk_str<256> license_serial=license_settings.get("serial");
    
    if( license_expires!=0 && time(0) > license_expires )
    {
      jdk_log( JDK_LOG_INFO, "License is expired" );	
      license_valid=false;
    }
    
    if( license_valid==1 && license_serial.cmp( orig_serial )==0 )
    {
      jdk_log( JDK_LOG_INFO, "License is valid" );
      settings.set("license_validated_serial", license_serial );		
    }
    else
    {
      jdk_log( JDK_LOG_INFO, "License is not valid" );
      settings.set("license_valid","0");
    }
  }	
  else
  {
    jdk_log( JDK_LOG_INFO, "License is not valid" );
    settings.set("license_valid","0");
  }	
}

