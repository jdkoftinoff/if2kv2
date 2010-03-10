
#include "jdk_world.h"
#include "jdk_url.h"


int main()
{
  jdk_string_url url_text;
  while( jdk_read_string_line(&url_text,stdin) )
  {
    jdk_url url(url_text);
    fprintf( stdout, "'%s' : // '%s' : '%d' '%s'\n",
             url.protocol.c_str(),
             url.host.c_str(),
             url.port,
             url.path.c_str()
      );
  }
  return 0;
}

