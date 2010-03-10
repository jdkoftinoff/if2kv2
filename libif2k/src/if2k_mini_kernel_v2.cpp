#include "jdk_world.h"
#include <assert.h>
#include "if2k_mini_kernel_v2.h"
#include "jdk_url.h"
#include "jdk_util.h"
#include "jdk_cgi.h"

#define PREFIX "kernel."


void mark_file_hidden( const jdk_string &fname )
{
#if JDK_IS_WIN32
  BOOL r=SetFileAttributesA( fname.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM );
  if( !r )
    jdk_log_debug3( "Error setting file '%s' to hidden/system attribs", fname.c_str() );
#else
#warning no mark_file_unhidden on platform
#endif
}


void mark_file_unhidden( const jdk_string &fname )
{
#if JDK_IS_WIN32
  BOOL r=SetFileAttributesA( fname.c_str(), FILE_ATTRIBUTE_NORMAL );
  if( !r )
    jdk_log_debug3( "Error setting file '%s' to normal attribs", fname.c_str() );
#else
#warning no mark_file_unhidden on platform
#endif
}
               
if2k_mini_kernel_v2::if2k_mini_kernel_v2(if2k_mini_kernel_v2_settings_collection &settings_collection_ )
  :
  settings_collection( settings_collection_ ), 
  settings( settings_collection_.get_merged_settings() )
{
  int i;
  for( i=0; i<8; ++i )
  {
    precompiled_good_urls[i] = 0;
    precompiled_good_nntp[i] = 0;
    precompiled_bad_urls[i] = 0;
    precompiled_postbad_urls[i] = 0;
    precompiled_bad_nntp[i] = 0;
    precompiled_bad_phrases[i] = 0;

    user_good_urls[i] = 0;
    user_good_nntp[i] = 0;
    user_bad_urls[i] = 0;
    user_postbad_urls[i] = 0;
    user_bad_nntp[i] = 0;
    user_bad_phrases[i] = 0;
  }
}

if2k_mini_kernel_v2::~if2k_mini_kernel_v2()
{
  for( int category=0; category<8; ++category )
  {
    delete precompiled_good_urls[category];
    delete precompiled_bad_urls[category];
    delete precompiled_postbad_urls[category];
    delete precompiled_bad_phrases[category];
    delete precompiled_good_nntp[category];
    delete precompiled_bad_nntp[category];

    delete user_good_urls[category];
    delete user_bad_urls[category];
    delete user_postbad_urls[category];
    delete user_bad_phrases[category];
    delete user_good_nntp[category];
    delete user_bad_nntp[category];

  }
}

