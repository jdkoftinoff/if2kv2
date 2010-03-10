#include "jdk_world.h"
#include "if2k_pattern_expander.h"

/// Manages the recursive expansion of phrase patterns
struct if2_expstring_list
{
private:
  explicit if2_expstring_list( const if2_expstring_list & );
  const if2_expstring_list & operator = ( const if2_expstring_list & );
public:
  ///
  if2_expstring_list()
    : num(0), next(0)
  {
    for(int i=0; i<16; ++i )
    {
      strings[i]=0;
    }
  }
  
  
  ///
  ~if2_expstring_list() { delete next; }
  

  
  ///
  void add_string( const char *s )
  {
    if( num<16 )
    {
      strings[num++]=s;
    }
  }
  
  
  /**
     Adds a single string as an optional word for this phrase section. 
  */
  
  ///
  void exp_strings( const char *pre, if2_pattern_target &dest, short flags, bool remove=false )	   
    /**
       Recursively forms all combinations of phrases from this point
       in the phrase.  Adds the phrases to the specified WordList object.
       All the phrases will be prefixed by the 'pre' string.
    */
  {
    for( int i=0; i<num; ++i )
    {
      jdk_str<1024> new_pre;
      
      if( next )
      {
        if( *pre && *strings[i] )
          new_pre.form( "%s%s", pre, strings[i] );
        else
        {
          new_pre.form( "%s%s", pre, strings[i] );
        }
        
        next->exp_strings( new_pre.c_str(), dest, flags );
      }
      else
      {
        if( *pre )
          new_pre.form( "%s%s", pre, strings[i] );
        else
          new_pre.form( "%s", strings[i] );
        
        if( remove )
          dest.remove( new_pre );
        else
          dest.add( new_pre, flags );
      }
    }
    if( num==0 )
    {
      if( next )
      {
        next->exp_strings( pre, dest, flags );
      }
    }
  }
  
  
  /// number of strings in this section
  int num;
  /// Points to the next string list
  if2_expstring_list *next;
  
  /// space for the actual strings for this section.
  const char *strings[16];
};


bool if2_pattern_expander_standard::expand(
  const jdk_string &in_,
  if2_pattern_target &target,
  short flags,
  bool remove,
  const jdk_string &prefix
  )
{
  jdk_str<4096> in;
  
  in.cpy( prefix );
  in.cat( in_ );
  
  // if the phrase is not bracketed, then
  // there is only one 'word' - no optional phrase 
  // variations
  // 
  if( in.get(0)!='[' && in.get(0)!='{' )
  {
    target.add( in, flags );
    return true;
  }
  
  // find the length of the string including null
  // 
  size_t l=in.len()+1;
  
  // p is what we use to mark each possible end-of subphrase marker "," and "]" and "}"
  // with a null
  // 
  char *p=in.c_str();
  
  // w is what we use to keep track of the beginning 
  // of each 'word'
  // 
  char *w=0;
  
  if2_expstring_list top;
  if2_expstring_list *cur=&top;
  bool in_bracket_section=false;
  
  for( size_t a=0; a<l; ++a, ++p )
  {			   
    if( *p=='\0' )
    {
      // is it a string end without an ending bracket?
      if( in_bracket_section==true )
      {
        
        // yup, assume ending bracket
        if( w )
        {
          cur->add_string(w);
          w=0;
        }				
      }
      
      break;
    }
    
    
    
    // is it the beginning of a subphrase section?
    // 
    if( *p=='[' || *p=='{' )
    {
      in_bracket_section=true;
      // yes, kill this character with a null
      // 	
      *p='\0';
      
      // if we have a subphrase before this then add it.
      // 
      if( w )
      {
        cur->add_string( w );
        w=0;
      }
      
      // remember the next character is the beginning of the next subphrase
      // 
      w=p+1;
      
      // allocate a new string list object for this section
      cur->next=new if2_expstring_list;
      cur=cur->next;
      continue;		
    }
    else 
      // is it the end of a subphrase section?
      if( *p==']' || *p=='}' )
      {
        in_bracket_section=false;
        // yes, kill this character with a null
        *p='\0';
        
        // if this characters ends the last subphrase then add it to the current list
        // 
        if( w )
        {
          cur->add_string( w );
          w=0;
        }
        continue;
      }
      else
        // is is the end of a single subphrase?
        // 
        if( *p==',' && in_bracket_section )
        {
          // yes, kill this character with a null
          *p='\0';
          
          // if this character ends the last subphrase then add it to the current list
          if( w )
          {
            cur->add_string( w );
          }
          
          // remember the next character is the beginning of the next subphrase
          // 
          w=p+1;
          
          continue;
        }
        else if( in_bracket_section==false )
        {
          // assume open bracket if it is missing
          in_bracket_section=true;
          w=p;
          cur->next=new if2_expstring_list;
          cur=cur->next;
        }		
    
  }
  
  top.exp_strings("",target,flags,remove);
  return true;
}
