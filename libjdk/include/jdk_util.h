#ifndef _JDK_UTIL_H
#define _JDK_UTIL_H
#include "jdk_world.h"

#if JDK_IS_VCPP || JDK_IS_VCPP8
#define jdk_mkdir(a,b) CreateDirectoryA((a),0)
#define jdk_chdir(a) SetCurrentDirectoryA((a))
#else
#include <sys/types.h>
#include <sys/stat.h>
#define jdk_mkdir(a,b) mkdir((a),(b))
#define jdk_chdir(a) chdir((a))
#endif

inline size_t jdk_strlen( const char *s )
{
	const char *p = s;
	while( *p )
		p++;
	return size_t( p-s );
}

inline bool jdk_isalnum( char c )
{
	return (c>='0'&& c<='9') || (c>='A' && c<='Z') || (c>='a' && c<='z');
}


inline bool jdk_isspace( char c )
{
	return c==' ' || c=='\t' || c=='\r' || c=='\n';	
}

inline bool jdk_isprint( char c )
{
	return c>=0x20 && c<0x7e;	
}


inline bool jdk_isalpha( char c )
{
	return (c>='A' && c<='Z') || (c>='a' && c<='z');		
}

inline char jdk_toupper( char c )
{
	if( c>='a' && c<='z' )
	{
		c-='a'-'A';
	}
	return c;
}

inline char jdk_tolower( char c )
{
	if( c>='A' && c<='Z' )
	{	
		c+='a'-'A';
	}
	
	return c;
}

inline bool jdk_ispunct( char c )
{
	return jdk_isprint(c) && !jdk_isspace(c) && !jdk_isalnum(c);
}

struct tm * jdk_localtime( time_t *t );
FILE *jdk_fopen( const char *fname, const char *mode );
void jdk_strcat( char *dest, const char *src );
void jdk_strcpy( char *dest, const char *src );

int jdk_sscanf( const char *src, const char *fmt, ... );
int jdk_vsprintf( char *s, const char *fmt, va_list ap );
int jdk_sprintf( char *s, const char *fmt, ... );

bool jdk_strncat( char *dest, const char *src, size_t maxlen );
bool jdk_strncat( char *dest, char c, int maxlen );
bool jdk_strncpy( char *dest, const char *src, size_t maxlen );
char * jdk_strupr( char *str );
char * jdk_strlower( char *str );
int jdk_strcmp( const char *str1, const char *str2 );
int jdk_strncmp( const char *str1, const char *str2, size_t maxlen );
int jdk_stricmp( const char *str1, const char *str2 );
int jdk_strnicmp( const char *str1, const char *str2, size_t maxlen );
char * jdk_strip_crlf( char *buf );
char * jdk_strip_endws( char *buf );
char * jdk_strip_begws( char *buf );
char * jdk_parsequoted( char *dest, const char *src, size_t maxlen );
bool jdk_formpath( char *dest, const char *path, const char *file, const char *extension, size_t maxlen );



bool jdk_set_app_name( const char *name );
const char *jdk_get_app_name();

void jdk_set_home_dir( const char *dir );
bool jdk_get_home_dir( char *dest, size_t max_len );

bool jdk_process_path( const char *src, char *dest, size_t max_len );

void jdk_sleep( int seconds );
void jdk_sleepms( int milliseconds );

#if JDK_IS_WIN32
bool jdk_util_registry_get_string( HKEY area, const char *keyname, const char *field, char *result, int max_len );
bool jdk_util_registry_get_dword( HKEY area, const char *keyname, const char *field, DWORD *result );
bool jdk_util_registry_set_string( HKEY area, const char *keyname, const char *field, const char *s );
bool jdk_util_registry_set_dword( HKEY area, const char *keyname, const char *field, DWORD val );
bool jdk_util_registry_delete_key( HKEY area, const char *keyname, const char *field );
bool jdk_internet_is_connected();
#endif