void if2k_mini_kernel_v2::load()
{
  int i;

  override_allow = settings.get_bool( PREFIX "override.allow" );
  jdk_log( JDK_LOG_DEBUG2, "override allow = %d", override_allow );
  override_now = settings.get_bool( PREFIX "override.now" );
  override_unknown_only = settings.get_bool( PREFIX "override.unknown.only" );

  restart_triggered = false;
  precompile_triggered = false;

  jdk_string_filename tmp_filename;
  blocking_enable = settings.get_bool(PREFIX "blocking.enable");
  jdk_log( JDK_LOG_DEBUG2, "blocking enable = %d", blocking_enable );

  block_bad = settings.get_bool(PREFIX "blocking.block.bad");
  block_unknown = settings.get_bool(PREFIX "blocking.block.unknown");
  block_bad_images = settings.get_bool(PREFIX "blocking.image.redirect.enable");
  nntp_redirect_group = settings.get(PREFIX "blocking.nntp.redirect");

  censor_bad_url_in_content = settings.get_bool(PREFIX "blocking.censor.url.content" );
  censor_bad_phrase_in_content = settings.get_bool(PREFIX "blocking.censor.phrase.content" );

  block_sites_with_bad_url_in_content = settings.get_bool(PREFIX "blocking.block.url.content" );
  block_sites_with_bad_phrase_in_content = settings.get_bool(PREFIX "blocking.block.phrase.content" );
  content_match_threshold = settings.get_long(PREFIX "blocking.content.match.threshold" );

  if( !settings.get_bool("license.valid") )
  {
    block_bad=false;
    block_unknown=false;
    block_bad_images=false;
    censor_bad_url_in_content=false;
    censor_bad_phrase_in_content=false;
    block_sites_with_bad_url_in_content=false;
    block_sites_with_bad_phrase_in_content=false;
  }

  for( i=0; i<8; ++i )
  {
    jdk_setting_key key;
    key.form(  PREFIX "db.category.%d.enable", i+1 );

    categories_enable[i] = settings.get_bool(key);

    key.form(  PREFIX "db.category.%d.name", i+1 );
    categories_name[i] = settings.get( key );

    key.form(  PREFIX "db.good.category.%d.enable", i+1 );

    good_categories_enable[i] = settings.get_bool(key);

    key.form(  PREFIX "db.good.category.%d.name", i+1 );
    good_categories_name[i] = settings.get( key );

    delete precompiled_good_urls[i];
    delete precompiled_bad_urls[i];
    delete precompiled_postbad_urls[i];
    delete precompiled_bad_phrases[i];
    delete precompiled_good_nntp[i];
    delete precompiled_bad_nntp[i];

    delete user_good_urls[i];
    delete user_bad_urls[i];
    delete user_postbad_urls[i];
    delete user_bad_phrases[i];
    delete user_good_nntp[i];
    delete user_bad_nntp[i];

    precompiled_good_urls[i] = 0;
    precompiled_good_nntp[i] = 0;
    precompiled_bad_urls[i] = 0;
    precompiled_postbad_urls[i] = 0;
    precompiled_bad_nntp[i] = 0;
    precompiled_bad_phrases[i] = 0;

    user_good_urls[i] = 0;
    user_good_nntp[i] = 0;
    user_bad_urls[i] = 0;
    user_postbad_urls[i] = 0;
    user_bad_nntp[i] = 0;
    user_bad_phrases[i] = 0;
  }


  
  if( settings.get_bool(PREFIX "db.precompiled.enable")==true )
  {
    jdk_string_filename local_precompiled_dir( settings.get( PREFIX "db.precompiled.dir" ) );
    
    for( i=0; i<8; ++i )
    {
      tmp_filename.form( "%s/%02dgoodurl.txt.pre", local_precompiled_dir.c_str(), i+1 );
      
      precompiled_good_urls[i] = 
        new if2k_mini_url_scanner_precompiled( tmp_filename, "Built-in good URL", good_categories_name[i] );
      
      tmp_filename.form( "%s/%02dgoodnntp.txt.pre", local_precompiled_dir.c_str(), i+1 );
      precompiled_good_nntp[i] = 
        new if2k_mini_alphanumeric_scanner_precompiled( tmp_filename, "Built-in good newsgroup", good_categories_name[i] );               
      
      tmp_filename.form( "%s/%02dbadurl.txt.pre", local_precompiled_dir.c_str(), i+1 );
      precompiled_bad_urls[i] = 
        new if2k_mini_url_scanner_precompiled( tmp_filename, "Built-in bad URL", categories_name[i] );

      tmp_filename.form( "%s/%02dpostbadurl.txt.pre", local_precompiled_dir.c_str(), i+1 );
      precompiled_postbad_urls[i] = 
        new if2k_mini_url_scanner_precompiled( tmp_filename, "Built-in postbad URL", categories_name[i] );
      
      tmp_filename.form( "%s/%02dbadphr.txt.pre", local_precompiled_dir.c_str(), i+1 );
      precompiled_bad_phrases[i] = 
        new if2k_mini_alphanumeric_scanner_precompiled( tmp_filename, "Built-in bad phrase", categories_name[i] );
      
      tmp_filename.form( "%s/%02dbadnntp.txt.pre", local_precompiled_dir.c_str(), i+1 );
      precompiled_bad_nntp[i] =
        new if2k_mini_alphanumeric_scanner_precompiled( tmp_filename, "Built-in bad newsgroup", categories_name[i] );
    }
  }
  else
  {
    for( i=0; i<8; ++i )
    {
      precompiled_good_urls[i] = 0;
      precompiled_bad_urls[i] = 0;
      precompiled_bad_phrases[i] = 0;
      precompiled_good_nntp[i] = 0;
      precompiled_bad_nntp[i] = 0;
    }    
  }

  jdk_str<128> user_dir(PREFIX "db.user.dir");
  jdk_str<128> remote_dir(PREFIX "db.user.remote");
  jdk_str<128> cache_dir(PREFIX "db.user.remote.cache.dir");

  jdk_mkdir( settings.get( cache_dir ).c_str(), 0750 );

  for( i=0; i<8; ++i )
  {

    {
      tmp_filename.form( "%02dgoodurl.txt", i+1 );
      user_good_urls[i] =
        new if2k_mini_url_scanner( 
          settings, 
          tmp_filename, 
          user_dir, 
          remote_dir, 
          cache_dir,
          "User good URL", 
          good_categories_name[i], 
          "", 
          0 
          );
#if IF2K_MINI_NNTP
      tmp_filename.form( "%02dgoodnntp.txt", i+1 );
      user_good_nntp[i] =
        new if2k_mini_alphanumeric_scanner( 
          settings, 
          tmp_filename, 
          user_dir, 
          remote_dir, 
          cache_dir,
          "User good newsgroup", 
          good_categories_name[i], 
          "", 
          0 
          );
#endif
    }

    {
      tmp_filename.form( "%02dbadurl.txt", i+1 );
      user_bad_urls[i] =
        new if2k_mini_url_scanner( 
          settings, 
          tmp_filename, 
          user_dir, 
          remote_dir, 
          cache_dir,
          "User bad URL", 
          categories_name[i], 
          "", 
          0 );

      tmp_filename.form( "%02dpostbadurl.txt", i+1 );
      user_postbad_urls[i] =
        new if2k_mini_url_scanner( 
          settings, 
          tmp_filename, 
          user_dir, 
          remote_dir, 
          cache_dir,
          "User postbad URL", 
          categories_name[i], 
          "", 
          0 );
      
      tmp_filename.form( "%02dbadphr.txt", i+1 );
      user_bad_phrases[i] =
        new if2k_mini_alphanumeric_scanner( 
          settings, 
          tmp_filename, 
          user_dir, 
          remote_dir, 
          cache_dir,
          "User bad phrase",
          categories_name[i], 
          "", 
          0 
          );
#if IF2K_MINI_NNTP          
      tmp_filename.form( "%02dbadnntp.txt", i+1 );
      user_bad_nntp[i] =
        new if2k_mini_alphanumeric_scanner( 
          settings, 
          tmp_filename, 
          user_dir, 
          remote_dir, 
          cache_dir,
          "User bad newsgroup", 
          categories_name[i], 
          "", 
          0 
          );
#endif        
    }
  }


  override_allow = settings.get_bool( PREFIX "override.allow" );
  override_now = settings.get_bool( PREFIX "override.now" );
  override_unknown_only = settings.get_bool( PREFIX "override.unknown.only" );

  blocking_enable = settings.get_bool(PREFIX "blocking.enable");
  block_bad = settings.get_bool(PREFIX "blocking.block.bad");
  block_unknown = settings.get_bool(PREFIX "blocking.block.unknown");
  nntp_redirect_group = settings.get(PREFIX "blocking.nntp.redirect");
  block_bad_images = settings.get_bool(PREFIX "blocking.image.redirect.enable");

  censor_bad_url_in_content = settings.get_bool(PREFIX "blocking.censor.url.content" );
  censor_bad_phrase_in_content = settings.get_bool(PREFIX "blocking.censor.phrase.content" );

  block_sites_with_bad_url_in_content = settings.get_bool(PREFIX "blocking.block.url.content" );
  block_sites_with_bad_phrase_in_content = settings.get_bool(PREFIX "blocking.block.phrase.content" );
  content_match_threshold = settings.get_long(PREFIX "blocking.content.match.threshold" );

  for( i=0; i<8; ++i )
  {
    jdk_setting_key key;
    key.form(  PREFIX "db.category.%d.enable", i+1 );

    categories_enable[i] = settings.get_bool(key);

    key.form(  PREFIX "db.category.%d.name", i+1 );
    categories_name[i] = settings.get( key );

    key.form(  PREFIX "db.good.category.%d.enable", i+1 );

    good_categories_enable[i] = settings.get_bool(key);

    key.form(  PREFIX "db.good.category.%d.name", i+1 );
    good_categories_name[i] = settings.get( key );

  }
  
  for( i=0; i<8; ++i )
  {
    if( good_categories_enable[i] )
    {
      user_good_urls[ i ]->load_settings();
      
#if IF2K_MINI_NNTP 
      user_good_nntp[ i ]->load_settings();
#endif
    }
    if( categories_enable[i] )
    {
      user_bad_urls[ i ]->load_settings();      
      user_postbad_urls[ i ]->load_settings();
      user_bad_phrases[ i ]->load_settings();
      
#if IF2K_MINI_NNTP
      user_bad_nntp[ i ]->load_settings();
#endif
    }
  }
}

