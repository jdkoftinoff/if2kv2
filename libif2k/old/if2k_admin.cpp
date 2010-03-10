#include "jdk_world.h"

#include "if2k_admin.h"
#include "jdk_crypt.h"

jdk_question if2k_administration_questions[] =
{
  { "Currently in demonstration mode - Some options are not settable", jdk_question::type_none, "http://www.internetfilter.com/help.html#demo", "", "", "mode_demo" },
  { "Registration", jdk_question::type_none, "http://www.internetfilter.com/help.html#registration", "", "", "" },
  { "", jdk_question::type_text, "Product serial #", "serial", "", "" },	
  
  { "", jdk_question::type_text_readonly, "License Validity", "license_valid", "", "" },
  { "", jdk_question::type_text_readonly, "Licensed to", "license_name", "", "" },
  { "", jdk_question::type_text_readonly, "email", "license_email", "", "" },
  { "", jdk_question::type_text_readonly, "Maximum number of clients", "license_clients", "", "mode_server" },
  
  { "Administration", jdk_question::type_none, "http://www.internetfilter.com/help.html#admin", "", "", "" },
#if IF2K_USE_ADMIN_NAME	
  { "", jdk_question::type_text,"Administrator username", "admin_username", "SIZE=20", "" },
#endif
  { "", jdk_question::type_text,"Administrator password", "admin_password", "SIZE=20", "" },	
  { "", jdk_question::type_yesno, "Expert administrator mode", "mode_expert", "", "mode_server" },
  { "", jdk_question::type_text, "IP Address and port to listen to for proxy (change requires manual restart)", "interface", "", "mode_server" },
  { "", jdk_question::type_text, "IP Address and port to listen to for admin (change requires manual restart)", "admin_interface", "", "mode_server" },
  { "", jdk_question::type_text, "Additional HTTP Proxy to use for all web accesses", "further_proxy", "class=\"textareawide\"", "" },
  { "", jdk_question::type_text, "Reload settings time in minutes", "remote_update_rate", "SIZE=10", "mode_expert" },
  { "", jdk_question::type_text, "Remote URL to load settings from", "remote_settings", "SIZE=40 class=\"textareawide\"", "mode_expert" },
  { "", jdk_question::type_text, "URL of parent server", "parent_server_url", "SIZE=40 class=\"textareawide\"", "" },
  
#if IF2K_ENABLE_LUA
  { "", jdk_question::type_text, "Special scripting command", "lua_command", "SIZE=40 class=\"textareawide\"", "mode_expert" },
#endif
#if 0
//	{ "", jdk_question::type_text, "Local file name used to cache remote settings", "cached_settings", "SIZE=40 class=\"textareawide\"", "mode_expert" },
  { "", jdk_question::type_yesno, "HTTP tunnel mode", "tunnel_mode", "", "mode_expert" },
#endif
  { "Client access", jdk_question::type_none, "http://www.internetfilter.com/help.html#clients", "", "", "mode_server" },
  { "", jdk_question::type_multiline,"Allowed client IP address ranges", "allowed_client_ips", "ROWS=4 COLS=40 class=\"textareawide\"", "mode_server" },
  { "", jdk_question::type_multiline,"Blocked client IP address ranges", "blocked_client_ips", "ROWS=4 COLS=40 class=\"textareawide\"", "mode_server" },
  { "", jdk_question::type_multiline,"Client IP address ranges that are not filtered", "nofilter_client_ips", "ROWS=4 COLS=40 class=\"textareawide\"", "mode_server" },
#if JDK_IS_WIN32
//	{ "Outgoing connection blocking for other protocols", jdk_question::type_none, "http://www.internetfilter.com/help.html#outgoing", "", "", "mode_expert" },
//	{ "", jdk_question::type_multiline,"Blocked server IP address ranges and ports", "blocked_server_ips", "ROWS=3 COLS=30 class=\"textareawide\"", "mode_expert" },
#endif	
  { "Block page configuration", jdk_question::type_none, "http://www.internetfilter.com/help.html#blockpage", "", "", "mode_expert"},
  { "", jdk_question::type_yesno,"Use internal block page", "use_internal_blocked_page", "", "mode_expert" },
  { "", jdk_question::type_text, "External block page URL", "blocked_page", "SIZE=40 class=\"textareawide\"", "mode_expert" },	
  { "", jdk_question::type_yesno, "Fancy block page via cgi. ", "fancy_blocked_page", "", "mode_expert" },		
  { "", jdk_question::type_text, "URL of image to use for internal block page", "blocked_page_image", "SIZE=40 class=\"textareawide\"", "mode_expert"},
  { "", jdk_question::type_text, "URL of link to use for internal block page", "blocked_page_link", "SIZE=40 class=\"textareawide\"", "mode_expert"},		
  { "", jdk_question::type_text, "Internal block page title", "blocked_page_title", "SIZE=40 class=\"textareawide\"", "mode_expert"},
  
  { "Blocking behaviour", jdk_question::type_none, "http://www.internetfilter.com/help.html#blocking", "", "", ""},
  { "", jdk_question::type_yesno,"Block all sites that are not known good", "block_unknown", "", "mode_ifonly" },
  { "", jdk_question::type_yesno,"Block bad sites", "block_bad", "", "" },
  { "", jdk_question::type_yesno,"Allow user to override block", "allow_override", "", "" },
  { "", jdk_question::type_yesno,"Only allow override on unknown sites", "override_unknown_only", "", "mode_ifonly" },
  { "", jdk_question::type_number,"Time that overrides last in minutes", "override_timeout", "SIZE=20", "" },
  { "", jdk_question::type_text,"IP address of client to override blocking now", "new_overrides", "SIZE=40 class=\"textareawide\"", "mode_server" },
  
  { "Logging to parent server", jdk_question::type_none, "http://www.internetfilter.com/help.html#serverlog", "", "", "" },
  { "", jdk_question::type_yesno, "Enable logging to parent server", "loggerC_enable", "", "" },		
  { "", jdk_question::type_yesno, "Log all urls accessed during override mode", "loggerC_log_override", "", "" },
  { "", jdk_question::type_yesno, "Log all accesses to unknown sites", "loggerC_level_0", "", "mode_ifonly" },
  { "", jdk_question::type_yesno, "Log all accesses to known good sites", "loggerC_level_1", "", "mode_ifonly" },
  { "", jdk_question::type_yesno, "Log all accesses to bad sites", "loggerC_level_2", "", "" },
  { "", jdk_question::type_yesno, "Ignore image files when logging", "loggerC_ignore_images", "", "" },
  { "", jdk_question::type_text, "URL to log to", "loggerC_url", "SIZE=80  class=\"textareawide\"", "" },	
  
  
  { "Logging to local file", jdk_question::type_none, "http://www.internetfilter.com/help.html#filelog", "", "", "" },
  { "", jdk_question::type_yesno, "Enable logging to local file", "loggerA_enable", "", "" },		
  { "", jdk_question::type_yesno, "Log all urls accessed during override mode", "loggerA_log_override", "", "" },
  { "", jdk_question::type_yesno, "Log all accesses to unknown sites", "loggerA_level_0", "", "mode_ifonly" },
  { "", jdk_question::type_yesno, "Log all accesses to known good sites", "loggerA_level_1", "", "mode_ifonly" },
  { "", jdk_question::type_yesno, "Log all accesses to bad sites", "loggerA_level_2", "", "" },
  { "", jdk_question::type_yesno, "Ignore image files when logging", "loggerA_ignore_images", "", "" },
  { "", jdk_question::type_text, "File name to log to", "loggerA_file", "SIZE=40  class=\"textareawide\"", "" },	
  
  { "Logging via e-mail", jdk_question::type_none, "http://www.internetfilter.com/help.html#emaillog", "", "", "" },	
  { "", jdk_question::type_yesno, "Enable logging via e-mail", "loggerB_enable", "", "" },	
  { "", jdk_question::type_yesno, "Log all urls accessed during override mode", "loggerB_log_override", "", "" },
  { "", jdk_question::type_yesno, "Log all accesses to unknown sites", "loggerB_level_0", "", "mode_ifonly" },
  { "", jdk_question::type_yesno, "Log all accesses to known good sites", "loggerB_level_1", "", "mode_ifonly" },
  { "", jdk_question::type_yesno, "Log all accesses to bad sites", "loggerB_level_2", "", "" },
  { "", jdk_question::type_yesno, "Ignore image files when logging", "loggerB_ignore_images", "", "" },
  
  { "", jdk_question::type_text, "SMTP Server to send email to", "loggerB_smtp_server", "SIZE=40 class=\"textareawide\"", "" },
  { "", jdk_question::type_text, "Domain name to use as identification for SMTP", "loggerB_smtp_helo", "SIZE=40 class=\"textareawide\"", "" },
  { "", jdk_question::type_text, "Send mail FROM", "loggerB_mail_from", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_text, "Send mail TO", "loggerB_mail_to", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_text, "Send mail SUBJECT", "loggerB_mail_subject", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_text, "Send mail time in minutes", "loggerB_mail_minutes", "SIZE=40 class=\"textareawide\"", "" },	
  
//    { "Blocking pattern configuration", jdk_question::type_none, "http://www.internetfilter.com/help.html#patterns", "", "", "" },
  { "Bad site lists", jdk_question::type_none, "http://www.internetfilter.com/help.html#badsites", "", "", "" 	},
  
  { "", jdk_question::type_yesno, "Use default ${bad_urls1_name}", "bad_urls1_default_enable", "", "" },
  { "", jdk_question::type_yesno, "Use default ${bad_urls2_name}", "bad_urls2_default_enable", "", "" },
  { "", jdk_question::type_yesno, "Use default ${bad_urls3_name}", "bad_urls3_default_enable", "", "" },
  { "", jdk_question::type_yesno, "Use default ${bad_urls4_name}", "bad_urls4_default_enable", "", "" },
  
  { "", jdk_question::type_text, "Local filename of additional bad site list", "bad_urls_file", "SIZE=40 class=\"textareawide\"", "mode_expert" },
  { "", jdk_question::type_yesno, "Use configurable bad site list", "bad_urls_file_enable", "", "" },
  { "", jdk_question::type_yesno, "Edit configurable bad site list", "mode_textedit_bad_urls", "", "" },
  { "", jdk_question::type_multiline_text_file, "Configurable bad site list", "${bad_urls_file}", "ROWS=15 SIZE=60 class=\"textareawide\"", "mode_textedit_bad_urls" },	
  { "", jdk_question::type_yesno, "Use remote additional bad site list", "bad_urls_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote additional bad site list", "bad_urls_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_multiline, "IP addresses to block", "bad_site_ips", "ROWS=4 COLS=40 class=\"textareawide\"", "mode_expert" }, 
  { "", jdk_question::type_yesno, "Use remote category 1 bad site list", "bad_urls1_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 1 bad site list", "bad_urls1_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 2 bad site list", "bad_urls2_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 2 bad site list", "bad_urls2_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 3 bad site list", "bad_urls3_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 3 bad site list", "bad_urls3_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 4 bad site list", "bad_urls4_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 4 bad site list", "bad_urls4_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 5 bad site list", "bad_urls5_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 5 bad site list", "bad_urls5_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 6 bad site list", "bad_urls6_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 6 bad site list", "bad_urls6_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 7 bad site list", "bad_urls7_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 7 bad site list", "bad_urls7_url", "SIZE=40 class=\"textareawide\"", "" },	
  
  { "Good site lists", jdk_question::type_none, "http://www.internetfilter.com/help.html#goodsites", "", "", "mode_ifonly" 	},	
  { "", jdk_question::type_yesno, "Use default good site list", "good_urls_default_enable", "", "mode_ifonly" },	
  { "", jdk_question::type_text, "Local filename of additional good site list", "good_urls_file", "SIZE=40 class=\"textareawide\"", "mode_ifonly" },	
  { "", jdk_question::type_yesno, "Use configurable good site list", "good_urls_file_enable", "", "" },
  { "", jdk_question::type_yesno, "Edit configurable good site list", "mode_textedit_good_urls", "", "" },
  { "", jdk_question::type_multiline_text_file, "Configurable good site list", "${good_urls_file}", "ROWS=15 SIZE=60 class=\"textareawide\"", "mode_textedit_good_urls" },
  
  { "", jdk_question::type_yesno, "Use remote additional good site list", "good_urls_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of additional good site list", "good_urls_url", "SIZE=40 class=\"textareawide\"", "mode_ifonly" },
  { "", jdk_question::type_multiline, "IP addresses to allow", "good_site_ips", "ROWS=4 COLS=40 class=\"textareawide\"", "mode_expert" },
#if JDK_IS_WIN32	
  { "", jdk_question::type_multiline, "IP addresses to allow directly without using additional proxy (requires reboot)", "redir_allow", "ROWS=4 COLS=40 class=\"textareawide\"", "mode_expert" },
  { "", jdk_question::type_multiline, "IP addresses and ports to block (requires reboot)", "redir_block", "ROWS=4 COLS=40 class=\"textareawide\"", "mode_expert" },
  { "", jdk_question::type_yesno, "Block filesharing applications (requires reboot)", "redir_blockfileshare", "", "mode_expert" },
#endif
  { "Bad phrase lists", jdk_question::type_none, "http://www.internetfilter.com/help.html#badphrases", "", "", "" 	},		
  
  { "", jdk_question::type_yesno, "Block default ${bad_phrases1_name}", "bad_phrases1_default_enable", "", "" }, 
  { "", jdk_question::type_yesno, "Block default ${bad_phrases2_name}", "bad_phrases2_default_enable", "", "" },		
  { "", jdk_question::type_yesno, "Block default ${bad_phrases3_name}", "bad_phrases3_default_enable", "", "" },		
  { "", jdk_question::type_yesno, "Block default ${bad_phrases4_name}", "bad_phrases4_default_enable", "", "" },		
  
  { "", jdk_question::type_text, "Local filename of additional bad phrases", "bad_phrases_file", "SIZE=40 class=\"textareawide\"", "mode_expert" },	
  { "", jdk_question::type_yesno, "Use configurable bad phrases", "bad_phrases_file_enable", "", "" },
  { "", jdk_question::type_yesno, "Edit configurable bad phrases", "mode_textedit_phrases", "", "" },
  { "", jdk_question::type_multiline_text_file, "Configurable bad phrases", "${bad_phrases_file}", "ROWS=15 SIZE=60 class=\"textareawide\"", "mode_textedit_phrases" },
  { "", jdk_question::type_yesno, "Use remote additional bad phrase list", "bad_phrases_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of additional bad phrase list", "bad_phrases_url", "SIZE=40 class=\"textareawide\"", "" },
  { "", jdk_question::type_yesno, "Use remote category 1 bad phrase list", "bad_phrases1_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 1 bad phrase list", "bad_phrases1_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 2 bad phrase list", "bad_phrases2_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 2 bad phrase list", "bad_phrases2_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 3 bad phrase list", "bad_phrases3_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 3 bad phrase list", "bad_phrases3_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 4 bad phrase list", "bad_phrases4_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 4 bad phrase list", "bad_phrases4_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 5 bad phrase list", "bad_phrases5_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 5 bad phrase list", "bad_phrases5_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 6 bad phrase list", "bad_phrases6_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 6 bad phrase list", "bad_phrases6_url", "SIZE=40 class=\"textareawide\"", "" },	
  { "", jdk_question::type_yesno, "Use remote category 7 bad phrase list", "bad_phrases7_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of remote category 7 bad phrase list", "bad_phrases7_url", "SIZE=40 class=\"textareawide\"", "" },	
  
  
  { "Censor phrase lists", jdk_question::type_none, "http://www.internetfilter.com/help.html#censorphrases", "", "", "" 	},
  
  { "", jdk_question::type_yesno, "Block default ${censor_phrases1_name}", "censor_phrases1_default_enable", "", "" }, 
  { "", jdk_question::type_yesno, "Block default ${censor_phrases2_name}", "censor_phrases2_default_enable", "", "" },		
  { "", jdk_question::type_yesno, "Block default ${censor_phrases3_name}", "censor_phrases3_default_enable", "", "" },		
  { "", jdk_question::type_yesno, "Block default ${censor_phrases4_name}", "censor_phrases4_default_enable", "", "" },		
  
  { "", jdk_question::type_text, "Local filename of additional censor phrases", "censor_phrases_file", "SIZE=40 class=\"textareawide\"", "mode_expert" },
  { "", jdk_question::type_yesno, "Use configurable censor phrases", "censor_phrases_file_enable", "", "" },
  { "", jdk_question::type_yesno, "Edit configurable censor phrases", "mode_textedit_censorphrases", "", "" },
  { "", jdk_question::type_multiline_text_file, "Configurable censor phrases", "${censor_phrases_file}", "ROWS=15 SIZE=60 class=\"textareawide\"", "mode_textedit_censorphrases" },
  { "", jdk_question::type_yesno, "Use remote additional bad phrase list", "censor_phrases_url_enable", "", "" },
  { "", jdk_question::type_text, "URL of additional bad phrase list", "censor_phrases_url", "SIZE=40 class=\"textareawide\"", "" },
  
  
#if IF2K_USE_ADMIN_NAME		
  { "", jdk_question::type_hidden, "", "cgi_username", "", ""},
#endif	
  { "", jdk_question::type_hidden, "", "cgi_password", "", ""},		
  { "", jdk_question::type_hidden, "", "cgi_sid", "", "" },		
  { 0 }
};

