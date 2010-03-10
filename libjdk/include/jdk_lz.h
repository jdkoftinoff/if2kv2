#ifndef _JDK_LZ_H
#define _JDK_LZ_H

#include "jdk_dynbuf.h"
#include "jdk_bindir.h"
#include "jdk_log.h"

void jdk_lz_compress( 
					 const unsigned char *p_src_first,
					 size_t src_len,
					 unsigned char *p_dst_first,
					 size_t *p_dst_len,
					 unsigned char xorvalue=0x55
					 );

void jdk_lz_decompress(
					  const unsigned char * p_src_first,
					  size_t src_len,
					  unsigned char *p_dst_first,
					  size_t * p_dst_len,
					  unsigned char xorvalue=0x55
					  );

inline void jdk_lz_compress( const jdk_buf *in, jdk_dynbuf *out, unsigned char xorvalue=0x55 )
{
	size_t maxsize=in->get_data_length() * 2  + 128;
	out->clear();	
  
	if( maxsize>0 )
	{		
		out->resize( maxsize );
		size_t out_size=0;
		jdk_lz_compress( in->get_data(), in->get_data_length(), out->get_data(), &out_size, xorvalue );
    if( out_size>=out->get_buf_length() )
    {
      jdk_log( JDK_LOG_ERROR, "jdk_lz_compress buffer blown" );
      abort();
    }
		out->set_data_length( out_size );
    
	}
	
}

inline void jdk_lz_decompress( const jdk_buf *in, jdk_dynbuf *out, unsigned char xorvalue=0x55 )
{
	size_t maxsize=in->get_data_length() * 9 + 128;
	out->clear();	

	if( maxsize>0 )
	{		
		out->resize( maxsize );
		size_t out_size=0;
		jdk_lz_decompress( in->get_data(), in->get_data_length(), out->get_data(), &out_size, xorvalue );

    if( out_size>=out->get_buf_length() )
    {
      jdk_log( JDK_LOG_ERROR, "jdk_lz_decompress buffer blown" );
      abort();
    }

		out->set_data_length( out_size );
	}
	
}


inline bool jdk_lz_decompress( const char *bindir_group, const char *bindir_name, jdk_dynbuf *out, unsigned char xorvalue=0x55 )
{
	bool r=false;
	struct jdk_bindir *b = jdk_bindir_find( bindir_group, bindir_name );
	out->clear();		
	if( b )
	{		
		size_t maxsize=b->length * 9 + 128;
		if( maxsize>0 )
		{		
			out->resize( maxsize );
			size_t out_size=0;
			jdk_lz_decompress( b->data, b->length, out->get_data(), &out_size, xorvalue );
			out->set_data_length( out_size );
			r=true;
		}
	}	
	return r;
}


#endif

