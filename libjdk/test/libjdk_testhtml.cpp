#include "jdk_world.h"
#include "jdk_util.h"
#include "jdk_string.h"
#include "jdk_html.h"

#if 0
jdk_html_chunk *jdk_html_link( const char *link, jdk_html_chunk *contents )
{
  return new jdk_html_tag( "A", jdk_form<4096>("href=\"%s\"",link), contents );
}

jdk_html_chunk *jdk_html_center( jdk_html_chunk *contents )
{
  return new jdk_html_tag( "b", "", new jdk_html_tag( "p", "align=\"center\"", contents ) );
}

jdk_html_chunk *jdk_html_fancy_table( jdk_html_chunk *contents )
{
  return 
    jdk_html_center( new jdk_html_tag( "table", "width=85%", contents ) );
}

jdk_html_chunk *jdk_html_fancy_row( jdk_html_chunk *contents )
{
  return new jdk_html_tag( "tr", "", contents );	
}

jdk_html_chunk *jdk_html_fancy_cell( jdk_html_chunk *contents )
{
  return new jdk_html_tag( "td", "", contents );	
}

jdk_html_chunk *tag( const char *type, const char *options, jdk_html_chunk *contents=0, jdk_html_chunk *next=0 )
{
  return new jdk_html_tag( type, options, contents, next );
}

jdk_html_chunk *text( const char *t, jdk_html_chunk *next=0 )
{
  return new jdk_html_text( t, next );	
}


jdk_html_chunk *head( const char *title, jdk_html_chunk *next=0 )
{
  return tag( "head", "", tag( "title", "", new jdk_html_text(title) ), next );
}


int main( int argc, char **argv )
{
  jdk_html_tag mydoc( 
    "HTML", "",
    head( "My C++ Document" ),
    new jdk_html_tag( "BODY", "bgcolor=black textcolor=white",
                      new jdk_html_tag(
                        "CENTER", "",
                        new jdk_html_tag(
                          "b", "",
                          new jdk_html_text( "This is my centered bold text." ),					
                          jdk_html_center(
                            jdk_html_link( "http://www.jdkoftinoff.com/", new jdk_html_text( "My company's web page" )))
                          ),
                        new jdk_html_tag(
                          "P", "align=left",
                          new jdk_html_text( "And this is my left aligned paragraph." ),
                          new jdk_html_tag("hr", "size=8",
                                           jdk_html_fancy_table(
                                             jdk_html_fancy_row( jdk_html_fancy_cell( new jdk_html_text("Cell1")))
                                             )
                            )
                          )
			)
      )
    
    );
  
  
  jdk_dynbuf buf;
  mydoc.flatten(buf);
  buf.extract_stream(stdout);
  return 0;
}
#else

class my_doc : public jdk_html_document_generator
{
public:
  my_doc( const jdk_html_style &style_ ) : jdk_html_document_generator( style_ )
  {
  }
  
  jdk_html_chunk *generate_head( jdk_html_chunk *next )
  {
    return style.head( style.title( style.text("My Document")), next );	
  }
  
  jdk_html_chunk *generate_content( jdk_html_chunk *next )
  {
    jdk_html_chunk *last=next;
    for( int i=20; i>=1; --i )
    {
      jdk_str<1024> p;
      p.form( "This is paragraph %d", i );
      last = style.p( style.text( p ), last );
    }
    
    return style.heading1(style.text("This is heading1"),last);
    
  }
  
  jdk_html_chunk *generate_header( jdk_html_chunk *next )
  {
    return style.hr( "", style.center( style.p( style.text("This is the header"), style.hr("",next))));
  }
  
  jdk_html_chunk *generate_footer( jdk_html_chunk *next )
  {
    return style.hr( "", style.center( style.p( style.text("This is the footer"), style.hr("",next))));
  }
};

int main( int argc, char **argv )
{
  jdk_html_style_simple style;
  my_doc doc(style);
  
  jdk_html_chunk *chunk = doc.generate_document();
  jdk_dynbuf buf;
  
  chunk->flatten( buf );
  buf.extract_stream( stdout );
  delete chunk;
  return 0;
}


#endif
