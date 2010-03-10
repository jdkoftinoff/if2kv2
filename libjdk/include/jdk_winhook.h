#ifndef _JDK_WINHOOK_H
#define _JDK_WINHOOK_H

#include "jdk_winver.h"

#ifndef JDKHOOKLOG
#define JDKHOOKLOG(a)
#endif

#define JDK_WORDSWAP(a)		((((a)&0xff)<<8) + (((a)&0xff00)>>8))

#define VERIFY_POINTER(P,LEN) ((!IsBadReadPtr((void *)(P), (LEN)) && !IsBadWritePtr((void*)(P),(LEN))))
#define VERIFY_POINTER1(P) VERIFY_POINTER((P),4)

class jdk_win_crit_block
{
public:
   jdk_win_crit_block( CRITICAL_SECTION *crit_ )
	   : crit( crit_ )
   {
	   EnterCriticalSection( crit );
   }
   ~jdk_win_crit_block()
   {
	   LeaveCriticalSection(crit);	   	   
   }
private:
   CRITICAL_SECTION *crit;
};

class jdk_win_patch
{
public:
   jdk_win_patch( 
				  const char *dllname, 
				  const char *funcname,
				  void *new_func_
				  )
	   :
   windows_version( jdk_win_getver() ),
   module_handle( LoadLibrary(dllname) ),
   proc_addr((unsigned char *)GetProcAddress( module_handle, funcname ) ),
   new_func( new_func_ )
   {
	   InitializeCriticalSection(&crit_original);
	   JDKHOOKLOG(("// Windows version code %d\n",(int)windows_version));	   
	   if( module_handle && proc_addr )
	   {
//		   if( show_info( "original" ) )
		   {
			   patch();
		   }		   
	   }
   }
   
   virtual ~jdk_win_patch()
   {
	   if( module_handle && proc_addr )
	   {
//		   show_info( "patched" );
		   if( unpatch() )
		   {
//			   show_info( "final" );
		   }
	   }
   }

protected:   
   jdk_win_ver windows_version;
   HMODULE module_handle;
   unsigned char *proc_addr;
   DWORD original_pattern[2];
   void *new_func;
   CRITICAL_SECTION crit_original;
   
//   bool show_info(const char *s);
   bool patch();
   bool unpatch();
   
   bool make_patchable();
   bool make_unpatchable();
};

/*
inline bool jdk_win_patch::show_info( const char *s )
{		
	JDKHOOKLOG(("// address of function is 0x%08lx\n", (DWORD)proc_addr ));
	JDKHOOKLOG(("// first 16 %s bytes are:\n\t.dc.b ", s ));
	for( int i=0; i<16; ++i )
	{
		JDKHOOKLOG(( "0x%02x", proc_addr[i] ));
		if( i!=15 )			
		{
			JDKHOOKLOG((","));
		}
	}
	
	volatile DWORD *d = (volatile DWORD *)proc_addr;
	JDKHOOKLOG(("\n// first 4 DWORDS are:\n\t.dc.l " ));
	JDKHOOKLOG(("0x%08lx,0x%08lx,0x%08lx,0x%08lx\n",d[0],d[1],d[2],d[3] ));
	return true;
}
*/

inline bool jdk_win_patch::make_patchable()
{	
	DWORD old_protect;
	DWORD new_protect = PAGE_EXECUTE_WRITECOPY;
	JDKHOOKLOG(("// About to set protect bits to: 0x%08lx\n", new_protect ));
	
	if( !VirtualProtect( proc_addr, 4096, new_protect, &old_protect ) )
	{
		JDKHOOKLOG(("// VirtualProtect %08lx failed\n", new_protect ));
		return false;
	}
	JDKHOOKLOG(("// Original protect bits: %08lx\n", old_protect ));
	JDKHOOKLOG(("// writable protect bits: %08lx\n", new_protect ));

	DWORD verify_protect;
	if( !VirtualProtect( proc_addr, 4096, new_protect, &verify_protect ) )
	{
		JDKHOOKLOG(("// VirtualProtect %08lx failed\n", new_protect ));
		return false;
	}
	JDKHOOKLOG(("// verified protect bits: %08lx\n", verify_protect ));
	
	return true;
}

