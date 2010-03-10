#ifndef _IF2KKERNEL_INTERNAL_H
#define _IF2KKERNEL_INTERNAL_H

#include "jdk_world.h"
#include "if2k_mini_compile.h"
#include "if2k_mini_scanner.h"
#include "if2k_mini_config.h"


typedef unsigned int (*IF2K_MATCH_PROC)( 
  void * h, 
  const char * match, 
  unsigned int match_len, 
  unsigned int match_type, 
  const char *match_pos 
  );

template <class tree_traits>
class if2k_tree_event_to_callback : public if2k_mini_tree_event<tree_traits>
{
public:

	if2k_tree_event_to_callback(IF2K_MATCH_PROC cb_, void *h_)
    : cb( cb_ ), h( h_ )
    {
    }
	
	virtual ~if2k_tree_event_to_callback()
    {
    }
	
	virtual void operator () ( const jdk_tree<tree_traits> &tree, typename tree_traits::index_t match_item )
  {
    if( cb )
    {
      cb( h, "", 0, 0, 0 );
    }
  }
private:
  IF2K_MATCH_PROC cb;
  void *h;
};



template <class tree_traits_mmap, class tree_traits>
class if2k_scanner
{
public:
  typedef if2k_pattern_expander_standard pattern_expander_t;

  if2k_scanner( 
    const jdk_string &precompiled_file_name, 
    const jdk_string &noncompiled_file_name,
    unsigned int extra_value_
    )
    :
    precompiled_tree( precompiled_file_name ),
    noncompiled_tree(),
    extra_value( extra_value_ )
  {
    if( !noncompiled_file_name.is_clear() )
    {
      noncompiled_tree.clear();

      pattern_expander_t expander;
      jdk_string_filename prefix("");

      expander.load(
        noncompiled_file_name,
        noncompiled_tree,
        0,
        prefix
        );    
      noncompiled_tree.make_shortcuts();
    }
  }

  virtual ~if2k_scanner()
  {
  }

  unsigned int extra() const
  {
    return extra_value;
  }

  unsigned int scan(
    const char *data,
    unsigned int data_len,
    IF2K_MATCH_PROC callback
    )
  {
    bool found=false;
    {
        if2k_tree_event_to_callback<tree_traits_mmap> mmap_event(callback,this);
        found |= precompiled_tree.find( (const void *)data, data_len, mmap_event );
    }
    {
      if2k_tree_event_to_callback<tree_traits> event(callback,this);  
      found |= noncompiled_tree.find( (const void *)data, data_len, event );
    }
    return found;
  }

  unsigned int censor(
    const char *data,
    unsigned int data_len,
    IF2K_MATCH_PROC callback,
    char replacement
    )
  {
    bool found=false;
    {
        if2k_tree_event_to_callback<tree_traits_mmap> mmap_event(callback,this);
        found |= precompiled_tree.censor( (void *)data, data_len, mmap_event, replacement );
    }
    {
      if2k_tree_event_to_callback<tree_traits> event(callback,this);  
      found |= noncompiled_tree.censor( (void *)data, data_len, event, replacement );
    }
    return found;
  }


private:
  if2k_mini_tree<tree_traits_mmap> precompiled_tree;
  if2k_mini_tree<tree_traits> noncompiled_tree;
  unsigned int extra_value;
};


typedef if2k_scanner<if2k_mini_tree_traits_url_mmap,if2k_mini_tree_traits_url> if2k_url_scanner;
typedef if2k_scanner<if2k_mini_tree_traits_alphanumeric_mmap,if2k_mini_tree_traits_alphanumeric> if2k_phrase_scanner;

#endif
