#ifndef _JDK_IPV4_H
#define _JDK_IPV4_H

#include "jdk_socket.h"
#include "jdk_log.h"
#include "jdk_util.h"



class jdk_ipv4_ip_range
{
private:
	
    explicit jdk_ipv4_ip_range( const jdk_ipv4_ip_range & );
    const jdk_ipv4_ip_range & operator = ( const jdk_ipv4_ip_range & );
  
	static inline const char * skip_chars( const char *p, const char *c )
	{
		while(1)
		{
			const char *d=c;
			while( *d )
			{
				if( *p==*d )
				{
					break;	
				}				
				++d;
			}
			if( !*d )
			{
				break;	
			}
			++p;
		}
		return p;
	}
	
	static inline const char * my_atoi( int *val, const char *p, const char *seps="" )
	{
		bool valid=false;
		p = skip_chars( p, seps );
		
		*val=0;
		
		while( *p>='0' && *p<='9' )
		{
			*val=*val*10;
			*val += *p-'0';
			++p;
			valid=true;
		}
		
		return valid ? p : 0;
	}
	
public:
	jdk_ipv4_ip_range(
					  unsigned char _a,
					  unsigned char _b,
					  unsigned char _c,
					  unsigned char _d,
					  int _lowest_port=0,
					  int _highest_port=65535
					  )
	  :
	a(_a),
	b(_b),
	c(_c),
	d(_d),
	maska(0),
	maskb(0),
	maskc(0),
	maskd(0),
	lowest_port(_lowest_port),
	highest_port(_highest_port),
	next(0)
	{
	}
	

	jdk_ipv4_ip_range(
					  unsigned char _a,
					  unsigned char _b,
					  unsigned char _c,
					  unsigned char _d,
					  unsigned char _maska,
					  unsigned char _maskb,
					  unsigned char _maskc,
					  unsigned char _maskd,
					  int _lowest_port=0,
					  int _highest_port=65535					  
					  )
	  :
	a(_a),
	b(_b),
	c(_c),
	d(_d),
	maska(_maska),
	maskb(_maskb),
	maskc(_maskc),
	maskd(_maskd),	
	lowest_port(_lowest_port),
	highest_port(_highest_port),
	next(0)
	{
	}
	  
	
	jdk_ipv4_ip_range(
					  const char *ascii_ip_and_mask_list
					  )
	  :
	a(0),
	b(0),
	c(0),
	d(0),
	maska(0),
	maskb(0),
	maskc(0),
	maskd(0),	
	lowest_port(0),
	highest_port(65535),
	next(0)
	{
		// skip leading whitespace and commas
		const char *p = ascii_ip_and_mask_list;
		
		while(*p && (jdk_isspace(*p) || *p==',') )
		{
		  ++p;
		}
		
		if( *p )
		{			

			extract_ascii( p );
			// is there more entries in the list?
			while(*p && ((*p>='0'&&*p<='9') || (*p==':') || (*p=='-') || (*p=='.') || (*p=='/') ) )
			{
				++p;
			}

			while(*p && (jdk_isspace(*p) || *p==',') )
			{
			  ++p;
			}			

			if( *p )
			{
				// more numeric data?
				if( *p>='0' && *p<='9' )
				{
					// yup, create a new entry
					next = new jdk_ipv4_ip_range( p );	
				}				
			}						
		}		
	}
	
	bool extract_fmt1( const char *s )
	{
		int pa,pb,pc,pd;
		int pma,pmb,pmc,pmd;
		
		if( s )
			s=my_atoi( &pa, s, " \t\r\n" );
		if( s )
			s=my_atoi( &pb, s, "." );
		if( s )
		  	s=my_atoi( &pc, s, "." );
		if( s )
		  	s=my_atoi( &pd, s, "." );
		if( s )
		  	s=my_atoi( &pma, s, "/" );
		if( s )
		  	s=my_atoi( &pmb, s, "." );
		if( s )
		  	s=my_atoi( &pmc, s, "." );
		if( s )
		  	s=my_atoi( &pmd, s, "." );
		if( s )
		  	s=my_atoi( &lowest_port, s, ":" );
		if( s )
		  	s=my_atoi( &highest_port, s, "-" );
		if( s )
		{
			a=pa; b=pb; c=pc; d=pd;
			maska=pma; maskb=pmb; maskc=pmc; maskd=pmd;
			return true;
		}
		else
		{
			return false;	
		}				
	}

