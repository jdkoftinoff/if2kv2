#include "jdk_world.h"
#include "jdk_questions.h"
#include "jdk_error.h"

jdk_questions_asker::user_action 
jdk_questions_asker::ask_questions()
{
  int cur_question=0;
  user_action action=action_ok;
  result.merge( defaults );
  
  while(question_list[cur_question].title && action!=action_done && action!=action_cancel )
  {
    bool is_last_question=false;
    if( !question_list[cur_question+1].title )
    {
      is_last_question=true;	
    }		
    bool is_first_question= (cur_question==0);
    
    const jdk_question *cur = &question_list[cur_question];
    if( cur->enabled_on_field && strlen( cur->enabled_on_field )>0 )
    {
      if( result.find( cur->enabled_on_field) != -1 )
      {
        if( result.get_long( cur->enabled_on_field )!=1 )
        {
          cur_question++;
          continue;
        }				
      }			
    }
    
    
    switch( cur->type )
    {
    case jdk_question::type_none:
    {				
      action=ask_none(cur,is_first_question,is_last_question);
      break;
    }			
    case jdk_question::type_yesno:
    {
      action=ask_yesno(cur,is_first_question,is_last_question);
      break;				
    }		
    case jdk_question::type_text:
    {
      action=ask_text(cur,is_first_question,is_first_question);
      break;
    }
    case jdk_question::type_password:
    {
      action=ask_password(cur,is_first_question,is_first_question);
      break;
    }			
    case jdk_question::type_multiline:
    {
      action=ask_multiline(cur,is_first_question,is_first_question);
      break;
    }			
    case jdk_question::type_path:
    {
      action=ask_path(cur,is_first_question,is_first_question);
      break;
    }			
    case jdk_question::type_number:
    {
      action=ask_number(cur,is_first_question,is_first_question);
      break;
    }			
    case jdk_question::type_multiplechoice:
    {
      action=ask_multiplechoice(cur,is_first_question,is_first_question);
      break;				
    }			
    default:
    {
      JDK_THROW_ERROR( "unknown question type", "" );
      break;	
    }			
    }
    
    switch( action )
    {
    case action_ok:
    {				
      cur_question++;
      break;
    }
    case action_done:
    {
      // handled in while
      break;		
    }
    case action_cancel:
    {
      // handled in while
      break;
    }
    case action_next:
    {
      if( !is_last_question )
        cur_question++;
      break;
    }
    case action_previous:
    {
      if( !is_first_question )
        cur_question--;
      break;
    }
    default:
    {
      JDK_THROW_ERROR( "unknown question action", "" );
      break;
    }			
    }				
  }
  
  if( action==action_ok )
  {
    action=action_done;	
  }
  
  return action;
}

jdk_questions_asker_text::jdk_questions_asker_text(
  const jdk_question *question_list_,
  const jdk_settings &defaults_,
  jdk_settings &result_
  )
  : jdk_questions_asker( question_list_, defaults_, result_ )
{		
}

jdk_questions_asker_text::~jdk_questions_asker_text()
{
}


jdk_questions_asker::user_action 
jdk_questions_asker_text::ask_yesno(const jdk_question *q,bool first,bool last)
{
  jdk_str<128> s;
  user_action action = action_cancel;
  
  while(!feof(stdin))
  {		
    fprintf( stdout, "%s\n%s\n", q->title, q->question );
    fflush(stdout);
    if( jdk_read_string_line( &s, stdin ) )
    {
      if( s.is_clear() )
      {
        s=defaults.get(q->field);
        action=action_ok;
      }			
      else if( s.nicmp("y",1)==0 )
      {
        s.cpy("1");
        action=action_ok;								
      }
      else if( s.nicmp("n",1)==0 )
      {
        s.cpy("0");
        action=action_ok;								
      }
      else if( s.cmp("0")==0 )
      {
        action=action_ok;				
      }
      else if( s.cmp("1")==0 )
      {
        action=action_ok;				
      }
      
      if( action==action_ok )
      {
        result.set( q->field, s.c_str() );
        break;
      }
      
    }
  }
  return action;
}

