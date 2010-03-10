/* ioapi_mem2.c -- IO base function header for compress/uncompress .zip
   files using zlib + zip or unzip API

   This version of ioapi is designed to access memory rather than files.
   We do use a region of memory to put data in to and take it out of. We do
   not have auto-extending buffers and do not inform anyone else that the
   data has been written. It is really intended for accessing a zip archive
   embedded in an application such that I can write an installer with no
   external files. Creation of archives has not been attempted, although
   parts of the framework are present.

   Based on Unzip ioapi.c version 0.22, May 19th, 2003

   Copyright (C) 1998-2003 Gilles Vollant
             (C) 2003 Justin Fletcher
   
   Dynamically allocated memory version. Troels K 2004
      mem_close deletes the data: file is single-session. No filenames.

   This file is under the same license as the Unzip tool it is distributed
   with.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlib.h"
#include "ioapi.h"
#include "ioapi_mem.h"

static voidpf ZCALLBACK mem_open OF((
   voidpf opaque,
   const char* filename,
   int mode));

static uLong ZCALLBACK mem_read OF((
   voidpf opaque,
   voidpf stream,
   void* buf,
   uLong size));

static uLong ZCALLBACK mem_write OF((
   voidpf opaque,
   voidpf stream,
   const void* buf,
   uLong size));

static long ZCALLBACK mem_tell OF((
   voidpf opaque,
   voidpf stream));

static long ZCALLBACK mem_seek OF((
   voidpf opaque,
   voidpf stream,
   uLong offset,
   int origin));

static int ZCALLBACK mem_close OF((
   voidpf opaque,
   voidpf stream));

static int ZCALLBACK mem_error OF((
   voidpf opaque,
   voidpf stream));

typedef struct _MEMFILE
{
  void* buffer;    /* Base of the region of memory we're using */
  long length;   /* Size of the region of memory we're using */
  long position; /* Current offset in the area */
} MEMFILE;



static uLong ZCALLBACK mem_read (opaque, stream, buf, size)
   voidpf opaque;
   voidpf stream;
   void* buf;
   uLong size;
{
   MEMFILE* handle = (MEMFILE*) stream;

   if ( (handle->position + size) > (uLong)handle->length)
   {
      size = handle->length - handle->position;
   }
   memcpy(buf, ((char*)handle->buffer) + handle->position, size);
   handle->position+=size;
   return size;
}

static uLong ZCALLBACK mem_write (opaque, stream, buf, size)
   voidpf opaque;
   voidpf stream;
   const void* buf;
   uLong size;
{
   MEMFILE* handle = (MEMFILE*) stream;

   if( handle->buffer==0 )
   {
     handle->buffer = malloc( size );
     handle->length = size;
   }
   if ((handle->position + size) > (uLong)handle->length)
   {
      handle->length = handle->position + size;
      handle->buffer = realloc(handle->buffer, handle->length);
   }

   memcpy(((char*)handle->buffer) + handle->position, buf, size);
   handle->position+=size;

   return size;
}

static long ZCALLBACK mem_tell (opaque, stream)
   voidpf opaque;
   voidpf stream;
{
   MEMFILE *handle = (MEMFILE *)stream;
   return handle->position;
}

static long ZCALLBACK mem_seek (opaque, stream, offset, origin)
   voidpf opaque;
   voidpf stream;
   uLong offset;
   int origin;
{
   MEMFILE* handle = (MEMFILE*)stream;
   long new_pos=handle->position;

   switch( origin )
   {
   case ZLIB_FILEFUNC_SEEK_CUR:
     new_pos += offset;
     break;
   case ZLIB_FILEFUNC_SEEK_END:
     new_pos = handle->length + offset;
     break;
   case ZLIB_FILEFUNC_SEEK_SET:
     new_pos = offset;
     break;
   }

   if( new_pos<0 || new_pos>=handle->length )
   {
     return -1;
   }
   else
   {
     handle->position = new_pos;
     return new_pos;
   }
}

static int ZCALLBACK mem_close (opaque, stream)
   voidpf opaque;
   voidpf stream;
{
    MEMFILE *handle = (MEMFILE *)stream;

    /* Note that once we've written to the buffer we don't tell anyone
       about it here. Probably the opaque handle could be used to inform
       some other component of how much data was written.

       This, and other aspects of writing through this interface, has
       not been tested.
     */

    free (handle->buffer);
    free (handle);
    return 0;
}

static int ZCALLBACK mem_error (opaque, stream)
   voidpf opaque;
   voidpf stream;
{
    /*    MEMFILE *handle = (MEMFILE *)stream; */
    /* We never return errors */
    return 0;
}

static voidpf ZCALLBACK mem_open(opaque, filename, mode)
  voidpf opaque;
  const char* filename;
  int mode;
{
  return opaque;
}

void* mem_simple_create_file(zlib_filefunc_def* api, void* buffer, size_t buf_len)
{
    MEMFILE* handle = malloc(sizeof(*handle));
    api->zopen_file  = mem_open;
    api->zread_file  = mem_read;
    api->zwrite_file = mem_write;
    api->ztell_file  = mem_tell;
    api->zseek_file  = mem_seek;
    api->zclose_file = mem_close;
    api->zerror_file = mem_error;
    api->opaque      = handle;
    handle->position = 0;
    handle->buffer   = buffer;
    handle->length   = buf_len;
    return handle;
}
