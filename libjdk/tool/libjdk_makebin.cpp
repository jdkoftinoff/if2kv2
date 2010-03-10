#include "jdk_world.h"
#include "jdk_string.h"

int makebin_header( FILE *c_out, FILE *h_out, const char *prefix )
{
  jdk_str<128> upper_prefix(prefix);
  upper_prefix.upr();
  
  fprintf( h_out, "#ifndef __%s_H\n#define __%s_H\n#include \"jdk_bindir.h\"\n\n", upper_prefix.c_str(), upper_prefix.c_str() );
  fprintf( h_out, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n" );
  fprintf( h_out, "extern void register_%s();\n\n", prefix );
  fprintf( c_out, "#include \"%s.h\"\n\n", prefix );
  fprintf( c_out, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n" );
  return 0;
}

int makebin_contents( FILE *in, FILE *c_out, FILE *h_out, const char *prefix, const char *name, const char *rawname, const char *group )
{
  fseek( in, 0, SEEK_END );
  long len = ftell(in);
  fseek( in, 0, SEEK_SET );
  fprintf( h_out, "extern jdk_bindir dir_%s_%s;\n", prefix, name );
  
  fprintf( c_out, "unsigned char %s_%s[%ld] = {\n", prefix, name, len );
  
  bool first=true;
  while( !feof(in) )
  {
    int c=fgetc( in );
    if( c==EOF )
      break;
    if( !first )
    {			
      fprintf(c_out, ",\n" );
    }
    fprintf( c_out, "0x%02x", c );
    first=false;
  }
  
  fprintf( c_out, "\n};\n\n" );
  
  fprintf( c_out, "jdk_bindir dir_%s_%s =\n{\n\t\"%s\", \"%s\", %s_%s, %ld, 0\n};\n\n",
           prefix, name, group, rawname, prefix, name, len );
  
  return 0;
}

int makebin_dir_start( FILE *c_out, const char *prefix )
{
  fprintf( c_out, "\n\nvoid register_%s()\n{\n", prefix );
  return 0;	
}

int makebin_dir( FILE *c_out, const char *prefix, const char *name )
{
  fprintf( c_out, "\tjdk_bindir_register( &dir_%s_%s );\n", prefix, name );
  return 0;	
}

int makebin_dir_end( FILE *c_out, const char *prefix )
{
  fprintf( c_out, "}\n\n" );
  return 0;
}


int makebin_footer( FILE *c_out, FILE *h_out )
{	
  fprintf( h_out, "#ifdef __cplusplus\n}\n#endif\n\n" );
  fprintf( h_out, "#endif\n" );
  fprintf( c_out, "#ifdef __cplusplus\n}\n#endif\n\n" );
  fprintf( c_out, "\n\n" );
  return 0;
}



int main( int argc, char **argv )
{
  if( argc<4 )
  {
    fprintf( stderr, "libjdk_makebin usage:\n\tlibjdk_makebin <groupname> <symbolprefix> <inputfilelist...>\n\n" );
  }	
  else
  {
    jdk_string_filename c_fname;
    c_fname.cpy( argv[2] );
    c_fname.cat( ".cpp" );
    jdk_string_filename h_fname;
    h_fname.cpy( argv[2] );
    h_fname.cat( ".h" );
    jdk_string_filename prefix(argv[2]);
    char *p=prefix.rchr('/');
    if( p )
    {
      prefix.cpy( p+1 );
    }
    jdk_string_filename group(argv[1]);
    p=group.rchr('/');
    if( p )
    {
      group.cpy(p+1);
    }
    
    FILE *c_out = fopen( c_fname.c_str(), "wt" );
    FILE *h_out = fopen( h_fname.c_str(), "wt" );
    
    if( c_out && h_out )
    {			
      int i;
      makebin_header( c_out, h_out, prefix.c_str() );
      for( i=3; i<argc; ++i )
      {
        FILE *in = fopen( argv[i], "rb" );
        jdk_string_filename symbol( argv[i] );
        char *end_slash = symbol.rchr('/');
        if(end_slash)
        {
          symbol.cpy(end_slash+1);
        }
        jdk_string_filename rawname(symbol);
        
        for( int j=0; j<symbol.len(); ++j )
        {
          if( !jdk_isalnum(symbol.get(j)) )
          {
            symbol.set(j,'_');
          }
          
        }				
        
        if( in )
        {
          makebin_contents( in, c_out, h_out, group.c_str(), symbol.c_str(), rawname.c_str(), group.c_str() );
          
          fclose(in);
        }
        else
        {
          fprintf( stderr, "\nError opening '%s'\n", argv[i] );
          fclose( c_out);
          fclose( h_out );
          unlink( c_fname.c_str() );
          unlink( h_fname.c_str() );
          exit(1);
        }
      }
      
      makebin_dir_start( c_out, prefix.c_str() );
      
      for( i=3; i<argc; ++i )
      {
        jdk_string_filename symbol( argv[i] );
        char *end_slash = symbol.rchr('/');
        if(end_slash)
        {
          symbol.cpy(end_slash+1);
        }
        jdk_string_filename rawname(symbol);
        
        for( int j=0; j<symbol.len(); ++j )
        {
          if( !jdk_isalnum(symbol.get(j)) )
          {
            symbol.set(j,'_');
          }					
        }				
        
        makebin_dir( c_out, argv[1], symbol.c_str() );				
      }
      
      makebin_dir_end( c_out, argv[1] );
      
      makebin_footer( c_out, h_out );
    }
    
    
    if( c_out )
      fclose(c_out);
    if( h_out )
      fclose(h_out);
    
    return 0;
  }
  return 1;
}



