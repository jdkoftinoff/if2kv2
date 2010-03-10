#ifndef _JDK_STREAM_H
#define _JDK_STREAM_H

#include "jdk_string.h"
#include "jdk_dynbuf.h"
#include "jdk_socket.h"
#include "jdk_array.h"
#include "jdk_map.h"

class jdk_stream_style;
class jdk_stream_style_text;
class jdk_stream_style_bin;
class jdk_stream_style_xml;
class jdk_stream;
class jdk_stream_file;
class jdk_stream_dynbuf;
class jdk_stream_socket;


class jdk_stream_style
{
  explicit jdk_stream_style( const jdk_stream_style & );
  const jdk_stream_style operator = ( const jdk_stream_style & );
 public:
    explicit jdk_stream_style() : stream() {}
	virtual ~jdk_stream_style() {}
	inline void set_stream( jdk_stream *stream_ )
		{
			stream = stream_;
		}

	virtual bool read_start_tag( const char *required_tag=0 )=0;
	virtual bool read_end_tag( const char *required_tag=0 )=0;

	virtual bool write_start_tag( const char *tag=0 )=0;
	virtual bool write_end_tag( const char *tag=0 )=0;

	virtual bool read( jdk_string &result )=0;
	virtual bool write( const jdk_string &val ) =0;
	virtual bool read( jdk_dynbuf &result )=0;
	virtual bool write( const jdk_dynbuf &val )=0;

	// now all the fundamental types
	virtual bool read( signed char &result )=0;
	virtual bool write( const signed char &val )=0;
	virtual bool read( unsigned char &result )=0;
	virtual bool write( const unsigned char &val )=0;
	virtual bool read( int16 &result )=0;
	virtual bool write( const int16 &val )=0;
	virtual bool read( uint16 &result )=0;
	virtual bool write( const uint16 &val )=0;
	virtual bool read( int32 &result )=0;
	virtual bool write( const int32 &val )=0;
	virtual bool read( uint32 &result )=0;
	virtual bool write( const uint32 &val )=0;
	virtual bool read( float &result )=0;
	virtual bool write( const float &val )=0;
	virtual bool read( double &result )=0;
	virtual bool write( const double &val )=0;
 protected:
	jdk_stream *stream;

};

class jdk_stream_style_text : public jdk_stream_style
{
 public:
	bool read_start_tag( const char *required_tag );
	bool read_end_tag( const char *required_tag);

	bool write_start_tag( const char *tag );
	bool write_end_tag( const char *tag );

	bool read( jdk_string &result );
	bool write( const jdk_string &val );
	bool read( jdk_dynbuf &result );
	bool write( const jdk_dynbuf &val );
	bool read( signed char &result );
	bool write( const signed char &val );
	bool read( unsigned char &result );
	bool write( const unsigned char &val );
	bool read( int16 &result );
	bool write( const int16 &val );
	bool read( uint16 &result );
	bool write( const uint16 &val );
	bool read( int32 &result );
	bool write( const int32 &val );
	bool read( uint32 &result );
	bool write( const uint32 &val );
	bool read( float &result );
	bool write( const float &val );
	bool read( double &result );
	bool write( const double &val );
};

class  jdk_stream_style_bin : public jdk_stream_style
{
 public:
	bool read_start_tag( const char *required_tag );
	bool read_end_tag( const char *required_tag );

	bool write_start_tag( const char *tag );
	bool write_end_tag( const char *tag );

	bool read( jdk_string &result );
	bool write( const jdk_string &val );
	bool read( jdk_dynbuf &result );
	bool write( const jdk_dynbuf &val );
	bool read( signed char &result );
	bool write( const signed char &val );
	bool read( unsigned char &result );
	bool write( const unsigned char &val );
	bool read( int16 &result );
	bool write( const int16 &val );
	bool read( uint16 &result );
	bool write( const uint16 &val );
	bool read( int32 &result );
	bool write( const int32 &val );
	bool read( uint32 &result );
	bool write( const uint32 &val );
	bool read( float &result );
	bool write( const float &val );
	bool read( double &result );
	bool write( const double &val );
};