bool if2k_mini_kernel_v2::is_in_override_mode()
{
  bool r=override_now && override_allow;
  return r;
}

bool if2k_mini_kernel_v2::set_override_mode( bool f )
{
  if( override_allow )
  {
    override_now = f;
  }
  bool r=(override_allow && f) | (!override_allow && !f);
  jdk_log( JDK_LOG_DEBUG1, "Override mode set to %d (allow=%d,request=%d)",
    r, override_allow, override_now
    );
  return r;
}

bool if2k_mini_kernel_v2::update_install_settings( const jdk_settings &s )
{
#if JDK_IS_WIN32
  jdk_settings_win32registry  r;
  r.merge( s );
  bool res = r.save_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_INSTALL_REGISTRY_LOCATION );
#else
  jdk_string_filename f;

  f.form("install.txt");
  bool res = s.save_file( f );
#endif
  if( res )
  {
    jdk_log(JDK_LOG_DEBUG1, "Updated install settings" );
  }
  else
  {
    jdk_log(JDK_LOG_DEBUG1, "Error updating install settings" );
  }
  return res;

}


bool if2k_mini_kernel_v2::update_additional_settings( const jdk_settings &s )
{
#if JDK_IS_WIN32
  jdk_settings_win32registry  r;
  r.merge( s );
  bool res = r.save_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_ADDITIONAL_REGISTRY_LOCATION );
