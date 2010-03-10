#ifndef _JDK_HTML_H
#define _JDK_HTML_H

#include "jdk_string.h"
#include "jdk_dynbuf.h"
#include "jdk_cgi.h"
#include "jdk_questions.h"
#include "jdk_http.h"
#include "jdk_settings.h"

class jdk_html_chunk
{
public:
	explicit jdk_html_chunk()
	{
	}
	
	virtual ~jdk_html_chunk()
	{
	}
	
	virtual jdk_html_chunk * set_next( jdk_html_chunk *n ) = 0;
	virtual jdk_html_chunk *get_next() =0;
	virtual const jdk_html_chunk *get_next() const =0;

	virtual jdk_html_chunk *peek_next() =0;
	virtual const jdk_html_chunk *peek_next() const =0;

	virtual void flatten( jdk_dynbuf &buf ) const = 0;
};


class jdk_html_tag : public jdk_html_chunk
{
    jdk_html_tag( const jdk_html_tag & );
    const jdk_html_tag & operator = ( const jdk_html_tag & );
    
	const jdk_str<64> tagname;
	const jdk_str<2048> options;
	const jdk_html_chunk *contents;	
	jdk_html_chunk *next;
	bool is_container;
	
public:
	
    explicit jdk_html_tag(
				 const char *tagname_,
				 const char *options_,
				 const jdk_html_chunk *contents_=0,				 
				 bool is_container_=true,
				 jdk_html_chunk *next_=0
				 )
	  :
		tagname( tagname_ ),
		options( options_ ),
		contents( contents_ ),
		next( next_ ),
		is_container(is_container_)
	{
	}

    explicit jdk_html_tag(
				 const char *tagname_,
				 const jdk_string &options_,
				 const jdk_html_chunk *contents_=0,
				 bool is_container_=true,
				 jdk_html_chunk *next_=0
				 )
	  :
		tagname( tagname_ ),
		options( options_ ),
		contents( contents_ ),
		next( next_ ),
        is_container(is_container_)
	{
	}
	
	virtual ~jdk_html_tag()
	{
		delete contents;
		delete next;
	}
	
	
	virtual jdk_html_chunk * set_next( jdk_html_chunk *n )
	{
		if( next )
		{
			jdk_html_chunk *last = get_next();
			last->set_next( n );
		}
		else
		{			
			next=n;
		}
		
		return n;
	}
	
	virtual jdk_html_chunk *get_next()
	{
		if( next->peek_next() )
			return next->get_next();
		else
			return next;
	}

	virtual const jdk_html_chunk *get_next() const
	{
		if( next->peek_next() )
			return next->get_next();
		else
			return next;
	}
	
	virtual jdk_html_chunk *peek_next()
	{
		return next;	
	}
	
	virtual const jdk_html_chunk *peek_next() const
	{
		return next;	
	}
	
	
	virtual void flatten( jdk_dynbuf &buf ) const
	{
		buf.append_from_string( "<" );
		buf.append_from_string( tagname );
		if( !options.is_clear() )
		{			
			buf.append_from_string( " " );
			buf.append_from_string( options );
		}		
		buf.append_from_string( ">" );
		if( contents )
		{			
			contents->flatten( buf );
		}

        if( is_container )
	    {
    		buf.append_from_string( "</" );
    		buf.append_from_string( tagname );
    		buf.append_from_string( ">\r\n" );
	    }
		if( next )
		{
			next->flatten(buf);	
		}		
	}
	
};

class jdk_html_text : public jdk_html_chunk
{
    jdk_html_text( const jdk_html_text & );
    const jdk_html_text & operator = ( const jdk_html_text & );
    
	jdk_dynbuf s;
	jdk_html_chunk *next;	
public:
	explicit jdk_html_text(const char *str,jdk_html_chunk *next_=0) : s(), next( next_ )
	{
		s.append_from_string( str );
	}

	explicit jdk_html_text(const jdk_dynbuf &buf,jdk_html_chunk *next_=0) : s(), next( next_ )
	{
		s.append_from_buf( buf );
	}
	
