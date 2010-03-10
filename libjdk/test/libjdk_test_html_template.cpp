#include "jdk_world.h"
#include "jdk_html_template.h"
#include "jdk_string.h"

int main( int argc, char **argv )
{
  jdk_settings_text vars(0,argc,argv);
  
  jdk_dynbuf input;
  input.append_stream( stdin );
  
  jdk_dynbuf result;
  jdk_html_template( result, input, &vars );

  result.extract_stream( stdout );
  return 0;
}
