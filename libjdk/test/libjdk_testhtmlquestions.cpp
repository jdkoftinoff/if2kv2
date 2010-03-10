#include "jdk_world.h"
#include "jdk_daemon.h"
#include "jdk_log.h"
#include "jdk_socket.h"
#include "jdk_cgisettings.h"
#include "jdk_questions.h"
#include "jdk_server_family.h"
#include "jdk_html.h"
#include "jdk_http.h"
#include "jdk_http_server.h"



class my_first_validator : public jdk_http_server_post_validator
{
public:
  my_first_validator()
  {
  }
  
  ~my_first_validator()
  {
  }
  
  validation_response post_validate(
    const jdk_html_style &style, 
    const jdk_settings_text &postinfo,
    const jdk_http_request_header &request,
    const jdk_string &connection_id
    )
  {
    if( postinfo.find("Submit")!=-1 )
    {
      if( postinfo.get("Submit").icmp("OK")==0 )
      {
        // TODO: check values here
        return validation_response(
          style.doc( 
            style.body("",
                       style.heading1(
                         style.text("Thank you for the information"))))
          ,true);
        
      }
    }
    return validation_response( 
      style.doc( 
        style.body("",
                   style.heading1(
                     style.text("Validation error"))))					     				    
      ,false);
  }
  
};

class my_first_post_event : public jdk_http_server_post_event
{
public:
  my_first_post_event( jdk_settings_text &cur_values_ )
    : cur_values( cur_values_ )
  {
  }
  
  bool post_event( const jdk_settings_text &postinfo, const jdk_string &connection_id )
  {
    cur_values.merge( postinfo );
    cur_values.save_file(stdout);
    return true;
  }
  
private:
  jdk_settings_text &cur_values;
};


jdk_setting_description question_defaults[] =
{
  { "name", "", "" },
  { "age", "", "" },
  { "pass", "abcd", "" },
  { "like_chicken", "1", "" },
  { "like_beef", "0", "" },
  { "like_chicken_fried", "0", "" },
  { "like_beef_roasted", "0", "" },
  { "hidden_field", "xzyyz", "" },
  { "user_address", "", "" },
  { "icecream", "Vanilla", "" },
  { 0,0,0 }
};

jdk_question questions[] =
{
  { "Welcome to the question tester", jdk_question::type_none, "", "", "", ""},
  { "First Step", jdk_question::type_text, "What is your name?", "name", "", ""},
  { "", jdk_question::type_number, "How old are you?", "age", "", "", false, 1,200 },
  { "", jdk_question::type_password, "Give me a secret word:", "pass", "", "" },
  { "", jdk_question::type_yesno, "Do you like chicken?", "like_chicken", "", "" },
  { "", jdk_question::type_yesno, "Do you like beef?", "like_beef", "", "" },
  { "Fifth step", jdk_question::type_yesno, "Do you like fried chicken?", "like_chicken_fried", "", "like_chicken" },	
  { "Sixth step", jdk_question::type_yesno, "Do you like roast beef?", "like_beef_roasted", "", "like_beef" },		
  { "", jdk_question::type_multiline, "Type your address here", "user_address", "ROWS=10 COLS=40", "" },
  { "", jdk_question::type_multiplechoice, "Favourite ice cream", "icecream", "Vanilla\nChocolate\nStrawberry\nNeopolitan\nCherry", "" },
  { "", jdk_question::type_hidden, "", "hidden_field", "", ""},			
  { 0 }
};


class admin_server_child : public jdk_server_child
{
public:
  admin_server_child( const jdk_string &bind_addr, int max_requests_ )
    : generators(2),
      worker( generators ),
      server_sock( bind_addr ),
      max_requests( max_requests_ ),
      defaults( question_defaults )
  {	   	
    generators.add( 
      new jdk_http_server_generator_html_single_questions
      <jdk_questions_asker_html_generator_simple>(
        "/questions",
        new jdk_html_style_simple_black,
        questions,
        defaults,
        jdk_str<64>("response"), 
        jdk_str<64>("OK") 
        )
      );
    
    generators.add(
      new jdk_http_server_generator_html_post(
        "/response", 
        new jdk_html_style_simple_black, 
        new my_first_validator, 
        new my_first_post_event(defaults)
        )
      );
    
  }
  
  void update()
  {
    jdk_log( JDK_LOG_INFO, "admin_server_child::update" );
  }
  
  bool run()
  {		
    if( server_sock.accept(&client_sock) )
    {
      worker.handle_request(client_sock);
      client_sock.close();
      return --max_requests!=0;
    }
    else
    {
      return false;	
    }
  }			
  
private:
  jdk_http_server_generator_list generators;
  jdk_http_server_worker worker;
  jdk_inet_server_socket server_sock;
  jdk_inet_client_socket client_sock;
  int max_requests;
  jdk_settings_text defaults;
};


jdk_setting_description setting_descriptions[] =
{
  {"listen_count", "10", "TCP/IP listen queue size."},
  {"interface","127.0.0.1:8002", "IP address to bind to (0=ALL)."},
  {"use_further_proxy","0", "Enable connections to additional proxy." },
  {"further_proxy","", "Additional proxy URL." },
  {"log_type", "2", "Logging type: 0=none, 1=syslog, 2=stderr, 3=file." },
  {"log_file", "", "Logging file name." },
  {"log_detail", "8", "Logging detail level: 0=Errors, 1=Warnings, 2=Notices, 3=Info, 4-8=Debug" },
  
  {"cached_settings", "", "local file to hold remote settings" },
  {"remote_settings", "", "URL of remote settings" },
  {"username", "", "username for remote settings" },	
  {"password", "", "password for remote settings" },
  {"remote_connect_error_file", "", "URL of remote connect_error_file" },
  {"remote_bad_request_file", "", "URL of remote bad_request_file" },
  {"remote_redirect_file", "", "URL of remote_redirect_file" },
  {"remote_update_rate", "3600", "update rate in seconds" },
#if JDK_IS_UNIX     
  {"setuid", "0", "1 to set UID/GID on startup." },
  {"uid", "0", "UID to use." },
  {"gid", "0", "GID to use." },
  {"daemon", "0", "Become a background process." },
  {"pid_dir", "", "Directory to put PID file in: Blank=none." },     
  {"home", ".", "Home directory." },
  {"name", "libjdk_testhtmlquestions", "Name to use for logging." },          
#endif     	 
  {0,0}
};


int main( int argc, char **argv )
{
  jdk_settings_text settings( setting_descriptions, argc, argv, 0 );
#if 0	
  if( jdk_daemonize(
        (bool)settings.get_long("daemon"),
        settings.get("name").c_str(),
        settings.get("home").c_str(),
        settings.get("pid_dir").c_str() ) <0
    )
  {
    return 1;
  }   
#endif
  jdk_log_setup(
    settings.get_long( "log_type" ),
    settings.get( "log_file" ),
    settings.get_long( "log_detail" )
    );				  
  
  admin_server_child admin_child( settings.get("interface"), -1 );
  
  while( admin_child.run() );
  
  return 0;    
}


