#include "jdk_world.h"
#include "if2k_mini_config.h"
#include "if2k_mini_scanner.h"
#include "if2k_mini_kernel.h"
#include "if2k_mini_kernel_v2.h"

template <class scanner_t>
void run_test( scanner_t &scanner )
{
  // string to load text lines in
  jdk_str<4096> line_buffer;
  
  // read text lines until eof
  jdk_log( JDK_LOG_INFO, "Ready for input" );
  while( jdk_read_string_line( &line_buffer, stdin ) )
  {
    if( !line_buffer.is_clear() )
    {
      if2k_mini_kernel_v2_event<typename scanner_t::tree_traits_t> ev;
      if( scanner.find( line_buffer.c_str(), line_buffer.len(), ev ) )
      {
        jdk_str<4096> best_match_item;
        if( scanner.extract( best_match_item, ev.best_match_item ) )
        {
          jdk_log( 
            JDK_LOG_INFO, 
            "find() returned: best_match_item: '%s', largest_match_value: %d", 
            best_match_item.c_str(), 
            ev.largest_match_value 
            );
        }
      }
      else
      {
        jdk_log( JDK_LOG_INFO, "find() returned 0" );
      }
    }
  }
}


int main( int argc, char **argv )
{
  jdk_set_app_name( argv[0] );
  jdk_set_home_dir( "." );
  jdk_settings_text settings( 0, argc, argv );  
  jdk_log_setup( 
    settings.get_long( "log.type" ),
    settings.get( "log.file" ),
    settings.get_long( "log.detail" )
    );

  if( settings.get_long( "test.precompiled" )==1 )
  {
    if2k_mini_url_scanner_precompiled scanner(
      settings.get("test.local")
      );
    run_test( scanner );
  }
  else
  {
    if2k_mini_url_scanner scanner(
      settings,
      settings.get("test.filename"),
      settings.get("test.localdir"),
      settings.get("test.remoteurl"),
      "",
      10
      );
    run_test( scanner );
  }
}
