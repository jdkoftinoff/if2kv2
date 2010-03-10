#include "jdk_world.h"
#include "jdk_fast_url_lookup.h"
#include "jdk_cgi.h"
#include "jdk_url.h"

#if 0
int jdk_fast_url_partial_search( const void *a_, const void *b_ )
{
  const jdk_string *a = &(*(const jdk_fast_url_lookup::entry_t **)a_)->key;
  const jdk_string *b = &(*(const jdk_fast_url_lookup::entry_t **)b_)->key;
  int ret=0;
  
  // a is the full url, b is the partial url in our list
  
  if( a==b )
  {
    ret=0;
  }
  else if( !a )
  {
    ret=1;    // null entries go to end
  }
  else if( !b )
  {
    ret=-1;    // null entries go to end
  }
  else if( strstr( a->c_str(), b->c_str() )!=0 )
  {
    ret=0;    // a partial match is found!
  }
  else
  {
    ret=strcmp( a->c_str(), b->c_str() );    // no match so find which direction to go from here
  }
  return ret;
}

int jdk_fast_url_search( const void *a_, const void *b_ )
{
  const jdk_string *a = &(*(const jdk_fast_url_lookup::entry_t **)a_)->key;
  const jdk_string *b = &(*(const jdk_fast_url_lookup::entry_t **)b_)->key;
  int ret=0;
  
  // a is the full url, b is the partial url in our list
  
  if( a==b )
  {
    ret=0;
  }
  else if( !a )
  {
    ret=1;    // null entries go to end
  }
  else if( !b )
  {
    ret=-1;    // null entries go to end
  }
  else
  {
    ret=strcmp( a->c_str(), b->c_str() );    // no match so find which direction to go from here
  }
  return ret;
}


bool jdk_fast_url_simple_regex( const char *regex, const char *s )
{
  // if regex is fully contained in s, then we have a match
  if( strstr( s, regex )!=0 )
    return true;
  
  if( strcmp(regex,"*")==0 )  // * by itself matches anything
    return true;
  
  int regexlen = strlen( regex );
  int s_len = strlen( s );
  int s_pos=0;
  int reg_pos = 0;
  bool in_wildcard=false;  // ignore any initial non-matching characters
  int actual_matches=0;
  
  for( s_pos=0; s_pos<s_len && reg_pos<regexlen; ++s_pos )
  {
    // are we in a wildcard?
    
    if( in_wildcard )
    {
      // yup, does our string character match the next regex char?
      if( s[s_pos] ==  regex[reg_pos] )
      {
        // yup, so we are not in regex mode anymore.
        in_wildcard=false;
        actual_matches++;
        reg_pos++;
      }
    }
    else
    {
      // are we to go in wildcard mode?
      
      if( regex[reg_pos]=='*' )
      {
        // yup.
        in_wildcard = true;
        reg_pos++;  // skip the *
      }
      else // no, just keep matching characters
        if( s[s_pos] == regex[reg_pos] )
        {
          reg_pos++;  // matching so far
          actual_matches++;
        }
        else
        {
          return false; // not a match
        }
    }
  }
  // we got to the end of the strings, must be a match if we matched anything at all.
  return actual_matches>0;
}

int jdk_fast_url_regex_search( const void *a_, const void *b_ )
{
  const jdk_string *a = &(*(const jdk_fast_url_lookup::entry_t **)a_)->key;
  const jdk_string *b = &(*(const jdk_fast_url_lookup::entry_t **)b_)->key;
  int ret=0;
  
  // a is the full url, b is the partial url in our list
  
  if( a==b )
  {
    ret=0;
  }
  else if( !a )
  {
    ret=1;    // null entries go to end
  }
  else if( !b )
  {
    ret=-1;    // null entries go to end
  }
  else if( jdk_fast_url_simple_regex( b->c_str(), a->c_str() )==true)
  {
    ret=0;    // a partial match is found!
  }
  else
  {
    ret=strcmp( a->c_str(), b->c_str() );    // no match so find which direction to go from here
  }
  return ret;
}



jdk_fast_url_lookup::jdk_fast_url_lookup() : 
  url_list(),
  url_list_mutex()
{
  
}

jdk_fast_url_lookup::jdk_fast_url_lookup( const jdk_string &filename ) : 
url_list(),
url_list_mutex()
{
  load( filename );
}

jdk_fast_url_lookup::jdk_fast_url_lookup( const jdk_dynbuf &buf ) : 
url_list(),
url_list_mutex()
{
  load( buf );
}

jdk_fast_url_lookup::jdk_fast_url_lookup( FILE *f ) : 
url_list(),
url_list_mutex()
{
  load( f );
}


jdk_fast_url_lookup::~jdk_fast_url_lookup()
{
  
}