const char * if2k_html_style::my_stylesheet =
".textarea1 { width:75% }\r\n"
".textareawide { width:90% }\r\n"
".textareabig { width:90%; height:30% }\r\n"
".textareabig1 { width:90%; height:60% }\r\n";

if2k_html_style::~if2k_html_style()
{
}

jdk_html_chunk *if2k_html_style::head( jdk_html_chunk *contents, jdk_html_chunk *next ) const
{
  return 
    jdk_html_style_simple::head(
      tag( 
        "STYLE", "TYPE=\"text/css\"",
        unescaped_text(
          "<--\r\n",
          unescaped_text( 
            my_stylesheet,
            unescaped_text(
              text_css.get(),
              unescaped_text(
                "\r\n-->")))),
        true, 0 ),
      next);
}

jdk_html_chunk *if2k_html_style::body(  const char *options, jdk_html_chunk *contents, jdk_html_chunk *next ) const
{
  if( options && *options )
    return tag( "BODY", options, font( text_font.get().c_str(), contents, 0), true, next );
  else
    return tag( "BODY", body_options.get().c_str(), font( text_font.get().c_str(), contents, 0), true, next );
}

jdk_html_chunk *if2k_html_style::navigation_bar( jdk_html_chunk *next ) const
{
//	jdk_string_url help_link;
//	help_link.form( "http://%s/help.html", internal_hostname.get().c_str() );
//	jdk_string_url faq_link;
//	faq_link.form( "http://%s/faq.html", internal_hostname.get().c_str() );
  
  return
    product_links.get() ?
    center(
      p(
        font("face=\"Arial\" size=\"+1\"",
             link( "http://www.internetfilter.com/corporate.html",
                   unescaped_text("company&nbsp;info"),
                   unescaped_text("&nbsp;&nbsp;&nbsp;",
                                  link( "http://www.internetfilter.com/products.html",
                                        unescaped_text("products"),
					unescaped_text("&nbsp;&nbsp;&nbsp;",							
                                                       link( "http://www.internetfilter.com/help.html",
                                                             unescaped_text("help"),
                                                             unescaped_text("&nbsp;&nbsp;&nbsp;",							
                                                                            link( "http://www.internetfilter.com/faq.html",
                                                                                  unescaped_text("faq's"),
                                                                                  unescaped_text("&nbsp;&nbsp;&nbsp;",							
                                                                                                 link( "mailto:jeffk@internetfilter.com",
                                                                                                       unescaped_text("contact&nbsp;tech"),
                                                                                                       unescaped_text("&nbsp;&nbsp;&nbsp;",							
                                                                                                                      link( "mailto:turner@internetfilter.com",
                                                                                                                            unescaped_text("contact&nbsp;sales"),
                                                                                                                            unescaped_text("&nbsp;&nbsp;&nbsp;",							
                                                                                                                                           link( "http://www.internetfilter.com/index.html",
                                                                                                                                                 unescaped_text("home"),
                                                                                                                                                 0)))))))))))))),
        0),
      next)
    :
    next;
}

