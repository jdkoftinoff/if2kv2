#include "jdk_world.h"
#include "jdk_serial_number.h"
#include "jdk_settings.h"
#include "if2k_mini_config.h"


extern jdk_setting_description if2k_mini_kernel_defaults[];

int main( int argc, char **argv )
{
  jdk_set_app_name( argv[0] );
  jdk_settings settings( if2k_mini_kernel_defaults, argc, argv );  
  
  jdk_serial_number serial;
  serial.load( settings );

  if( argc>1 )
  {
    jdk_settings input_license_map;
    input_license_map.load_file( argv[1] );
    serial.load( input_license_map );
  }

  // get the date from the license
  {
    struct tm t;
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_mday = 1;
    t.tm_mon = 0;
    t.tm_year = 0;
    t.tm_wday = 0;
    t.tm_yday = 0;
    t.tm_isdst = 0;
    t.tm_gmtoff = 0;
    
    int month, day, year;
    if( sscanf( serial.expires.c_str(), "%d/%d/%d", &year, &month, &day )!=3 )
    {
      fprintf( stderr, "Error parsing date\n" );
      abort();
    }
    
    t.tm_mon = month-1;
    t.tm_mday = day;
    t.tm_year = year - 1900;
    
    time_t time_code = mktime( &t );
    serial.expires.form( "%04d/%02d/%02d", t.tm_year + 1900, t.tm_mon+1, t.tm_mday );
    serial.expires_code.form( "%lu", (unsigned long) time_code );         
  }

  {
    struct timeval tv;
    if( gettimeofday(&tv,0)==0 )
    {
      fprintf( stderr, "Current time is %lu\n", (unsigned long)tv.tv_sec );
    }
//    unsigned long expires_code = strtoul( serial.expires_code.c_str(),0,10 );
    
//    fprintf( stderr, "Time of expiry is: %lu (%s)\n", expires_code, expires_date );
  }
  
  serial.serial=serial.calculate_code( settings.get( "product.id" ) );

  if( serial.is_valid_code(settings.get("product.id")) )
  {
    if( argc>1 )
    {
      serial.save( jdk_string_filename(argv[1]) );
    }
    else
    {
      jdk_settings output_license_map;
      serial.save( output_license_map );

      output_license_map.save_file( stdout, "\r\n" );
    }
    return 0; 
  }
  return 1;
}
