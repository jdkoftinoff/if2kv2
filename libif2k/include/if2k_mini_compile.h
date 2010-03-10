#ifndef _H_IF2K_MINI_COMPILE_
#define _H_IF2K_MINI_COMPILE_

#include "if2k_mini_tree.h"
#include "jdk_settings.h"
#include "if2k_mini_config.h"

inline bool if2k_mini_compile_url( const jdk_string_filename &input_filename, const jdk_string_filename &output_filename, volatile int *progress=0 )
{
  jdk_str<64> prefix;

  bool r = if2k_mini_tree_compile<if2k_mini_tree_traits_url,if2k_pattern_expander_standard>(
    input_filename,
    output_filename,
    0,
    prefix,
    progress
    );
  jdk_log( JDK_LOG_INFO, "Compiled %s urls into %s (%s)", input_filename.c_str(), output_filename.c_str(), r ? "Success" : "Fail" );
  return r;
}

inline bool if2k_mini_compile_alphanumeric( const jdk_string_filename &input_filename, const jdk_string_filename &output_filename, volatile int *progress=0 )
{
  jdk_str<64> prefix;

  bool r = if2k_mini_tree_compile<if2k_mini_tree_traits_alphanumeric,if2k_pattern_expander_standard>(
    input_filename,
    output_filename,
    0,
    prefix,
    progress
    );
  jdk_log( JDK_LOG_INFO, "Compiled %s alphanumeric into %s (%s)", input_filename.c_str(), output_filename.c_str(), r ? "Success" : "Fail" );
  return r;
}

inline void if2k_mini_compile_all( const jdk_settings &settings, volatile int *progress1=0, volatile int *progress2=0 )
{
  jdk_log( JDK_LOG_INFO, "Starting precompiling of internet filter database" );

  jdk_string_filename local_precompiled_dir( settings.get("kernel.db.precompiled.dir" ) );
  
  for( int i=0; i<8; ++i )
  {
    jdk_string_filename input_filename, output_filename;
    
    input_filename.form( "%s/%02dgoodurl.txt", local_precompiled_dir.c_str(), i+1 );
    output_filename.form( "%s/%02dgoodurl.txt.pre", local_precompiled_dir.c_str(), i+1 );
    if2k_mini_compile_url( input_filename, output_filename, progress2 );
    if( progress1 )
      (*progress1)++;

    input_filename.form( "%s/%02dbadurl.txt", local_precompiled_dir.c_str(), i+1 );
    output_filename.form( "%s/%02dbadurl.txt.pre", local_precompiled_dir.c_str(), i+1 );
    if2k_mini_compile_url( input_filename, output_filename, progress2 );
    if( progress1 )
      (*progress1)++;

    input_filename.form( "%s/%02dpostbadurl.txt", local_precompiled_dir.c_str(), i+1 );
    output_filename.form( "%s/%02dpostbadurl.txt.pre", local_precompiled_dir.c_str(), i+1 );
    if2k_mini_compile_url( input_filename, output_filename, progress2 );
    if( progress1 )
      (*progress1)++;

    input_filename.form( "%s/%02dgoodnntp.txt", local_precompiled_dir.c_str(), i+1 );
    output_filename.form( "%s/%02dgoodnntp.txt.pre", local_precompiled_dir.c_str(), i+1 );
    if2k_mini_compile_alphanumeric( input_filename, output_filename, progress2 );
    if( progress1 )
      (*progress1)++;

    input_filename.form( "%s/%02dbadphr.txt", local_precompiled_dir.c_str(), i+1 );
    output_filename.form( "%s/%02dbadphr.txt.pre", local_precompiled_dir.c_str(), i+1 );
    if2k_mini_compile_alphanumeric( input_filename, output_filename, progress2 );
    if( progress1 )
      (*progress1)++;

    input_filename.form( "%s/%02dbadnntp.txt", local_precompiled_dir.c_str(), i+1 );
    output_filename.form( "%s/%02dbadnntp.txt.pre", local_precompiled_dir.c_str(), i+1 );
    if2k_mini_compile_alphanumeric( input_filename, output_filename, progress2 );
    if( progress1 )
      (*progress1)++;
  }

  jdk_log( JDK_LOG_INFO, "Completed precompiling of internet filter database" );
}

#endif