jdk_html_chunk *if2k_html_style::header( jdk_html_chunk *next ) const
{
  jdk_string_url fixed_logo_url = product_logo.get();
  if( fixed_logo_url.get(0)=='/' )
  {
    fixed_logo_url.prepend( internal_hostname.c_str() );
    fixed_logo_url.prepend( "http://" );
  }
  else
  {
    fixed_logo_url = product_logo.get();
  }
  
  jdk_str<512> product_title_and_version = product_title.get();
  product_title_and_version.cat( " " );
  product_title_and_version.cat( IF2K_VERSION_STRING );
#if 1
  return
    center(
      navigation_bar(
        hr(
          "width=80%",
          table( 
            "BORDER=0 ROWS=1 COLS=2 WIDTH=80%",
            table_row(
              "",
              table_cell(
                "WIDTH=50% ALIGN=CENTER",
                image_link(
                  fixed_logo_url.c_str(),
                  product_homepage.get().c_str(),
                  0,0),
                table_cell(
                  "WIDTH=50% ALIGN=CENTER",
                  font( 
                    "size=+4",
                    italic(
                      unescaped_text( product_title_and_version ))),0)),0),
            hr( 
              "width=80%",
              0 )))),
      next);
  
#else
  return
    center(
      navigation_bar(
        hr(
          "width=80%",
          p(
            image_link(
              fixed_logo_url.c_str(),
              product_homepage.get().c_str(),
              0,0),
            p(
              font( "size=+4",
                    italic(
                      unescaped_text( product_title.get() ))),
              hr(
                "width=60%",
                0))))), 
      next );
#endif
}

