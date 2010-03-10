
#if !JDK_IS_WIN32 // goddam streams dont work on cygwin with win32api
#include <iostream>
#include <cstring>
#include <stdlib.h>

#include "jdk_pool.h"

struct simple_string
{
  simple_string() 
  {
    *buf=0;
  }
  
  explicit simple_string( const char *s )
  {
    std::strncpy( buf, s, 31 );	
  }
  
  
  char buf[32];	
};

inline	int operator == ( const simple_string &a, const simple_string &b )
{
  return std::strcmp( a.buf, b.buf)==0;
}


inline std::ostream & operator << ( std::ostream &ostr, const simple_string &s )
{
  ostr << s.buf;
  return ostr;
}

template <typename A,typename B,typename C>
inline std::ostream & operator << ( std::ostream &ostr, const jdk_pool<A,B,C> &p )
{
  for( typename jdk_pool<A,B,C>::id_t i=0; i<=p.get_size(); ++i )
  {
    if( p.get_usage(i)>0 )
    {
      ostr << "id: " << i << ", usage: " << p.get_usage(i) << ", data: " <<
        *p.get( i ) << std::endl;
    }
  }
  return ostr;
}


typedef jdk_pool<simple_string, short, short> sjdk_pool;

int main()
{
  sjdk_pool p(128);
  
  simple_string a("jeff");
  sjdk_pool::id_t aid = p.find_or_allocate(a);
  
  std::cout << a << " is " << aid << std::endl;
  simple_string b("koftinoff");
  
  sjdk_pool::id_t bid = p.find_or_allocate(b);
  std::cout << b << " is " << bid << std::endl;
  
  sjdk_pool::id_t cid = p.find_or_allocate(b);
  std::cout << "cid is " << cid << std::endl;
  
  std::cout << p;
  
  return 0;
}
#else
int main()
{
  return 0;	
}

#endif