    explicit jdk_html_text(const jdk_string &str,jdk_html_chunk *next_=0) : s(), next( next_ )
	{
		s.append_from_string( str );
	}
		
	virtual ~jdk_html_text()
	{
		delete next;
	}

	virtual jdk_html_chunk * set_next( jdk_html_chunk *n )
	{
		if( next )
		{
			jdk_html_chunk *last = get_next();
			last->set_next( n );
		}
		else
		{			
			next=n;
		}
		
		return n;
	}
	
	virtual jdk_html_chunk *get_next()
	{
		if( next->peek_next() )
			return next->get_next();
		else
			return next;
	}

	virtual const jdk_html_chunk *get_next() const
	{
		if( next->peek_next() )
			return next->get_next();
		else
			return next;
	}
	
	virtual jdk_html_chunk *peek_next()
	{
		return next;	
	}
	
	virtual const jdk_html_chunk *peek_next() const
	{
		return next;	
	}
	
	
	virtual void flatten( jdk_dynbuf &buf ) const
	{
		if( jdk_html_requires_escaping(s) )
		{
			// html special characters must be translated
			jdk_dynbuf tmp( s.get_data_length()*4 );
			jdk_html_amp_escape_text((const char *)(s.get_data()),(char *)(tmp.get_data()),s.get_data_length()*4);
			tmp.set_data_length( (int)jdk_strlen( (const char *)(tmp.get_data()) ) );	
			buf.append_from_buf( tmp );
		}
		else
		{
			// no special characters, so we can do this faster like this
			buf.append_from_buf(s);
		}
		
		// now flatten the next one in line
		if( next )
		  next->flatten(buf);
	}
};

class jdk_html_unescaped_text : public jdk_html_chunk
{
  jdk_html_unescaped_text( const jdk_html_unescaped_text & );
  const jdk_html_unescaped_text & operator = ( const jdk_html_unescaped_text & );
  
	jdk_dynbuf s;
	jdk_html_chunk *next;	
public:
	explicit jdk_html_unescaped_text(const char *str,jdk_html_chunk *next_=0) : s(), next( next_ )
	{
		s.append_from_string( str );
	}

    explicit jdk_html_unescaped_text(const jdk_dynbuf &buf,jdk_html_chunk *next_=0) : s(), next( next_ )
	{
		s.append_from_buf( buf );
	}
	
    explicit jdk_html_unescaped_text(const jdk_string &str,jdk_html_chunk *next_=0) : s(), next( next_ )
	{
		s.append_from_string( str );
	}
		
	virtual ~jdk_html_unescaped_text()
	{
		delete next;
	}

	virtual jdk_html_chunk * set_next( jdk_html_chunk *n )
	{
		if( next )
		{
			jdk_html_chunk *last = get_next();
			last->set_next( n );
		}
		else
		{			
			next=n;
		}
		
		return n;
	}
	
	virtual jdk_html_chunk *get_next()
	{
		if( next->peek_next() )
			return next->get_next();
		else
			return next;
	}

	virtual const jdk_html_chunk *get_next() const
	{
		if( next->peek_next() )
			return next->get_next();
		else
			return next;
	}
	
	virtual jdk_html_chunk *peek_next()
	{
		return next;	
	}
	
	virtual const jdk_html_chunk *peek_next() const
	{
		return next;	
	}
	
	
	virtual void flatten( jdk_dynbuf &buf ) const
	{
		buf.append_from_buf(s);
		
		// now flatten the next one in line
		if( next )
		  next->flatten(buf);
	}
};



class jdk_html_style
{
public:
	explicit jdk_html_style()
	{
	}
	
	virtual ~jdk_html_style()
	{
	}
		
	virtual jdk_html_chunk *tag( 
								const char *type, 
								const char *options,
								jdk_html_chunk *contents=0,
								bool is_container=true,
								jdk_html_chunk *next=0 
								) const
	{
		return new jdk_html_tag( type, options, contents, is_container, next );
	}

