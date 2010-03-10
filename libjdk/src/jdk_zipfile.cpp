#include "jdk_world.h"

#if JDK_HAS_ZIP
#include "jdk_zipfile.h"
#include "jdk_buf.h"
#include "zlib.h"
#include "zip.h"
#include "unzip.h"
#include "ioapi.h"
#include "ioapi_mem.h"

static voidpf ZCALLBACK jdkbuf_open (
   voidpf opaque,
   const char* filename,
   int mode);

static uLong ZCALLBACK jdkbuf_read (
   voidpf opaque,
   voidpf stream,
   void* buf,
   uLong size);

static uLong ZCALLBACK jdkbuf_write (
   voidpf opaque,
   voidpf stream,
   const void* buf,
   uLong size);

static long ZCALLBACK jdkbuf_tell (
   voidpf opaque,
   voidpf stream);

static long ZCALLBACK jdkbuf_seek (
   voidpf opaque,
   voidpf stream,
   uLong offset,
   int origin);

static int ZCALLBACK jdkbuf_close (
   voidpf opaque,
   voidpf stream);

static int ZCALLBACK jdkbuf_error (
   voidpf opaque,
   voidpf stream);

struct JDKBUFFILE
{
  JDKBUFFILE( jdk_buf &b ) : buf(b), position(0) {}
  jdk_buf &buf;
  long position; /* Current offset in the area */
};



static unsigned long ZCALLBACK jdkbuf_read(
  voidpf opaque,
  voidpf stream,
  void* buf,
  unsigned long size
  )
{
   JDKBUFFILE* handle = (JDKBUFFILE*) stream;

   if ( (handle->position + size) > (unsigned long)handle->buf.get_data_length())
   {
      size = handle->buf.get_data_length() - handle->position;
   }
   memcpy(buf, ((char*)handle->buf.get_data()) + handle->position, size);
   handle->position+=size;
   return size;
}

static unsigned long ZCALLBACK jdkbuf_write (
  voidpf opaque,
  voidpf stream,
  const void* buf,
  unsigned long size
  )
{
  JDKBUFFILE* handle = (JDKBUFFILE*) stream;
  unsigned long newsize = handle->position + size;
  unsigned long cursize = long(handle->buf.get_buf_length());
  if ( newsize > cursize )
  {
    handle->buf.expand( newsize );
  }
  
  memcpy(((char*)handle->buf.get_data()) + handle->position, buf, size);
  handle->buf.set_data_length( newsize );
  handle->position+=size;
  
  return size;
}

static long ZCALLBACK jdkbuf_tell (
  voidpf opaque,
  voidpf stream
  )
{
   JDKBUFFILE *handle = (JDKBUFFILE *)stream;
   return handle->position;
}

static long ZCALLBACK jdkbuf_seek (
  voidpf opaque,
  voidpf stream,
  unsigned long offset,
  int origin
  )
{
   JDKBUFFILE* handle = (JDKBUFFILE*)stream;
   long new_pos=handle->position;

   switch( origin )
   {
   case ZLIB_FILEFUNC_SEEK_CUR:
     new_pos += offset;
     break;
   case ZLIB_FILEFUNC_SEEK_END:
     new_pos = handle->buf.get_data_length() + offset;
     break;
   case ZLIB_FILEFUNC_SEEK_SET:
     new_pos = offset;
     break;
   }

   if( new_pos<0 || new_pos>handle->buf.get_data_length() )
   {
     return -1;
   }
   else
   {
     handle->position = new_pos;
     return 0;
   }
}

static int ZCALLBACK jdkbuf_close (
  voidpf opaque,
  voidpf stream
  )
{
    JDKBUFFILE *handle = (JDKBUFFILE *)stream;

    /* Note that once we've written to the buffer we don't tell anyone
       about it here. Probably the opaque handle could be used to inform
       some other component of how much data was written.

       This, and other aspects of writing through this interface, has
       not been tested.
     */

    delete handle;
    return 0;
}

static int ZCALLBACK jdkbuf_error (
  voidpf opaque,
  voidpf stream
  )
{
    /*    JDKBUFFILE *handle = (JDKBUFFILE *)stream; */
    /* We never return errors */
    return 0;
}

static voidpf ZCALLBACK jdkbuf_open(
  voidpf opaque,
  const char* filename,
  int mode
  )
{
  return opaque;
}

void* jdkbuf_simple_create_file(zlib_filefunc_def* api, jdk_buf &buf )
{
  JDKBUFFILE* handle = new JDKBUFFILE( buf );
  api->zopen_file  = jdkbuf_open;
  api->zread_file  = jdkbuf_read;
  api->zwrite_file = jdkbuf_write;
  api->ztell_file  = jdkbuf_tell;
  api->zseek_file  = jdkbuf_seek;
  api->zclose_file = jdkbuf_close;
  api->zerror_file = jdkbuf_error;
  api->opaque      = handle;
  handle->position = 0;
  return handle;
}


struct zlib_zip_pimpl_t
{
  zipFile zf;
  void * handle;
  zlib_filefunc_def funcs;
};

struct zlib_unzip_pimpl_t
{
  unzFile uf;
  void *handle;
  zlib_filefunc_def funcs;
  unz_global_info global_info;
};


jdk_zipfile_reader::jdk_zipfile_reader( const jdk_buf &buf_ )
  : pImpl(0), buf(buf_)
{
  zlib_unzip_pimpl_t *z = (zlib_unzip_pimpl_t *)new zlib_unzip_pimpl_t;
  pImpl = (void *)z;

  z->handle = jdkbuf_simple_create_file( &z->funcs, (jdk_buf&)buf );

  z->uf = unzOpen2( "", &z->funcs );
  unzGetGlobalInfo( z->uf, &z->global_info );
}

