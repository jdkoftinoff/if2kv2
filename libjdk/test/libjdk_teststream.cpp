#include "jdk_world.h"
#include "jdk_stream.h"

int main()
{
  jdk_stream_file s( new jdk_stream_style_xml, stdout );
  
  jdk_str<64> line("hello");
  int32 a=1;
  float b=2.3;
  double c=3.4;
  jdk_array< jdk_str<64> > ar;
  ar.add( new jdk_str<64>("item 1") );
  ar.add( new jdk_str<64>("item 2") );
  ar.add( new jdk_str<64>("item 3") );
  ar.add( new jdk_str<64>("item 4") );
  ar.add( new jdk_str<64>("item 5") );
  ar.add( new jdk_str<64>("item 6") );
  ar.add( new jdk_str<64>("item 7") );
  ar.add( new jdk_str<64>("item 8") );
  
  jdk_map< jdk_str<64>, int32 > m;
  m.add( jdk_str<64>("Jeff"), 124567 );
  m.add( jdk_str<64>("Tracy"), 234567 );
  m.add( jdk_str<64>("Mom"), 34567 );
  m.add( jdk_str<64>("Dad"), 4567 );
  
  jdk_stream_write( s, line, "line" );
  jdk_stream_write( s, a, "a" );
  jdk_stream_write( s, b, "b" );
  jdk_stream_write( s, c, "c" );
  jdk_stream_write( s, ar, "ar" );
  jdk_stream_write( s, m, "m" );
}