class jdk_stream_style_xml : public jdk_stream_style
{
 public:
	bool read_start_tag( const char *required_tag );
	bool read_end_tag( const char *required_tag );

	bool write_start_tag( const char *tag );
	bool write_end_tag( const char *tag );

	bool read( jdk_string &result );
	bool write( const jdk_string &val );
	bool read( jdk_dynbuf &result );
	bool write( const jdk_dynbuf &val );
	bool read( signed char &result );
	bool write( const signed char &val );
	bool read( unsigned char &result );
	bool write( const unsigned char &val );
	bool read( int16 &result );
	bool write( const int16 &val );
	bool read( uint16 &result );
	bool write( const uint16 &val );
	bool read( int32 &result );
	bool write( const int32 &val );
	bool read( uint32 &result );
	bool write( const uint32 &val );
	bool read( float &result );
	bool write( const float &val );
	bool read( double &result );
	bool write( const double &val );
};

class jdk_stream
{
 public:
	jdk_stream() {}
	virtual ~jdk_stream() {}

	virtual void close()=0;
	virtual bool eof()=0;
	virtual bool error()=0;
	virtual bool read( jdk_string &result )=0;
	virtual bool write( const jdk_string &val ) =0;
	virtual bool read( jdk_dynbuf &buf, int len=-1 )=0;
	virtual bool write( const jdk_dynbuf &buf )=0;
	virtual bool read( void *buf, size_t sz )=0;
	virtual bool write( const void *buf, size_t sz )=0;
	virtual jdk_stream_style &style()=0;
};


inline bool jdk_stream_read( jdk_stream &stream, jdk_string &result, const char *tag )
{
	return stream.style().read_start_tag(tag) && stream.style().read(result) && stream.style().read_end_tag(tag);
}


inline bool jdk_stream_write( jdk_stream &stream, const jdk_string &val, const char *tag )
{
	return stream.style().write_start_tag(tag) && stream.style().write(val) && stream.style().write_end_tag(tag);
}

inline bool jdk_stream_read( jdk_stream &stream, jdk_dynbuf &result, const char *tag )
{
	return stream.style().read_start_tag(tag) && stream.style().read(result) && stream.style().read_end_tag(tag);
}


inline bool jdk_stream_write( jdk_stream &stream, const jdk_dynbuf &val, const char *tag )
{
	return stream.style().write_start_tag(tag) && stream.style().write(val) && stream.style().write_end_tag(tag);
}

inline bool jdk_stream_read( jdk_stream &stream, signed char &result, const char *tag )
{
	return stream.style().read_start_tag(tag) && stream.style().read(result) && stream.style().read_end_tag(tag);
}


inline bool jdk_stream_write( jdk_stream &stream, const signed char &val, const char *tag )
{
	return stream.style().write_start_tag(tag) && stream.style().write(val) && stream.style().write_end_tag(tag);
}

inline bool jdk_stream_read( jdk_stream &stream, unsigned char &result, const char *tag )
{
	return stream.style().read_start_tag(tag) && stream.style().read(result) && stream.style().read_end_tag(tag);
}


inline bool jdk_stream_write( jdk_stream &stream, const unsigned char &val, const char *tag )
{
	return stream.style().write_start_tag(tag) && stream.style().write(val) && stream.style().write_end_tag(tag);
}

inline bool jdk_stream_read( jdk_stream &stream, int16 &result, const char *tag )
{
	return stream.style().read_start_tag(tag) && stream.style().read(result) && stream.style().read_end_tag(tag);
}


inline bool jdk_stream_write( jdk_stream &stream, const int16 &val, const char *tag )
{
	return stream.style().write_start_tag(tag) && stream.style().write(val) && stream.style().write_end_tag(tag);
}