#else
  jdk_string_filename f;

  f.form("additional.txt");
  bool res = s.save_file( f );

  jdk_log(JDK_LOG_DEBUG1, "update rate set to %s", s.get("kernel.settings.update.rate").c_str() );
#endif
  if( res )
  {
    jdk_log(JDK_LOG_DEBUG1, "Updated additional settings" );
  }
  else
  {
    jdk_log(JDK_LOG_DEBUG1, "Error updating additional settings" );
  }
  return res;

}


bool if2k_mini_kernel_v2::update_license_settings( const jdk_settings &s )
{
#if JDK_IS_WIN32
  jdk_settings_win32registry  r;
  r.merge( s );
  bool res = r.save_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_LICENSE_REGISTRY_LOCATION );
#else
  jdk_string_filename f;

  f.form("license.txt");
  bool res = s.save_file( f);
#endif
  if( res )
  {
    jdk_log(JDK_LOG_DEBUG1, "Updated license settings" );
  }
  else
  {
    jdk_log(JDK_LOG_DEBUG1, "Error updating license settings" );
  }
  return res;
}


bool if2k_mini_kernel_v2::get_predb_file( const jdk_string_filename &base, jdk_buf &b )
{
  b.clear();

  jdk_string_filename f;
  f.form( "predb/%s", base.c_str() );
  bool r = b.append_from_file( f );
  if( r )
  {
    jdk_log(JDK_LOG_DEBUG1, "Read predb file '%s' with data %d bytes", 
      f.c_str(),
      b.get_data_length()
      );
  }
  else
  {
    jdk_log(JDK_LOG_DEBUG1, "Error reading predb file '%s' with data %d bytes",
      f.c_str(),
      b.get_data_length()
      );
  }

  return r;
}


