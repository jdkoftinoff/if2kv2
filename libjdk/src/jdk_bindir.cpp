#include "jdk_world.h"
#include "jdk_util.h"
#include "jdk_bindir.h"


struct jdk_bindir *jdk_bindir_top=0;
struct jdk_bindir *jdk_bindir_bottom=0;


jdk_bindir *jdk_bindir_find( const char *group, const char *name )
{
  jdk_bindir *d = jdk_bindir_top;
  
  while( d )
  {
    if( jdk_strcmp( d->group, group )==0 && jdk_strcmp( d->name, name )==0 )
    {
      break;	
    }		
    d=d->next;
  }
  
  return d;
}