inline bool jdk_stream_read( jdk_stream &stream, uint16 &result, const char *tag )
{
	return stream.style().read_start_tag(tag) && stream.style().read(result) && stream.style().read_end_tag(tag);
}


inline bool jdk_stream_write( jdk_stream &stream, const uint16 &val, const char *tag )
{
	return stream.style().write_start_tag(tag) && stream.style().write(val) && stream.style().write_end_tag(tag);
}


inline bool jdk_stream_read( jdk_stream &stream, int32 &result, const char *tag )
{
	return stream.style().read_start_tag(tag) && stream.style().read(result) && stream.style().read_end_tag(tag);
}


inline bool jdk_stream_write( jdk_stream &stream, const int32 &val, const char *tag )
{
	return stream.style().write_start_tag(tag) && stream.style().write(val) && stream.style().write_end_tag(tag);
}

inline bool jdk_stream_read( jdk_stream &stream, uint32 &result, const char *tag )
{
	return stream.style().read_start_tag(tag) && stream.style().read(result) && stream.style().read_end_tag(tag);
}


inline bool jdk_stream_write( jdk_stream &stream, const uint32 &val, const char *tag )
{
	return stream.style().write_start_tag(tag) && stream.style().write(val) && stream.style().write_end_tag(tag);
}

inline bool jdk_stream_read( jdk_stream &stream, float &result, const char *tag )
{
	return stream.style().read_start_tag(tag) && stream.style().read(result) && stream.style().read_end_tag(tag);
}


inline bool jdk_stream_write( jdk_stream &stream, const float &val, const char *tag )
{
	return stream.style().write_start_tag(tag) && stream.style().write(val) && stream.style().write_end_tag(tag);
}

inline bool jdk_stream_read( jdk_stream &stream, double &result, const char *tag )
{
	return stream.style().read_start_tag(tag) && stream.style().read(result) && stream.style().read_end_tag(tag);
}


inline bool jdk_stream_write( jdk_stream &stream, const double &val, const char *tag )
{
	return stream.style().write_start_tag(tag) && stream.style().write(val) && stream.style().write_end_tag(tag);
}


template <class KEYTYPE,class VALUETYPE>
inline bool jdk_stream_read( jdk_stream &stream, jdk_pair<KEYTYPE,VALUETYPE> &result, const char *tag )
{
	if( !stream.style().read_start_tag(tag) )
		return false;
	if( !jdk_stream_read( stream, result.key, "key" ) )
		return false;
	if( !jdk_stream_read( stream, result.value, "value" ) )
		return false;
	if( !stream.style().read_end_tag(tag) )
		return false;
	return true;
}

template <class KEYTYPE,class VALUETYPE>
inline bool jdk_stream_write( jdk_stream &stream, const jdk_pair<KEYTYPE,VALUETYPE> &val, const char *tag )
{
	if( !stream.style().write_start_tag(tag) )
		return false;
	if( !jdk_stream_write( stream, val.key, "key" ) )
		return false;
	if( !jdk_stream_write( stream, val.value, "value" ) )
		return false;
	if( !stream.style().write_end_tag(tag) )
		return false;
	return true;
}

template <class KEYTYPE,class VALUETYPE>
inline bool jdk_stream_read( jdk_stream &stream, jdk_map<KEYTYPE,VALUETYPE> &result, const char *tag )
{
	if( !stream.style().read_start_tag(tag) )
		return false;

	int32 count=0;
	if( !jdk_stream_read( stream,  count, "count" ) )
	{
		return false;
	}
	for( int i=0; i<count; ++i )
	{
		jdk_pair<KEYTYPE,VALUETYPE> *entry = new jdk_pair<KEYTYPE,VALUETYPE>;
		if( !jdk_stream_read( stream, *entry, "item" ) )
		{
			delete entry;
			return false;
		}
		result.add( entry );
	}

	if( !stream.style().read_end_tag(tag) )
		return false;

	return true;
}

