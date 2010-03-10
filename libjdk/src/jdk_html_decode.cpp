#include "jdk_world.h"
#include "jdk_html_decode.h"
#include "jdk_util.h"

jdk_html_decode::jdk_html_decode() : 
  state( IN_CONTENT ),
  tagname(),
  in_script(false), 
  in_style(false)
{	
}

jdk_html_decode::~jdk_html_decode()
{
}

void jdk_html_decode::clear()
{
  state=IN_CONTENT;
  tagname.clear();
  in_script=false;
  in_style=false;
}

void jdk_html_decode::process( const jdk_buf &in, jdk_dynbuf &out )
{
  for( size_t i=0; i<in.get_data_length(); ++i )
  {
    char c[2];
    c[0]=in.get_data()[i];
    c[1]='\0';
    if( c[0]=='\n' || c[0]=='\r' )
      c[0]=' ';
    
    switch( state )
    {
    case IN_CONTENT:
    {
      if(c[0]=='<')
      {
        state=IN_LESS_THAN_FIRST;
      }
      else if(!in_script && !in_style && jdk_isspace(c[0]))
      {
        out.append_from_string(c);
        state=IN_WHITESPACE;
      }
      else if(!in_script && !in_style && c[0]=='&')
      {
        state=IN_CONTENT_AMP;
      }
      else if(!in_script && !in_style )
      {
        out.append_from_string(c);
      }
      break;
    }
    case IN_CONTENT_AMP:
    {
      if(c[0]==';')
      {
        state=IN_CONTENT;
      }
      else if(c[0]=='<' )
      {
        state=IN_LESS_THAN_FIRST;
      }
      break;
    }
    case IN_WHITESPACE:
    {
      if( !jdk_isspace(c[0]))
      {
        if(c[0]=='<')
        {
          state=IN_LESS_THAN_FIRST;
        }
        else if( c[0]=='&')
        {
          state=IN_CONTENT_AMP;
        }					
        else
        {
          out.append_from_string(c);
          state=IN_CONTENT;
        }
      }
      break;
    }
    case IN_LESS_THAN_FIRST:
    {
      if(c[0]=='"')
      {
        state=IN_TAG_QUOTE;
      }
      else if(c[0]=='\\')
      {
        state=IN_LESS_THAN_FIRST;
      }
      else if(c[0]=='!')
      {
        state=IN_LESS_THAN_SECOND;
      }
      else if( c[0]=='>' )
      {
        state=IN_CONTENT;
      }
      else
      {
        state=IN_TAGNAME;
        tagname.cat(c);
      }				
      break;
    }			
    case IN_LESS_THAN_SECOND:
    {
      if(c[0]=='"')
      {
        state=IN_TAG_QUOTE;
      }				
      else if(c[0]=='-')
      {
        state=IN_LESS_THAN_THIRD;
      }
      else if( c[0]=='>' )
      {
        state=IN_CONTENT;
      }
      else
      {
        state=IN_TAG;
      }				
      break;
    }
    case IN_LESS_THAN_THIRD:
    {
      if(c[0]=='"')
      {
        state=IN_TAG_QUOTE;
      }				
      else 
        if( c[0]=='-' )
        {
          state=IN_COMMENT;
        }
        else if( c[0]=='>' )
        {
          state=IN_CONTENT;
        }
        else
        {
          state=IN_TAG;
        }								
      break;
    }
    case IN_TAGNAME:			
    {
      if(c[0]=='"')
      {
        state=IN_TAG_QUOTE;
      }				
      else 				
        if( c[0]=='>' )
        {
          state=IN_CONTENT;
          if( tagname.icmp("SCRIPT")==0 )
          {
            in_script=true;
          }
          else if( tagname.icmp("/SCRIPT")==0 )
          {
            in_script=false;
          }
          else if( tagname.icmp("STYLE")==0 )
          {
            in_style=true;
          }
          else if( tagname.icmp("/STYLE")==0 )
          {
            in_style=false;
          }
          else if( tagname.icmp("/TR")==0 || tagname.icmp("HR")==0 )
          {
            out.append_from_string("\n\n");						
          }
          else if( tagname.icmp("BR")==0 || tagname.icmp("P")==0 )
          {
            out.append_from_string("\n");
          }					
          tagname.clear();
        }
        else if(jdk_isspace(c[0]) )
        {
          state=IN_TAG;					
        }
        else
        {
          tagname.cat(c);
        }
      
      break; 
    }
    
    case IN_TAG:
    {
      if(c[0]=='"')
      {
        state=IN_TAG_QUOTE;
      }				
      else 				
        if( c[0]=='>' )
        {
          state=IN_CONTENT;
          if( tagname.icmp("SCRIPT")==0 )
          {
            in_script=true;
          }
          if( tagname.icmp("/SCRIPT")==0 )
          {
            in_script=false;
          }
          else if( tagname.icmp("STYLE")==0 )
          {
            in_style=true;
          }
          else if( tagname.icmp("/STYLE")==0 )
          {
            in_style=false;
          }										
          else if( tagname.icmp("/TR")==0 || tagname.icmp("HR")==0 )
          {
            out.append_from_string("\n\n");						
          }
          else if( tagname.icmp("BR")==0 || tagname.icmp("P")==0 )
          {
            out.append_from_string("\n");
          }					
          tagname.clear();
        }
      break;
    }
    case IN_COMMENT:
    {
      if(c[0]=='"')
      {
        state=IN_COMMENT_QUOTE;
      }				
      else 				
        if( c[0]=='-' )
        {
          state=IN_COMMENT_END_FIRST;
        }
      break;
    }
    case IN_COMMENT_END_FIRST:
    {
      if(c[0]=='"')
      {
        state=IN_COMMENT_QUOTE;
      }				
      else 								
        if( c[0]=='-' )
        {
          state=IN_COMMENT_END_SECOND;
        }
        else
        {
          state=IN_COMMENT;
        }
      break;
    }
    case IN_COMMENT_END_SECOND:
    {
      if(c[0]=='"')
      {
        state=IN_COMMENT_QUOTE;
      }				
      else 								
        if( c[0]=='>' )
        {
          state=IN_CONTENT;
        }
        else
        {
          state=IN_COMMENT;
        }
      break;
    }
    case IN_TAG_QUOTE:
    {
      if(c[0]=='"')
      {
        state=IN_TAG;
      }
      break;
    }
    case IN_COMMENT_QUOTE:
    {
      if(c[0]=='"')
      {
        state=IN_COMMENT;
      }
      break;
    }
    }
    
  }
}




