#ifndef __JDK_ELASTIC_QUEUE_H
#define __JDK_ELASTIC_QUEUE_H

#include "jdk_log.h"
#include "jdk_dynbuf.h"


template <class T>
class jdk_elastic_queue
{
  explicit jdk_elastic_queue( const jdk_elastic_queue & );
  const jdk_elastic_queue & operator = (const jdk_elastic_queue & );
public:

  inline explicit jdk_elastic_queue( int start_size=4096 )
    :
    buf( new jdk_dynbuf(start_size * sizeof(T)) ),
    next_in( 0 ),
    next_out( 0 ),
    buffer_size(start_size),
    data_length(0)
    {      
    }
    
  ~jdk_elastic_queue() 
    {
      delete buf;
    }

  void clear()
    {
      next_in=0;
      next_out=0;
      data_length=0;
    }

  int get_data_length() const
    {
      return data_length;
    }

  int get_available_space() const
    {
      return buffer_size; // faked! this is elastic!
    }

  bool can_put( int sz=1 ) const
    {
      return true;
    }

  void put( const T *o, int len )
    {
      for( int i=0; i<len; ++i )
      {
        put( o[i] );
      }
    }

  void put( const T &o )
    {
      int sz=1;
      int pre_off=next_out;
      if( next_in<next_out )
      {
        pre_off=next_in;
      }
      validate();
      if( sz>=(buffer_size-data_length) ) 
      {
        int new_buffer_size = buffer_size * 3 / 2;  // increase buf size by 50%
        jdk_dynbuf *newbuf = new jdk_dynbuf( new_buffer_size );

        if( next_in>next_out )
        {
          newbuf->append_from_data( buf->get_data() + (next_out * sizeof(T)),  (data_length*sizeof(T))  );
        }
        else
        {
          newbuf->append_from_data( buf->get_data() + (next_out * sizeof(T)),  ((buffer_size-next_out)*sizeof(T)) );
          newbuf->append_from_data( buf->get_data(), (next_in * sizeof(T)) );
        }
        delete buf;

        buf = newbuf;

        buffer_size = new_buffer_size;
        
        if( next_out>next_in )
        {
          next_out=0;
          next_in=data_length;
        }

#if JDK_DEBUG
        jdk_log( JDK_LOG_DEBUG3, "expanded size to %d", buffer_size );
#endif
        validate();
      }
      
      T *p = (T *)buf->get_data();
      p[next_in] = o;
      
      int new_next_in = next_in+1;
      if( new_next_in==buffer_size )
      {
        new_next_in=0;
      }
      next_in = new_next_in;
      data_length+=sz;
      if( next_in<=next_out )
        buf->set_data_length( (signed)(data_length+pre_off) * sizeof(T) );
      else
        buf->set_data_length( (signed)(data_length+pre_off) * sizeof(T) );
      validate();
    }

  bool can_get( int sz=1 ) const
    {
      return sz<get_data_length();
    }

  T peek() const
    {
      validate();
      const T *p = (const T *)buf->get_data();
      return p[next_out];
    }

  void peek( jdk_dynbuf &result, int sz=0 ) const
    {
      if( sz==0 )
      {
        sz = get_data_length();
      }
      result.clear();
      if( result.get_buf_length()<(size_t)(sizeof(T) *sz) )
      {
        result.expand( sizeof(T) * sz +1 );
      }
      peek( (T *)result.get_data(), sz );
      result.set_data_length( sizeof(T)*sz );
    }

  void peek( T *result, int sz ) const
    {
      const T *p = (const T *)buf->get_data();
      int out_num=next_out;
      for( int i=0; i<sz; ++i )
      {
        result[i] = p[out_num];
        out_num++;
        if( out_num==buffer_size )
        {
          out_num=0;
        }
      }
      validate();
    }

  void skip( int sz=1 )
    {
      int new_next_out=next_out+sz;
      if( new_next_out >= buffer_size )
      {
        new_next_out-=buffer_size;
      }
      next_out=new_next_out;
      data_length-=sz;
      validate();
    }

  void dump_state() const
    {
      jdk_log( JDK_LOG_DEBUG3, "dynbuf size: %d", buf->get_buf_length() );
      jdk_log( JDK_LOG_DEBUG3, "dynbuf data len: %d", buf->get_data_length() );
      jdk_log( JDK_LOG_DEBUG3, "buffer_size: %d", buffer_size );
      jdk_log( JDK_LOG_DEBUG3, "data_length: %d", data_length );
      jdk_log( JDK_LOG_DEBUG3, "next_in: %d", next_in );
      jdk_log( JDK_LOG_DEBUG3, "next_out: %d", next_out );
    }

  void validate() const
    {
#if JDK_DEBUG  
      if( next_in<0 ||  next_in>=buffer_size)
      {
        dump_state();
        JDK_THROW_ERROR( "elastic queue validation failed (next_in)", "" );
      }

      if( next_out<0 ||  next_out>=buffer_size)
      {
        dump_state();
        JDK_THROW_ERROR( "elastic queue validation failed (next_out)", "" );
      }

      if( data_length<0 || (size_t)((data_length) * sizeof(T))>buf->get_data_length() )
      {
        dump_state();
        JDK_THROW_ERROR( "elastic queue validation failed (data_length)", "" );
      }

      if( buffer_size < 0 )
      {
        dump_state();
        JDK_THROW_ERROR( "elastic queue validation failed (negative data_length)", "");
      }

      if( (buffer_size * sizeof(T)) != buf->get_buf_length() )
      {
        dump_state();
        JDK_THROW_ERROR( "elastic queue validation failed (data_length)", "" );
      }
      
      if( next_out<=next_in )
      {
        if( data_length!=(next_in-next_out) )
        {
          dump_state();
          JDK_THROW_ERROR( "elastic queue validation failed (next_in mismatch 1)", "" );
        }
      }
      else
      {
        if( data_length!=(buffer_size + (next_in-next_out)) )
        {
          dump_state();
          JDK_THROW_ERROR( "elastic queue validation failed (next_in mismatch 2)", "" );
        }      
      }
#endif
    }

protected:
  jdk_dynbuf *buf;
  int next_in;
  int next_out;
  int buffer_size;
  int data_length;
};

#endif
