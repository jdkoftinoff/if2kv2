#ifndef __JDK_ZIPFILE_H
#define __JDK_ZIPFILE_H

#include "jdk_string.h"

#if JDK_HAS_ZIP

class jdk_zipfile_reader;
class jdk_zipfile_writer;
class jdk_zipfile_enumerator;

#include "jdk_buf.h"
#include "jdk_dynbuf.h"

class jdk_zipfile_reader
{
  jdk_zipfile_reader( const jdk_zipfile_reader &o );
  jdk_zipfile_reader & operator = ( const jdk_zipfile_reader &o );
  
public:
  jdk_zipfile_reader( const jdk_buf &buf );
  virtual ~jdk_zipfile_reader();
  
  virtual int  count() const;
  virtual bool get_filename( int filenum, jdk_string_filename &f ) const;

  virtual bool extract_to_buf( const jdk_string_filename &file_in_zip, jdk_buf &result ) const;
  virtual bool extract_to_file( const jdk_string_filename &file_in_zip, const jdk_string_filename &result_filename ) const;

protected:
  void *pImpl;
  const jdk_buf &buf;
};


class jdk_zipfile_writer
{
  jdk_zipfile_writer( const jdk_zipfile_writer &o );
  jdk_zipfile_writer & operator = ( const jdk_zipfile_writer &o );
  
public:
  jdk_zipfile_writer( jdk_buf &buf );
  virtual ~jdk_zipfile_writer();
  virtual bool add_from_buf( const jdk_string_filename &file_in_zip, const jdk_buf &buf );
  virtual bool add_from_file( const jdk_string_filename &file_in_zip, const jdk_string_filename &source_filename );

private:
  void *pImpl;
  jdk_buf &buf;
};


class jdk_zipfile_enumerator
{
public:
  jdk_zipfile_enumerator( const jdk_zipfile_reader &z_ )
    : z( z_ ), cur_filenum( 0 )
    {
      ;
    }

  bool get_filename( jdk_string_filename &f ) const
    {
      bool r = false;
      if( cur_filenum < z.count() )
      {
        r = z.get_filename( cur_filenum, f );
      }
      return r;
    }

  bool next()
    {
      cur_filenum++;
      return cur_filenum<z.count();
    }

private:
  const jdk_zipfile_reader &z;
  int cur_filenum;
};

#endif

#endif
