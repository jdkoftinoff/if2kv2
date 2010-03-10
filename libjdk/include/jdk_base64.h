#ifndef _JDK_BASE64
#define _JDK_BASE64

#include "jdk_string.h"
#include "jdk_dynbuf.h"

bool jdk_base64_encode( const void *in, size_t in_len, char *out, size_t out_len );
bool jdk_base64_decode( const void *in, size_t in_len, unsigned char *out, size_t out_len );

inline bool jdk_base64_encode( const char *in, size_t in_len, char *out, size_t out_len )
{
   return jdk_base64_encode( (const void *)in, in_len, out, out_len );
}

   
inline bool jdk_base64_decode( const char *in, size_t in_len, char *out, size_t out_len )
{
   return jdk_base64_decode( (const void *)in, in_len, (unsigned char *)out, out_len );   
}


inline bool jdk_base64_encode( const unsigned char *in, size_t in_len, unsigned char *out, size_t out_len )
{
   return jdk_base64_encode( (const void *)in, in_len, (char *)out, out_len );   
}

inline bool jdk_base64_decode( const unsigned char *in, size_t in_len, unsigned char *out, size_t out_len )
{
   return jdk_base64_decode( (const void *)in, in_len, (unsigned char *)out, out_len );   
}



inline bool jdk_base64_encode( const jdk_buf &in, jdk_dynbuf &out )
{
	size_t r=0;
	size_t sz=(in.get_data_length()+1)/3 * 4;
	if( out.resize(sz) )
	{
		r=jdk_base64_encode( in.get_data(), in.get_data_length(), out.get_data(), sz );
	}
	return r!=0;
}

inline bool jdk_base64_decode( const jdk_buf &in, jdk_dynbuf &out )
{
	size_t r=0;
	size_t sz=in.get_data_length()/4 * 3 +1;

	if( out.resize(sz) )
	{
		r=jdk_base64_decode( in.get_data(), in.get_data_length(), out.get_data(), sz );
		out.set_data_length(r);
	}
	return r!=0;
}

inline bool jdk_base64_encode( const jdk_string &in, jdk_string &out )
{
	size_t r=jdk_base64_encode( in.c_str(), in.len(), out.c_str(), out.getmaxlen()-1 );
	out.set( r, '\0' );
	return r!=0;
}

inline bool jdk_base64_decode( const jdk_string &in, jdk_string &out )
{
	size_t r=jdk_base64_decode( in.c_str(), in.len(), out.c_str(), out.getmaxlen()-1 );
	out.set( r, '\0' );
	return r!=0;
}


#endif