template <class KEYTYPE,class VALUETYPE>
inline bool jdk_stream_write( jdk_stream &stream, const jdk_map<KEYTYPE,VALUETYPE> &val, const char *tag )
{
	if( !stream.style().write_start_tag(tag) )
		return false;

	int32 count=0;
	int num=val.count();
	for( int i=0; i<num; ++i )
	{
		if( val.get(i) )
			++count;
	}
	if( !jdk_stream_write( stream, count, "count" ))
	{
		return false;
	}
	for( int j=0; j<num; ++j )
	{
		const jdk_pair<KEYTYPE,VALUETYPE> *entry =val.get(j);
		if( entry )
		{
			if( !jdk_stream_write( stream, *entry, "item" ) )
			{
				return false;
			}
		}
	}
	if( !stream.style().write_end_tag(tag) )
		return false;

	return true;
}

template <class T>
inline bool jdk_stream_read( jdk_stream &stream, jdk_array<T> &result, const char *tag )
{
	if( !stream.style().read_start_tag(tag) )
		return false;

	int32 count=0;
	if( !jdk_stream_read( stream,  count, "count" ) )
	{
		return false;
	}
	for( int i=0; i<count; ++i )
	{
		T *entry = new T;
		if( !jdk_stream_read( stream, *entry, "item" ) )
		{
			delete entry;
			return false;
		}
		result.add( entry );
	}
	if( !stream.style().read_end_tag(tag) )
		return false;

	return true;
}

template <class T>
inline bool jdk_stream_write( jdk_stream &stream, const jdk_array<T> &val, const char *tag )
{
	if( !stream.style().write_start_tag(tag) )
		return false;

	int32 count=0;
	int num=val.count();
	for( int i=0; i<num; ++i )
	{
		if( val.get(i) )
			++count;
	}
	if( !jdk_stream_write( stream, count, "count" ))
	{
		return false;
	}
	for( int j=0; j<num; ++j )
	{
		const T *entry =val.get(j);
		if( entry )
		{
			if( !jdk_stream_write( stream, *entry, "item" ) )
			{
				return false;
			}
		}
	}

	if( !stream.style().write_end_tag(tag) )
		return false;

	return true;
}

class jdk_stream_file : public jdk_stream
{
  explicit jdk_stream_file( const jdk_stream_file & );
  const jdk_stream_file & operator =( const jdk_stream_file & );
 public:
    explicit jdk_stream_file( jdk_stream_style *style_, const jdk_string_filename &fname, const char *openmode ) :
		f(), current_style( style_ )
		{
			current_style->set_stream( this );
			f=jdk_fopen(fname.c_str(),openmode);			
		}

    explicit jdk_stream_file( jdk_stream_style *style_, FILE *f_ ) :
		f(f_), current_style( style_ )
		{
			current_style->set_stream( this );
		}

	~jdk_stream_file()
		{
			close();
			delete current_style;
		}

	void close()
		{
			fclose(f);
			f=0;
		}

	bool eof()
		{
			return feof(f)!=0;
		}

	bool error()
		{
			return ferror(f)!=0;
		}

	bool read( jdk_string &result )
		{
			return jdk_read_string_line( &result, f );
		}

	bool write( const jdk_string &val )
		{
			return jdk_write_string_line( &val, f );
		}

	bool read( jdk_dynbuf &result, int len )
		{
			return result.append_from_stream( f ); 
		}

	bool write( const jdk_dynbuf &val )
		{
			return val.extract_to_stream( f );
		}

	bool read( void *buf, size_t sz )
		{
			return fread(buf,sz,1,f)==1;
		}

	bool write( const void *buf, size_t sz )
		{
			return fwrite(buf,sz,1,f)==1;
		}

	jdk_stream_style &style()
		{
			return *current_style;
		}

 private:
	FILE *f;
	jdk_stream_style *current_style;
};



class jdk_stream_dynbuf : public jdk_stream
{
  explicit jdk_stream_dynbuf( const jdk_stream_dynbuf & );
  const jdk_stream_dynbuf operator = ( const jdk_stream_dynbuf & );
  