inline bool jdk_win_patch::make_unpatchable()
{
	DWORD old_protect;
	DWORD orig_protect=PAGE_EXECUTE_READ;
	
	JDKHOOKLOG(("// About to set protect bits to: 0x%08lx\n", orig_protect ));
	if( !VirtualProtect( proc_addr, 4096, orig_protect, &old_protect ) )
	{
		JDKHOOKLOG(("// VirtualProtect 0x%08lx failed\n", orig_protect ));
		return false;
	}
	JDKHOOKLOG(("// writable protect bits: 0x%08lx\n", old_protect ));
	JDKHOOKLOG(("// normal protect bits: 0x%08lx\n", orig_protect ));

	DWORD verify_protect;
	if( !VirtualProtect( proc_addr, 4096, orig_protect, &verify_protect ) )
	{
		JDKHOOKLOG(("// VirtualProtect 0x%08lx failed\n", orig_protect ));
		return false;
	}
	JDKHOOKLOG(("// verified protect bits: 0x%08lx\n", verify_protect ));
	
	return true;
}


inline bool jdk_win_patch::patch()
{
	JDKHOOKLOG(("// About to Patch\n" ));	

	
	if( !make_patchable() )
	{
		return false;
	}
	
	volatile DWORD *d= (volatile DWORD *)(proc_addr);
	original_pattern[0] = d[0];
	original_pattern[1] = d[1];

	volatile DWORD *jmp_param= (volatile DWORD *)(proc_addr+2);
	proc_addr[0] = 0xff;  // long jump
	proc_addr[1] = 0x25;  // indirect
	*jmp_param = (DWORD)&new_func;

	JDKHOOKLOG(("// Patched.\n"));
	
	if( !make_unpatchable() )
	{
		return false;
	}
	return true;
}

inline bool jdk_win_patch::unpatch()
{	
	JDKHOOKLOG(("// About to Un-Patch\n" ));


	if( !make_patchable() )
	{
		return false;
	}
	
	volatile DWORD *d= (volatile DWORD *)proc_addr;
	
	d[0] = original_pattern[0];
	d[1] = original_pattern[1];

	JDKHOOKLOG(("// Un-Patched.\n"));	
	
	if( !make_unpatchable() )
	{
		return false;
	}

	return true;
}

class jdk_win_patch_connect : public jdk_win_patch
{
public:
   jdk_win_patch_connect()
	   : jdk_win_patch( "ws2_32.dll", "connect", (void *)first_stage )
	   {
		   singleton=this;
	   }

protected:
   virtual int my_connect( int a, struct sockaddr_in *b, int c ) = 0;
   
   static WINAPI int first_stage( int a, struct sockaddr_in *b, int c );   
   int call_original( int a, struct sockaddr_in *b, int c );
   static jdk_win_patch_connect *singleton;
};

#if JDK_WINHOOK_STATICS
jdk_win_patch_connect *jdk_win_patch_connect::singleton=0;
#endif

inline int jdk_win_patch_connect::call_original( int a, struct sockaddr_in *b, int c )
{
	int ret=-1;
	jdk_win_crit_block block( &singleton->crit_original );
	
	if( singleton->unpatch() )
	{
		int (WINAPI *original)(int,void *,int) = (int (WINAPI *)(int,void *,int)) singleton->proc_addr;
		ret = original( a,b,c );
		singleton->patch();
	}
	return ret;
}

inline WINAPI int jdk_win_patch_connect::first_stage( int a, struct sockaddr_in *b, int c )
{
	return singleton->my_connect(a,b,c);
}



#endif
