#include "jdk_world.h"
#include "jdk_html.h"

jdk_html_chunk *jdk_html_generate_form( 
  const jdk_html_style &style,
  const char *form_table_options,
  const jdk_question *question_list, 
  const jdk_settings &defaults,
  const jdk_string &action_url,											
  const jdk_string &submit_button_name,
  jdk_html_chunk *next 
  )
{
  const jdk_question *cur_question=question_list;
  jdk_html_chunk *form_guts = style.text("");
  
  while( cur_question && cur_question->title )
  {
    jdk_html_chunk *question_guts = style.text("");
    bool question_disabled=false;
    
    // check to see if this question is to be disabled based on the value of an existing setting
    if( cur_question->enabled_on_field && *cur_question->enabled_on_field)
    {	        
      if( defaults.find( cur_question->enabled_on_field )==-1 ||
          defaults.get_long( cur_question->enabled_on_field)==0 )
      {
        question_disabled=true;
      }
    }	   	    
    
    if( !question_disabled )
    {
      jdk_str<4096> default_value;
      
      // get the current value of the field
      
      if( cur_question->field && *cur_question->field )
      {
        int pos = defaults.find(cur_question->field );
        if( pos!=-1 )
        {
          default_value.cpy(defaults.get(pos)->value);
        }
      }
      
      // is it a text entry field?
      if( cur_question->type==jdk_question::type_text)
      {
        // set the type of html form item to text.
        
        jdk_str<512> field_options;
        field_options.form( "type=\"TEXT\" name=\"%s\" value=\"%s\" %s",
                            cur_question->field, 
                            default_value.c_str(), 
                            cur_question->multichoices 
          );
        
        question_guts->set_next( 
          style.tag(
            "INPUT", field_options.c_str(),0,false));
      }
      else if( cur_question->type==jdk_question::type_text_readonly)
      {
        // it is a non-editable text field. it is not really a form item at all.
        question_guts->set_next( style.text(default_value) );
      }
      else if( cur_question->type==jdk_question::type_yesno )
      {
#if 1			
        // it is a yes or no radio button choice.
        jdk_str<512> field_options;
        field_options.form( "type=\"RADIO\" name=\"%s\" value=\"1\" %s",
                            cur_question->field, 
                            default_value.cmp("1")==0 ? "CHECKED" : ""
          );
        
        question_guts->set_next( 
          style.tag(
            "INPUT", field_options.c_str(), 0, false,
            style.unescaped_text(" Yes &nbsp;&nbsp;&nbsp ")));
        
        field_options.form( "type=\"RADIO\" name=\"%s\" value=\"0\" %s",
                            cur_question->field, 
                            default_value.cmp("1")!=0 ? "CHECKED" : ""
          );
        
        question_guts->set_next( 
          style.tag(
            "INPUT", field_options.c_str(), 0, false,
            style.unescaped_text( "No ")));
        
#else
        // it is a yes or no check box
        jdk_str<512> field_options;
        field_options.form( "type=\"CHECKBOX\" name=\"%s\" value=\"1\" %s",
                            cur_question->field, 
                            default_value.cmp("1")==0 ? "CHECKED" : ""
          );
        
        question_guts->set_next( 
          style.tag(
            "INPUT", field_options.c_str(), 0, false
            ));
        
#endif			
      }
      else if( cur_question->type==jdk_question::type_password )
      {
        // it is a password field
        jdk_str<512> field_options;
        field_options.form( "type=\"PASSWORD\" name=\"%s\" value=\"%s\" %s",
                            cur_question->field, 
                            default_value.c_str(), 
                            cur_question->multichoices 
          );
        question_guts->set_next( 
          style.tag(
            "INPUT", field_options.c_str(),0,false));
      }
      else if( cur_question->type==jdk_question::type_number )
      {
        jdk_str<512> field_options;
        field_options.form( "type=\"TEXT\" name=\"%s\" value=\"%s\" %s",
                            cur_question->field, 
                            default_value.c_str(), 
                            cur_question->multichoices 
          );
        question_guts->set_next( 
          style.tag(
            "INPUT", field_options.c_str(),0,false));
      }
      else if( cur_question->type==jdk_question::type_path )
      {
        jdk_str<512> field_options;
        field_options.form( "type=\"TEXT\" name=\"%s\" value=\"%s\" %s",
                            cur_question->field, 
                            default_value.c_str(), 
                            cur_question->multichoices 
          );
        question_guts->set_next( 
          style.tag(
            "INPUT", field_options.c_str(),0,false));
      }
      else if( cur_question->type==jdk_question::type_multiline )
      {
        
        jdk_str<512> field_options;
        field_options.form( "type=\"TEXT\" %s name=\"%s\" %s", 
                            cur_question->multichoices, 
                            cur_question->field, 
                            cur_question->multichoices
          );
        question_guts->set_next( 
          style.tag(
            "TEXTAREA", field_options.c_str(), 
            style.text( default_value.c_str() ) ));
      }
      else if( cur_question->type==jdk_question::type_multiline_text_file )
      {
        // this is a text file that we must read.
        
        jdk_str<512> field_options;
        field_options.form( "type=\"TEXT\" %s name=\"%s\" %s", 
                            cur_question->multichoices, 
                            cur_question->field, 
                            cur_question->multichoices
          );
        // default_value is the file name to edit. It may have ${xxx} expansions in it.
        
        jdk_string_filename file_to_edit;
        defaults.expand_string( cur_question->field, file_to_edit );
        jdk_log_debug4( "html questions: file_to_edit is '%s'", file_to_edit.c_str() );
        // read the file into a dynbuf
        jdk_dynbuf text_file;
        text_file.append_from_file( file_to_edit );
        
        question_guts->set_next( 
          style.tag(
            "TEXTAREA", field_options.c_str(), 
            style.text( text_file ) ));
      }			
      else if( cur_question->type==jdk_question::type_multiplechoice )
      {			
        jdk_str<512> field_options;
        field_options.form( "name=\"%s\"", cur_question->field );
        
        jdk_html_chunk *select_guts=0;
        
        jdk_str<512> option_name;
        const jdk_string &cur_value = default_value;
        size_t len=strlen( cur_question->multichoices );
        
        for( size_t i=0; i<len+1; ++i )
        {
          if( cur_question->multichoices[i]=='\r' || 
              cur_question->multichoices[i]=='\n' || 
              cur_question->multichoices[i]=='\0' )
          {				    
            jdk_str<64> flags;
            if( option_name.icmp( cur_value )==0 )
              flags.cpy( "SELECTED" );
            jdk_html_chunk *c = style.tag( "OPTION", flags.c_str(), style.text(option_name.c_str()), false );
            if( !select_guts )
              select_guts = c;
            else
              select_guts->set_next(c);
            option_name.clear();
          }
          else
          {
            option_name.cat(  cur_question->multichoices[i] );
          }
        }
        
        question_guts->set_next( 
          style.tag(
            "SELECT", field_options.c_str(), 
            select_guts ));
      }
      
      if( cur_question->type==jdk_question::type_hidden  )
      {
        jdk_str<512> field_options;
        field_options.form( "type=\"HIDDEN\" name=\"%s\" value=\"%s\"",
                            cur_question->field, 
                            default_value.c_str() 
          );
        question_guts->set_next( 
          style.tag(
            "INPUT", field_options.c_str(),0,false));			
      }
      
      // is this entry really just a title?
      if( cur_question->title && *cur_question->title )
      {
        // is the a link we have to link the title to?
        if( cur_question->question ) // question is a help link
        {
          form_guts->set_next(
            style.table_row( "",
                             style.tag( "TH", "COLSPAN=2",
                                        style.br( "", 
                                                  style.hr( "", 
                                                            style.p(
                                                              style.link( 
                                                                cur_question->question,											
                                                                style.text( cur_question->title )), 
                                                              style.hr()))))));					
        }
        else
        {
          // just put the title in.
          form_guts->set_next(
            style.table_row( "",
                             style.tag( "TH", "COLSPAN=2",
                                        style.br( "", style.hr( "", style.p( style.text( cur_question->title ), style.hr()))))));
        }
      }			    
      
      if( cur_question->type==jdk_question::type_none || cur_question->type==jdk_question::type_hidden )
      {		
        form_guts->set_next( question_guts );
      }
      else
      {
        // expand any variable substitutions within the question itself
        jdk_str<4096> expanded_question;
        defaults.expand_string( cur_question->question, expanded_question );
        
        jdk_html_chunk *question_row =
          style.table_row( "",
                           style.table_cell( "ALIGN=LEFT VALIGN=TOP WIDTH=40%", 
                                             style.text( expanded_question ),
                                             style.table_cell( "ALIGN=LEFT VALIGN=TOP WIDTH=60%", question_guts )));
        
        form_guts->set_next( question_row );
      }
    }
    else
    {
      form_guts->set_next( question_guts );
    }
    
    cur_question++;
  }
  form_guts = style.table( form_table_options, form_guts );
  
  jdk_str<512> submit_button_options;
  submit_button_options.form("type=\"SUBMIT\" name=\"Submit\" value=\"%s\"",submit_button_name.c_str());
  
  form_guts->set_next(
    style.p(
      style.tag( "INPUT", 
                 submit_button_options.c_str(),
                 0, false)));
  
  jdk_str<1024> form_options;
  form_options.form( "ACTION=\"%s\" method=\"POST\"",action_url.c_str() );  
  return style.tag("FORM",form_options,form_guts,true,
                   next
    );
}