bool if2k_mini_kernel_v2::get_db_file( const jdk_string_filename &base, jdk_buf &b )
{
  b.clear();

  jdk_string_filename f;
  f.form( "db/%s", base.c_str() );
  bool r = b.append_from_file( f );
  if( r )
  {
    jdk_log(JDK_LOG_DEBUG1, "Read db file '%s' with data %d bytes", 
      f.c_str(),
      b.get_data_length()
      );
  }
  else
  {
    jdk_log(JDK_LOG_DEBUG1, "Error reading db file '%s' with data %d bytes",
      f.c_str(),
      b.get_data_length()
      );
  }

  return r;
}


void if2k_mini_kernel_v2::fix_url_list( jdk_dynbuf &dest, const jdk_buf &src )
{
  jdk_string_url line;
  int pos=0;

  while( (pos=src.extract_to_string( line, pos ))>0 )
  {
    line.strip_begendws();

    line.remove_prefix("http://");
    line.remove_prefix("www.");

    jdk_string_url final_line;
    jdk_cgi_escape_with_amp( line, final_line );
    dest.append_from_string( final_line );
    dest.append_from_string( "\r\n" );
  }
}

bool if2k_mini_kernel_v2::set_predb_file( const jdk_string_filename &base, const jdk_buf &b_ )
{
  jdk_dynbuf b;
  fix_url_list( b, b_ );

  jdk_string_filename f;
  f.form( "predb/%s", base.c_str() );
  bool r = b.extract_to_file( f );
  
  if( r )
  {
    jdk_log(JDK_LOG_DEBUG1, "Updated predb file '%s' with data %d bytes", 
      f.c_str(),
      b.get_data_length()
      );
  }
  else
  {
    jdk_log(JDK_LOG_DEBUG1, "Error updating predb file '%s' with data %d bytes",
      f.c_str(),
      b.get_data_length()
      );
  }
  return r;
}


bool if2k_mini_kernel_v2::set_db_file( const jdk_string_filename &base, const jdk_buf &b_ )
{
  jdk_dynbuf b;
  fix_url_list( b, b_ );

  jdk_string_filename f;
  f.form( "db/%s", base.c_str() );

  mark_file_unhidden( f );
  bool r=b.extract_to_file( f );
  mark_file_hidden( f );
  if( r )
  {
    jdk_log(JDK_LOG_DEBUG1, "Updated db file '%s' with data %d bytes", 
      f.c_str(),
      b.get_data_length()
      );
  }
  else
  {
    jdk_log(JDK_LOG_DEBUG1, "Error updating db file '%s' with data %d bytes",
      f.c_str(),
      b.get_data_length()
      );
  }
  return r;
}


bool if2k_mini_kernel_v2::trigger_restart()
{
  restart_triggered=true;
  jdk_log(JDK_LOG_INFO, "Triggering restart" );
  
  return true;
}


bool if2k_mini_kernel_v2::trigger_precompile()
{
  precompile_triggered=true;
  jdk_log(JDK_LOG_INFO, "Triggering precompile" );
  return true;
}

bool if2k_mini_kernel_v2::is_restart_triggered()
{
  return restart_triggered;
}

bool if2k_mini_kernel_v2::is_precompile_triggered()
{
  return precompile_triggered;
}