jdk_html_chunk *if2k_html_style::footer( jdk_html_chunk *next ) const
{
  return hr("width=80%",navigation_bar(next));
}



jdk_html_chunk *if2k_blockpage_document_generator::generate_head( jdk_html_chunk *next )
{
  return style.head( style.title( style.unescaped_text( settings.get("blocked_page_title") ) ), next );
}

jdk_html_chunk *if2k_blockpage_document_generator::generate_violation_info( jdk_html_chunk *next )	  
{
  if( global_settings->get_long("license_valid")!=1 )
  {
    return style.center(
      style.p(
        style.unescaped_text(
          text_block_invalid.get().c_str() ),
        style.p(
          style.link(
            loginlink.get().c_str(), 
            style.unescaped_text(loginmsg.get().c_str())))));							
  }
  else	  		
    if( cgiparams.find("url" )>=0 && cgiparams.find("reason")>=0 && cgiparams.find("match")>=0 && cgiparams.find("ref")>=0 )
    {
      const jdk_string &violation_url = cgiparams.get("url");
      const jdk_string &violation_reason = cgiparams.get("reason");
      
      jdk_str<256> text_reason;
      if( violation_reason.ncmp("1",1)==0 )
      {
        text_reason = settings.get("text_block_reason1");
      }
      else if( violation_reason.ncmp("2",1)==0 )
      {
        text_reason = settings.get("text_block_reason2");
      }
      else if( violation_reason.ncmp("3",1)==0 )
      {
        text_reason = settings.get("text_block_reason3");
        text_reason.cat( " '" );
        text_reason.cat( cgiparams.get("match") );
        text_reason.cat( "'" );	
      }						
      else if( violation_reason.ncmp("4",1)==0 )
      {
        text_reason = settings.get("text_block_reason4");
        text_reason.cat( " '" );
        text_reason.cat( cgiparams.get("match") );
        text_reason.cat( "'" );					
      }						
      
      
      return 
        style.p( 
          style.unescaped_text(block_info1.get().c_str(),0),
          style.p(
            style.unescaped_text(violation_url,0),
            style.p(
              style.unescaped_text(block_info2.get().c_str(),
                                   style.unescaped_text( text_reason,0)),
              style.p(
                style.text(block_info3.get().c_str(),
                           next)))));
    }
    else
    {
      return style.center( style.p( style.unescaped_text( settings.get("blocked_page_title") )),next);
    }				
}

