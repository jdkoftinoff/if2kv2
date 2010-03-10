#include "jdk_world.h"
#include "jdk_ipv4.h"

int main()
{
  jdk_ipv4_ip_range *ip=0;
  
  jdk_str<4096> line;
  
  while( jdk_read_string_line( &line, stdin ) )
  {
    if( line.len()>0 )
    {
      jdk_ipv4_ip_range *tmp = new jdk_ipv4_ip_range( line.c_str() );
      if( ip==0 )
        ip=tmp;
      else
        ip->add( tmp );				
    }
    else
    {
      break;	
    }
    
  }
  
  if( ip )
  {		
    ip->dump( stdout );
    
    while( jdk_read_string_line( &line, stdin ) )
    {
      int pa,pb,pc,pd,port;
      
      if( sscanf( line.c_str(), "%d.%d.%d.%d:%d",
                  &pa, &pb, &pc, &pd,
                  &port
            )==5 )
      {
        if( ip->is_ip_in_range(pa,pb,pc,pd,port) )
        {
          printf( "in range\n" );	
        }
        else
        {
          printf( "not in range\n" );
        }								
      }
    }
    
  }
  
  return 0;		
  
}
