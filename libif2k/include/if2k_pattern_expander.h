#ifndef __IF2_PATTERN_EXPANDER_H
#define __IF2_PATTERN_EXPANDER_H

#include "jdk_buf.h"
#include "jdk_string.h"

class if2_pattern_target
{
public:
   virtual ~if2_pattern_target() {}
   virtual void add( const jdk_string &s, short flags ) = 0;
   virtual void remove( const jdk_string &s ) = 0; 
};

typedef if2_pattern_target if2k_pattern_target;

class if2_pattern_expander
{
public:
   explicit if2_pattern_expander()
   {
   }
   
   virtual ~if2_pattern_expander() 
   {
   }
   
   virtual bool expand( 
					   const jdk_string &in, 
					   if2_pattern_target &target,				    
					   short flags,
					   bool remove,
					   const jdk_string &prefix
				    ) = 0;

    virtual bool load_remove( 
							 const jdk_buf &buf, 
							 if2_pattern_target &target,
							 const jdk_string &prefix
							 )
    {
	  int pos=0;
	  jdk_str<4096> str;
	  
	  while( (pos=buf.extract_to_string(str,pos))>=0 )
	  {
		 if( str.len()>4 )
		 {
			if( !expand( str, target, 0, true, prefix ) )
			{
			    return false;
			}				  
		 }			 
	  }
	  return true;    
	}				 
            
   virtual bool load( 
					 const jdk_buf &buf,
					 if2_pattern_target &target,				  
					 short flags,
					 const jdk_string &prefix,
           volatile int *progress = 0
				  )
   {
	  int pos=0;
	  jdk_str<4096> str;
	  progress=0;
	  while( (pos=buf.extract_to_string(str,pos))>=0 )
	  {
     progress++;
		 if( str.len()>4 )
		 {
			if( !expand( str, target, flags, false, prefix ) )
			{
			    return false;
			}				  
		 }			 
	  }
	  return true;
   }
   
   
   virtual bool load(
	   const jdk_string_filename &fname,
	   if2_pattern_target &target,				 
	   short flags,
	   const jdk_string &prefix,
     volatile int *progress=0
	   )
	   {
		   jdk_str<4096> str;
       progress=0;
		   FILE *f=jdk_fopen(fname.c_str(),"rt");
		   if( f )		
		   {
			   while( jdk_read_string_line( &str, f ) )
			   {
           progress++;
				   if( str.len()>4 )
				   {			
					   if( !expand( str, target, flags, false, prefix ) )
					   {
						   fclose(f);
						   return false;
					   }				  
				   }
			   }			 
			   fclose(f);
			   return true;
		   }
		   else
		   {			 
			   return false;
		   }
	   }

   
   virtual bool load_remove(
							const jdk_string_filename &fname,
							if2_pattern_target &target,
							const jdk_string &prefix
							)
   {
	  jdk_str<4096> str;
	  FILE *f=jdk_fopen(fname.c_str(),"rt");
	  if( f )		
	  {
		 while( jdk_read_string_line( &str, f ) )
		 {
			if( !expand( str, target, 0, true, prefix ) )
			{
			    fclose(f);
			    return false;
			}				  
			
		 }			 
		 fclose(f);
		 return true;
	  }
	  else
	  {			 
		 return false;
	  }
	  
   }
   
   
};

typedef if2_pattern_expander if2k_pattern_expander;

class if2_pattern_expander_standard : public if2_pattern_expander
{
public:
   explicit if2_pattern_expander_standard()
   {
   }
   
   bool expand(
			   const jdk_string &in, 
			   if2_pattern_target &target,			
			   short flags,
			   bool remove,
			   const jdk_string &prefix
			);

						
};

typedef if2_pattern_expander_standard if2k_pattern_expander_standard;


#endif
