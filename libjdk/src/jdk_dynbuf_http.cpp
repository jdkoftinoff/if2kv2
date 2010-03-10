#include "jdk_world.h"
#include "jdk_dynbuf.h"
#include "jdk_httprequest.h"

bool jdk_buf::append_from_url( const char *url, unsigned long max_len, const char *proxy, bool use_ssl )
{
  jdk_http_response_header response_header;
  if( jdk_http_get( url, this, max_len, &response_header, proxy, use_ssl )==200 )
  {
    return true;
  }
  return false;
}

