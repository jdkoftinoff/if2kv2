#ifndef __IF2_KERNEL_H

#define __IF2_KERNEL_H

#include "jdk_settings.h"
#include "if2k_logger.h"
#include "if2k_scanner.h"


class if2_kernel_base
{
public:
	if2_kernel_base() 
	{	
	}
	
	virtual ~if2_kernel_base()
	{
	}   

	virtual if2_multi_logger &get_logger() = 0;
	
	virtual void update() = 0;
	

	virtual if2_kernel_result verify_url( 
						   const char *client_address,
						   const char *url,
						   jdk_string &match_phrase,
						   bool override=false,
						   bool override_unknown_only=false
						   ) = 0;


	virtual if2_kernel_result verify_post_data(
								 const char *client_address,
								 const char *url,
								 const char *data,
								 int data_len,
								 jdk_string &match_phrase,
								 bool override=false,
        						 bool override_unknown_only=false
								 ) = 0;
	
	
	virtual if2_kernel_result verify_received_data(
									 const char *client_address,
									 const char *url,
									 const char *data,
									 int data_len,
									 jdk_string &match_phrase,
									 bool override=false,									 
           						     bool override_unknown_only=false
									 ) =0;

	
	virtual if2_kernel_result verify_newsgroup_name(
									 const char *name_string,
								 	 int name_len
									) = 0;

	
	virtual if2_kernel_result verify_incoming_email(
									 const char *data,
									 int data_len
									) = 0;
	
};

template <class tree_traits>
class if2_kernel_standard_event  : public if2_scanner_event<tree_traits>
{
public:
	if2_kernel_standard_event()
  :
    best_match_item(),
    largest_match_value()
	{
		best_match_item = -1;
		largest_match_value = -1;
	}
		
	
	virtual void operator () ( const jdk_tree<tree_traits> &tree, typename tree_traits::index_t match_item )
	{
		typename tree_traits::flags_t value = tree.get(match_item).flags;
		
		if(value > largest_match_value )
		{
			best_match_item = match_item;
			largest_match_value = value;
		}
	}	
	
	typename tree_traits::index_t best_match_item;
	typename tree_traits::flags_t largest_match_value;
};


class if2_kernel_standard : public if2_kernel_base
{
    if2_kernel_standard( const if2_kernel_standard & );
    const if2_kernel_standard operator = ( const if2_kernel_standard & );
    
public:
   if2_kernel_standard( const jdk_settings &settings_ );
	~if2_kernel_standard()
	{
	    delete good_urls;
	    delete bad_urls;
	    delete bad_phrases;
		delete censor_phrases;
		delete good_newsgroups;
		delete bad_newsgroups;
		delete bad_email_content;
	}
	
   
	virtual if2_multi_logger &get_logger()
	{
		return logger;	
	}

	virtual void update();		
	
	virtual if2_kernel_result verify_url(
						   const char *client_address,
						   const char *url,
						   jdk_string &match_phrase,
						   bool override,
						   bool override_unknown_only
						   );

	virtual if2_kernel_result verify_post_data(
								 const char *client_address,
								 const char *url,
								 const char *data,
						       	 int data_len,
								 jdk_string &match_phrase,
        						 bool override,
        					     bool override_unknown_only
								 );
	
	virtual if2_kernel_result verify_received_data(
									 const char *client_address,
									 const char *url,
									 const char *data,
									 int data_len,
									 jdk_string &match_phrase,
          						     bool override,
              						 bool override_unknown_only
									 );
		
	virtual  if2_kernel_result verify_newsgroup_name(
									 const char *name_string,
								 	 int name_len
									)
	{
		return if2_kernel_result(0,true);
	}

	
	virtual  if2_kernel_result verify_incoming_email(
									 const char *data,
									 int data_len
									)
	{
		return if2_kernel_result(0,true);

	}


private:
	const jdk_settings &settings;
	if2_multi_logger logger;	
    if2_pattern_expander_standard expander;				  
	jdk_live_long block_unknown;
	jdk_live_long block_bad;
	if2_scanner<if2_tree_traits_url> *good_urls;
	if2_scanner<if2_tree_traits_url> *bad_urls;	
	if2_scanner<if2_tree_traits_alphanumeric> *bad_phrases;
	if2_scanner<if2_tree_traits_alphanumeric> *censor_phrases;
	if2_scanner<if2_tree_traits_alphanumeric> *good_newsgroups;
	if2_scanner<if2_tree_traits_alphanumeric> *bad_newsgroups;
	if2_scanner<if2_tree_traits_alphanumeric> *bad_email_content;
public:
	jdk_recursivemutex mutex;	
};

#endif