void if2k_mini_kernel_v2::verify_url( 
    if2k_mini_kernel_request &request
    )
{
  request.is_blocked=false;
  request.is_good=false;
  request.is_bad=false;
  request.is_postbad=false;
  request.is_unknown=true;
  

  if( override_now || !blocking_enable)
  {
    jdk_log( JDK_LOG_DEBUG2, "No blocking now (%d,%d)", override_now, blocking_enable );
    return;
  }
  else
  {
    jdk_log( JDK_LOG_DEBUG3, "Blocking now (%d,%d)", override_now, blocking_enable );
  }

  jdk_url url( request.get_url() );
  jdk_string_url url_string( jdk_url::normalize_and_unescape(request.get_url()) );

  if( url.host.chr('*') )
  {
    jdk_log( JDK_LOG_DEBUG1, "Blocked url with '*' in hostname" );
    request.is_blocked=true;
    return;
  }

  // kill any last slash in url
  if( url_string.get( url_string.len()-1 )=='/' )
    url_string.set( url_string.len()-1, '\0' );  

  int i;
  jdk_log( JDK_LOG_DEBUG1, "Verifying normalized URL: %s", url_string.c_str() );

  for( i=0; i<8; ++i )
  {
    if( good_categories_enable[i] )
    {
      if( if2k_mini_kernel_v2_scan_good( precompiled_good_urls[i], request, url_string ) )
        break;
      
      if( if2k_mini_kernel_v2_scan_good( user_good_urls[i], request, url_string ) )
        break;

      if( if2k_mini_kernel_v2_scan_good( precompiled_good_nntp[i], request, url_string ) )
        break;
      
      if( if2k_mini_kernel_v2_scan_good( user_good_nntp[i], request, url_string ) )
        break;

    }
  }

  for( i=0; i<8; ++i )
  {
    if( categories_enable[i] )
    {
      if( if2k_mini_kernel_v2_scan_bad( precompiled_bad_urls[i], request, url_string ) )
        break;
      
      if( if2k_mini_kernel_v2_scan_bad( user_bad_urls[i], request, url_string ) )
        break;

      if( if2k_mini_kernel_v2_scan_postbad( precompiled_postbad_urls[i], request, url_string ) )
        break;
      
      if( if2k_mini_kernel_v2_scan_postbad( user_postbad_urls[i], request, url_string ) )
        break;


      if( if2k_mini_kernel_v2_scan_bad( precompiled_bad_phrases[i], request, url_string ) )
        break;

      if( if2k_mini_kernel_v2_scan_bad( user_bad_phrases[i], request, url_string ) )
        break;

      if( if2k_mini_kernel_v2_scan_bad( precompiled_bad_nntp[i], request, url_string ) )
        break;
      
      if( if2k_mini_kernel_v2_scan_bad( user_bad_nntp[i], request, url_string ) )
        break;

    }
  }

    
  
  if( blocking_enable &&  block_bad && ( request.is_bad || request.is_postbad ) )
  {
    request.is_blocked=true;
  }

  if( blocking_enable && block_unknown && request.is_unknown  )
  {
    request.is_blocked=true;
  }

  if( request.is_good && !request.is_postbad )
  {
    request.is_blocked=false;
  }
        
  jdk_log( JDK_LOG_DEBUG1, "blocked=%d  unknown=%d  good=%d  bad=%d  postbad=%d  block_bad=%d  block_unknown=%d '%s'",  
           (int)request.is_blocked, 
           (int)request.is_unknown, 
           (int)request.is_good, 
           (int)request.is_bad,
           (int)request.is_postbad,
           (int)block_bad, 
           (int)block_unknown, 
           url_string.c_str()
    );
  
  
  if( request.is_blocked )
  {
    if( request.is_unknown )
    {
      request.set_reason( "Unknown URL" );
    }
    else if( request.is_bad || request.is_postbad )
    {
      request.set_reason( "Known Bad URL" );
    }
    jdk_log( JDK_LOG_INFO, "url=%s category=%s type=%s match=%s",
             url_string.c_str(),
             request.get_category().c_str(),
             request.get_matchtype().c_str(),
             request.get_match().c_str()
      );
  }
}