jdk_html_decode_to_links::jdk_html_decode_to_links() : 
  state( IN_CONTENT ), 
  tagname(),
  tagproperty(),
  tagpropertyvalue(),
  in_script(false), 
  in_style(false),
  tag_start_point(0)
{	
}

jdk_html_decode_to_links::~jdk_html_decode_to_links()
{
}

void jdk_html_decode_to_links::clear()
{
  state=IN_CONTENT;
  tagname.clear();
  in_script=false;
  in_style=false;
}

void jdk_html_decode_to_links::process( const jdk_buf &in, jdk_dynbuf &out )
{
  for( size_t i=0; i<in.get_data_length(); ++i )
  {
    char c[2];
    c[0]=in.get_data()[i];
    c[1]='\0';
    if( c[0]=='\n' || c[0]=='\r' )
      c[0]=' ';
    
    switch( state )
    {
    case IN_CONTENT:
    {
      if(c[0]=='<')
      {
        state=IN_LESS_THAN_FIRST;
        tag_start_point = i+1;
      }
      else if(!in_script && !in_style && jdk_isspace(c[0]))
      {
        state=IN_WHITESPACE;
      }
      else if(!in_script && !in_style && c[0]=='&')
      {
        state=IN_CONTENT_AMP;
      }
      else if(!in_script && !in_style )
      {
        //out.append_from_string(c);
      }
      break;
    }
    case IN_CONTENT_AMP:
    {
      if(c[0]==';')
      {
        state=IN_CONTENT;
      }
      else if(c[0]=='<' )
      {
        state=IN_LESS_THAN_FIRST;
        tag_start_point=i+1;
      }
      break;
    }
    case IN_WHITESPACE:
    {
      if( !jdk_isspace(c[0]))
      {
        if(c[0]=='<')
        {
          state=IN_LESS_THAN_FIRST;
          tag_start_point=i+1;
        }
        else if( c[0]=='&')
        {
          state=IN_CONTENT_AMP;
        }					
        else
        {
          //out.append_from_string(c);
          state=IN_CONTENT;
        }
      }
      break;
    }
    case IN_LESS_THAN_FIRST:
    {
      if(jdk_isspace(c[0]))
      {
        state=IN_TAG_SPACE;
      }
      else
        if(c[0]=='"'||c[0]=='\'')
        {
          state=IN_TAG_QUOTE;
        }
        else if(c[0]=='\\')
        {
          state=IN_LESS_THAN_FIRST;
        }
        else if(c[0]=='!')
        {
          state=IN_LESS_THAN_SECOND;
        }
        else if( c[0]=='>' )
        {
          state=IN_CONTENT;
          tagfound( in, tag_start_point, i-tag_start_point+1, out ); 	
        }
        else
        {
          state=IN_TAGNAME;
          tagname.cat(c);
        }				
      break;
    }
    case IN_LESS_THAN_SECOND:
    {
      if(jdk_isspace(c[0]))
      {
        state=IN_TAG_SPACE;
      }
      else
        if(c[0]=='"'||c[0]=='\'')
        {
          state=IN_TAG_QUOTE;
        }				
        else if(c[0]=='-')
        {
          state=IN_LESS_THAN_THIRD;
        }
        else if( c[0]=='>' )
        {
          state=IN_CONTENT;
          
          tagfound( in, tag_start_point, i-tag_start_point+1, out ); 		
        }
        else
        {
          state=IN_TAG;
        }				
      break;
    }
    case IN_LESS_THAN_THIRD:
    {
      if(jdk_isspace(c[0]))
      {
        state=IN_TAG_SPACE;
      }
      else
        if(c[0]=='"'||c[0]=='\'')
        {
          state=IN_TAG_QUOTE;
        }				
        else 
          if( c[0]=='-' )
          {
            state=IN_COMMENT;
          }
          else if( c[0]=='>' )
          {
            state=IN_CONTENT;
            
            tagfound( in, tag_start_point, i-tag_start_point+1, out ); 	
          }
          else
          {
            state=IN_TAG;
          }								
      break;
    }
    case IN_TAGNAME:			
    {
      if(jdk_isspace(c[0]))
      {
        state=IN_TAG_SPACE;
      }
      else
        if(c[0]=='"'||c[0]=='\'')
        {
          state=IN_TAG_QUOTE;
        }				
        else 				
          if( c[0]=='>' )
          {
            state=IN_CONTENT;
            jdk_str<8192> full_tag;
            
            tagfound( in, tag_start_point, i-tag_start_point+1, out ); 	
          }
          else if(jdk_isspace(c[0]) )
          {
            state=IN_TAG;					
          }
          else
          {
            tagname.cat(c);
          }
      
      break; 
    }
    
    case IN_TAG_SPACE:
    case IN_TAG_PROPERTY:			
    case IN_TAG:
    {
      if(jdk_isspace(c[0]))
      {
        state=IN_TAG_SPACE;
      }
      else
        if(c[0]=='"'||c[0]=='\'')
        {
          state=IN_TAG_QUOTE;
        }				
        else 				
          if( c[0]=='>' )
          {
            state=IN_CONTENT;
            
            tagfound( in, tag_start_point, i-tag_start_point+1, out ); 	
          }
      break;
    }
    case IN_COMMENT:
    {
      if(c[0]=='"')
      {
        state=IN_COMMENT_QUOTE;
      }				
      else 				
        if( c[0]=='-' )
        {
          state=IN_COMMENT_END_FIRST;
          
        }
      break;
    }
    case IN_COMMENT_END_FIRST:
    {
      if(c[0]=='"')
      {
        state=IN_COMMENT_QUOTE;
      }				
      else 								
        if( c[0]=='-' )
        {
          state=IN_COMMENT_END_SECOND;
        }
        else
        {
          state=IN_COMMENT;
        }
      break;
    }
    case IN_COMMENT_END_SECOND:
    {
      if(c[0]=='"')
      {
        state=IN_COMMENT_QUOTE;
      }				
      else 								
        if( c[0]=='>' )
        {
          state=IN_CONTENT;
        }
        else
        {
          state=IN_COMMENT;
        }
      break;
    }
    case IN_TAG_QUOTE:
    {
      if(c[0]=='"' || c[0]=='\'')
      {
        state=IN_TAG;
      }
      break;
    }
    case IN_COMMENT_QUOTE:
    {
      if(c[0]=='"' || c[0]=='\'')
      {
        state=IN_COMMENT;
      }
      break;
    }
    }
    
  }
}

