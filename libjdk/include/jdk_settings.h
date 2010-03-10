#ifndef _JDK_SETTINGS_H
#define _JDK_SETTINGS_H

#include "jdk_map.h"
#include "jdk_valmap.h"
#include "jdk_string.h"
#include "jdk_dynbuf.h"
#include "jdk_log.h"
#include "jdk_error.h"
#include "jdk_thread.h"
#include "jdk_crypt.h"

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#include "jdk_string_objc.h"
#endif

typedef jdk_str<128> jdk_setting_key;
typedef jdk_str<4096> jdk_setting_value;
typedef jdk_pair< jdk_setting_key, jdk_setting_value > jdk_setting;

struct jdk_setting_description
{
  const char *name;
  const char *default_value;
  const char *description;
};

void jdk_setting_description_dump( const jdk_setting_description *, FILE * );

class jdk_settings
{
public:
	typedef jdk_setting_key key_t;
	typedef jdk_setting_value value_t;
	typedef jdk_setting pair_t;
  
	jdk_settings() : map(), dummy_value()
  {
  }
	
	explicit jdk_settings( const jdk_settings &o )
    : map( o.map ), dummy_value()
  {
  }
	
	virtual ~jdk_settings() {}
  
  const jdk_settings &operator = ( const jdk_settings &o )
  {
    if( &o!=this )
    {
      clear();
      merge(o);
    }
    return *this;
  }
  
  explicit jdk_settings( 
    const jdk_setting_description *default_settings, 
    int argc=0, char **argv=0,
    const char *config_file=0
    );
  
  explicit jdk_settings( FILE *f ) : map(), dummy_value()
  {
    load_file( f );
  }
  
  explicit jdk_settings( const char *fname ): map(), dummy_value()
  {
    load_file( fname );
  }
  
  explicit jdk_settings( const jdk_string_filename &fname ): map(), dummy_value()
  {
    load_file( fname );
  }
  
  explicit jdk_settings( const jdk_dynbuf &buf ): map(), dummy_value()
  {
    load_buf( buf );
  }
	
	void clear()
  {
    map.clear();
  }
  
	void erase( int i )
  {
    map.remove(i);
  }
  
  void erase( 
    const key_t &k 
    )
  {
    int i=find(k);
    if( i!=-1 )
    {
      map.remove(i);
    }
  }
  
  void erase_prefixed(
    const key_t &prefix,
    const key_t &k
    )
  {
    key_t key(prefix);
    key.cat(k);
    erase( key );
  }
  
	size_t count() const
  {
    return map.count();
  }
  
  bool set_args( 
    const jdk_setting_description *default_settings, 
    int argc, char **argv,
    const char *config_file=0 
    );
  
  bool parse_line( 
    const jdk_string &line_buf 
    );
  
	bool load_file( 
    FILE *f 
    );
  
	bool load_file( 
    const char *fname 
    );
  
	bool load_buf( 
    const jdk_dynbuf &buf 
    );
  
	bool load_file( 
    const jdk_string_filename &fname 
    )
  {
    return load_file( fname.c_str() );
  }
  
	bool save_file( 
    FILE *f, 
    const char *line_end="\n", 
    bool use_quotes=true
    ) const;
	
	bool save_buf( 
    jdk_dynbuf *buf, 
    const char *line_end="\n",
    bool use_quotes=true
    ) const;
  
	bool save_file(
    const char *fname,
    const char *line_end="\n",
    bool use_quotes=true
    ) const;
  
	bool save_file( 
    const jdk_string &fname,
    const char *line_end="\n",
    bool use_quotes=true
    ) const
  {
    return save_file( 
      fname.c_str(),
      line_end,
      use_quotes
      );
  }
  
	
	int find( const key_t &k, int start_pos=0 ) const
  {
    return map.find( k, start_pos );
  }
	
  bool set( const jdk_setting &s )
  {
    return map.set( s.key, s.value );
  }
  
  bool set( const key_t &k, const value_t &v )
  {
    return map.set( k, v );
  }
  
#ifdef __OBJC__
  bool set( const key_t &k, NSString *nsv )
  {
    return set( k, jdk_string_from_NSString<value_t>(nsv) );
  }
  
  bool set( NSString *nsk, NSString *nsv )
  {
    return set( jdk_string_from_NSString<key_t>(nsk), jdk_string_from_NSString<value_t>(nsv) );
  }  
  
