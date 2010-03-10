#include "jdk_world.h"
#include "if2k_client.h"

int main( int argc, char **argv )
{
  if2k_client_settings settings;
  if2k_client client;
  client.set_connection( 
    jdk_string_url("127.0.0.1:8000"),
    jdk_string_url(""),
    jdk_string_url("password")
    );
  
  bool r = client.load_settings( settings );
  fprintf( stderr, "load_settings returns %d\n", (int)r );
  jdk_dynbuf txt;
  settings.settings.save_buf(&txt);
  fprintf( stdout, "\n\nSETTINGS:\n" );
  txt.extract_stream( stdout );
  
  for( int i=0; i<if2k_client_settings::num_lists; ++i )
  {
    for( int j=0; j<if2k_client_settings::num_categories; ++j )
    {
      fprintf( stdout, "FILE %d,%d field: %s file: %s title: %s (enabled=%d)\n", 
               i,j,
               settings.name_base[i][j].c_str(),
               settings.get_list_category_file( i,j ).c_str(),
               settings.get_list_category_title( i, j ).c_str(),
               (int)settings.get_list_category_file_enable(i,j)
        );
      
      settings.list[i][j].extract_stream( stdout );
    }
  }
  
  return 0;
}
