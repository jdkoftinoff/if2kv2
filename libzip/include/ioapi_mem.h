#ifndef _ZLIBIOAPI_MEM_H
#define _ZLIBIOAPI_MEM_H

#include "ioapi.h"

#ifdef __cplusplus
extern "C" {
#endif

  ZEXTERN void* mem_simple_create_file(zlib_filefunc_def* api, void* buffer, size_t buf_len);

#ifdef __cplusplus
}
#endif

#endif