void if2k_mini_kernel_v2::verify_received_data(
    if2k_mini_kernel_request &request,
    jdk_buf &data
  )
{
  if( data.get_data_length() > 256L*1024L*1024L )
  {
    jdk_log( JDK_LOG_ERROR, "Internal buffer too large (receive)" );
    assert( data.get_data_length()>0 );
    return;
  }
  if( override_now || !blocking_enable)
  {
    jdk_log( JDK_LOG_DEBUG2, "No blocking now (%d,%d)", override_now, blocking_enable );
    return;
  }
  else
  {
    jdk_log( JDK_LOG_DEBUG3, "Blocking now (%d,%d)", override_now, blocking_enable );
  }

  if( blocking_enable && block_bad && !request.is_good )
  {
    int bad_phrase_count = 0;
    int bad_url_count = 0;


    for( int i=0; i<8; ++i )
    {
      if( categories_enable[i] )
      {
        if( censor_bad_url_in_content==true )
        {
          jdk_log( JDK_LOG_DEBUG4, "censor url=%d", censor_bad_url_in_content );
          bad_url_count += if2k_mini_kernel_v2_censor( precompiled_bad_urls[i], request, data );        
          bad_url_count += if2k_mini_kernel_v2_censor( user_bad_urls[i], request, data );
          bad_url_count += if2k_mini_kernel_v2_censor( precompiled_postbad_urls[i], request, data );        
          bad_url_count += if2k_mini_kernel_v2_censor( user_postbad_urls[i], request, data );
        }

        if( censor_bad_phrase_in_content==true )
        {
          jdk_log( JDK_LOG_DEBUG4, "censor url=%d", censor_bad_phrase_in_content );
          bad_phrase_count += if2k_mini_kernel_v2_censor( precompiled_bad_nntp[i], request, data );
        
          bad_phrase_count += if2k_mini_kernel_v2_censor( user_bad_nntp[i], request, data );
          
          bad_phrase_count += if2k_mini_kernel_v2_censor( precompiled_bad_phrases[i], request, data );
          
          bad_phrase_count += if2k_mini_kernel_v2_censor( user_bad_phrases[i], request, data );
        }
      }            
    }

    if( block_sites_with_bad_url_in_content && bad_url_count >= content_match_threshold 
        || block_sites_with_bad_phrase_in_content && bad_phrase_count >= content_match_threshold )
    {
      jdk_log( JDK_LOG_DEBUG4, "too many matches (%d or %d > %d), blocking content", bad_url_count, bad_phrase_count, content_match_threshold );
      request.is_blocked=true;
    }
  }
  
  if( request.is_blocked )
  {
    if( request.is_bad )
    {
      request.set_reason( "Known bad URL" );
    }
    jdk_log( JDK_LOG_INFO, "url=%s category=%s type=%s match=%s",
             request.get_url().c_str(),
             request.get_category().c_str(),
             request.get_matchtype().c_str(),
             request.get_match().c_str()
      );
  }

  if( blocking_enable && block_bad && ( request.is_bad || request.is_postbad ) )
  {
    request.is_blocked=true;
  }

  if( request.is_good )
  {
    request.is_blocked=false;
  }

  if( request.is_blocked )
  {
    if( request.is_bad )
    {
      request.set_reason( "Bad received data" );
    }

    jdk_log( JDK_LOG_INFO, "url=%s category=%s type=%s match=%s",
             request.get_url().c_str(),
             request.get_category().c_str(),
             request.get_matchtype().c_str(),
             request.get_match().c_str()
      );
  }

}



