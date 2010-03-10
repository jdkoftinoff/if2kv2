#ifndef _JDK_FAST_URL_LOOKUP_H
#define _JDK_FAST_URL_LOOKUP_H

#include "jdk_array.h"
#include "jdk_string.h"
#include "jdk_thread.h"
#include "jdk_dynbuf.h"
#include "jdk_pair.h"

class jdk_fast_url_lookup
{
public:
    typedef jdk_str<384> short_url_t;
    typedef jdk_pair< short_url_t, short_url_t > entry_t;

    jdk_fast_url_lookup();
    explicit jdk_fast_url_lookup( const jdk_string &filename );
    explicit jdk_fast_url_lookup( const jdk_dynbuf &buf );
    explicit jdk_fast_url_lookup( FILE *f );   
    
    virtual ~jdk_fast_url_lookup();
    
	void begin_modifications();    
    void clear();
    void add_url( const jdk_string &url, const jdk_string *associate=0 );
    void end_modifications();
    
    bool find_partial_url( const jdk_string &url, jdk_string *associate=0 ) const;
    bool find_regex_url( const jdk_string &url, jdk_string *associate=0 ) const;
    bool find_complete_url( const jdk_string &url, jdk_string *associate=0 ) const;

    virtual bool load( FILE *f );
    virtual bool load( const jdk_dynbuf &buf );
    virtual bool load( const jdk_string &filename );
        
    virtual bool save( FILE *f ) const;
    virtual bool save( jdk_dynbuf &buf ) const;
    virtual bool save( const jdk_string &filename ) const;
    
    bool get( int num, jdk_string &out ) const;
    bool getrandom( jdk_string &out ) const;
    size_t count() const { return url_list.count(); }    
    
protected:
    virtual bool encode_url( const jdk_string &in_url, jdk_string &out_encoded ) const;
    virtual bool decode_url( const jdk_string &in_encoded, jdk_string &out_url ) const;
    
    bool normalize_url( const jdk_string &in_url, jdk_string &out_url ) const;

    jdk_array< entry_t > url_list;
#if JDK_HAS_THREADS    
    mutable jdk_mutex url_list_mutex;
#endif    
    
};

#endif

