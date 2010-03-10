#include "wnworld.h"
#include "wndelself.h"

char *wndelself_buf;
HMODULE wndelself_module;

int WNExitAndDeleteSelf()
{
	char    buf[MAX_PATH];
	
	wndelself_buf = buf;
	wndelself_module = GetModuleHandle(0);
	GetModuleFileName(wndelself_module, wndelself_buf, MAX_PATH);
	CloseHandle((HANDLE)4);
#if 0	
	asm(
"			movl     _wndelself_buf, %eax\n"
"           pushl    $0\n"
"           pushl    $0\n"
"		    pushl    %eax\n"
"			pushl    $_ExitProcess@4\n"
"			pushl    _wndelself_module\n"
"			pushl    $_DeleteFileA@4\n"
"			pushl    $_UnmapViewOfFile@4\n"
"			ret\n"
     );
#endif		   
	
	return 0;
}