void if2k_mini_kernel_v2::verify_transmitted_data(
    if2k_mini_kernel_request &request,
    jdk_buf &data
  )
{
  if( data.get_data_length() > 256L*1024L*1024L )
  {
    jdk_log( JDK_LOG_ERROR, "Internal buffer too large (transmit)." );
    return;
  }

  if( override_now || !blocking_enable)
  {
    jdk_log( JDK_LOG_DEBUG2, "No blocking now (%d,%d)", override_now, blocking_enable );
    return;
  }
  else
  {
    jdk_log( JDK_LOG_DEBUG3, "Blocking now (%d,%d)", override_now, blocking_enable );
  }

  if( blocking_enable && block_bad && !request.is_good )
  {
    int bad_phrase_count = 0;
    int bad_url_count = 0;

    for( int i=0; i<8; ++i )
    {
      if( categories_enable[i] )
      {
        if( censor_bad_url_in_content==true )
        {
          bad_url_count += if2k_mini_kernel_v2_censor( precompiled_bad_urls[i], request, data );        
          bad_url_count += if2k_mini_kernel_v2_censor( user_bad_urls[i], request, data );
          bad_url_count += if2k_mini_kernel_v2_censor( precompiled_postbad_urls[i], request, data );        
          bad_url_count += if2k_mini_kernel_v2_censor( user_postbad_urls[i], request, data );

        }

        if( censor_bad_phrase_in_content==true )
        {
          bad_phrase_count += if2k_mini_kernel_v2_censor( precompiled_bad_nntp[i], request, data );
        
          bad_phrase_count += if2k_mini_kernel_v2_censor( user_bad_nntp[i], request, data );
          
          bad_phrase_count += if2k_mini_kernel_v2_censor( precompiled_bad_phrases[i], request, data );
          
          bad_phrase_count += if2k_mini_kernel_v2_censor( user_bad_phrases[i], request, data );
        }
      }            
    }

    if( block_sites_with_bad_url_in_content && bad_url_count >= content_match_threshold 
        || block_sites_with_bad_phrase_in_content && bad_phrase_count >= content_match_threshold )
    {
      request.is_blocked=true;
    }
  }
  
  if( request.is_blocked )
  {
    if( request.is_bad )
    {
      request.set_reason( "Known Bad URL" );
    }
    jdk_log( JDK_LOG_INFO, "url=%s category=%s type=%s match=%s",
             request.get_url().c_str(),
             request.get_category().c_str(),
             request.get_matchtype().c_str(),
             request.get_match().c_str()
      );
  }

  if( blocking_enable && block_bad && (request.is_bad || request.is_postbad ) )
  {
    request.is_blocked=true;
  }

  if( request.is_good )
  {
    request.is_blocked=false;
  }

  if( request.is_blocked )
  {
    if( request.is_bad )
    {
      request.set_reason( "Bad received data" );
    }

    jdk_log( JDK_LOG_INFO, "url=%s category=%s type=%s match=%s",
             request.get_url().c_str(),
             request.get_category().c_str(),
             request.get_matchtype().c_str(),
             request.get_match().c_str()
      );
  }

}

void if2k_mini_kernel_v2::verify_nntp_group(
    if2k_mini_kernel_request &request,
    jdk_string &group
  )
{
  if( override_now || !blocking_enable)
  {
    jdk_log( JDK_LOG_DEBUG2, "No blocking now (%d,%d)", override_now, blocking_enable );
    return;
  }
  else
  {
    jdk_log( JDK_LOG_DEBUG3, "Blocking now (%d,%d)", override_now, blocking_enable );
  }

  if( blocking_enable && block_bad )
  {
    int i;
    for( i=0; i<8; ++i )
    {
      if( good_categories_enable[i] )
      {
        if( if2k_mini_kernel_v2_scan_good( precompiled_good_nntp[i], request, group ) )
          break;
               
        if( if2k_mini_kernel_v2_scan_good( user_good_nntp[i], request, group ) )
          break;        
      }            
    }

    for( i=0; i<8; ++i )
    {
      if( categories_enable[i] )
      {
        if( if2k_mini_kernel_v2_scan_bad( precompiled_bad_nntp[i], request, group ) )
          break;
               
        if( if2k_mini_kernel_v2_scan_bad( user_bad_nntp[i], request, group ) )
          break;        
      }            
    }
  }

  if( blocking_enable && block_bad && request.is_bad )
  {
    request.is_blocked=true;
  }

  if( request.is_good )
  {
    request.is_blocked=false;
  }

  if( request.is_blocked )
  {
    if( request.is_bad )
    {
      request.set_reason( "Bad newsgroup" );
    }

    jdk_log( JDK_LOG_INFO, "url=%s category=%s type=%s match=%s",
             request.get_url().c_str(),
             request.get_category().c_str(),
             request.get_matchtype().c_str(),
             request.get_match().c_str()
      );
  }
  
}

void if2k_mini_kernel_v2::verify_nntp_subject(
    if2k_mini_kernel_request &request,
    jdk_string &subject
  )
{
  // todo: determine if content is bad
}

void if2k_mini_kernel_v2::verify_nntp_article(
    if2k_mini_kernel_request &request,
    jdk_buf &data
  )
{
  // todo: determine if content is bad
}