  NSString *get( NSString *nsk ) const
  {
    return jdk_string_to_NSString( get( jdk_string_from_NSString<key_t>(nsk) ) );
  }
  
  NSString *get_NSString( const char *k ) const
  {
    return jdk_string_to_NSString( get( k ) );
  }
  
  
#endif
  
	const pair_t * get( size_t i ) const
  {
    return map.get(i);
  }
  
	pair_t * get( size_t i )
  {
    return map.get(i);
  }
	
  
	const value_t & get( 
    const key_t &k
    ) const 
  {
    const jdk_setting_value *r=0;		
    
    int i = map.find( k, 0 );
    
    if( i>=0 )
    {
      r=&map.getvalue(i);
    }
    else
    {
      return dummy_value;
    }
    
    return *r;
  } 
  
  int find_prefixed( const key_t &prefix, const key_t &suffix, int start_pos=0 ) const
  {
    key_t key(prefix);
    key.cat(suffix);
    return find( key, start_pos );		
  }
	
	const value_t & get_prefixed( const key_t &prefix, const key_t &suffix ) const
  {
    key_t key(prefix);
    key.cat(suffix);
    return get( key );
  }
  
  bool get_bool( const key_t &k ) const
  {
    return (bool)(get_long( k )!=0);
  }
  
  bool get_prefixed_bool( const key_t &prefix, const key_t &suffix ) const
  {
    return (bool)(get_prefixed_long(prefix, suffix)!=0);
  }
  
  
  long get_long( const key_t &k  ) const
  {
    return get(k).strtol();
  }
  
	long get_prefixed_long( const key_t &prefix, const key_t &suffix ) const
  {
    key_t key(prefix);
    key.cat(suffix);
    return get_long( key );
  }
  
  
  unsigned long get_ulong( const key_t &k  ) const
  {
    return get(k).strtoul();
  }
  
  unsigned long get_prefixed_ulong( const key_t &prefix, const key_t &suffix ) const
  {
    key_t key(prefix);
    key.cat(suffix);
    return get_ulong( key );      
  }
  
  float get_float( const key_t &k ) const
  {
    return get(k).strtof();
  }
  
  float get_prefixed_float( const key_t &prefix, const key_t &suffix ) const
  {
    key_t key(prefix);
    key.cat(suffix);
    return get_float( key );      
  }
  
  double get_double( const key_t &k ) const
  {
    return get(k).strtod();
  }
  
  double get_prefixed_double( const key_t &prefix, const key_t &suffix ) const
  {
    key_t key(prefix);
    key.cat(suffix);
    return get_double( key );      
  }
  
  bool set_bool( const key_t &k, bool v )
  {
    return set_long( k, (long)v );
  }
  
  bool set_prefixed_bool( const key_t &prefix, const key_t &suffix, bool v )
  {
    key_t key(prefix);
    key.cat(suffix);
    return set_bool( key, v );      
  }
  
  bool set_long( const key_t &k, long lv )
  {
    jdk_str<64> v;
    v.form("%ld", lv );
    return set( k, v );
  }
  
  bool set_prefixed_long( const key_t &prefix, const key_t &suffix, long v )
  {
    key_t key(prefix);
    key.cat(suffix);
    return set_long( key, v );      
  }
  
  
  bool set_ulong( const key_t &k, unsigned long lv )
  {
    jdk_str<64> v;
    v.form("%lu", lv );
    return set( k, v );
  }
  
  bool set_prefixed_ulong( const key_t &prefix, const key_t &suffix, unsigned long v )
  {
    key_t key(prefix);
    key.cat(suffix);
    return set_ulong( key, v );      
  }
  
  bool set_float( const key_t &k, float f )
  {
    jdk_str<64> v;
    v.form("%f", f );
    return set(k,v);
  }
  
  bool set_prefixed_float( const key_t &prefix, const key_t &suffix, float v )
  {
    key_t key(prefix);
    key.cat(suffix);
    return set_float( key, v );      
  }
  
  bool set_double( const key_t &k, double f )
  {
    jdk_str<64> v;
    v.form("%f", f );
    return set(k,v);
  }
  
  bool set_prefixed_double( const key_t &prefix, const key_t &suffix, double v )
  {
    key_t key(prefix);
    key.cat(suffix);
    return set_double( key, v );      
  }
  