void jdk_fast_url_lookup::begin_modifications()
{
#if JDK_HAS_THREADS
  url_list_mutex.get();
#endif
}

void jdk_fast_url_lookup::clear()
{
  url_list.clear();
}

void jdk_fast_url_lookup::add_url( const jdk_string &url, const jdk_string *associate )
{
  entry_t entry;
  
  if( normalize_url(url,entry.key) )
  {    
    if( associate )
    {
      entry.value.cpy( *associate );
    }
    // add our string to our array
    url_list.add( new entry_t(entry) );
  }
}

void jdk_fast_url_lookup::end_modifications()
{
#if JDK_HAS_THREADS
  // re-sort all the entries so we can find entries with binary_search
  //url_list.sort();
  
  url_list_mutex.release();
#endif
}

bool jdk_fast_url_lookup::find_regex_url( const jdk_string &url, jdk_string *associate ) const
{
  entry_t normalized_url;
  
  if( normalize_url(url,normalized_url.key) )
  {
#if JDK_HAS_THREADS
    jdk_synchronized(url_list_mutex);
#endif
    for( int i=0; i<url_list.count(); ++i )
    {
      if( url_list.get(i) )
      {
        if( jdk_fast_url_simple_regex( url_list.get(i)->key.c_str(), normalized_url.key.c_str() )  )
        {
          if( associate )
            associate->cpy( url_list.get(i)->value );
          return true;
        }
      }
    }
    
    const entry_t *match = url_list.binary_search( normalized_url, jdk_fast_url_regex_search );
    if( match )
    {
      if( associate )
        associate->cpy( match->value );
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool jdk_fast_url_lookup::find_partial_url( const jdk_string &url, jdk_string *associate  ) const
{
  entry_t normalized_url;
  
  if( normalize_url(url,normalized_url.key) )
  {
#if JDK_HAS_THREADS
    jdk_synchronized(url_list_mutex);
#endif	
    const entry_t *match = url_list.binary_search( normalized_url, jdk_fast_url_partial_search );
    if( match )
    {
      if( associate )
        associate->cpy( match->value );
      return true;
    }
    else
    {
      return false;
    }	
  }
  else
  {
    return false;
  }
}

bool jdk_fast_url_lookup::find_complete_url( const jdk_string &url, jdk_string *associate  ) const
{
  entry_t normalized_url;
  
  if( normalize_url(url,normalized_url.key) )
  {
#if JDK_HAS_THREADS
    jdk_synchronized(url_list_mutex);
#endif
    const entry_t *match = url_list.binary_search( normalized_url,jdk_fast_url_search );
    if( match )
    {
      if( associate )
        associate->cpy( match->value );
      return true;
    }
    else
    {
      return false;
    }
    
  }
  else
  {
    return false;
  }
}

bool jdk_fast_url_lookup::normalize_url( const jdk_string &url,jdk_string &normalized_url ) const
{
  // blow apart the given url into its component pieces
  jdk_url exploded_url;
  
  if( exploded_url.explode( url ) )
  {    
    // the explosion succeeded so we should add this url.
    
    // make the hostname part lowercase - it is case insensitive
    exploded_url.host.lower();
    
    // unexplode the pieces of the url into our new string
    // we do this manually because we want to skip the http:// part
    // as well as the port if default
    // and the directory name if there is no directory specified
    
    if( exploded_url.port==80 )
    {
      if( (exploded_url.path.cmp("/")==0 || exploded_url.path.is_clear() ) )
      {
        normalized_url = exploded_url.host;
      }
      else
      {
        normalized_url.form( "%s%s",
                             exploded_url.host.c_str(),
                             exploded_url.path.c_str()
          );
      }
    }
    else
    {
      if( (exploded_url.path.cmp("/")==0 || exploded_url.path.is_clear() ) )
      {
        normalized_url.form( "%s:%d",
                             exploded_url.host.c_str(),
                             exploded_url.port
          );
      }
      else
      {
        normalized_url.form( "%s:%d%s",
                             exploded_url.host.c_str(),
                             exploded_url.port,
                             exploded_url.path.c_str()
          );
      }
      
    }
    
    
    
    // take out any cgi-style escape characters from the url
    // we can do this in place since it will only shorten the url.
    jdk_cgi_unescape( normalized_url, normalized_url );
    return true;
  }
  else
  {
    return false;
  }
}


bool jdk_fast_url_lookup::save( FILE *f ) const
{
#if JDK_HAS_THREADS
  jdk_synchronized(url_list_mutex);
#endif
  for( int i=0; i<url_list.count(); ++i )
  {
    const entry_t *p = url_list.get( i );
    if(p)
    {
      jdk_str<1024> encoded;
      
      if( encode_url( p->key, encoded ) )
      {
        if( !p->value.is_clear() )
        {
          encoded.cat( " " );
          encoded.cat( p->value );
        }
        
        if( !jdk_write_string_line( &encoded, f ) )
        {
          return false;
        }
      }
    }    
  }
  return true;
}

bool jdk_fast_url_lookup::save( const jdk_string &filename ) const
{
  FILE *f = fopen( filename.c_str(), "wt" );
  if( f )
  {
    bool ret= save(f);
    fclose(f);
    return ret;
  }
  else
  {
    return false;
  }
}

bool jdk_fast_url_lookup::save( jdk_dynbuf &buf ) const
{
#if JDK_HAS_THREADS
  jdk_synchronized(url_list_mutex);
#endif
  for( int i=0; i<url_list.count(); ++i )
  {
    const entry_t *p = url_list.get( i );
    if(p)
    {
      jdk_str<1024> encoded;
      
      if( encode_url( p->key, encoded ) )
      {
        if( !p->value.is_clear() )
        {
          encoded.cat( " " );
          encoded.cat( p->value );
        }
        encoded.cat( "\n" );		    
        
        buf.append_from_string( encoded );		
      }
    }    
  }
  return true;
}


bool jdk_fast_url_lookup::load( const jdk_dynbuf &buf )
{
#if JDK_HAS_THREADS
  jdk_synchronized(url_list_mutex);
#endif
  clear();
  jdk_str<4096> encoded_line;
  jdk_str<4096> line;
  int pos=0;
  
  while( (pos=buf.extract_to_string( encoded_line, pos ))>0 )
  {
    if( decode_url( encoded_line, line ) )
    {
      line.strip_begws();
      line.strip_endws();
      
      entry_t entry;
      pos=line.extract_token( 0, &entry.key, " \t" );
      if( pos>=0 )
      {
        line.extract_token( pos, &entry.value, " \t" );			    
        add_url( entry.key, &entry.value );		    
      }
    }
    else
    {
      break;
    }
  }
  
  url_list.sort();    
  return pos>0;
}

bool jdk_fast_url_lookup::load( FILE *f )
{
#if JDK_HAS_THREADS
  jdk_synchronized(url_list_mutex);
#endif
  clear();
  jdk_str<4096> encoded_line;
  jdk_str<4096> line;
  
  while( jdk_read_string_line( &encoded_line, f ) )
  {
    if( decode_url( encoded_line, line ) )
    {
      line.strip_begws();
      line.strip_endws();
      
      entry_t entry;
      int pos=line.extract_token( 0, &entry.key, " \t" );
      if( pos>=0 )
      {
        line.extract_token( pos, &entry.value, " \t" );			    
        add_url( entry.key, &entry.value );		    
      }
    }
    else
    {
      break;
    }
  }
  fclose(f); 
  
  url_list.sort();    
  return !ferror(f);
}

bool jdk_fast_url_lookup::load( const jdk_string &filename )
{
  FILE *f = fopen( filename.c_str(), "rt" );
  if( f )
  {
    bool ret= load(f);
    fclose(f);
    return ret;
  }
  else
  {
    return false;
  }
  
}


bool jdk_fast_url_lookup::encode_url( const jdk_string &in_url, jdk_string &out_encoded ) const
{
  out_encoded = in_url;
  out_encoded.strip_begws();
  out_encoded.strip_endws();
  
  return true;
}

bool jdk_fast_url_lookup::decode_url( const jdk_string &in_encoded, jdk_string &out_url ) const
{
  out_url = in_encoded;
  out_url.strip_begws();
  out_url.strip_endws();
  return true;
}

bool jdk_fast_url_lookup::get( int num, jdk_string &out ) const
{
#if JDK_HAS_THREADS
  jdk_synchronized(url_list_mutex);
#endif    
  if( num>=0 && num<url_list.count() )
  {
    const entry_t *entry = url_list.get( num );
    if( entry )
    {
      jdk_url normal_url;
      if( normal_url.explode( entry->key ) )
      {
        out = normal_url.unexplode();
        return true;
      }
    }
  }
  
  return false;
}

bool jdk_fast_url_lookup::getrandom( jdk_string &out ) const
{
#if JDK_HAS_THREADS
  jdk_synchronized(url_list_mutex);
#endif
  if( url_list.count()>0 )
  {
#if 0
#if JDK_IS_WIN32
    int num = random( url_list.getnum() ) % url_list.getnum();
#else
    int num = random() % url_list.getnum();
#endif
#else
    static unsigned int state=0;
    state+=1;
    int num = (state/4) % url_list.count();
#endif
    
    const short_url_t *item = &url_list.get( num )->key;
    if( item )
    {
      jdk_url normal_url;
      if( normal_url.explode( *item ) )
      {
        out = normal_url.unexplode();
        return true;
      }
    }
  }    
  out.clear();
  return false;
  
}

#endif

