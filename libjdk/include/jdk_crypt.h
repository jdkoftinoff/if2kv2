#ifndef _JDK_CRYPT_H
#define _JDK_CRYPT_H

#include "jdk_util.h"
#include "jdk_string.h"

class jdk_encryptor
{
public:
	jdk_encryptor() {}
	virtual ~jdk_encryptor() {}

	virtual bool encrypt_line(
							  const unsigned char *inbuf,
							  unsigned char *outbuf,
							  int outbuf_len 
							  ) = 0;

};

class jdk_decryptor
{
public:
	jdk_decryptor() {}
	virtual ~jdk_decryptor() {}

	virtual bool decrypt_line(
							 const unsigned char *inbuf,
							 unsigned char *outbuf,
							 int outbuf_len
							 ) = 0;

};


class jdk_encryptornone : public jdk_encryptor
{
public:
	jdk_encryptornone( unsigned long key=0xdead )
	{
	}

	virtual ~jdk_encryptornone() {}

	virtual bool encrypt_line(
    				const unsigned char *inbuf,
                    unsigned char *outbuf,
					int outbuf_len
                    )
	{
		jdk_strncpy( (char*)outbuf, (char*)inbuf, outbuf_len );
		return true;
	}

};



class jdk_decryptornone : public jdk_decryptor
{
public:
	jdk_decryptornone( unsigned long key=0 ) 
	{
	}

	virtual ~jdk_decryptornone() {}

	virtual bool decrypt_line( const unsigned char *inbuf, unsigned char *outbuf, int outbuf_len )
	{
		jdk_strncpy( (char *)outbuf, (char *)inbuf, outbuf_len );
		return true;
	}
};

class jdk_encryptor_simple : public jdk_encryptor
{
public:
	jdk_encryptor_simple( unsigned long key=0xdead ) : state(key)
	{
		srand( static_cast<unsigned int>(time(0)) );
	}

	virtual ~jdk_encryptor_simple() {}

	virtual bool encrypt_line(
							  const unsigned char *inbuf,
							  unsigned char *outbuf,
							  size_t outbuf_len
							  )
	{
		size_t len = strlen((char *)inbuf);
		if( len>0 && len+1<outbuf_len)
		{
			int o=((rand()&0x3f) + 0x40) ^ (state&0x1f);
			
			outbuf[0] = o;
			state+=o;               
			state += (state>>8);
			
			for( size_t i=0; i<len; ++i )
			{
				int v = inbuf[i];

				if( v>=0x20 )
				{
					o = v ^ (state&0x1f );
				}
				else
				{
					o = v;
				}

				outbuf[i+1] = o;
				state += o;
				state += (state>>8);
			}
			outbuf[len+1]='\0';
		}
		else
		{
			*outbuf='\0';
		}
		return true;

	}


private:
	unsigned long state;
};


class jdk_decryptor_simple : public jdk_decryptor
{
public:
	jdk_decryptor_simple( unsigned long key ) : state( key )
	{
	}

	virtual ~jdk_decryptor_simple() {}

	virtual bool decrypt_line( 
							  const unsigned char *inbuf,
							  unsigned char *outbuf,
							  size_t outbuf_len
							  )
	{
		size_t len = strlen((char *)inbuf);
		//int o;
		
		if( len>1 && len<outbuf_len )
		{
			//strrev((char*)inbuf);
			// get first throwaway character
			//o=(inbuf[0]) ^ (state&0x1f);

			state+=inbuf[0];
			state += (state>>8);
			
			for( size_t i=0; i<len-1; i++ )
			{
				int v = inbuf[i+1];
				
				if( v>=0x20 )
				{
					outbuf[i] = (unsigned char)(v ^ (state&0x1f));
				}
				else
				{
					outbuf[i] = (unsigned char)v;
				}
				
				state+= v;
				state += (state>>8);
			}
			outbuf[len-1] ='\0';
		}
		else
		{
			*outbuf='\0';
		}
		return true;
		
	}


private:
	unsigned long state;
};

class jdk_encryptor_complex : public jdk_encryptor
{
public:
	jdk_encryptor_complex( unsigned long key=0xab12dead ) : state(key)
	{
	}

	virtual ~jdk_encryptor_complex() {}

	virtual bool encrypt_line(
							  const unsigned char *inbuf,
							  unsigned char *outbuf,
							  size_t outbuf_len
							  )
	{
		size_t len = jdk_strlen((char *)inbuf);
		if( len>0 && (len*2)+4<outbuf_len)
		{			
			int o;
			
			for( int j=0; j<4; ++j )
			{
				o=((rand()&0x1f) + 0x40) ^ (state&0x1f);
			
				outbuf[j] = o;
				state+=o;               
				state += (state>>4);
			}
			
			for( size_t i=0; i<len; ++i )
			{
				int v = inbuf[i];

				if( v>=0x40 )
				{
					o = v ^ (state&0x1f);
				}
				else
				{
					o = v ^ (state&0xf);
				}

				outbuf[(i*2)+4] = o;
				outbuf[(i*2)+5] = ((rand()&0x1f)) + (o&0x1f) + 0x20;
			  	state += o;
				state += (state>>4);
			}
			outbuf[len*2+4]='\0';
		}
		else
		{
			*outbuf='\0';
		}
		return true;

	}


private:
	unsigned long state;
};


class jdk_decryptor_complex : public jdk_decryptor
{
public:
	jdk_decryptor_complex( unsigned long key=0xab12dead ) : state(key)
	{
	}

	virtual ~jdk_decryptor_complex() {}

	virtual bool decrypt_line( 
							  const unsigned char *inbuf,
							  unsigned char *outbuf,
							  size_t outbuf_len
							  )
	{
		size_t len = jdk_strlen((char *)inbuf);
		
		if( len>1 && len<(outbuf_len*2+3) )
		{
			for( size_t j=0; j<4; ++j )
			{
				state+=inbuf[j];
				state += (state>>4);
			}
			
			for( size_t i=4; i<len; i+=2 )
			{
				int v = inbuf[i];
				
				if( v>=0x40 )
				{
					outbuf[(i-4)/2] = (unsigned char)(v ^ (state&0x1f));
				}
				else
				{
					outbuf[(i-4)/2] = (unsigned char)(v ^ (state&0xf));
				}
				
				state+= v;
				state += (state>>4);
			}
			outbuf[(len-4)/2] ='\0';
		}
		else
		{
			*outbuf='\0';
		}
		return true;
		
	}


private:
	unsigned long state;
};


template <class ENCRYPTOR>
inline bool jdk_encrypt_single_line( unsigned long key, const char *in, char *out, int out_len )
{
	ENCRYPTOR enc( key );
	
	return enc.encrypt_line( (const unsigned char *)in, (unsigned char *)out, out_len );
}

template <class DECRYPTOR>
inline bool jdk_decrypt_single_line( unsigned long key, const char *in, char *out, int out_len )
{
	DECRYPTOR enc( key );
	
	return enc.decrypt_line( (const unsigned char *)in, (unsigned char *)out, out_len );
}



#endif
