#include "jdk_world.h"
#include "if2k_config.h"
#include "if2k_admin_textedit.h"
#include "if2k_admin.h"

jdk_html_chunk *
if2k_generator_textedit::create_html(	
  const char *fname,
  const jdk_http_request_header &request,
  const jdk_dynbuf &request_data,
  jdk_http_response_header &response,
  const jdk_string &connection_id
  )
{
  jdk_dynbuf text_file;
  text_file.append_from_file( filename.get() );
  
  jdk_html_chunk *form_guts=0;
  
  form_guts =
    style.tag( 
      "TEXTAREA", 
      "CLASS=\"textareabig1\"",
      style.text( text_file ),
      true
      );
  
  form_guts = 
    style.tag(
      "FORM",
      "ACTION=\"/admin-textedit-set.cgi\" method=\"POST\"",
      form_guts,
      true
      );
  
  
  return 
    style.doc( 
      style.head(
        0,
        style.body(
          "",
          style.header(
            style.center(
              style.font( 
                "size=\"+3\"",
                style.p(
                  style.unescaped_text(
                    title.get() )),
                style.tag(
                  "FORM",
                  "ACTION=\"/admin-textedit-set.cgi\" method=\"POST\"",
                  form_guts,
                  true)),
              style.footer())))));
}


jdk_html_chunk *
if2k_generator_textedit_set::create_html(	
  const char *fname,
  const jdk_http_request_header &request,
  const jdk_dynbuf &request_data,
  jdk_http_response_header &response,
  const jdk_string &connection_id
  )
{
  return 
    style.doc( 
      style.head(
        0,
        style.body(
          "",
          style.header(
            style.text( 
              "hello set", 
              style.footer())))));
}