jdk_html_chunk *if2k_blockpage_document_generator::generate_admin_section( jdk_html_chunk *next )
{
  bool override_is_option=false;
  
  if( settings.get_long("allow_override") && cgiparams.find("url")>=0 && cgiparams.find("reason")>=0 )
  {
    if( settings.get_long("override_unknown_only")==0 )
    {
      override_is_option = true;
    }
    else
    {
      int reason = cgiparams.get_long("reason");
      
      if( reason!=1 && reason!=3 )
      {
        override_is_option = true;
      }
    }
  }
  
  
  if( override_is_option )
  {
    jdk_string_url escaped_bad_url;
    jdk_cgi_escape_with_amp(cgiparams.get("url"),escaped_bad_url);
//        int override_timeout = settings.get_long("override_timeout");
//        jdk_str<1024> line1;
//        line1.form( "You may temporarily override this block for the next %d minute%c by  ", override_timeout, override_timeout==1 ? ' ' : 's' );
//        line1.form( , override_timeout, override_timeout==1 ? ' ' : 's' );
    
    jdk_string_url line2;
    line2.form("%surl=%s", override_url.c_str(), escaped_bad_url.c_str());
    
    return style.center(
      style.p(
        style.unescaped_text(
          text_block_override.get().c_str(),
          style.link(
            line2.c_str(),
            style.unescaped_text(text_block_override_click.get().c_str())))),
      style.p(
        style.unescaped_text(
          text_block_override_warn.get().c_str()),
        
        settings.get_long("enable_login")==1 ?
        
        style.p(
          style.link(
            loginlink.get().c_str(), 
            style.unescaped_text(loginmsg.get().c_str())),
          next)
        : next ));
  }
  else
    return settings.get_long("enable_login")==1 ?
      style.center(
        style.p(
          style.link(
            loginlink.get().c_str(), 
            style.unescaped_text(loginmsg.get().c_str())),
          next))
      :
      next;
  
}	


jdk_html_chunk *if2k_blockpage_document_generator::generate_content( jdk_html_chunk *next )
{		
  return 
    style.center(
      generate_violation_info(
        generate_admin_section(
          next)));
}

jdk_html_chunk *if2k_blockpage_document_generator::generate_header( jdk_html_chunk *next )
{
  return style.center( 
    style.p( 
      style.image_link( 
        settings.get("blocked_page_image").c_str(), 
        settings.get("blocked_page_link").c_str(),0
        ) ) 
    , next );
}

jdk_html_chunk *if2k_blockpage_document_generator::generate_footer( jdk_html_chunk *next )
{
  return next;
}


jdk_html_chunk *
if2k_http_generator_blockpage::create_html(	
  const char *fname,
  const jdk_http_request_header &request,
  const jdk_dynbuf &request_data,
  jdk_http_response_header &response,
  const jdk_string &connection_id
  )
{
  jdk_settings_text cgiinfo;	
  jdk_cgi_loadsettings( &cgiinfo, fname );
  if2k_blockpage_document_generator gen(style,cgiinfo,request,settings,override_url.get() );
  
  return gen.generate_document();	
}