jdk_questions_asker::user_action 
jdk_questions_asker_text::ask_none(const jdk_question *q,bool first,bool last)
{
  jdk_str<128> s;
  user_action action = action_cancel;
  
  while(!feof(stdin))
  {		
    fprintf( stdout, "%s\n%s\nPress enter to continue.\n", q->title, q->question );
    if( jdk_read_string_line( &s, stdin ) )
    {
      action=action_ok;
      break;
    }
  }
  return action;
}

jdk_questions_asker::user_action 
jdk_questions_asker_text::ask_text(const jdk_question *q,bool first,bool last)
{
  jdk_str<128> s;
  user_action action = action_cancel;
  
  while(!feof(stdin))
  {		
    fprintf( stdout, "%s\n%s\n", q->title, q->question );
    fflush(stdout);
    if( jdk_read_string_line( &s, stdin ) )
    {
      action=action_ok;
      
      if( action==action_ok )
      {
        result.set( q->field, s.c_str() );
        break;
      }
      
    }
  }
  return action;
}

jdk_questions_asker::user_action 
jdk_questions_asker_text::ask_multiplechoice(const jdk_question *q,bool first,bool last)
{
  jdk_str<128> s;
  user_action action = action_cancel;
  
  while(!feof(stdin))
  {		
    fprintf( stdout, "%s\n%s\nchoices:\n%s:\n", q->title, q->question, q->multichoices );
    fflush(stdout);
    if( jdk_read_string_line( &s, stdin ) )
    {
      action=action_ok;		
      if( action==action_ok )
      {
        result.set( q->field, s.c_str() );
        break;
      }
      
    }
  }
  return action;
}



jdk_questions_asker::user_action
jdk_questions_asker_text::ask_password(const jdk_question *q,bool first,bool last)
{
  jdk_str<128> s;
  user_action action = action_cancel;
  
  while(!feof(stdin))
  {		
    fprintf( stdout, "%s\n%s\n", q->title, q->question );
    fflush(stdout);
    if( jdk_read_string_line( &s, stdin ) )
    {
      action=action_ok;			
      if( action==action_ok )
      {
        result.set( q->field, s.c_str() );
        break;
      }
      
    }
  }
  return action;
}

jdk_questions_asker::user_action
jdk_questions_asker_text::ask_multiline(const jdk_question *q,bool first,bool last)
{
  jdk_str<128> s;
  user_action action = action_cancel;
  
  while(!feof(stdin))
  {		
    fprintf( stdout, "%s\n%s\n", q->title, q->question );
    fflush(stdout);
    if( jdk_read_string_line( &s, stdin ) )
    {
      action=action_ok;			
      if( action==action_ok )
      {
        result.set( q->field, s.c_str() );
        break;
      }
      
    }
  }
  return action;
}


jdk_questions_asker::user_action
jdk_questions_asker_text::ask_path(const jdk_question *q,bool first,bool last)
{
  jdk_str<128> s;
  user_action action = action_cancel;
  
  while(!feof(stdin))
  {		
    fprintf( stdout, "%s\n%s\n", q->title, q->question );
    fflush(stdout);
    if( jdk_read_string_line( &s, stdin ) )
    {
      action=action_ok;			
      if( action==action_ok )
      {
        result.set( q->field, s.c_str() );
        break;
      }
      
    }
  }
  return action;	
}

jdk_questions_asker::user_action
jdk_questions_asker_text::ask_number(const jdk_question *q,bool first,bool last)
{
  jdk_str<128> s;
  user_action action = action_cancel;
  
  while(!feof(stdin))
  {		
    fprintf( stdout, "%s\n%s\n", q->title, q->question );
    fflush(stdout);
    if( jdk_read_string_line( &s, stdin ) )
    {
      // TODO: handle limits
      int value=s.strtol(10);
      if( value >= q->lower_limit && value <= q->upper_limit )
      {
        action=action_ok;			
      }
      if( action==action_ok )
      {
        result.set( q->field, s.c_str() );
        break;
      }
      
    }
  }
  return action;	
}





