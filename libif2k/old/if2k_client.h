#ifndef _IF2K_CLIENT_H
#define _IF2K_CLIENT_H

#include "if2k_config.h"
#include "jdk_string.h"
#include "jdk_util.h"
#include "jdk_map.h"
#include "jdk_settings.h"
#include "jdk_cgisettings.h"
#include "jdk_dynbuf.h"

class if2k_client_settings
{
public:
	enum 
	{
		list_goodurls,
		list_badurls,
		list_badphrases,
		list_goodnews,
		list_badnews,
		list_censoredphrases,
		num_lists
	};
	
	enum
	{
		num_categories=8
	};
	
	if2k_client_settings() : settings()
	{
		int j;
		
		name_base[list_goodurls][0].cpy( "good_urls_" );
		
		for( j=1; j<num_categories; ++j )
		{
			name_base[list_goodurls][j].form("good_urls%d_", j);
		}
		name_base[list_badurls][0].cpy( "bad_urls_" );
		for( j=1; j<num_categories; ++j )
		{
			name_base[list_badurls][j].form("bad_urls%d_", j);
		}
		name_base[list_badphrases][0].cpy( "bad_phrases_" );
		for( j=1; j<num_categories; ++j )
		{
			name_base[list_badphrases][j].form("bad_phrases%d_", j);
		}
		name_base[list_goodnews][0].cpy( "good_news_" );
		for( j=1; j<num_categories; ++j )
		{
			name_base[list_goodnews][j].form("good_news%d_", j);
		}
		name_base[list_badnews][0].cpy( "bad_news_" );
		for( j=1; j<num_categories; ++j )
		{
			name_base[list_badnews][j].form("bad_news%d_", j);
		}
		name_base[list_censoredphrases][0].cpy( "censored_phrases_" );
		for( j=1; j<num_categories; ++j )
		{
			name_base[list_censoredphrases][j].form("censored_phrases%d_", j);
		}		
	}
	
	virtual ~if2k_client_settings()
	{
	}
	
	virtual void clear() 
	{
		settings.clear();
		for( int i=0; i<num_lists; ++i )
		{
			for( int j=0; j<num_categories; ++j )
			{
				list[i][j].clear();
			}
		}
	}
	
	const jdk_string &get_list_category_title( int lnum, int cat_num ) const
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("name");
		
		return settings.get(field);
	}
	
	void set_list_category_title( int lnum, int cat_num, const jdk_string &s )
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("name");
		settings.set(field,s);
	}


	const jdk_string &get_list_category_file( int lnum, int cat_num )  const
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("file");
		
		return settings.get(field);
	}
	
	void set_list_category_file( int lnum, int cat_num, const jdk_string &s )
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("file");
		settings.set(field,s);
	}
		
	bool get_list_category_file_enable( int lnum, int cat_num )  const
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("file_enable");
		return bool(settings.get_long(field));
	}

	void set_list_category_file_enable( int lnum, int cat_num, bool en )
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("file_enable");
		settings.set_long(field,(long)en);
	}

	bool get_list_category_default_enable( int lnum, int cat_num )  const
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("default_enable");
		return bool(settings.get_long(field));
	}

	void set_list_category_default_enable( int lnum, int cat_num, bool en )
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("default_enable");
		settings.set_long(field,(long)en);
	}

	const jdk_string &get_list_category_url( int lnum, int cat_num )  const
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("url");
		
		return settings.get(field);
	}
	
	void set_list_category_url( int lnum, int cat_num, const jdk_string &s )
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("url");
		settings.set(field,s);
	}


	bool get_list_category_url_enable( int lnum, int cat_num )  const
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("url_enable");
		return bool(settings.get_long(field));
	}

	void set_list_category_url_enable( int lnum, int cat_num, bool en )
	{
		jdk_str<256> field = name_base[lnum][cat_num];
		field.cat("url_enable");
		settings.set_long(field,(long)en);
	}
	
	jdk_settings_text settings;
	
	jdk_dynbuf list[num_lists][num_categories];
	jdk_str<128> name_base[num_lists][num_categories];
	
	// todo: add title strings .. ie: "Bad Urls 1 - Pornography"
};

class if2k_client
{
public:
	if2k_client();
	virtual ~if2k_client();

	virtual void set_connection(
							 const jdk_string &server_ip_,
							 const jdk_string &proxy_ip_,
							 const jdk_string &pass_ 
							 );

	virtual bool load_settings( 
		if2k_client_settings &settings
		);

	virtual bool save_settings(
		const if2k_client_settings &settings
		);

	virtual bool apply_setting(
		const jdk_settings_text &map
		);

	virtual bool trigger_reload();

	virtual bool add_script( 
		const jdk_buf &script
		);


	virtual bool execute_script(
						  const char *request_type,
						  const jdk_str<128> &script,
						  const jdk_settings_text &params,
						  const jdk_buf &additional,
						  jdk_buf &response
						  );

private:						  
	jdk_string_url proxy_ip;
	jdk_string_url server_ip;
	jdk_string_url pass;
};


#endif