jdk_zipfile_reader::~jdk_zipfile_reader()
{
  zlib_unzip_pimpl_t *z = (zlib_unzip_pimpl_t *)pImpl;
  unzClose( z->uf );
  delete z;
}

int jdk_zipfile_reader::count() const
{
  zlib_unzip_pimpl_t *z = (zlib_unzip_pimpl_t *)pImpl;
  return z->global_info.number_entry; 
}


bool jdk_zipfile_reader::get_filename( int filenum, jdk_string_filename &f ) const
{
  zlib_unzip_pimpl_t *z = (zlib_unzip_pimpl_t *)pImpl;
  unzGoToFirstFile( z->uf );
  for( int i=0; i<filenum; ++i )
  {
    if( unzGoToNextFile( z->uf )!=UNZ_OK )
      return false;
  }
  
  if( unzGetCurrentFileInfo( 
        z->uf, 
        0, // unz_file_info *
        f.c_str(),
        f.getmaxlen(),
        0, // extraField buffer
        0, // extraField buffer size
        0, // comment buffer
        0  // comment buffer size
        ) != UNZ_OK )
  {
    return false;
  }
        
  return true;
}

bool jdk_zipfile_reader::extract_to_buf( const jdk_string_filename &file_in_zip, jdk_buf &result ) const
{
  bool r = false;
  zlib_unzip_pimpl_t *z = (zlib_unzip_pimpl_t *)pImpl;

  if( unzLocateFile( z->uf, file_in_zip.c_str(), 1 )==UNZ_OK )
  {
    unz_file_info file_info;

    if( unzGetCurrentFileInfo( 
          z->uf, 
          &file_info, // unz_file_info *
          0,
          0,
          0, // extraField buffer
          0, // extraField buffer size
          0, // comment buffer
          0  // comment buffer size
          ) == UNZ_OK )
    {
      if( unzOpenCurrentFile( z->uf ) == UNZ_OK )
      {
        result.set_data_length(0);
        if( result.resize( file_info.uncompressed_size ) )
        {
          if( unzReadCurrentFile( z->uf, result.get_data(), file_info.uncompressed_size ) == UNZ_OK )
          {
            result.set_data_length( file_info.uncompressed_size );
            r=true;
          }
        }
        unzCloseCurrentFile( z->uf );
      }
      
    }
    
  }
  
  return r;
}

bool jdk_zipfile_reader::extract_to_file( const jdk_string_filename &file_in_zip, const jdk_string_filename &result_filename ) const
{
  bool r=false;
  jdk_dynbuf result;
  if( extract_to_buf( file_in_zip, result ) )
  {
    r=result.extract_to_file( result_filename );
  }
  return r;
}




jdk_zipfile_writer::jdk_zipfile_writer( jdk_buf &buf_ )
  : pImpl(0), buf(buf_)
{
  zlib_zip_pimpl_t *z = new zlib_zip_pimpl_t;
  pImpl = (void *)z;
  z->handle = jdkbuf_simple_create_file( &z->funcs, buf_ );
  z->zf = zipOpen2( "", APPEND_STATUS_CREATE, 0, &z->funcs );

}

jdk_zipfile_writer::~jdk_zipfile_writer()
{
  zlib_zip_pimpl_t *z = (zlib_zip_pimpl_t *)pImpl;
  zipClose( z->zf, "" );
  delete z;
}

bool jdk_zipfile_writer::add_from_buf( const jdk_string_filename &file_in_zip, const jdk_buf &file_buf )
{
  zlib_zip_pimpl_t *z = (zlib_zip_pimpl_t *)pImpl;
  zip_fileinfo zipfi;

  zipfi.tmz_date.tm_sec = zipfi.tmz_date.tm_min = zipfi.tmz_date.tm_hour =
    zipfi.tmz_date.tm_mday = zipfi.tmz_date.tm_mon = zipfi.tmz_date.tm_year = 0;
  zipfi.dosDate = 0;
  zipfi.internal_fa = 0;
  zipfi.external_fa = 0;

  time_t tc = time(0);
  struct tm *t = jdk_localtime( &tc );

  zipfi.tmz_date.tm_sec = t->tm_sec;
  zipfi.tmz_date.tm_min = t->tm_min;
  zipfi.tmz_date.tm_hour = t->tm_min;
  zipfi.tmz_date.tm_mday = t->tm_mday;
  zipfi.tmz_date.tm_mon = t->tm_mon;
  zipfi.tmz_date.tm_year = t->tm_year;

  zipOpenNewFileInZip(
    z->zf,
    file_in_zip.c_str(),
    &zipfi, // zip_fileinfo*
    0, // extrafield_local,
    0, //size_extrafield
    0, //extrafield_global,
    0, //size_extrafield_global
    0, //comment
    Z_DEFLATED,
    Z_DEFAULT_COMPRESSION
    );

  zipWriteInFileInZip( 
    z->zf,
    file_buf.get_data(),
    file_buf.get_data_length()
    );
  zipCloseFileInZip( z->zf );
    
  return false;
}

bool jdk_zipfile_writer::add_from_file( const jdk_string_filename &file_in_zip, const jdk_string_filename &source_filename )
{
  bool r=false;
  jdk_dynbuf f;
  if( f.append_from_file( source_filename ) )
  {
    r = add_from_buf( file_in_zip, f );
  }
  return r;
}



#endif
