#ifndef _JDK_TABLE_H
#define _JDK_TABLE_H

#include "jdk_error.h"
#include "jdk_fasttree.h"
#include "jdk_string.h"
#include "jdk_dynbuf.h"

template <class VALUE>
class jdk_table;

template <class VALUE>
class jdk_table
{
private:
  jdk_fasttree<VALUE *,32,char> tree;
public:
  typedef VALUE value_t;
  typedef VALUE value_type;

  jdk_table() 
    {
    }

  jdk_table( const jdk_table<VALUE> &o) 
    {    
    }

  const jdk_table & operator = ( const jdk_table<VALUE> &o )
    {      
    }

  ~jdk_table() 
    {
    }

  bool validate() const
    {
    }

  bool dump( jdk_buf &b ) const
    {
    }


  void clear()
    {
    }

  bool set( const jdk_string &k, VALUE *v )
    {
    }

  bool remove( const jdk_string &k )
    {
    }

  bool merge( const jdk_table<VALUE> &o )
    {
    }

  VALUE *get( const jdk_string &k )
    {
    }

  VALUE *get( const jdk_string &k ) const
    {
    }

  int get_int( const jdk_string &k, int default_value=0 ) const
    {
    }

  void set_int( const jdk_string &k, int value )
    {
    }

  bool get_bool( const jdk_string &k, bool default_value=false ) const
    {
    }

  void set_bool( const jdk_string &k, bool value )
    {
    }

  float get_float( const jdk_string &k, float default_value=0.0f ) const
    {
    }

  void set_float( const jdk_string &k, float value )
    {
    }

  double get_double( const jdk_string &k, double default_value=0.0f ) const
    {
    }

  void set_double( const jdk_string &k, double value )
    {
    }
};


template <class TABLE>
class jdk_table_prefixed
{
private:
  TABLE table;
  jdk_str<256> prefix;

public:
  jdk_table_prefixed( TABLE &o, const jdk_string &prefix_ ) 
    :
    table(o),
    prefix(prefix_)
    {
    }

  jdk_table_prefixed( const jdk_table_prefixed<TABLE> &o) 
    {    
    }

  const jdk_table_prefixed & operator = ( const jdk_table_prefixed<TABLE> &o )
    {
    }

  ~jdk_table_prefixed() 
    {
    }

  bool validate() const
    {
    }

  bool dump( jdk_buf &b ) const
    {
    }

  void clear()
    {
    }

  bool set( const jdk_string &k, VALUE *v )
    {
    }

  bool remove( const jdk_string &k )
    {
    }

  bool merge( const jdk_table<VALUE> &o )
    {
    }

  VALUE *get( const jdk_string &k )
    {
    }

  VALUE *get( const jdk_string &k ) const
    {
    }

  int get_int( const jdk_string &k, int default_value=0 ) const
    {
    }

  void set_int( const jdk_string &k, int value )
    {
    }

  bool get_bool( const jdk_string &k, bool default_value=false ) const
    {
    }

  void set_bool( const jdk_string &k, bool value )
    {
    }

  float get_float( const jdk_string &k, float default_value=0.0f ) const
    {
    }

  void set_float( const jdk_string &k, float value )
    {
    }

  double get_double( const jdk_string &k, double default_value=0.0f ) const
    {
    }

  void set_double( const jdk_string &k, double value )
    {
    }

};


#if defined(JDK_TABLE_TEST)
#include "jdk_test.h"


int JDK_TABLE_TEST()
{
  
}

#endif

#endif
