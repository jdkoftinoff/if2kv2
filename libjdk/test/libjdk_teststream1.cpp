#include "jdk_world.h"
#include "jdk_stream.h"

int main()
{
  jdk_stream_file s( new jdk_stream_style_xml, stdin );
  jdk_stream_file o( new jdk_stream_style_text, stdout );
  
  jdk_str<64> line;
  int32 a=0;
  float b=0;
  double c=0;
  jdk_array< jdk_str<64> > ar;
  jdk_map< jdk_str<64>, int32 > m;
  
  jdk_stream_read( s, line, "line" );
  jdk_stream_write( o, line, "line" );
  
  jdk_stream_read( s, a, "a" );
  jdk_stream_write( o, a, "a" );
  
  jdk_stream_read( s, b, "b" );
  jdk_stream_write( o, b, "b" );
  
  jdk_stream_read( s, c, "c" );
  jdk_stream_write( o, c, "c" );
  
  jdk_stream_read( s, ar, "ar" );
  jdk_stream_write( o, ar, "ar" );
  
  jdk_stream_read( s, m, "m" );
  jdk_stream_write( o, m, "m" );
  
}










