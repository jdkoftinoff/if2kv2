#ifndef _IF2KKERNEL_H
#define _IF2KKERNEL_H

#ifdef WIN32
#ifdef IF2KKERNEL_EXPORTS
#define IF2KKERNEL_API __declspec(dllexport)
#else
#define IF2KKERNEL_API __declspec(dllimport)
#endif
#else
#define LPCSTR const char *
#define LPSTR char *
#define DWORD unsigned long
#define IF2KKERNEL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void * IF2K_URL_SCANNER_HANDLE;
typedef void * IF2K_PHRASE_SCANNER_HANDLE;


// definitions for callback procedures when a match is found

typedef DWORD (*IF2K_URL_MATCH_PROC)( 
  IF2K_URL_SCANNER_HANDLE h, 
  LPCSTR match, 
  DWORD match_len, 
  DWORD match_type, 
  LPCSTR *match_pos 
  );

typedef DWORD (*IF2K_PHRASE_MATCH_PROC)( 
  IF2K_PHRASE_SCANNER_HANDLE h, 
  LPCSTR match, 
  DWORD match_len, 
  DWORD match_type, 
  LPCSTR *match_pos 
  );


// functions to precompile phrase and url lists

IF2KKERNEL_API 
DWORD if2k_precompile_url( 
  LPCSTR input_file_name, 
  LPCSTR output_file_name 
  );

IF2KKERNEL_API 
DWORD if2k_precompile_phrase( 
  LPCSTR input_file_name, 
  LPCSTR output_file_name 
  );


// The URL scanner

IF2KKERNEL_API 
IF2K_URL_SCANNER_HANDLE if2k_url_scanner_init( 
  LPCSTR precompiled_file_name,
  LPCSTR noncompiled_file_name,
  DWORD extra
  );

IF2KKERNEL_API 
DWORD if2k_url_scanner_extra( 
  IF2K_URL_SCANNER_HANDLE h
  );

IF2KKERNEL_API 
DWORD if2k_url_scanner_scan( 
  IF2K_URL_SCANNER_HANDLE h, 
  LPCSTR data,
  DWORD data_len,
  IF2K_URL_MATCH_PROC callback
  );

IF2KKERNEL_API 
DWORD if2k_url_scanner_censor( 
  IF2K_URL_SCANNER_HANDLE h, 
  LPCSTR data,
  DWORD data_len,
  IF2K_URL_MATCH_PROC callback,
  char replacement
  );

IF2KKERNEL_API 
void if2k_url_scanner_free( 
  IF2K_URL_SCANNER_HANDLE h 
  );


// The Phrase Scanner

IF2KKERNEL_API 
IF2K_PHRASE_SCANNER_HANDLE if2k_phrase_scanner_init( 
  LPCSTR precompiled_file_name, 
  LPCSTR noncompiled_file_name,
  DWORD extra
  );

IF2KKERNEL_API 
DWORD if2k_phrase_scanner_extra( 
  IF2K_PHRASE_SCANNER_HANDLE h
  );


IF2KKERNEL_API 
DWORD if2k_phrase_scanner_scan( 
  IF2K_PHRASE_SCANNER_HANDLE h, 
  LPCSTR data,
  DWORD data_len,
  IF2K_PHRASE_MATCH_PROC callback
  );

IF2KKERNEL_API 
DWORD if2k_phrase_scanner_censor( 
  IF2K_PHRASE_SCANNER_HANDLE h, 
  LPCSTR data,
  DWORD data_len,
  IF2K_PHRASE_MATCH_PROC callback,
  char replacement
  );

IF2KKERNEL_API 
void if2k_phrase_scanner_free( 
  IF2K_PHRASE_SCANNER_HANDLE h 
  );


#ifdef __cplusplus
}
#endif

#endif