//#define jdk_null_pointer(a) abort()
#ifndef jdk_null_pointer
extern "C" int jdk_log(
			int detail_level,
			const char *fmt, 
			... 
		    );

inline void jdk_null_pointer( const char *reason ) 
{
	jdk_log( 0, "NULL POINTER: %s", reason );
	abort();
}
#endif

inline bool jdk_strncat( char *dest, const char *src, size_t maxlen ) 
{
	if( !dest || !src )
	{
		jdk_null_pointer("jdk_strncat");
		return false;	
	}	
	
	size_t len = strlen(dest);
	size_t srclen = strlen(src);
	
	if( len+srclen>=maxlen-1 )
	{
		return false;
	}
	
	jdk_strcat( dest, src );
	return true;
} 

inline bool jdk_strncat( char *dest, const char *src, int maxlen ) 
{
  return jdk_strncat( dest, src, (size_t)maxlen );
} 



inline bool jdk_strncat( char *dest, char c, size_t maxlen ) 
{
	if( !dest )
	{
		jdk_null_pointer("jdk_strncat");
		return false;
	}

	size_t len = strlen( dest );
	if( len+1 >= maxlen-1 )
	{
		return false;
	}

	dest[len] = c;
	dest[len+1] = '\0';

	return true;	
} 


inline bool jdk_strncpy( char *dest, const char *src, size_t maxlen ) 
{
	if( !dest || !src )
	{
		jdk_null_pointer("jdk_strncpy");
		return false;	
	}	
	
	size_t len=0;
	
	while( *src )
	{	
		if( len>=maxlen-1 )
		{
			*dest='\0';
			return false;
		}		
		
		*dest++ = *src++;
		len++;
	}
	*dest = '\0';
	
	return true;
} 

inline char * jdk_strupr( char *str ) 
{
	if( !str )
	{
		jdk_null_pointer("jdk_strupr");
		return 0;	
	}	
	
	char *p = str;
	while( *p )
	{	
		*p = jdk_toupper( *p );
		++p;
	}	
	return str;
} 

inline char * jdk_strlower( char *str ) 
{
	if( !str )
	{
		jdk_null_pointer("jdk_strlower");
		return 0;
	}	
	
	char *p=str;
	
	while( *p )
	{
		*p = jdk_tolower( *p );
		++p;
	}
	return str;
} 

inline int jdk_strcmp( const char *str1, const char *str2 ) 
{
	if( !str1 || !str2 )
	{
		jdk_null_pointer("jdk_strcmp");
		return 0;	
	}	
	
	do
	{
		char c1=*str1;
		char c2=*str2;

		if( c1<c2 )
		{
			return -1;
		}
		if( c1>c2 )
		{
			return 1;
		}
		
		if( c1==0 || c2==0 )
		{
			break;
		}
		
		
		str1++;
		str2++;
	} while( 1 );
	
	return 0;
} 

inline int jdk_strncmp( const char *str1, const char *str2, size_t maxlen ) 
{
	if( !str1 || !str2 )
	{
		jdk_null_pointer("jdk_strncmp");
		return 0;	
	}	
	
	size_t len=0;
	
	do
	{
		char c1=*str1;
		char c2=*str2;

		
		if( c1<c2 )
		{
			return -1;
		}
		if( c1>c2 )
		{
			return 1;
		}
		
		if( c1==0 || c2==0 )
		{
			break;
		}
				
		str1++;
		str2++;
		len++;
		
	} while( len<maxlen );
	
	return 0;
} 

inline int jdk_stricmp( const char *str1, const char *str2 ) 
{
	if( !str1 || !str2 )
	{
		jdk_null_pointer("jdk_stricmp");
		return 0;
	}	
	
	do
	{
		char c1=*str1;
		char c2=*str2;

		if( c1>='A' && c1<='Z' )
		{
			c1 += ('a'-'A');
		}
		if( c2>='A' && c2<='Z' )
		{
			c2 += ('a'-'A');
		}
		
		if( c1<c2 )
		{
			return -1;
		}
		if( c1>c2 )
		{
			return 1;
		}
		
		if( c1==0 || c2==0 )
		{
			break;
		}
		
		
		str1++;
		str2++;
	} while( 1 );
	
	return 0;
}