bool if2k_http_generator_getsettings::handle_request(
  const jdk_http_request_header &request,
  const jdk_dynbuf &request_data,
  jdk_http_response_header &response,
  jdk_dynbuf &response_data,
  const jdk_string &connection_id
  )
{
  response.clear();
  response_data.clear();
  
  jdk_str<4096> cgi_name(request.get_url().path);
  
  //jdk_log_debug4( "is it override? path=%s expecting %s", request.get_url().path.c_str(), prefix.c_str() );
  
  if( cgi_name.nicmp( prefix, prefix.len() )==0 )
  {
    jdk_log_debug2( "generating settings file for client" );	
    
    jdk_settings_text response_text;
    jdk_settings_encrypt<IF2K_ENCRYPTOR>( ENCRYPTION_KEY, settings, response_text );
//		response_text.merge(settings);
    
    response_text.erase( "block_unknown" );
    response_text.erase( "block_bad" );		
    response_text.erase( "allow_override" );
    response_text.erase( "override_unknown_only" );		
    response_text.erase( "if2k_version" );
    response_text.erase( "admin_password" );
    response_text.erase( "secret_password" );
    response_text.erase( "interface" );
    response_text.erase( "admin_interface" );
    response_text.erase( "cached_settings" );
    response_text.erase( "remote_settings" );
    response_text.erase( "main_server_url" );		
    response_text.erase( "home" );
    response_text.erase( "listen_count" );		
    response_text.erase( "name" );
    response_text.erase( "listen" );				
    response_text.erase( "pid_dir" );
    response_text.erase( "threads" );		
    response_text.erase( "tunnel_mode" );		
    response_text.erase( "daemon" );
    response_text.erase( "log_file" );
    response_text.erase( "setuid" );
    response_text.erase( "uid" );
    response_text.erase( "fork" );
    response_text.erase( "use_fork" );
    response_text.erase( "mode_ifonly" );
    response_text.erase( "mode_expert" );
    response_text.erase( "mode_server" );
    response_text.erase( "mode_demo" );
    response_text.erase( "serial" );
    response_text.erase( "license_valid" );
    response_text.erase( "license_name" );
    response_text.erase( "license_clients");
    response_text.erase( "license_email");
    response_text.erase( "license_expires");
    response_text.erase( "license_validated_serial");
    
    response_text.save_buf(&response_data);
    
    
    
    response.set_http_version(jdk_str<16>("HTTP/1.0"));
    response.set_http_response_code( 200 );
    response.set_http_response_text( jdk_str<8>("OK") );
    response.add_entry(	"Content-Type:", "text/plain; charset=UTF-8" );		
    response.add_entry(	"Pragma:", "no-cache" );					
    response.add_entry(	"Cache-Control:", "no-cache" );					
    response.add_entry( "Expires:", "-1" );
    
    response.set_valid();
    return true;
  }
  
  return false;
}

bool if2k_http_generator_getfile::handle_request(
  const jdk_http_request_header &request,
  const jdk_dynbuf &request_data,
  jdk_http_response_header &response,
  jdk_dynbuf &response_data,
  const jdk_string &connection_id
  )
{
  response.clear();
  response_data.clear();
  
  jdk_str<4096> cgi_name(request.get_url().path);
  
  //jdk_log_debug4( "is it override? path=%s expecting %s", request.get_url().path.c_str(), prefix.c_str() );
  
  if( cgi_name.nicmp( prefix, prefix.len() )==0 )
  {
    
    jdk_settings_text postinfo;
    jdk_cgi_loadsettings( &postinfo, cgi_name.c_str()+prefix.len() );
    if( postinfo.find("key" )>=0 )
    {
      jdk_log_debug2( "generating db file for client: %s", postinfo.get("key").c_str() );	
      
      response_data.append_from_file( settings.get( postinfo.get("key") ) );
      
      response.set_http_version(jdk_str<16>("HTTP/1.0"));
      response.set_http_response_code( 200 );
      response.set_http_response_text( jdk_str<8>("OK") );
      response.add_entry(	"Content-Type:", "text/plain; charset=UTF-8" );		
      response.add_entry(	"Pragma:", "no-cache" );					
      response.add_entry(	"Cache-Control:", "no-cache" );					
      response.add_entry( "Expires:", "-1" );
      
      response.set_valid();
      return true;
    }
  }
  
  return false;
}


bool if2k_http_generator_override::handle_request(
  const jdk_http_request_header &request,
  const jdk_dynbuf &request_data,
  jdk_http_response_header &response,
  jdk_dynbuf &response_data,
  const jdk_string &connection_id
  )
{		
  response.clear();
  response_data.clear();
  
  jdk_str<4096> cgi_name(request.get_url().path);
  
  //jdk_log_debug4( "is it override? path=%s expecting %s", request.get_url().path.c_str(), prefix.c_str() );
  
  if( cgi_name.nicmp( prefix, prefix.len() )==0 && settings.get_long("allow_override")==1 )
  {
    //jdk_html_chunk *chunk=0;
    jdk_settings_text postinfo;
    jdk_log_debug2( "generating /override/, about to get url" );	
    jdk_cgi_loadsettings( &postinfo, cgi_name.c_str()+prefix.len() );
    
    if( postinfo.find("url" )>=0 )
    {            
      override_map->add_override( connection_id, jdk_get_time_seconds() + settings.get_long("override_timeout")*60 );
      
      response.set_http_version(jdk_str<16>("HTTP/1.0"));
      response.set_http_response_code( 302 );
      response.set_http_response_text( jdk_str<8>("MOVED") );			
      response.set_valid();
      
      jdk_string_url original_url = postinfo.get("url");
      jdk_string_url escaped_url;
      jdk_cgi_escape(original_url,escaped_url);
      
      response.add_entry( "Location:", escaped_url.c_str() );
      jdk_log_debug2( "generating /override/, doing override and redirect to %s", original_url.c_str() );	
      
      return true;
    }
    else
    {
      return false;
    }
  }
  
  return false;
}




jdk_html_chunk *if2k_adminlogin_document_generator::generate_head( jdk_html_chunk *next )
{
  return style.head( style.title( style.unescaped_text( product_name.get().c_str() ) ), next );
}



