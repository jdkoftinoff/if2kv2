#ifndef _JDK_NIO_H
#define _JDK_NIO_H

#include "jdk_world.h"

BEGIN_NAMESPACE(jdk);
BEGIN_NAMESPACE(nio);


class selector_t;

class channel_t;
class socket_channel_t;
class driver_channel_t;

template <typename T=unsigned char>
class bufferslice_t
{
public:
  bufferslice_t<T> & operator = (const bufferslice_t<T> &o )
  {
    if( this != &o )
    {
      _data = o._data;
      _sz = o._sz;
    }
    return *this;
  }

  explicit bufferslice_t( const bufferslice_t<T> &o ) : _data(o.data), _sz(o.sz) {}

  explicit bufferslice_t( const T *data, int sz ) : _data( data ), _sz( sz ) {}  

  ~bufferslice_t() {}

  const T *get( int offset=0 ) const { return &_data[offset]; }
  
  int length() const { return sz; }

private:
  const T *_data;
  int _sz;
};



template <typename T=unsigned char>
class buffer_t
{
public:
  typedef T element_t;

  buffer_t<T> & operator = ( const buffer_t<T> &o );

  explicit buffer_t( const buffer_t<T> &o );

  explicit buffer_t( int capacity=8192 );

  ~buffer_t();

  void clear();

  bool append( T *data, int sz );

  T *get( int offset=0 );

  const T *get( int offset=0 ) const;

  int pos() const;

  int available() const;

  int length() const;
};

template <typename T=unsigned char>
class bufferlist_t
{
public:
  typedef T element_t;

  bufferlist_t<T> & operator = ( const bufferlist_t<T> &o );

  explicit bufferlist_t( const bufferlist_t<T> &o );

  explicit bufferlist_t( int max_buffers, int capacity );

  ~bufferlist_t();

  void clear();
  
  bool append( T *data, int sz );

  T *get( int offset=0 );
  const T *get( int offset=0 ) const;

  int pos() const;
  int available() const;
  int length() const;

private:
  buffer_t<T> *_bufs;
  int _max_buffers;
  int _capacity;
};


class selector_t
{
private:
  selector_t & operator = ( const selector_t & );
  explicit selector_t( const selector_t & );

public:
  explicit selector_t() {}
  virtual ~selector_t() {}

  virtual void wake_on_readable( channel_t &channel, bool f );
  virtual void wake_on_writable( channel_t &channel, bool f );
  virtual void forget( channel_t &channel );

  virtual void do_select( int timeout_in_ms=0 );

  virtual bool is_readable( channel_t &channel );
  virtual bool is_writable( channel_t &channel );

};



class channel_t
{
private:
  channel_t & operator = ( const channel_t & );
  explicit channel_t( const channel_t & );

public:
  explicit channel_t() {}
  virtual ~channel_t() {}

  virtual void close() = 0;
  virtual bool is_open() = 0;
  virtual int get_fd() = 0;
};


class socket_channel_t
{
private:
  socket_channel_t & operator = ( const socket_channel_t & );
  explicit socket_channel_t( const channel_t & );

public:
  explicit socket_channel_t();
};

class driver_channel_t
{
private:
  driver_channel_t & operator = ( const driver_channel_t & );
  explicit driver_channel_t( const driver_t & );

public:
  explicit driver_channel_t();
};

END_NAMESPACE(nio);
END_NAMESPACE(jdk);

#endif
