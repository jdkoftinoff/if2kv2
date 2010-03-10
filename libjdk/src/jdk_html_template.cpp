
#include "jdk_world.h"
#include "jdk_html_template.h"
#include "jdk_string.h"
#include "jdk_util.h"
#include "jdk_cgi.h"

bool jdk_html_template( 
  jdk_dynbuf &result, 
  const jdk_buf &src, 
  const jdk_settings *vars1, 
  const jdk_settings *vars2 
  )
{
  const char *src_p = (const char *)src.get_data();
  size_t len = src.get_data_length();

  result.clear( len+4096 );

  bool in_script_region=false;
  bool in_dollar_sign=false;

  jdk_str<256> cur_var_name;
  char prev_c = 0;
  char c = 0;

  for( size_t i=0; i<len; ++i )
  {
    prev_c = c;
    c = *src_p++;
    
    if( !in_script_region )
    {
      // we are in normal html section

      if( prev_c=='<' && c=='?' )
      {
        // a '<?' in html section marks the beginning of script area
        in_script_region = true;
        continue;
      }
      else if( prev_c=='<' && c!='?' )
      {
        // any other code prefixed by '<' is straight html and is copied to the result
        result.append_from_data( &prev_c, 1 );
        result.append_from_data( &c, 1 );
        continue;
      }
      if( c=='<' )
      {
        // a '<' character is ignored until we see the next character
        continue;
      }      
    }

    
    if( in_script_region )
    {
      // we are in the script region
      if( prev_c=='?' && c=='>' )
      {
        // any '?>' code means to go back in html mode
        in_script_region=false;
        continue;
      }

      if( in_dollar_sign )
      {
        // we are in dollar sign mode, so we are building up or using a variable name
        if( jdk_isalnum(c) || c=='.' )
        {
          // alphanumeric characters and period are valid variable name characters
          cur_var_name.cat(c);
          continue;
        }
        else if( cur_var_name.len()>0 )
        {
          // We have a fully specified variable name, now we need to look it up
          bool found=false;
          if( vars2 )
          {
            // first, look it up in the vars2 table
            int entry = vars2->find( cur_var_name );
            if( entry>=0 )
            {
              // found it! get it and convert it to html and stick it in our output
              jdk_str<4096> value;
              jdk_html_amp_escape_text( vars2->get( entry )->value, value );
              result.append_from_string( value );
              found=true;
            }
          }
          
          if( !found && vars1 )
          {
            // not found in vars2, look in vars1 
            int entry = vars1->find( cur_var_name );
            if( entry>=0 )
            {
              // found it in vars1! get it and convert it to html and stick it in our output
              jdk_str<4096> value;
              jdk_html_amp_escape_text( vars1->get( entry )->value, value );
              result.append_from_string( value );
              found = true;
            }
          }
          
          // A dollarsign is special in script mode
          if( c!='$' )
            in_dollar_sign = false;
          cur_var_name.clear();
        }
      }
      if( c=='$' )
      {
        in_dollar_sign = true;
        continue;
      }
    }
    else
    {
      // the current character is not interesting and therefore just copied to the result
      result.append_from_data( &c, 1 );
    }
  }

  return true;
}