	bool extract_fmt2( const char *s )
	{
		int pa,pb,pc,pd;
		int pma,pmb,pmc,pmd;
		
		if( s )
			s=my_atoi( &pa, s, " \t\r\n" );
		if( s )
			s=my_atoi( &pb, s, "." );
		if( s )
		  	s=my_atoi( &pc, s, "." );
		if( s )
		  	s=my_atoi( &pd, s, "." );
		if( s )
		  	s=my_atoi( &pma, s, "/" );
		if( s )
		  	s=my_atoi( &pmb, s, "." );
		if( s )
		  	s=my_atoi( &pmc, s, "." );
		if( s )
		  	s=my_atoi( &pmd, s, "." );
		if( s )
		  	s=my_atoi( &lowest_port, s, ":" );
		if( s )
		{
			a=pa; b=pb; c=pc; d=pd;
			maska=pma; maskb=pmb; maskc=pmc; maskd=pmd;
			highest_port=lowest_port;
			return true;
		}
		else
		{
			return false;	
		}				
	}

	bool extract_fmt3( const char *s )
	{
		int pa,pb,pc,pd;
		int pma,pmb,pmc,pmd;
		
		if( s )
			s=my_atoi( &pa, s, " \t\r\n" );
		if( s )
			s=my_atoi( &pb, s, "." );
		if( s )
		  	s=my_atoi( &pc, s, "." );
		if( s )
		  	s=my_atoi( &pd, s, "." );
		if( s )
		  	s=my_atoi( &pma, s, "/" );
		if( s )
		  	s=my_atoi( &pmb, s, "." );
		if( s )
		  	s=my_atoi( &pmc, s, "." );
		if( s )
		  	s=my_atoi( &pmd, s, "." );
		if( s )
		{
			a=pa; b=pb; c=pc; d=pd;
			maska=pma; maskb=pmb; maskc=pmc; maskd=pmd;
			lowest_port=0;
			highest_port=65535;
			return true;
		}
		else
		{
			return false;	
		}				
	}	

	bool extract_fmt4( const char *s )
	{
		int pa,pb,pc,pd;
		
		if( s )
			s=my_atoi( &pa, s, " \t\r\n" );
		if( s )
			s=my_atoi( &pb, s, "." );
		if( s )
		  	s=my_atoi( &pc, s, "." );
		if( s )
		  	s=my_atoi( &pd, s, "." );
		if( s )
		  	s=my_atoi( &lowest_port, s, ":" );
		if( s )
		  	s=my_atoi( &highest_port, s, "-" );
		
		if( s )
		{
			a=pa; b=pb; c=pc; d=pd;
			return true;
		}
		else
		{
			return false;	
		}				
	}	

	bool extract_fmt5( const char *s )
	{
		int pa,pb,pc,pd;
		
		if( s )
			s=my_atoi( &pa, s, " \t\r\n" );
		if( s )
			s=my_atoi( &pb, s, "." );
		if( s )
		  	s=my_atoi( &pc, s, "." );
		if( s )
		  	s=my_atoi( &pd, s, "." );
		if( s )
		  	s=my_atoi( &lowest_port, s, ":" );
		
		if( s )
		{
			a=pa; b=pb; c=pc; d=pd;
			highest_port=lowest_port;
			return true;
		}
		else
		{
			return false;	
		}				
	}	