inline int jdk_strnicmp( const char *str1, const char *str2, size_t maxlen ) 
{
	if( !str1 || !str2 )
	{
		jdk_null_pointer("jdk_strnicmp");
		return 0;	
	}	
	
	size_t len=0;
	
	do
	{
		char c1=*str1;
		char c2=*str2;

		if( c1>='A' && c1<='Z' )
		{
			c1 += ('a'-'A');
		}
		if( c2>='A' && c2<='Z' )
		{
			c2 += ('a'-'A');
		}
		
		if( c1<c2 )
		{
			return -1;
		}
		if( c1>c2 )
		{
			return 1;
		}
		
		if( c1==0 || c2==0 )
		{
			break;
		}
				
		str1++;
		str2++;
		len++;
		
	} while( len<maxlen );
	
	return 0;
}

inline char * jdk_strip_crlf( char *buf ) 
{
	if( !buf )
	{
		jdk_null_pointer("jdk_strip_crlf");
		return 0;	
	}
	
	char *p=buf;
	
	while( *p )
	{
		if( *p=='\r' || *p=='\n' )
		{
			*p='\0';
			break;
		}		
		++p;
	}

	return buf;
} 

inline char * jdk_strip_endws( char *buf ) 
{
	if( !buf )
	{
		jdk_null_pointer("jdk_strip_endws");
		return 0;	
	}
	
  	for( size_t i=strlen(buf)-1; i>=0; --i )	
	{
		char c = buf[i];
		
		if( c=='\r' || c=='\n' || c==' ' || c=='\t' )
		{
			buf[i]='\0';
		}		
		else
		{
			break;
		}
		
	}

	return buf;
} 

inline char * jdk_strip_begws( char *buf ) 
{	
	if( !buf )
	{
		jdk_null_pointer("jdk_strip_begws");
		return 0;	
	}
	
	size_t skip = strspn(buf,"\r\n \t" );
	
	if( skip!=0 )
	{
      size_t len = strlen(buf) - skip;

      for( size_t i=0; i<=len; ++i )
      {
        buf[i] = buf[i+skip];
      }
	}
	return buf;
} 

inline char * jdk_parsequoted( char *dest, const char *src, size_t maxlen ) 
{
	if( !dest || !src )
	{
		jdk_null_pointer("jdk_parsequoted");
		return 0;	
	}
	
	char *d = dest;
	const char *first_quote=0;
	size_t curlen=0;
	
	while( *src )
	{
		if( *src == '"' )
		{
			first_quote = src;
			break;
		}
		++src;
	}
	
	if( first_quote )
	{
		// found first quote.
		// skip over it
		++src;
		
		// now copy things over to dest
		
		while( *src )
		{
			if( *src=='"' )
			{
				*d++ = 0;
				
				// we completed the string!
				return d;
			}
			
			*d++ = *src++;
			
			curlen++;
			
			if( curlen >= maxlen )
			{
				// too long!
				break;
			}
			
		}
		
	}
		
	// either too long or not matching quotes or no quote.
	
	return 0;	
	
}

inline void jdk_sleep( int seconds )
{
#if JDK_IS_WIN32
    Sleep( seconds*1000 );
#else
    sleep( seconds );
#endif
}

inline void jdk_sleepms( int milliseconds )
{
#if JDK_IS_WIN32
    Sleep( milliseconds );
#else
    usleep( milliseconds*1000 );
#endif

}

inline const char *jdk_find_extension( const char *path )
{
	size_t pos=strlen(path);
	while( --pos>=0 )
	{
		if( path[pos]=='.' )
		  return &path[pos];
		if( path[pos]=='/' || path[pos]=='\\' )
		  return 0;
	}
	return 0;
}

inline unsigned long jdk_get_time_seconds()
{
    return (unsigned long)time(0);
}



#endif