 public:
	jdk_stream_dynbuf( jdk_stream_style *style_, jdk_dynbuf &dynbuf_ ) :
		current_style( style_ ),
		dynbuf(dynbuf_),
		pos(0)
		{
			current_style->set_stream( this );
		}

	~jdk_stream_dynbuf()
		{
			delete current_style;
		}

	void close()
		{
			// nothing
		}

	bool eof()
		{
			return pos>=dynbuf.get_data_length();
		}

	bool error()
		{
			return false;
		}

	bool read( jdk_string &result )
		{
			size_t len = dynbuf.extract_to_string( result, pos );
			if( len>=0 )
			{
				pos+=len;
			}
			return len>=0;
		}

	bool write( const jdk_string &val )
		{
			return dynbuf.append_from_string( val );
		}

	bool read( jdk_dynbuf &result, int len )
		{
			bool r = result.append_from_buf( dynbuf, (int)pos, len );
			if( len==-1 )
				pos = dynbuf.get_data_length();
			else if( r )
				pos+=len;
			return r;
		}

	bool write( const jdk_dynbuf &val )
		{
			return dynbuf.append_from_data( val.get_data(), val.get_data_length() );
		}

	bool read( void *buf, size_t sz )
		{
			bool r = dynbuf.extract_to_data( (unsigned char *)buf, (int)pos, int(sz) );
			if( r )
				pos+=sz;
			return r;
		}

	bool write( const void *buf, size_t sz )
		{
			return dynbuf.append_from_data( (const unsigned char *)buf, int(sz) );
		}

	jdk_stream_style &style()
		{
			return *current_style;
		}

 private:
	jdk_stream_style *current_style;
	jdk_dynbuf &dynbuf;
	size_t pos;
};

class jdk_stream_socket : public jdk_stream
{
  explicit jdk_stream_socket( const jdk_stream_socket & );
  const jdk_stream_socket operator = ( const jdk_stream_socket & );
 public:
	jdk_stream_socket( jdk_stream_style *style_, jdk_client_socket &sock_ ) :
		current_style( style_ ),
		sock( sock_ )
		{
			current_style->set_stream( this );
		}

	~jdk_stream_socket()
		{
			close();
			delete current_style;
		}

	void close()
		{
			sock.close();
		}

	bool eof()
		{
			return sock.is_open_for_reading();
		}

	bool error()
		{
			return sock.is_open_for_writing() && sock.is_open_for_reading();
		}

	bool read( jdk_string &result )
		{
			return sock.read_string( result )>=0;
		}

	bool write( const jdk_string &val )
		{
			return sock.write_string_block( val )!=0;
		}

	bool read( jdk_dynbuf &result, int len )
		{
			int todo=len;
			if( todo<0 || todo>10*1024*1024 )
				return false; // have some sense!

			while( sock.is_open_for_reading() && todo>0 )
			{
				int cur = sock.read_buf( result, todo );
				if( cur<=0 )
					return false;
				todo -= cur;
			}
			return todo==0;
		}

	bool write( const jdk_dynbuf &val )
		{
			return sock.write_data_block( val )!=0;
		}

	bool read( void *buf, size_t sz )
		{
			unsigned char *p = (unsigned char *)buf;
			int todo=int(sz);
			if( todo<0 || todo>10*1024*1024 )
				return false; // have some sense!

			while( sock.is_open_for_reading() && todo>0 )
			{
				int cur = sock.read_data( p, todo );
				if( cur<=0 )
					return false;
				todo -= cur;
				p+=cur;
			}
			return todo==0;
		}

	bool write( const void *buf, size_t sz )
		{
			return (size_t)sock.write_data_block( buf, (unsigned long)sz )==sz;
		}

	jdk_stream_style &style()
		{
			return *current_style;
		}
 private:
	jdk_stream_style *current_style;
	jdk_client_socket &sock;
};

#endif