  void copy_prefixed( const jdk_settings &s, const char *prefix )
  {
    size_t prefix_len = strlen(prefix);
    for( size_t i=0; i<s.count(); ++i )
    {
      const pair_t *a = s.get(i);
      
      if( a )
      {
        if( a->key.ncmp( prefix, prefix_len )==0 )
        {
          set( *a );
        }
      }
    }
  }
  
  void copy( 
    const jdk_settings &s, 
    const char *key
    )
  {
    key_t k(key);
    set( k, s.get(k) );
  }
  
  bool merge( const jdk_settings &s );
  bool set_defaults( const jdk_setting_description *default_settings);
  
  void expand_string( 
    const jdk_string &in, 
    jdk_string &out 
    ) const
  {
    expand_string( in.c_str(), out );
  }
  
  void expand_string( const char *p, jdk_string &out ) const;
  
protected:	
	
	jdk_map< jdk_setting_key, jdk_setting_value > map;
	const jdk_setting_value dummy_value;
};

typedef jdk_settings jdk_settings_text;

template <class ENCRYPTOR>
inline void
jdk_settings_encrypt(
  unsigned long enc_key,
  const jdk_settings &in,
  jdk_settings &out
  )
{
//  jdk_synchronized( in.get_mutex() );
  for( int i=0; i<in.count(); ++i )
  {
		const jdk_settings::pair_t *a = in.get(i);
    
		if( a )
		{
			if( a->value.get(0)!=0x01)
			{
				int key_sum = 0;
				int key_len = a->key.len();
				for(int j=0; i<key_len; ++i )
				{
					key_sum += (unsigned char)a->key.get(j);
				}
        
				ENCRYPTOR enc(enc_key+key_sum);
				jdk_settings::value_t enc_value;
				enc_value.set(0,0x01);
				enc.encrypt_line(
          (const unsigned char *)a->value.c_str(),
          (unsigned char *)enc_value.c_str()+1,
          enc_value.getmaxlen()-1
          );
				out.set( a->key, enc_value );
			}
			else
			{
				out.set( a->key, a->value );	// it is already encrypted so copy it
			}
		}
	}
}

template <class DECRYPTOR>
inline void
jdk_settings_decrypt(
  unsigned long enc_key,
  const jdk_settings &in,
  jdk_settings &out,
  bool allow_unencrypted=false
  )
{
//  jdk_synchronized( in.get_mutex() );
  for( int i=0; i<in.count(); ++i )
  {
		const jdk_settings::pair_t *a = in.get(i);
    
		if( a )
		{
			if( a->value.get(0)==0x01)
			{
				// calc the sum of the key chars
				int key_sum = 0;
				int key_len = a->key.len();
				for(int j=0; j<key_len; ++j )
				{
					key_sum += (unsigned char)a->key.get(j);
				}
				
				DECRYPTOR dec(enc_key+key_sum);		
				jdk_settings::value_t dec_value;
				dec_value.set(0,0x01);
				dec.decrypt_line(
          (const unsigned char *)a->value.c_str()+1,
          (unsigned char *)dec_value.c_str(),
          dec_value.getmaxlen()
          );
				out.set( a->key, dec_value );
			}
			else if( allow_unencrypted )
			{
				out.set( a->key, a->value ); // value is already encrypted so just copy it
			}
		}
	}
}



#if JDK_IS_WIN32
class jdk_settings_win32registry : public jdk_settings
{
public:
  jdk_settings_win32registry()
  {
  }
  
  jdk_settings_win32registry( HKEY area, const char *keyname )
  {
    load_registry( area, keyname );
  }
  
  jdk_settings_win32registry(
    HKEY area, const char *keyname,
    const jdk_setting_description *default_settings,
    int argc=0, char **argv=0
    )
  {
    set_args( default_settings, argc, argv );
    load_registry( area, keyname );
  }
  
  jdk_settings_win32registry( const jdk_settings_win32registry &o )
    : jdk_settings( o )
  {	
  }
  
  bool load_registry( HKEY area, const char *keyname );
  
  bool save_registry( HKEY area, const char *keyname ) const;
};
#endif

#if JDK_IS_MACOSX
class jdk_settings_macosx_plist : public jdk_settings
{
public:
  jdk_settings_macosx_plist()
  {
  }
  
  jdk_settings_macosx_plist( const char *fname)
  {
    load_plist( fname );
  }
  
  
  jdk_settings_macosx_plist(
    const jdk_setting_description *default_settings,
    int argc=0, char **argv=0,
    const char *fname = 0
    )
  {
    set_args( default_settings, argc, argv );
    if( fname )
      load_plist( fname );
  }
  
