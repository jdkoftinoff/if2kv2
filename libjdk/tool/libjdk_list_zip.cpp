#include "jdk_world.h"
#include "jdk_zipfile.h"


int main( int argc, char **argv )
{
  if( argc>1 )
  {
    jdk_string_filename zip_filename(argv[1]);
    jdk_dynbuf zip_buf;
    if( zip_buf.append_from_file( zip_filename ) )
    {
      jdk_zipfile_reader z( zip_buf );

      jdk_zipfile_enumerator i( z );
      jdk_string_filename f_in_zipfile;
      
      while( i.get_filename( f_in_zipfile ) )
      {
        fprintf( stdout, "%s\n", f_in_zipfile.c_str() );
        i.next();
      }
     
    }
  }
  return 0;
}