jdk_html_chunk *if2k_adminlogin_document_generator::generate_content( jdk_html_chunk *next )
{		
  jdk_str<128> form_options;
  form_options.form("action=\"%s\" method=\"POST\"",action_url.c_str());
  jdk_str<256> form_sid;
  form_sid.form("type=\"HIDDEN\" name=\"cgi_sid\" value=\"%lu\"", global_session_id );
  
#if IF2K_USE_ADMIN_NAME	
  return 
    style.center(
      style.tag( "FORM", form_options.c_str(),
                 style.table( "", 
                              style.table_row( "",
                                               style.table_cell( "",
                                                                 style.unescaped_text( text_login_username.get() ),
                                                                 style.table_cell( "",
                                                                                   style.tag( "INPUT", "type=\"TEXT\" name=\"cgi_username\" value=\"\"",0,false))),
                                               style.table_row( "",
                                                                style.table_cell( "",
                                                                                  style.unescaped_text( text_login_password.get() ),
                                                                                  style.table_cell( "",
                                                                                                    style.tag( "INPUT", "type=\"PASSWORD\" name=\"cgi_password\" value=\"\"",0,false))),
                                                                style.table_row( "",
                                                                                 style.table_cell( "COLSPAN=2 ALIGN=RIGHT",
                                                                                                   style.tag( "INPUT", "type=\"SUBMIT\" name=\"Submit\" value=\"OK\"",0,false)))),
                                               style.tag( "INPUT", form_sid.c_str(), 0, false )))),
      next);
#else
  return 
    style.center(
      style.tag( "FORM", form_options.c_str(),
                 style.table( "", 
                              style.table_row( "",
                                               style.table_cell( "",
                                                                 style.unescaped_text( text_login_password.get() ),
                                                                 style.table_cell( "",
                                                                                   style.tag( "INPUT", "type=\"PASSWORD\" name=\"cgi_password\" value=\"\"",0,false))),
                                               style.table_row( "",
                                                                style.table_cell( "COLSPAN=2 ALIGN=RIGHT",
                                                                                  style.tag( "INPUT", "type=\"SUBMIT\" name=\"Submit\" value=\"OK\"",0,false)))),
                              style.tag( "INPUT", form_sid.c_str(), 0, false ))),
      next);
  
#endif
}

jdk_html_chunk *if2k_adminlogin_document_generator::generate_header( jdk_html_chunk *next )
{
  return style.header(
    style.center(
      style.p(
        style.font("size=+2",
                   style.italic(
                     style.unescaped_text(text_login_title.get().c_str())))),
      next));
}

jdk_html_chunk *if2k_adminlogin_document_generator::generate_footer( jdk_html_chunk *next )
{
  return style.footer(next);
}




jdk_html_chunk *
if2k_http_generator_adminlogin::create_html(	
  const char *fname,
  const jdk_http_request_header &request,
  const jdk_dynbuf &request_data,
  jdk_http_response_header &response,
  const jdk_string &connection_id
  )
{
  if( settings.get_long("enable_login")==1 )
  {
    jdk_settings_text cgiinfo;
    //jdk_cgi_loadsettings( &cgiinfo, fname );
    if2k_adminlogin_document_generator gen(style,cgiinfo,request,settings,action_url);
    return gen.generate_document();
  }
  else
  {
    return 0;
  }
}


jdk_html_chunk *if2k_questions_asker::generate_head( jdk_html_chunk *next )
{
  return 
    style.head( 
      style.title( 
        style.unescaped_text(text_title.get())),
      next );
}

jdk_html_chunk *if2k_questions_asker::generate_header( jdk_html_chunk *next )
{
  return style.header(
    style.center(
      style.p(
        style.font( "size=+2",
                    style.italic(
                      style.unescaped_text(text_header.get())))),
      next));
}

jdk_html_chunk *if2k_questions_asker::generate_footer( jdk_html_chunk *next )
{
  return style.footer(next);
}


jdk_http_server_post_validator::validation_response 
if2k_http_administration::post_validate(
  const jdk_html_style &style, 
  const jdk_settings_text &postinfo,
  const jdk_http_request_header &request,
  const jdk_string &connection_id
  )
{
  if( 
#if IF2K_USE_ADMIN_NAME
    postinfo.find("cgi_username")!=-1 &&
#endif
    postinfo.find("cgi_password")!=-1 
    && postinfo.find("cgi_sid")!=-1
    && postinfo.get_ulong("cgi_sid")==global_session_id
    )
  {		
#if IF2K_USE_ADMIN_NAME		
    if( (postinfo.get("cgi_username").cmp(settings.get("admin_username"))==0 &&
         postinfo.get("cgi_password").cmp(settings.get("admin_password"))==0)
        || (postinfo.get("cgi_username").cmp(settings.get("secret_username"))==0 &&
            postinfo.get("cgi_password").cmp(settings.get("secret_password"))==0 ))
#else
      if( (postinfo.get("cgi_password").cmp(settings.get("admin_password"))==0)
          || (postinfo.get("cgi_password").cmp(settings.get("secret_password"))==0) )
#endif			
      {			    
        jdk_settings_text defaults;
        defaults.merge( *global_settings );			   
        defaults.merge( postinfo );
        
        // generate a new session id now and stick it in the hidden field
        global_session_id = rand();
        defaults.set_ulong("cgi_sid", global_session_id );
        
        if2k_questions_asker gen(
          style,
          postinfo,
          request,
          if2k_administration_questions,
          defaults,
          submit_url,
          settings.get("text_filter_settings_ok"),
          settings
          );
        return validation_response( gen.generate_document(), true );
      }
      else
      {
        return 
          validation_response(
            style.doc( 
              style.body("",
                         style.header(
                           style.center(
                             style.heading1(
                               style.unescaped_text(text_login_reject.get())),
                             style.center(
                               style.p(
                                 style.link( login_url.get().c_str(),
                                             style.unescaped_text( text_loginmsg.get() ))),
                               style.footer()))))),
            true);				
      }		
  }
  else
    return validation_response( 
      style.doc( 
        style.header(
          style.body("",
                     style.center(
                       style.heading1(
                         style.unescaped_text(
                           text_login_error.get())),
                       style.center(
                         style.p(
                           style.link( login_url.get().c_str(),
                                       style.unescaped_text( text_loginmsg.get() ))),
                         style.footer()))))),			    
      false);
}