  jdk_settings_macosx_plist( const jdk_settings_macosx_plist &o )
    : jdk_settings( o )
  {	
  }
  
  bool set_args( 
    const jdk_setting_description *default_settings, 
    int argc, char **argv, 
    const char *config_file=0 
    );
  
  bool load_plist( const char *fname );   
	bool load_plist( const jdk_string &fname )
  {
    return load_plist( fname.c_str() );
  }
	
  bool save_plist( const char *fname ) const;
  bool save_plist( const jdk_string &fname ) const
  {
    return save_plist( fname.c_str() );
  }
};
#endif

#if 0
template <class STR>
class jdk_live_string
{
public:
  explicit jdk_live_string( 
    const jdk_settings &settings_, 
    const jdk_string &field_prefix_
    )
    : settings( settings_ ), field(), mutex(), prev_value()
  {
    field.cat( field_prefix_ );
    
    int pos=settings.find(field);
    if( pos!=-1 )
      prev_value=settings.get(pos)->value;	
  }
  
  explicit jdk_live_string( 
    const jdk_settings &settings_, 
    const jdk_string &field_prefix_, 
    const jdk_string &field_midfix_
    )
    : settings( settings_ ), field(), mutex(), prev_value()
  {
    field.cat( field_prefix_ );
    field.cat( field_midfix_ );
    int pos=settings.find(field);
    if( pos!=-1 )
      prev_value=settings.get(pos)->value;	
    
  }
  
  explicit jdk_live_string( 
    const jdk_settings &settings_, 
    const jdk_string &field_prefix_, 
    const jdk_string &field_midfix_, 
    const jdk_string &field_suffix_
    )
    : settings( settings_ ), field(), mutex(), prev_value()
  {
    field.cat( field_prefix_ );
    field.cat( field_midfix_ );
    field.cat( field_suffix_ );
    int pos=settings.find(field);
    if( pos!=-1 )
      prev_value=settings.get(pos)->value;	
    
  }
  
  explicit jdk_live_string( 
    const jdk_settings &settings_, 
    const char *field_prefix_=0, 
    const char *field_midfix_=0, 
    const char *field_suffix_=0
    )
    : settings( settings_ ), field(), mutex(), prev_value()
  {
    if( field_prefix_ )
      field.cat( field_prefix_ );
    if( field_midfix_ )
      field.cat( field_midfix_ );
    if( field_suffix_ )
      field.cat( field_suffix_ );
    int pos=settings.find(field);
    if( pos!=-1 )
      prev_value=settings.get(pos)->value;	
    
  }
  
  inline void set_field(		            
    const char *field_prefix_, 
    const char *field_midfix_=0, 
    const char *field_suffix_=0
    )
  {
    jdk_synchronized( mutex );	
    field.cpy( field_prefix_ );
    if( field_midfix_ )
      field.cat( field_midfix_ );
    if( field_suffix_ )
      field.cat( field_suffix_ );
    
  }
  
  inline STR get() const
  {
    jdk_synchronized( mutex );
    jdk_synchronized1( settings.get_mutex() );
    
    int pos=settings.find(field);
    if( pos!=-1 )
      return STR(settings.get(pos)->value);
    else
      return STR();    
  }
  
  inline void get(jdk_string &s) const
  {
    jdk_synchronized( mutex );
    jdk_synchronized1( settings.get_mutex() );
    
    int pos=settings.find(field);
    if( pos!=-1 )
      s.cpy(settings.get(pos)->value);
    else
      s.clear();
  }
  
  
  inline operator STR () const
  {
    jdk_synchronized( mutex );
    jdk_synchronized1( settings.get_mutex() );
    
    int pos=settings.find(field);
    if( pos!=-1 )
      return STR(settings.get(pos)->value);
    else
      return STR();
  }
  
  inline bool has_changed()
  {
    jdk_synchronized( mutex );
    jdk_synchronized1( settings.get_mutex() );
    
    int pos=settings.find(field);
    bool r=false;
    
    if( pos!=-1 )
    {
      if( settings.get(pos)->value.cmp( prev_value )!=0 )
      {
        prev_value = settings.get(pos)->value;
        r=true;
      }
    }
    else
    {
      if( !prev_value.is_clear() )
      {
        prev_value.clear();
        r=true;
      }
    }
    return r;
  }
  
  
  
private:				   
  const jdk_settings &settings;
  jdk_str<128> field;    
  mutable jdk_mutex mutex;
  STR prev_value;
};

