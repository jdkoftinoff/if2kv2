#ifndef __IF2K_MINI_SCANNER_H
#define __IF2K_MINI_SCANNER_H

#include "if2k_mini_config.h"

#include "jdk_tree.h"
#include "jdk_dynbuf.h"
#include "jdk_remote_buf.h"
#include "jdk_lz.h"
#include "if2k_mini_tree.h"
#include "if2k_pattern_expander.h"

template <class tree_traits>
class if2k_mini_scanner_precompiled : public if2k_mini_tree<tree_traits>
{
  public:

  if2k_mini_scanner_precompiled( 
    const jdk_string_filename &filename
    ) 
    :    
    if2k_mini_tree<tree_traits>( filename )
    {
    }

  if2k_mini_scanner_precompiled( 
    const jdk_string_filename &filename,
    const jdk_string &tree_type_,
    const jdk_string &tree_category_
    ) 
    :    
    if2k_mini_tree<tree_traits>( filename, tree_type_, tree_category_ )
    {
    }

  if2k_mini_scanner_precompiled( 
    const jdk_string_filename &filename,
    const char *tree_type_,
    const jdk_string &tree_category_
    ) 
    :    
    if2k_mini_tree<tree_traits>( filename, tree_type_, tree_category_ )
    {
    }


  virtual ~if2k_mini_scanner_precompiled()
    {
    }

protected:
};



template <class tree_traits, class pattern_expander>
class if2k_mini_scanner : public if2k_mini_tree<tree_traits>
{
  public:
  typedef pattern_expander pattern_expander_t;

  if2k_mini_scanner( 
    const jdk_settings &settings_, 
    const jdk_string &base_filename_,
    const jdk_string &setting_name_local_dir_,
    const jdk_string &setting_name_remote_dir_,
    const jdk_string &setting_name_cache_dir_,
    const jdk_string &tree_type_,
    const jdk_string &tree_category_,
    const char *line_prefix_,
    int flags_
    ) 
    :
    if2k_mini_tree<tree_traits>( tree_type_, tree_category_ ),
    settings( settings_ ),
    base_filename( base_filename_ ),
    setting_name_local_dir( setting_name_local_dir_ ),
    setting_name_remote_dir( setting_name_remote_dir_ ),
    setting_name_cache_dir( setting_name_cache_dir_ ),
    prefix(line_prefix_),
    flags(flags_)
    {
      load_settings();
    }

  if2k_mini_scanner( 
    const jdk_settings &settings_, 
    const jdk_string &base_filename_,
    const jdk_string &setting_name_local_dir_,
    const jdk_string &setting_name_remote_dir_,
    const jdk_string &setting_name_cache_dir_,
    const char *tree_type_,
    const jdk_string &tree_category_,
    const char *line_prefix_,
    int flags_
    ) 
    :
    if2k_mini_tree<tree_traits>( tree_type_, tree_category_ ),
    settings( settings_ ),
    base_filename( base_filename_ ),
    setting_name_local_dir( setting_name_local_dir_ ),
    setting_name_remote_dir( setting_name_remote_dir_ ),
    setting_name_cache_dir( setting_name_cache_dir_ ),
    prefix(line_prefix_),
    flags(flags_)
    {
      load_settings();
    }

  void load_settings()
    {
      jdk_string_filename local_file;

      jdk_string_url remote_url;
      if( settings.get( setting_name_remote_dir ).len()>5 )
      {
        remote_url.form("%s/%s",settings.get( setting_name_remote_dir ).c_str(), base_filename.c_str() );
        local_file.form("%s/%s",settings.get( setting_name_cache_dir ).c_str(), base_filename.c_str() );
      }
      else
      {
        local_file.form("%s/%s",settings.get( setting_name_local_dir ).c_str(), base_filename.c_str() );
      }

      jdk_string_url http_proxy_auth_basic_username = settings.get("http.proxy.auth.basic.username");
      jdk_string_url http_proxy_auth_basic_password = settings.get("http.proxy.auth.basic.password");

      jdk_remote_buf data( 
        local_file,
        remote_url,
        settings.get("http.proxy"),
        2*1024*1024,
        false,
        http_proxy_auth_basic_username,
        http_proxy_auth_basic_password,
        true
        );
      
      if( !remote_url.is_clear() )
      {
        data.check_and_grab();
      }

      {
        if2k_mini_tree<tree_traits>::clear();
        pattern_expander_t expander;
        expander.load( 
          data.get_buf(),
          *this,
          flags,
          prefix
          );           
        
        if2k_mini_tree<tree_traits>::make_shortcuts();
      }
    }      
    
  virtual ~if2k_mini_scanner()
    {
    }


protected:

  const jdk_settings &settings;
  jdk_string_filename base_filename;
  jdk_string_url setting_name_local_dir;
  jdk_string_url setting_name_remote_dir;
  jdk_string_url setting_name_cache_dir;
  const jdk_str<256> prefix;
  int flags;
};



typedef if2k_mini_scanner<if2k_mini_tree_traits_url,if2k_pattern_expander_standard> if2k_mini_url_scanner;
typedef if2k_mini_scanner<if2k_mini_tree_traits_alphanumeric,if2k_pattern_expander_standard> if2k_mini_alphanumeric_scanner;

typedef if2k_mini_scanner_precompiled<if2k_mini_tree_traits_url_mmap> if2k_mini_url_scanner_precompiled;
typedef if2k_mini_scanner_precompiled<if2k_mini_tree_traits_alphanumeric_mmap> if2k_mini_alphanumeric_scanner_precompiled;

#endif
