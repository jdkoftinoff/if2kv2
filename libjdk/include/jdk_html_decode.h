#ifndef _JDK_HTML_DECODE_H
#define _JDK_HTML_DECODE_H

#include "jdk_cgi.h"

class jdk_html_decode_base
{
public:
  virtual ~jdk_html_decode_base() {}
  
  virtual void clear() = 0;
  virtual void process( const jdk_buf &in, jdk_dynbuf &out ) =0;  
};


class jdk_html_decode : public jdk_html_decode_base
{
public:
   explicit jdk_html_decode();
   virtual ~jdk_html_decode();
   
   void clear();
   virtual void process( const jdk_buf &in, jdk_dynbuf &out );
   
protected:
   enum
   {
	   IN_CONTENT,
	   IN_CONTENT_AMP,
	   IN_WHITESPACE,
	   IN_LESS_THAN_FIRST,
	   IN_LESS_THAN_SECOND,
	   IN_LESS_THAN_THIRD,
	   IN_TAG,	
	   IN_TAGNAME,
	   IN_COMMENT,
	   IN_COMMENT_END_FIRST,
	   IN_COMMENT_END_SECOND,
	   IN_COMMENT_QUOTE,
	   IN_TAG_QUOTE
   } state;
   jdk_str<256> tagname;
   bool in_script;
   bool in_style;
};


class jdk_html_decode_to_links : public jdk_html_decode_base
{
public:
   explicit jdk_html_decode_to_links();
   virtual ~jdk_html_decode_to_links();
   
   void clear();

   virtual void process( const jdk_buf &in, jdk_dynbuf &out );
   
protected:

   virtual void tagfound( const jdk_buf &in, size_t start_point, size_t len, jdk_dynbuf &out );
   virtual void contentfound( const jdk_buf &in, size_t start_point, size_t len, jdk_dynbuf &out );

   enum
   {
	   IN_CONTENT,
	   IN_CONTENT_AMP,
	   IN_WHITESPACE,
	   IN_LESS_THAN_FIRST,
	   IN_LESS_THAN_SECOND,
	   IN_LESS_THAN_THIRD,
	   IN_TAG,	
	   IN_TAGNAME,
	   IN_COMMENT,
	   IN_COMMENT_END_FIRST,
	   IN_COMMENT_END_SECOND,
	   IN_COMMENT_QUOTE,
	   IN_TAG_QUOTE,
	   IN_TAG_SPACE,
	   IN_TAG_PROPERTY
   } state;
   jdk_str<256> tagname;
   jdk_str<256> tagproperty;
   jdk_str<8192> tagpropertyvalue;
   bool in_script;
   bool in_style;
   size_t tag_start_point;
};

#endif