typedef jdk_live_string<jdk_string_url> jdk_live_string_url;
typedef jdk_live_string<jdk_string_filename> jdk_live_string_filename;

class jdk_live_long
{
public:
  explicit jdk_live_long( 
    const jdk_settings &settings_, 
    const jdk_string &field_prefix_
    )
    : settings( settings_ ), field(), prev_value(0), mutex()
  {
    field.cat( field_prefix_ );
    int pos=settings.find(field);
    if( pos!=-1 )
      prev_value = settings.get(pos)->value.strtol();
    
  }
  
  explicit jdk_live_long( 
    const jdk_settings &settings_, 
    const jdk_string &field_prefix_, 
    const jdk_string &field_midfix_
    )
    : settings( settings_ ), field(),  prev_value(0), mutex()
  {
    field.cat( field_prefix_ );
    field.cat( field_midfix_ );
    
    int pos=settings.find(field);	
    if( pos!=-1 )
      prev_value = settings.get(pos)->value.strtol();
    
  }
  
  explicit jdk_live_long( 
    const jdk_settings &settings_, 
    const jdk_string &field_prefix_, 
    const jdk_string &field_midfix_, 
    const jdk_string &field_suffix_
    )
    : settings( settings_ ), field(),  prev_value(0), mutex()
  {
    field.cat( field_prefix_ );
    field.cat( field_midfix_ );
    field.cat( field_suffix_ );
    
    int pos=settings.find(field);
    
    if( pos!=-1 )
      prev_value = settings.get(pos)->value.strtol();
    
  }
  
  explicit jdk_live_long( 
    const jdk_settings &settings_, 
    const char *field_prefix_=0, 
    const char *field_midfix_=0, 
    const char *field_suffix_=0
    )
    : settings( settings_ ), field(),  prev_value(0), mutex()
  {
    if( field_prefix_ )
      field.cat( field_prefix_ );
    if( field_midfix_ )
      field.cat( field_midfix_ );
    if( field_suffix_ )
      field.cat( field_suffix_ );
    
    int pos=settings.find(field);
    
    if( pos!=-1 )
      prev_value = settings.get(pos)->value.strtol();
    
  }
  
  inline void set_field(		            
    const char *field_prefix_, 
    const char *field_midfix_=0, 
    const char *field_suffix_=0
    )
  {
    jdk_synchronized(mutex);
    field.cpy( field_prefix_ );
    if( field_midfix_ )
      field.cat( field_midfix_ );
    if( field_suffix_ )
      field.cat( field_suffix_ );
  }
  
  inline long get() const
  {
    jdk_synchronized(mutex);    
    jdk_synchronized1( settings.get_mutex() );
    
    int pos=settings.find(field);
    if( pos!=-1 )
      return settings.get(pos)->value.strtol();
    else
      return 0;    
  }
  
  inline operator long () const
  {
    jdk_synchronized(mutex);    
    jdk_synchronized1( settings.get_mutex() );
    
    int pos=settings.find(field);
    if( pos!=-1 )
      return settings.get(pos)->value.strtol();
    else
      return 0;
  }
  
  inline bool has_changed()
  {
    int pos=settings.find(field);
    bool r=false;
    
    if( pos!=-1 )
    {
      long v=settings.get(pos)->value.strtol();
      
      if( v!=prev_value )
      {
        prev_value = v;
        r=true;
      }
    }
    else
    {
      if( prev_value!=0 )
      {
        prev_value = 0;
        r=true;
      }
    }
    return r;
  }
  
  
private:				   
  const jdk_settings &settings;
  jdk_str<128> field;        
  long prev_value;
  mutable jdk_mutex mutex;
};
#endif

template <class T>
class jdk_settings_prefixed
{
public:
	typedef jdk_setting_key key_t;
	typedef jdk_setting_value value_t;
	typedef jdk_setting pair_t;
  
  explicit jdk_settings_prefixed( 
    T &s, 
    const char *prefix_ 
    )
    : settings(s), prefix(prefix_)
  {
  }
  
  value_t get( 
    int i 
    ) const
  {
    return settings.get(i);
  }
  
  value_t get( 
    const key_t &k 
    ) const
  {
    return settings.get_prefixed( prefix, k );
  }
  
  value_t get( 
    const key_t &k, 
    const value_t &default_value 
    ) const
  {
    return settings.get_prefixed( prefix, k, default_value );
  }
  
