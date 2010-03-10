#ifndef _JDK_ARCHIVE_H
#define _JDK_ARCHIVE_H


#include <iostream>


template <class OSTREAM> 
OSTREAM & jdk_archive_indent( OSTREAM &s, int level )
{
	int i;
	
	for( i=0; i<level; ++i )
	{
		s << ' ';	
	}

	return s;
}


template <class ISTREAM>
bool jdk_until_block_end( ISTREAM &s )
{
	s >> std::ws;
	
	int x = s.get();
	if( x=='}' )
	{
		return false;
	}
	else
	{
		s.putback(x);
		s >> std::ws;		
		return true;
	}	
}

template <class OSTREAM>
OSTREAM & jdk_begin_block( OSTREAM &s, int level, const char *name=0 )
{	
	if( name )
	{	
		s << "\n";
		jdk_archive_indent( s, level );		
		s << "# " << name << "\n";
	}	

	jdk_archive_indent( s, level );	
	s << '{' << "\n";
	return s;
}

template <class OSTREAM> 
OSTREAM & jdk_end_block( OSTREAM &s, int level )
{
	jdk_archive_indent( s, level );   	
	s << '}' << "\n";
	return s;
}

template <class ISTREAM>
ISTREAM & jdk_find_begin_block( ISTREAM &s )
{
	int c;
	s >> std::ws;	
	do 
	{
		c=s.get();
		if( c=='#' )
		{
			// eat entire line
			s.ignore(1,'\n');
			s >> std::ws;
		}
		
	} while( c!='{' );

	s >> std::ws;	
	return s;
}


template <class ISTREAM> 
ISTREAM & jdk_find_end_block( ISTREAM &s )
{
	int paren_count=1;
	
	int c;
	s >> std::ws;	
	do 
	{
		c=s.get();
		
		if( c=='#' )
		{
			// eat entire line
			s.ignore(1,'\n');
			s >> std::ws;
		}
		else
		if( c=='{' )
		{
			paren_count++;
		}
		else if( c=='}' )
		{
			paren_count--;
		}		
	} while( c!='}' && paren_count==0 );
	
	return s;
}



template <class OSTREAM, class CONTAINER>
OSTREAM & jdk_archive_container( OSTREAM &s, const CONTAINER &v, int level, const char *name = 0 )
{
	jdk_begin_block( s, level, name );
	
	for( size_t i = 0; i<v.size(); i++ )
	{
		jdk_archive( s, v[i], level+1 );
	}
	jdk_end_block( s, level );
	
	return s;
}

template <class OSTREAM>
OSTREAM & jdk_archive_empty_container( OSTREAM &s, int level, const char *name = 0 )
{
	jdk_begin_block( s, level, name );	
	jdk_end_block( s, level );
	
	return s;
}

template <class OSTREAM, class CONTAINER>
OSTREAM & jdk_archive_container_ptr( OSTREAM &s, const CONTAINER *v, int level, const char *name = 0 )
{
	jdk_begin_block( s, level, name );
	
	if( v && v->size()>0 )
	{		
		for( size_t i = 0; i<v->size(); i++ )
		{
			jdk_archive( s, (*v)[i], level+1 );
		}

	}

	jdk_end_block( s, level );		
	
	return s;
}


template <class ISTREAM, class CONTAINER>
ISTREAM & jdk_unarchive_container ( ISTREAM &s, CONTAINER &v )
{
	jdk_find_begin_block( s );

	while( jdk_until_block_end(s) )
	{					
		typename CONTAINER::value_type tmp;
		
		jdk_unarchive( s, tmp );
		v.push_back( tmp );
	}

	return s;
}

template <class ISTREAM, class CONTAINER>
ISTREAM & jdk_unarchive_container_ptr ( ISTREAM &s, CONTAINER * &v )
{
	jdk_find_begin_block( s );

	delete v;
	v = new CONTAINER;
	
	while( jdk_until_block_end(s) )
	{					
		typename CONTAINER::value_type tmp;
		
		jdk_unarchive( s, tmp );
		v->push_back( tmp );
	}
	
	if( v->size()==0 )
	{
		delete v;
		v=0;
	}
	

	return s;
}



template <class ISTREAM>
ISTREAM & jdk_unarchive( ISTREAM &stream, std::string &s )
{
	stream >> std::ws;
	
	s.erase();


	
	// find open quote
	int c;	
	do 
	{

		c=stream.get();

		if( c=='#' )
		{
			// eat entire line
			stream.ignore(1,'\n');
			stream >> std::ws;
		}		
	} while( c!='"' );
	
	char buf[2];
	// suck string with translation
	while(1)
	{
		c=stream.get();
		
		if( c=='\\' )
		{
			c=stream.get();
			
			switch(c)
			{
				case 't':
					c='\t';
					break;
				case 'n':
					c='\n';
					break;				
			    case 'r':
					c='\r';
					break;
				
				default:
					break;
			}			
		}
	   	else if( c=='"' )
		{
			break;	
		}		
		buf[0] = c;
		buf[1] = 0;
		s.append(buf);
   	}
	return stream;
}

template <class OSTREAM>
OSTREAM & jdk_archive( OSTREAM &stream, const std::string &s, int level )
{
	jdk_archive_indent( stream, level );

	stream << '"';
	
	for( size_t i=0; i<s.length(); ++i )
	{
		char c=s[i];
		
		switch( c )
		{
			case '"':
				stream << "\\\"";
				break;
			case '\n':
				stream << "\\n";
				break;
		    case '\r':
				stream << "\\r";
				break;
		    case '\t':
				stream << "\\t";
				break;
		    case '\\':
				stream << "\\\\";
				break;			
			default:
				stream << c;
				break;
		}
		
	}
	
	stream << '"' << "\n";
	
	return stream;	
}



#endif