	virtual jdk_html_chunk *tag( 
								const char *type, 
								const jdk_string &options,
								jdk_html_chunk *contents=0,
								bool is_container=true,
								jdk_html_chunk *next=0 
								) const
	{
		return new jdk_html_tag( type, options, contents, is_container, next );
	}

	
	virtual jdk_html_chunk *text(
								const char *text_,
								jdk_html_chunk *next=0 
								) const
	{
		return new jdk_html_text( text_, next );
	}

	virtual jdk_html_chunk *text(
								const jdk_string &text_,
								jdk_html_chunk *next=0 
								) const
	{
		return new jdk_html_text( text_, next );
	}

	virtual jdk_html_chunk *text(
								const jdk_dynbuf &text_,
								jdk_html_chunk *next=0 
								) const
	{
		return new jdk_html_text( text_, next );
	}
	
	virtual jdk_html_chunk *unescaped_text(
								const char *text_,
								jdk_html_chunk *next=0 
								) const
	{
		return new jdk_html_unescaped_text( text_, next );
	}

	virtual jdk_html_chunk *unescaped_text(
								const jdk_string &text_,
								jdk_html_chunk *next=0 
								) const
	{
		return new jdk_html_unescaped_text( text_, next );
	}

	virtual jdk_html_chunk *unescaped_text(
								const jdk_dynbuf &text_,
								jdk_html_chunk *next=0 
								) const
	{
		return new jdk_html_unescaped_text( text_, next );
	}
	
	
	virtual jdk_html_chunk *font(
								const char *fontinfo,
								jdk_html_chunk *contents,
								jdk_html_chunk *next=0
								) const = 0;
	virtual jdk_html_chunk *italic(
								jdk_html_chunk *contents,
								jdk_html_chunk *next=0
								) const = 0;
	virtual jdk_html_chunk *bold(
								jdk_html_chunk *contents,
								jdk_html_chunk *next=0
								) const = 0;
	virtual jdk_html_chunk *header( jdk_html_chunk *next ) const = 0;
	virtual jdk_html_chunk *footer( jdk_html_chunk *next=0 ) const = 0;	
	virtual jdk_html_chunk *doc( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *body(  const char *options, jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *head( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *title( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;	
	virtual jdk_html_chunk *center( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;		
	virtual jdk_html_chunk *heading1( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *heading2( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *heading3( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *heading4( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *heading5( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *p( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *list( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *olist( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;	
	virtual jdk_html_chunk *list_item( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *image( const char *image_src, jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *link( const char *link_dest, jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *image_link( const char *image_src, const char *link_dest,jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *table( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *table_row( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *table_cell( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *table_title_row( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *table_title_cell( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *table_blank_cell( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *hr( const char *options="", jdk_html_chunk *next=0 ) const = 0;
	virtual jdk_html_chunk *br( const char *options="", jdk_html_chunk *next=0 ) const = 0;	
	virtual jdk_html_chunk *pre( jdk_html_chunk *contents, jdk_html_chunk *next=0 ) const = 0;
};

class jdk_html_style_simple : public jdk_html_style
{
public:
	explicit jdk_html_style_simple()
	{
	}
	
	virtual ~jdk_html_style_simple()
	{
	}
	
	jdk_html_chunk *font(
						const char *fontinfo,
						jdk_html_chunk *contents,
						jdk_html_chunk *next=0
						) const
	{
		return tag( "FONT", fontinfo, contents, true, next );
	}
	
	
	virtual jdk_html_chunk *italic(
								jdk_html_chunk *contents,
								jdk_html_chunk *next=0
								) const
								
	{
		return tag( "I", "", contents, true, next );
	}
	
	virtual jdk_html_chunk *bold(
								jdk_html_chunk *contents,
								jdk_html_chunk *next=0
								) const
	{
		return tag( "B", "", contents, true, next );
	}
	

	jdk_html_chunk *header( jdk_html_chunk *next ) const
	{
		return next;
	}
	
	jdk_html_chunk *footer( jdk_html_chunk *next ) const
	{
		return next;
	}	
		
	jdk_html_chunk *doc( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "HTML", "", contents, true, next );
	}
	
	jdk_html_chunk *body(  const char *options, jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "BODY", options, contents, true, next );
	}
	
	jdk_html_chunk *head( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "HEAD", "", contents, true, next );
	}

	jdk_html_chunk *title( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
			//return tag( "TITLE", "", contents, true, next );
			delete contents; // TODO: HACK!
			return next;
	}

	jdk_html_chunk *center( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "CENTER", "", contents, true, next );	
	}
	
	jdk_html_chunk *heading1( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "H1", "", contents, true, next );
	}
	
	jdk_html_chunk *heading2( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "H2", "", contents, true, next );
	}
	
	jdk_html_chunk *heading3( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "H3", "", contents, true, next );
	}
	
	jdk_html_chunk *heading4( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "H4", "", contents, true, next );
	}
	
	jdk_html_chunk *heading5( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "H5", "", contents, true, next );
	}
	
	jdk_html_chunk *p( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "P", "", contents, true, next );
	}
	
	jdk_html_chunk *list( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "UL", "", contents, true, next );
	}

	jdk_html_chunk *olist( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "OL", "", contents, true, next );
	}

	jdk_html_chunk *list_item( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "LI", "", contents, true, next);
	}	
	
	jdk_html_chunk *image( const char *image_src, jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
//		jdk_str<1024> escaped_img;
//		jdk_cgi_escape( image_src, escaped_img.c_str(), 1024 );
		jdk_str<1024> img;
		img.form( "SRC=\"%s\" BORDER=NO", image_src );
		return tag( "IMG", img, contents, false, next );
	}
	
	jdk_html_chunk *link( const char *link_dest, jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
//		jdk_str<1024> escaped_l;
//		jdk_cgi_escape( link_dest, escaped_l.c_str(), 1024 );
	
		jdk_str<1024> l;
		l.form( "HREF=\"%s\"", link_dest);
		return tag( "A", l, contents, true, next );
	}
	
	jdk_html_chunk *image_link( const char *image_src, const char *link_dest, jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return link( link_dest, image( image_src, contents, 0 ), next );
	}
	
	jdk_html_chunk *table( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag( "TABLE", options, contents, true, next );
	}
	
	jdk_html_chunk *table_row( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag("TR",options,contents,true,next);
	}
	
	jdk_html_chunk *table_cell( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag("TD",options,contents,true,next);
	}
	
	jdk_html_chunk *table_title_row( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag("TR",options,contents,true,next);
	}
	
	jdk_html_chunk *table_title_cell( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag("TD",options,contents,true,next);
	}
	
	jdk_html_chunk *table_blank_cell( const char *options, jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag("TD",options,text("&nbsp;"),true,next);
	}
	
	jdk_html_chunk *hr( const char *options, jdk_html_chunk *next ) const
	{
		return tag("HR",options,0,false,next);
	}
	jdk_html_chunk *br( const char *options, jdk_html_chunk *next ) const
	{
		return tag("BR",options,0,false,next);
	}
	
	jdk_html_chunk *pre( jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
		return tag("PRE","",contents,true,next);
	}	
};

class jdk_html_style_simple_black : public jdk_html_style_simple
{
public:
	explicit jdk_html_style_simple_black()
	{
	}
	
	virtual ~jdk_html_style_simple_black()
	{
	}
	
	jdk_html_chunk *body(  const char *options, jdk_html_chunk *contents, jdk_html_chunk *next ) const
	{
	    jdk_str<256> fullopt("BGCOLOR=BLACK TEXT=WHITE LINK=RED VLINK=RED ALINK=RED ");
	    fullopt.cat(options);

		return tag( "BODY", fullopt.c_str(), contents, true, next );
	}
};


class jdk_html_document_generator
{
public:
	jdk_html_document_generator( const jdk_html_style &style_ )
	  : style( style_ )
	{
	}
	
	virtual ~jdk_html_document_generator()
	{		
	}
	
	virtual jdk_html_chunk *generate_document()
	{
		return style.doc(
					generate_head( 
						generate_body(0)
					)
				);
	}
	
	virtual jdk_html_chunk *generate_body( jdk_html_chunk *next )
	{
	    return 
	    style.body( "", 
	      generate_header(
	      generate_content(		
    	  generate_footer(
		  next))));
	}
	
	virtual jdk_html_chunk *generate_head( jdk_html_chunk *next )
	{
		return next;
	}
	
	virtual jdk_html_chunk *generate_content( jdk_html_chunk *next ) = 0;	
	
	virtual jdk_html_chunk *generate_header( jdk_html_chunk *next )
	{
		return style.header(next);
	}
	
	virtual jdk_html_chunk *generate_footer( jdk_html_chunk *next )
	{
		return style.footer(next);
	}

protected:
	const jdk_html_style &style;
};


// this is a function template because many compilers do not like member templates yet
template <class MAPTYPE>
inline jdk_html_chunk *jdk_html_generate_table(
	                                        const jdk_html_style &style, 
	                                        const MAPTYPE &map, 
						                    const char *title_name, 
								            const char *title_value,
									        const char *table_options,
										    const char *title_options,
										    const char *left_cell_options,
										    const char *right_cell_options,
									        jdk_html_chunk *next = 0
										 )
	{
		jdk_html_chunk *table_contents=
			style.table_title_row( "", 
				style.table_title_cell( title_options, 
					style.text(title_name), 
				style.table_title_cell( title_options, 
					style.text(title_value))));
					
		jdk_html_chunk *cur = table_contents;
		for( int i=0; i<map.count(); ++i )
		{
			const typename MAPTYPE::pair_t *s = map.get(i);
			if( s )
			{
				cur = cur->set_next( 
					style.table_row( "", 
						style.table_cell( left_cell_options,
							style.text( s->key ),
						style.table_cell( right_cell_options,
							style.text( s->value )))));
			}
		}
		
		return style.table( table_options, table_contents, next);	    
	}
	
jdk_html_chunk *jdk_html_generate_form( 
                                    const jdk_html_style &style,
                                    const char *form_table_options,
                                    const jdk_question *question_list, 
          	    				    const jdk_settings &defaults,
	    					        const jdk_string &action_url,											
	    			                const jdk_string &submit_button_name, 						
	    					        jdk_html_chunk *next 
							    );


class jdk_questions_asker_html_generator_simple : public jdk_html_document_generator
{
  jdk_questions_asker_html_generator_simple(     const jdk_questions_asker_html_generator_simple & );
  const jdk_questions_asker_html_generator_simple & operator = (     const jdk_questions_asker_html_generator_simple & );


public:
    explicit jdk_questions_asker_html_generator_simple(
                                                       const jdk_html_style &style_, 
                                                       const jdk_settings &cgiparams_,
                                                       const jdk_http_request_header &request_,
                                                       const jdk_question *question_list_,
                                                       const jdk_settings &defaults_,
                                                       const jdk_string &action_url_,
                                                       const jdk_string &submit_label_
                                                       )
    :
      jdk_html_document_generator( style_ ), 
      cgiparams( cgiparams_ ), 
      request(request_),
      question_list( question_list_ ),
      defaults( defaults_ ),
      action_url( action_url_ ),
      submit_label( submit_label_ )
	{
	}									
																									
	~jdk_questions_asker_html_generator_simple()
	{
	}

    
	virtual jdk_html_chunk *generate_head( jdk_html_chunk *next )
	{
		return style.head( style.title( style.text( "" ) ), next );
	}
	
	virtual jdk_html_chunk *generate_form( jdk_html_chunk *next )
	{
	    return 			
	      jdk_html_generate_form(
			 style,
 			 "BORDER=0 WIDTH=90%", 
			 question_list,
			 defaults,
			 action_url,
			 submit_label,
			 next
			 );

	}

	jdk_html_chunk *generate_content( jdk_html_chunk *next )
	{
	    return 
	      style.center(	
	       generate_form(
	       next));
	}
				
protected:
	const jdk_settings &cgiparams;
	const jdk_http_request_header &request;
	const jdk_question *question_list;	
  const jdk_settings &defaults;
  const jdk_str<1024> action_url;
  const jdk_str<128> submit_label;
};


#endif
