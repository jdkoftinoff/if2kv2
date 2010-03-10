#include "jdk_world.h"
#include "if2k_kernel.h"
#include "jdk_cgi.h"

if2_kernel_standard::if2_kernel_standard( const jdk_settings &settings_ )
  :	
  settings( settings_ ),
  logger(settings),
  expander(),
  block_unknown(settings,"block_unknown"),
  block_bad(settings,"block_bad"),  
  good_urls(),
  bad_urls(),
  bad_phrases(),
  censor_phrases(),
  good_newsgroups(),
  bad_newsgroups(),
  bad_email_content(),
  mutex("if2_kernel")
{
  jdk_synchronized(mutex);
  good_urls =
    new if2_scanner<if2_tree_traits_url>(
      settings,
      jdk_str<64>("good_urls_precomp"),
      jdk_str<64>("good_urls"),
      expander,
      8,
      100,
      jdk_str<128>("")
      );
  
  bad_urls =
    new if2_scanner<if2_tree_traits_url>(
      settings,
      jdk_str<64>("bad_urls_precomp"),
      jdk_str<64>("bad_urls"),
      expander,
      8,
      200,
      jdk_str<128>("")
      );
  
  bad_phrases =
    new if2_scanner<if2_tree_traits_alphanumeric>(
      settings,
      jdk_str<64>("bad_phrases_precomp"),
      jdk_str<64>("bad_phrases"),
      expander,
      8,
      300,
      jdk_str<128>("")
      );
  
  censor_phrases =
    new if2_scanner<if2_tree_traits_alphanumeric>(
      settings,
      jdk_str<64>("censor_phrases_precomp"),
      jdk_str<64>("censor_phrases"),
      expander,
      8,
      400,
      jdk_str<128>("")
      );
  
  good_newsgroups =
    new if2_scanner<if2_tree_traits_alphanumeric>(
      settings,
      jdk_str<64>("good_newsgroups_precomp"),
      jdk_str<64>("good_newsgroups"),
      expander,
      8,
      500,
      jdk_str<128>("")
      );
  
  bad_newsgroups =
    new if2_scanner<if2_tree_traits_alphanumeric>(
      settings,
      jdk_str<64>("bad_newsgroups_precomp"),
      jdk_str<64>("bad_newsgroups"),
      expander,
      8,
      600,
      jdk_str<128>("")
      );
  
  bad_email_content =
    new if2_scanner<if2_tree_traits_alphanumeric>(
      settings,
      jdk_str<64>("bad_email_content_precomp"),
      jdk_str<64>("bad_email_content"),
      expander,
      8,
      700,
      jdk_str<128>("") 
      );
  
}


template <class SCANNER>
static inline void if2_scanner_reload( 
  jdk_recursivemutex &mutex,
  SCANNER **scanner,
  const jdk_settings &settings,
  const jdk_string &precomp,
  const jdk_string &setting_prefix,
  if2_pattern_expander &expander,
  int num_files,
  int code_offset,
  const jdk_string &pattern_prefix
  )
{
  SCANNER *new_scanner = new SCANNER( settings, precomp, setting_prefix, expander, num_files, code_offset, pattern_prefix );
  SCANNER *old_scanner = *scanner;
  
  {		
    jdk_synchronized( mutex );
    *scanner = new_scanner;
    delete old_scanner;
  }
  
}


void if2_kernel_standard::update()
{
  if2_scanner_reload(
    mutex,
    &good_urls, 
    settings,jdk_str<64>("good_urls_precomp"),
    jdk_str<64>("good_urls"),
    expander,
    8,100,jdk_str<128>("")
    );
  
  if2_scanner_reload( 
    mutex,
    &bad_urls,
    settings,jdk_str<64>("bad_urls_precomp"),
    jdk_str<64>("bad_urls"),
    expander,
    8,200,jdk_str<128>("")
    );
  
  
  if2_scanner_reload( 
    mutex,
    &bad_phrases,
    settings,jdk_str<64>("bad_phrases_precomp"),
    jdk_str<64>("bad_phrases"),
    expander,
    8,300,jdk_str<128>("")
    );
  
  if2_scanner_reload( 
    mutex,
    &censor_phrases,
    settings,jdk_str<64>("censor_phrases_precomp"),
    jdk_str<64>("censor_phrases"),
    expander,
    8,400,jdk_str<128>("")
    );
  
  if2_scanner_reload( 
    mutex,
    &good_newsgroups,
    settings,jdk_str<64>("good_newsgroups_precomp"),
    jdk_str<64>("good_newsgroups"),
    expander,
    8,500,jdk_str<128>("")
    );
  
  if2_scanner_reload( 
    mutex,
    &bad_newsgroups,
    settings,jdk_str<64>("bad_newsgroups_precomp"),
    jdk_str<64>("bad_newsgroups"),
    expander,
    8,600,jdk_str<128>("")
    );
  
  if2_scanner_reload( 
    mutex,
    &bad_email_content,
    settings,jdk_str<64>("bad_email_content_precomp"),
    jdk_str<64>("bad_email_content"),
    expander,
    8,700,jdk_str<128>("")
    );
  
  
}