void jdk_html_decode_to_links::tagfound(
  const jdk_buf &in,
  size_t start_point,
  size_t len,
  jdk_dynbuf &out
  )
{
  if( tagname.icmp("A")==0 )
  {
    jdk_str<8192> full_tag;
    full_tag.ncpy( (const char *)in.get_data()+start_point, len );
    
    char * http_pos = full_tag.str("http://");
    if( !http_pos )
      http_pos = full_tag.str("mailto:" );
    
    if( http_pos )
    {
      jdk_str<8192> full_url;
      full_url.cpy( http_pos );
      if( *(http_pos-1)=='\'' || *(http_pos-1)=='"' )
      {
        char *end_quote=full_url.chr( *(http_pos-1) );
        
        if( end_quote )
          *end_quote = 0;
      }
      else
      {
        char *end_pos=full_url.chr( ' ' );
        if( end_pos )
          *end_pos=0;
      }
      
      char *last_fix = full_url.chr('\'');
      if( last_fix )
        *last_fix=0;
      last_fix = full_url.chr('"');
      if( last_fix )
        *last_fix=0;
      
      out.append_from_string( full_url );
      out.append_from_string( "\n" );
    }
    
    
  }
  
  if( tagname.icmp("SCRIPT")==0 )
  {
    in_script=true;
  }
  else if( tagname.icmp("/SCRIPT")==0 )
  {
    in_script=false;
  }
  else if( tagname.icmp("STYLE")==0 )
  {
    in_style=true;
  }
  else if( tagname.icmp("/STYLE")==0 )
  {
    in_style=false;
  }
  else if( tagname.icmp("/TR")==0 || tagname.icmp("HR")==0 )
  {
  }
  else if( tagname.icmp("BR")==0 || tagname.icmp("P")==0 )
  {
  }					
  tagname.clear();
  
}

void jdk_html_decode_to_links::contentfound(
  const jdk_buf &in,
  size_t start_point,
  size_t len,
  jdk_dynbuf &out
  )
{
  
}

