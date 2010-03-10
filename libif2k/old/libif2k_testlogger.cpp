#include "jdk_world.h"
#include "jdk_util.h"
#include "jdk_log.h"
#include "if2k_logger.h"

int main( int argc, char **argv )
{
  jdk_set_app_name( "libif2_testlogger" );
  jdk_log_setup( JDK_LOG_DEST_STDERR, 0, 8 );
  jdk_settings_text settings( 0, argc, argv );
  
  if2_multi_logger logger( settings );
  
  for( int i=0; i<16; ++i )
  {
    logger.log( if2_kernel_result(i,true), "127.0.0.1", "http://www.testurl.com/", "PATTERN", false );	
    logger.log( if2_kernel_result(i,false), "127.0.0.1", "http://www.testurl.com/", "PATTERN", false );			
    logger.log( if2_kernel_result(i,true), "127.0.0.1", "http://www.testurl.com/", "PATTERN", true );	
    logger.log( if2_kernel_result(i,false), "127.0.0.1", "http://www.testurl.com/", "PATTERN", true );			
    
  }
  return 0;
}