jdk_html_chunk *
if2k_http_generator_adminset::create_html(	
  const char *fname,
  const jdk_http_request_header &request,
  const jdk_dynbuf &request_data,
  jdk_http_response_header &response,
  const jdk_string &connection_id
  )
{
  jdk_settings_text postinfo;	
  jdk_cgi_loadsettings( &postinfo, request_data );
  
#if 0
  {
    jdk_settings_text::pair_t * p;
    int count = postinfo.count();
    jdk_log_debug3( "count=%d", count );
    for( int i=0; i<count; ++i )
    {
      p=postinfo.get(i);
      if( p )
        jdk_log_debug3( "p%02d: %s=%s", i, p->key.c_str(), p->value.c_str() );
    }
  }
#endif	
  unsigned long old_session_id = global_session_id;
  // create a new session id for next login
  global_session_id = rand();
  
#if IF2K_USE_ADMIN_NAME
  if( 
    ( postinfo.get_ulong("cgi_sid")==old_session_id)
    &&
    ((postinfo.get("cgi_username").cmp(settings.get("admin_username"))==0 &&
      postinfo.get("cgi_password").cmp(settings.get("admin_password"))==0)
     
     || (postinfo.get("cgi_username").cmp(settings.get("secret_username"))==0 &&
         postinfo.get("cgi_password").cmp(settings.get("secret_password"))==0 )))
#else
    unsigned long cgi_sid = postinfo.get_ulong("cgi_sid");
  
  if(
    ( cgi_sid==old_session_id)
    &&
    ((postinfo.get("cgi_password").cmp(settings.get("admin_password"))==0 )
     || (postinfo.get("cgi_password").cmp(settings.get("secret_password"))==0 )))
#endif
  {	
    
    if( global_settings )
    {
      jdk_log( JDK_LOG_NOTICE, "Administrator options changed - reloading settings" );
      jdk_synchronized( global_settings->get_mutex() );
      
      // go through all setting keys searching for ones that start with $. The values
      // for these are to be stored in a file specified by the setting by that name
      
      for( int i=0; i<postinfo.count(); ++i )
      {
        const jdk_setting *s = postinfo.get( i );
        
        if( s )
        {
          if( s->key.get(0)=='$' )
          {
            // found one!
            jdk_log( JDK_LOG_DEBUG1, "Found ${} post key %s", s->key.c_str() );
            jdk_string_filename file_to_write;
            //if( postinfo.find(s->key)!=-1 )
            {
              //	postinfo.expand_string( s->key, file_to_write );
            }
            //else
            {
              global_settings->expand_string( s->key, file_to_write );
            }
            
            jdk_log( JDK_LOG_DEBUG1, "Found %s expands to '%s'", s->key.c_str(), file_to_write.c_str() );
            
            if( !file_to_write.is_clear() )
            {
              FILE *f=fopen( file_to_write.c_str(), "wb" );
              if(f)
              {
                if( s->value.len()==0 || fwrite( s->value.c_str(), s->value.len(), 1, f )==1 )
                {
                  jdk_log( JDK_LOG_DEBUG1, "File '%s' rewritten by administrator", file_to_write.c_str() );
                }
                else
                {
                  jdk_log( JDK_LOG_ERROR, "Error writing file '%s'", file_to_write.c_str() );			
                }
                fclose(f);
              }
              else
              {
                jdk_log( JDK_LOG_ERROR, "Error opening file '%s'", file_to_write.c_str() );	
              }
            }
            postinfo.erase(i);
          }
          else
          {
            jdk_log( JDK_LOG_DEBUG4, "POST key '%s' value '%s'", s->key.c_str(), s->value.c_str() );					
          }
        }
      }
      
      
      global_settings->merge( postinfo );
#if IF2K_USE_ADMIN_NAME			
      global_settings->erase( "cgi_username" );
#endif			
      global_settings->erase( "cgi_password" );
      global_settings->erase( "cgi_sid" );
      global_settings->erase( "Submit" );
      
      
      if( global_settings->find("new_overrides")!=-1 )
      {					
        const jdk_string &new_overrides = global_settings->get( "new_overrides" );
        long override_timeout = global_settings->get_long( "override_timeout" );
        
//					jdk_log( JDK_LOG_INFO, "Adding override for %s for %d min.", new_overrides.c_str(), override_timeout );
        override_map->add_override( 
          new_overrides,
          override_timeout*60 + jdk_get_time_seconds()
          );
        
        global_settings->erase( "new_overrides" );					
      }
      
      {
        jdk_synchronized1( global_settings->get_mutex() );
        save_global_settings();			
      }
      
      load_global_settings();
      
      if( global_shared )
        global_shared->update();
      if( global_kernel )
        global_kernel->update();		   			
    }
    
    if( global_settings->get_long("license_valid")==0 || global_settings->get_long("mode_demo")==1 )
    {
      return 
        style.doc(
          style.body("",
                     style.header(
                       style.center(
                         style.p(
                           style.font( "size=+2",									
                                       style.unescaped_text(text_set_demo.get())),
                           style.p(
                             style.link( login_url.get().c_str(),
                                         style.unescaped_text( text_loginmsg.get() )))))), 
                     style.footer()));
    }
    else 
    {
      return 
        style.doc(
          style.body("",
                     style.header(
                       style.center(
                         style.p(
                           style.font( "size=+2",
                                       style.unescaped_text(text_set_good.get())),
                           style.p(
                             style.link( login_url.get().c_str(),
                                         style.unescaped_text( text_loginmsg.get() )))))),
                     style.footer()));
    }
    
  }
  else
  {
    return 
      style.doc( 
        style.body("",
                   style.header(
                     style.center(
                       style.p(
                         style.font( "size=+2",
                                     style.unescaped_text("Incorrect username or password or sessione error.")),
                         style.p(
                           style.link( login_url.get().c_str(),
                                       style.unescaped_text( "Click here to try again")))))),
                   style.footer()));
  }				
}



