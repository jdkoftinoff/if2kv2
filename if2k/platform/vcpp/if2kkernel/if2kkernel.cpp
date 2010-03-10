// if2kkernel.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "if2kkernel.h"

#include "if2kkernel_internal.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif

// functions to precompile phrase and url lists


IF2KKERNEL_API 
DWORD if2k_precompile_url( 
  LPCSTR input_file_name, 
  LPCSTR output_file_name 
  )
{
  jdk_string_filename in( input_file_name ? input_file_name : "" );
  jdk_string_filename out( output_file_name ? output_file_name : "" );

  return if2k_mini_compile_url( in, out );
}



IF2KKERNEL_API 
DWORD if2k_precompile_phrase( 
  LPCSTR input_file_name, 
  LPCSTR output_file_name 
  )
{
  jdk_string_filename in( input_file_name ? input_file_name : "" );
  jdk_string_filename out( output_file_name ? output_file_name : "" );

  return if2k_mini_compile_alphanumeric( in, out );
}


// The URL scanner

IF2KKERNEL_API 
IF2K_URL_SCANNER_HANDLE if2k_url_scanner_init( 
  LPCSTR precompiled_file_name,
  LPCSTR noncompiled_file_name,
  DWORD extra
  )
{
  jdk_string_filename precomp_file( precompiled_file_name ? precompiled_file_name : "" );
  jdk_string_filename noncomp_file( noncompiled_file_name ? noncompiled_file_name : "" );

  return (IF2K_URL_SCANNER_HANDLE) new if2k_url_scanner( 
    precomp_file,
    noncomp_file, 
    extra 
    );
}


IF2KKERNEL_API 
DWORD if2k_url_scanner_extra( 
  IF2K_URL_SCANNER_HANDLE h
  )
{
  return (DWORD)( ((if2k_url_scanner *)h)->extra() );
}

IF2KKERNEL_API 
DWORD if2k_url_scanner_scan( 
  IF2K_URL_SCANNER_HANDLE h, 
  LPCSTR data,
  DWORD data_len,
  IF2K_URL_MATCH_PROC callback
  )
{
  return (DWORD)( ((if2k_url_scanner *)h)->scan( 
                    data, 
                    data_len, 
                    (IF2K_MATCH_PROC)callback
                    ));
}

IF2KKERNEL_API 
DWORD if2k_url_scanner_censor( 
  IF2K_URL_SCANNER_HANDLE h, 
  LPCSTR data,
  DWORD data_len,
  IF2K_URL_MATCH_PROC callback,
  char replacement
  )
{
  return (DWORD)( ((if2k_url_scanner *)h)->censor( 
                    data, 
                    data_len, 
                    (IF2K_MATCH_PROC)callback,
                    replacement
                    ));
}


IF2KKERNEL_API 
void if2k_url_scanner_free( 
  IF2K_URL_SCANNER_HANDLE h 
  )
{
  delete ((if2k_url_scanner *)h);
}


// The Phrase Scanner

IF2KKERNEL_API 
IF2K_PHRASE_SCANNER_HANDLE if2k_phrase_scanner_init( 
  LPCSTR precompiled_file_name, 
  LPCSTR noncompiled_file_name,
  DWORD extra
  )
{
  jdk_string_filename precomp_file( precompiled_file_name ? precompiled_file_name : "" );
  jdk_string_filename noncomp_file( noncompiled_file_name ? noncompiled_file_name : "" );

  return (IF2K_PHRASE_SCANNER_HANDLE) new if2k_phrase_scanner( 
    precomp_file,
    noncomp_file, 
    extra 
    );
}


IF2KKERNEL_API 
DWORD if2k_phrase_scanner_extra( 
  IF2K_PHRASE_SCANNER_HANDLE h
  )
{
  return (DWORD)( ((if2k_phrase_scanner *)h)->extra() );
}

IF2KKERNEL_API 
DWORD if2k_phrase_scanner_scan( 
  IF2K_URL_SCANNER_HANDLE h, 
  LPCSTR data,
  DWORD data_len,
  IF2K_PHRASE_MATCH_PROC callback
  )
{
  return (DWORD)( ((if2k_phrase_scanner *)h)->scan( 
                    data, 
                    data_len, 
                    (IF2K_MATCH_PROC)callback 
                    ));
}

IF2KKERNEL_API 
DWORD if2k_phrase_scanner_censor( 
  IF2K_URL_SCANNER_HANDLE h, 
  LPCSTR data,
  DWORD data_len,
  IF2K_PHRASE_MATCH_PROC callback,
  char replacement
  )
{
  return (DWORD)( ((if2k_phrase_scanner *)h)->censor( 
                    data, 
                    data_len, 
                    (IF2K_MATCH_PROC)callback,
                    replacement
                    ));
}

IF2KKERNEL_API 
void if2k_phrase_scanner_free( 
  IF2K_PHRASE_SCANNER_HANDLE h 
  )
{
  delete ((if2k_phrase_scanner *)h);
}



#ifdef __cplusplus
}
#endif