if2_kernel_result if2_kernel_standard::verify_url( 
  const char *client_address,
  const char *url_unescaped,
  jdk_string &match_phrase,
  bool override,
  bool override_unknown_only
  )
{
  int match_code=0;
  jdk_url normalized_url( url_unescaped );
  jdk_str<4096> url;
  
  jdk_cgi_unescape( normalized_url.unexplode().c_str(), url.c_str(), url.getmaxlen() );
  
  // kill any last slash in url
  if( url.get( url.len()-1 )=='/' )
    url.set( url.len()-1, '\0' );
  
  jdk_log( JDK_LOG_DEBUG3, "Verifying url: '%s'", url.c_str() );
  
  match_phrase.clear();
  
  // first, check if it is a known good site
  {		
    if2_kernel_standard_event<if2_tree_traits_url> event;
    
    bool f;
    char found_string[4096];
    
    
    {
      jdk_synchronized( mutex );			
      
      f = good_urls->find(
        url.c_str(),
        url.len(),
        event
        );
      if( f )
      {
        jdk_log( JDK_LOG_DEBUG2, "Good URL: '%s' match=%03d", url.c_str(), event.largest_match_value );
        
        match_code = event.largest_match_value;
        
        int len=good_urls->extract(
          found_string,
          sizeof( found_string ),
          event.best_match_item );
        
        found_string[len] = '\0';		    
      }						
    }
    
    if( f )
    {
      // this is a good URL
      jdk_synchronized( mutex );			
      if2_kernel_result result(match_code,true);
      logger.log( result, client_address, url.c_str(), found_string, override );
      match_phrase.cpy( found_string );
      return result;
    }
  }
  
  // next, check if it is a known bad site
  {
    if2_kernel_standard_event<if2_tree_traits_url> event;
    
    bool f;
    char found_string[4096];
    
    {				
      jdk_synchronized( mutex );		    
      f=bad_urls->find( url.c_str(), url.len(), event );
      if( f )
      {
        jdk_log( JDK_LOG_DEBUG2, "Bad URL: '%s' match=%03d", url.c_str(), event.largest_match_value );
	
        int len=bad_urls->extract( found_string, sizeof( found_string ), event.best_match_item );
        found_string[len] = '\0';		    
      }			
    }
    
    
    if(f)
    {
      jdk_synchronized(mutex);
      // this is a bad URL
      if2_kernel_result result(event.largest_match_value,!block_bad);
      logger.log( result, client_address, url.c_str(), found_string, override && !override_unknown_only );
      match_phrase.cpy( found_string );
      return result;
    }
  }
  
  // cehck to see if the URL matches a bad phrase
  {		
    if2_kernel_standard_event<if2_tree_traits_alphanumeric> event;
    
    bool f;
    char found_string[4096];
    
    {
      jdk_synchronized(mutex);
      f=bad_phrases->find( url.c_str(), url.len(), event );
      
      if( f )
      {
        jdk_log( JDK_LOG_DEBUG2, "Bad phrase in URL: '%s' match=%03d", url.c_str(), event.largest_match_value );
        
        int len=bad_phrases->extract( found_string, sizeof( found_string ), event.best_match_item );
        found_string[len] = '\0';		    
      }
    }
    
    if(f)
    {
      // this is a bad URL
      jdk_synchronized(mutex);
      if2_kernel_result result(event.largest_match_value,!block_bad);
      logger.log( result, client_address, url.c_str(), found_string, override );
      match_phrase.cpy( found_string );
      return result;
    }
  }
  
  // this is an unknown URL
  {
    jdk_log( JDK_LOG_DEBUG2, "Unknown URL: '%s'", url.c_str() );
    
    jdk_synchronized(mutex);
    if2_kernel_result result(0, !block_unknown );
    logger.log( result, client_address, url.c_str(), "", override );
    return result;		
  }
  
}



if2_kernel_result if2_kernel_standard::verify_post_data(
  const char *client_address,
  const char *url,
  const char *data,
  int data_len,
  jdk_string &match_phrase,
  bool override,
  bool override_unknown_only
  )
{
  match_phrase.clear();
  {		
    if2_kernel_standard_event<if2_tree_traits_alphanumeric> event;
    bool f;
    char found_string[4096];
    
    {
      jdk_synchronized(mutex);
      f =bad_phrases->find( data, data_len, event );
      
      if( f )
      {				
        int len=bad_phrases->extract( found_string, sizeof( found_string ), event.best_match_item );
        found_string[len] = '\0';
        jdk_log( JDK_LOG_DEBUG2, "Bad phrase in post data: '%s' match=%03d", found_string, event.largest_match_value );
      }
      
    }
    
    if( f )
    {
      jdk_synchronized(mutex);
      // this is bad data
      if2_kernel_result result(event.largest_match_value,!block_bad);			
      logger.log( result, client_address, url, found_string, override  );
      match_phrase.cpy( found_string );
      return result;
    }
    
  }
  
  
  return if2_kernel_result(0,true);
}


if2_kernel_result if2_kernel_standard::verify_received_data(
  const char *client_address,
  const char *url,
  const char *data,
  int data_len,
  jdk_string &match_phrase,
  bool override,
  bool override_unknown_only
  )
{
  match_phrase.clear();
  {		
    if2_kernel_standard_event<if2_tree_traits_alphanumeric> event;
    
    bool f;
    char found_string[4096];
    
    {
      jdk_synchronized(mutex);
      f= bad_phrases->find( data, data_len, event );
      if( f )
      {				
        int len=bad_phrases->extract( found_string, sizeof( found_string ), event.best_match_item );
        found_string[len] = '\0';
        jdk_log( JDK_LOG_DEBUG2, "Bad phrase in received data: '%s' match=%03d", found_string, event.largest_match_value );
      }			
    }
    
    if( f )
    {		   
      // this is, bad data
      jdk_synchronized(mutex);
      if2_kernel_result result(event.largest_match_value,!block_bad);			
      logger.log( result, client_address, url, found_string, override );
      match_phrase.cpy( found_string );
      return result;
    }
  }
  
  return if2_kernel_result(0,true);
}