	bool extract_fmt6( const char *s )
	{
		int pa,pb,pc,pd;
		
		if( s )
			s=my_atoi( &pa, s, " \t\r\n" );
		if( s )
			s=my_atoi( &pb, s, "." );
		if( s )
		  	s=my_atoi( &pc, s, "." );
		if( s )
		  	s=my_atoi( &pd, s, "." );
		
		if( s )
		{
			a=pa; b=pb; c=pc; d=pd;
			lowest_port=0;
			highest_port=65535;
			return true;
		}
		else
		{
			return false;	
		}				
	}	
	
	void extract_ascii( const char *ascii_ip_and_mask )
	{
		if( !extract_fmt1(ascii_ip_and_mask)
		   && !extract_fmt2(ascii_ip_and_mask)
		   && !extract_fmt3(ascii_ip_and_mask)
		   && !extract_fmt4(ascii_ip_and_mask)
		   && !extract_fmt5(ascii_ip_and_mask)
		   && !extract_fmt6(ascii_ip_and_mask)
		   )
		{
			// do nothing but log an error
			//jdk_log( JDK_LOG_ERROR, "IPV4 address parse error: %s", ascii_ip_and_mask );
		}
		
	}	
	
	
	
	
	
	~jdk_ipv4_ip_range()
	{
		delete next;	
	}
	
	
	inline void add( jdk_ipv4_ip_range *r )
	{
		if( !next )
		  next=r;
		else
		{			
			jdk_ipv4_ip_range *n=next;
			while( n->next )
			{
				n=n->next;
			}
			n->next = r;
		}					
	}

	inline void dump( FILE *f ) const
	{
		fprintf( f, "%d.%d.%d.%d/%d.%d.%d.%d:%d-%d\n",
				a,b,c,d, maska, maskb, maskc, maskd, lowest_port, highest_port );
		if( next )
		{
			next->dump(f);
		}
		
	}
	
	
	inline bool is_ip_in_range( 
							   unsigned char testa,
							   unsigned char testb,
							   unsigned char testc,
							   unsigned char testd,
							   int testport								   
							   ) const
	{
		bool ret = false;
		
		const jdk_ipv4_ip_range *n=this;
		
		while(n)
		{
			ret = (testa & ~n->maska)==(n->a & ~n->maska)
			  	&& (testb & ~n->maskb)==(n->b & ~n->maskb)
				&& (testc & ~n->maskc)==(n->c & ~n->maskc)
				&& (testd & ~n->maskd)==(n->d & ~n->maskd)
				&& (testport>=n->lowest_port)
				&& (testport<=n->highest_port);
			if( ret )
			{
				break;
			}
			n=n->next;
		}		
		return ret;		
		
	} 
	
	inline bool is_ip_in_range( 
							   unsigned long testip, // in host byte order
							   int testport								   
							   ) const
	{
		bool ret = false;
		unsigned char testa = (unsigned char)((testip&0xff000000)>>24);
		unsigned char testb = (unsigned char)((testip&0x00ff0000)>>16);
		unsigned char testc = (unsigned char)((testip&0x0000ff00)>>8);
		unsigned char testd = (unsigned char)((testip&0x000000ff)>>0);
		
		const jdk_ipv4_ip_range *n=this;
		
		while(n)
		{
			ret = (testa & ~n->maska)==(n->a & ~n->maska)
			  	&& (testb & ~n->maskb)==(n->b & ~n->maskb)
				&& (testc & ~n->maskc)==(n->c & ~n->maskc)
				&& (testd & ~n->maskd)==(n->d & ~n->maskd)
				&& (testport>=n->lowest_port)
				&& (testport<=n->highest_port);
			if( ret )
			{
				break;
			}
			n=n->next;
		}		
		return ret;		
		
	} 

	
	unsigned char a,b,c,d;
	unsigned char maska, maskb, maskc, maskd;
	int lowest_port, highest_port;
	jdk_ipv4_ip_range *next;
};



#endif
