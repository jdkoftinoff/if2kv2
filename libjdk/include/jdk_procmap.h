#ifndef _JDK_PROCMAP_H
#define _JDK_PROCMAP_H

#include "jdk_string.h"
#include "jdk_fasttree.h"
#include "jdk_dynbuf.h"

template <class INTYPE, class OUTTYPE>
class jdk_procmap
{
public:
	typedef bool (*funcptr)( const INTYPE &in, OUTTYPE &out );
	
	jdk_procmap( const jdk_string &filename )
	{
		load_map( filename );
	}	
	
	virtual ~jdk_procmap()
	{		
	}
	
	inline funcptr lookup_function( int level, const jdk_string &funcname )
	{
		
		jdk_str<128> pretty;
#if JDK_IS_MACOSX
		pretty.form( "_proc%d_%s", level, funcname.c_str() );
#else
		pretty.form( "proc%d_%s", level, funcname.c_str() );
#endif	
		return functions.find( pretty.c_str(), pretty.len() ); 		
	}
	
	int call_function( int level, const jdk_string &funcname, const INTYPE &in, OUTTYPE &out )
	{
		funcptr f = lookup_function( level, funcname );
		if( f )
		{
			return (bool)f(in,out);
		}
		else
		{			
			return -1;
		}		
	}
	
	
private:
	inline bool load_map( const jdk_string &filename )
	{
		FILE *f = fopen( filename.c_str(), "rt" );
		if( f )
		{
			jdk_str<128> s;
			
			while( jdk_read_string_line( &s, f ) )
			{
				if( s.len()>4 )
				{
					char *p = s.chr(' ');
					if( p )
					{
						unsigned long addr = strtol( p+1, 0, 16 );
						*p=0;
						functions.add( s.c_str(), s.len(), (funcptr)addr );					
					}
					
				}			
			}
			
			
			fclose(f);
		}
		
		return false;
		
	}
	
	
	jdk_fasttree< funcptr, 32, char > functions;
};

#endif