  value_t get_prefixed( 
    const key_t &second_prefix, 
    const key_t &k 
    ) const
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    return settings.get_prefixed( total_prefix, k );
  }
  
  value_t get_prefixed( 
    const key_t &second_prefix, 
    const key_t &k, 
    const value_t &default_value 
    ) const
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    return settings.get_prefixed( total_prefix, k, default_value );
  }
  
  bool get_bool( 
    const key_t &k, 
    bool default_value=false  
    ) const
  {
    return settings.get_prefixed_bool(prefix,k,default_value);
  }
  
  bool get_prefixed_bool( 
    const key_t &second_prefix, 
    const key_t &k, 
    bool default_value=false 
    ) const
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    return settings.get_prefixed_bool( total_prefix, k );
  }
  
  long get_long( 
    const key_t &k,
    long default_value=0
    ) const
  {
    return settings.get_prefixed_long(prefix,k);
  }
  
  long get_prefixed_long( 
    const key_t &second_prefix,
    const key_t &k,
    long default_value=0 
    ) const
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    return settings.get_prefixed_long( total_prefix, k );
  }
  
  unsigned long get_ulong( 
    const key_t &k,
    unsigned long default_value=0
    ) const
  {
    return settings.get_prefixed_ulong(prefix,k);
  }
  
  unsigned long get_prefixed_ulong( 
    const key_t &second_prefix, 
    const key_t &k,
    unsigned long default_value=0
    ) const
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    return settings.get_prefixed_ulong( total_prefix, k );
  }
  
  bool set( 
    const key_t &k, 
    const value_t &v 
    )
  {
    return settings.set_prefixed(
      prefix,
      k, 
      v
      );
  }
  
  bool set_prefixed(
    const key_t &second_prefix,
    const key_t &k,
    const value_t &v
    )
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    return settings.set_prefixed( total_prefix, k, v );
  }
  
  bool set_bool( 
    const key_t &k, 
    bool v 
    )
  {
    return settings.set_prefixed_bool(
      prefix,
      k,
      v
      );
  }
  
  bool set_prefixed_bool(
    const key_t &second_prefix,
    const key_t &k,
    bool v
    )
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    return settings.set_prefixed_bool(
      total_prefix,
      k,
      v
      );
  }
  
  bool set_long( 
    const key_t &k, 
    long lv 
    )
  {
    return settings.set_prefixed_long(
      prefix,
      k,
      lv
      );
  }
  
  bool set_prefixed_long(
    const key_t &second_prefix,
    const key_t &k,
    long v
    )
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    return settings.set_prefixed_long(
      total_prefix,
      k,
      v
      );
  }
  
  
  bool set_ulong( 
    const key_t &k, 
    unsigned long lv 
    )
  {
    return settings.set_prefixed_ulong(
      prefix,
      k,
      lv
      );
  }
  
  bool set_prefixed_ulong(
    const key_t &second_prefix,
    const key_t &k,
    unsigned long v
    )
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    return settings.set_prefixed_ulong(
      total_prefix,
      k,
      v
      );
  }
  
  
  bool set_float(
    const key_t &k,
    float f
    )
  {
    return settings.set_prefixed_float(
      prefix,
      k,
      f
      );
  }
  
  bool set_prefixed_float(
    const key_t &second_prefix,
    const key_t &k,
    float v
    )
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    return settings.set_prefixed_float(
      total_prefix,
      k,
      v
      );
  }
  
  
  bool set_double(
    const key_t &k,
    double f
    )
  {
    return settings.set_prefixed_double(
      prefix,
      k,
      f
      );
  }
  
  bool set_prefixed_double(
    const key_t &second_prefix,
    const key_t &k,
    double v
    )
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    return settings.set_prefixed_double(
      total_prefix,
      k,
      v
      );
  }
  
  int find( 
    const key_t &k, 
    int start_pos=0 
    ) const
  {
    return settings.find_prefixed( 
      prefix, 
      k, 
      start_pos 
      );
  }
  
  int find_prefixed(
    const key_t &second_prefix,
    const key_t &k,
    int start_pos=0
    ) const
  {
    key_t total_prefix(prefix);
    total_prefix.cat(second_prefix);
    
    return settings.find_prefixed(
      total_prefix,
      k,
      start_pos
      );
  }
  
protected:
  T &settings;
  jdk_str<256> prefix;
};

#endif
