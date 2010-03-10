#include "jdk_world.h"
#include "jdk_string.h"
#include "jdk_atom.h"
#include "jdk_map.h"
#include "jdk_dynbuf.h"

struct atom_dictionary;
struct atom;
struct atom_intrinsic;
struct atom_environment;


struct atom_intrinsic
{
  atom_intrinsic() {}
  virtual ~atom_intrinsic() {}
  virtual bool exec( atom_environment *env, atom *expression ) = 0;
};

struct atom
{
  atom( const atom &o );
  atom & operator = ( const atom &o );

  atom( jdk_dynbuf *d, atom *n=0 );
  atom( atom_dictionary *sym, atom *n = 0 );
  atom( atom *l=0, atom *n=0 );
  atom( atom_intrinsic *i, atom *n=0 );
  
  ~atom();

  enum atom_type
  {
    type_data,
    type_symbol,
    type_list,
    type_intrinsic
  } type;


  jdk_dynbuf *data;
  atom_dictionary *symbol;
  atom_intrinsic *intrinsic;
  
  atom *list;
  atom *next;
};


struct atom_dictionary
{
  atom_dictionary( const atom_dictionary &o );
  
  atom_dictionary & operator = (const atom_dictionary &o );
  
  atom_dictionary( char v='\0', atom_dictionary *p=0, atom *a=0 );
  
  ~atom_dictionary();
  
  atom_dictionary *find( const char *s, int len );
  
  atom_dictionary *add( const char *s, int len );
  
  bool endpoint;
  char value;
  atom *contents;
  atom_dictionary *parent;
  atom_dictionary *table[128];
};

struct atom_environment
{
  atom_environment( const atom_environment &o )
  : 
  globals( o.globals ),
  params( o.params ),
  locals( o.locals ),
  parent(0)
  {
  }
  
  atom_environment & operator = ( const atom_environment &o )
  {
    if( &o!=this )
    {
      globals = o.globals;
      params = o.params;
      locals = o.locals;
      parent = 0;
    }
    return *this;
  }
  
  atom_environment( atom_environment *p=0);
  ~atom_environment() {}
  
  atom_dictionary globals;
  atom_dictionary params;
  atom_dictionary locals;
  atom_environment *parent;
};


#if 0
struct atom_parser
{
  atom_parser()
  :
  top(0),
  cur(0),
  mode(mode_start)
  {
  }
  
  bool parse( char c )
  {
    switch( mode )
    {
      case mode_start:
        return parse_start( c );
      case mode_paren:
        return parse_paren( c );
      case mode_bracket:
        return parse_bracket( c );
      case mode_brace:
        return parse_brace( c );
      case mode_string:
        return parse_string( c );
      case mode_symbol:
        return parse_symbol( c );
      case mode_data:
        return parse_data(c);
    }
  }
  
  bool parse_start( char c )
  {
  }
  
  bool parse_paren( char c )
  {
  }
  
  bool parse_bracket( char c )
  {
  }
  
  bool parse_brace( char c )
  {
  }
  
  bool parse_symbol( char c )
  {
  }
  
  bool parse_string( char c )
  {
  }
  
  bool parse_data( char c )
  {
  }
  
  
  atom *top;
  atom *cur;
  enum
  {
    mode_start,
    mode_paren,
    mode_bracket,
    mode_brace,
    mode_symbol,
    mode_string,
    mode_data
  } mode;
};
#endif


atom::atom( const atom &o )
:
type( o.type ),
data( o.data ),
symbol( o.symbol ),
intrinsic(0),
list(0),
next(0)
{
  if( o.list )
  {
    list=new atom( *o.list );
  }
  if( o.next )
  {
    next=new atom( *o.next );
  }
}


atom::atom( jdk_dynbuf *d, atom *n )
  : 
  type( type_data ),
  data( d ),
  symbol(0),
  intrinsic(0),
  list(0),
  next(n)
{
}
  
atom::atom( atom_dictionary *sym, atom *n )
  :
  type( type_symbol ),
  data( 0 ),
  symbol( sym ),
  intrinsic(0),
  list(0),
  next(n)
{
}

atom::atom( atom_intrinsic *i, atom *n )
:
type( type_symbol ),
data( 0 ),
symbol( 0 ),
intrinsic(i),
list(0),
next(n)
{
}


atom::atom( atom *l, atom *n )
  :
  type( type_list ),
  data( 0 ),
  symbol( 0 ),
  intrinsic( 0 ),
  list(l),
  next(n)
{
}
  
atom::~atom()
{
  delete next;
  if( type==type_list )
    delete list;
  if( type==type_data )
    delete data;
  if( type==type_intrinsic )
    delete intrinsic;
}
  


atom_dictionary::atom_dictionary( const atom_dictionary &o )
:
endpoint( o.endpoint ),
value( o.value ),
contents(0),
parent(0)
{
  if( o.contents )
  {
    contents = new atom( *o.contents );
  }
  
  for( int i=0; i<128; ++i )
  {
    table[i] = (o.table[i]!=0) ? new atom_dictionary(*o.table[i]) : 0;
  }
}

atom_dictionary & atom_dictionary::operator = (const atom_dictionary &o )
{
  if( &o!=this )
  {
    endpoint = o.endpoint;
    value = o.value;
    
    delete contents;
    if( o.contents )
    {
      contents = new atom( *o.contents );
    }
    
    for( int i=0; i<128; ++i )
    {
      delete table[i];
      table[i] = (o.table[i]!=0) ? new atom_dictionary(*o.table[i]) : 0;        
    }
  }
  return *this;
}

atom_dictionary::atom_dictionary( char v, atom_dictionary *p, atom *a)
: 
endpoint(false), 
value(v), 
contents(a),
parent(p) 
{
  for( int i=0; i<128; ++i )
  {
    table[i] = 0;
  }
}

atom_dictionary::~atom_dictionary()
{
  for( int i=0; i<128; ++i )
  {
    delete table[i];
  }
  if( contents )
  {
    delete contents;
  }
}

atom_dictionary *atom_dictionary::find( const char *s, int len )
{
  atom_dictionary *cur = this;
  
  for( int i=0; i<len; ++i )
  {
    if( cur->table[int(*s)] )
    {
      cur = cur->table[int(*s)];
      s++;
    }
    else
    {
      cur=0;
      break;
    }
  }
  
  if( cur->endpoint==false )
    cur=0;
  return cur;
}

atom_dictionary *atom_dictionary::add( const char *s, int len )
{
  atom_dictionary *cur = this;
  
  for( int i=0; i<len; ++i )
  {
    if( cur->table[int(*s)]==0 )
    {
      cur->table[int(*s)] = new atom_dictionary( *s, cur );      
    }
    cur = cur->table[int(*s)];
    s++;
  }
  cur->endpoint=true;
  return cur;    
}  

atom_environment::atom_environment( atom_environment *p)
:
globals(),
params(),
locals(),
parent( p )
{
}



int main()
{
  atom_dictionary d;
  atom *a = new atom();
  atom_dictionary *p=d.add( "ABCD", 4 );
  p->contents = a;
  return 0;
}




