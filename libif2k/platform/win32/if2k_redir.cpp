#if JDK_IS_WIN32

#include <winsock2.h>
#include <windows.h>

#include "jdk_winver.h"
#include "jdk_ipv4.h"
#include "if2k_mini_config.h"


typedef LONG (WINAPI *WIN_REGCLOSEKEY_PROC)(HKEY);
typedef LONG (WINAPI *WIN_REGCREATEKEYEXA_PROC)(HKEY,LPCSTR,DWORD,LPSTR,DWORD,REGSAM,LPSECURITY_ATTRIBUTES,PHKEY,PDWORD);
typedef LONG (WINAPI *WIN_REGQUERYVALUEEXA_PROC)(HKEY,LPCSTR,PDWORD,PDWORD,LPBYTE,PDWORD);

typedef int (WINAPI *WS2_SEND_PROC)(SOCKET,const char *,int,int);
typedef int (WINAPI *WS2_IOCTLSOCKET_PROC)(SOCKET,long,u_long *);
typedef int (WINAPI *WS2_GETLASTERROR_PROC)(void);
typedef int (WINAPI *RealConnectProc)(int,void*,int);

#define VERIFY_POINTER(P,LEN) ((!IsBadReadPtr((void *)(P), (LEN)) && !IsBadWritePtr((void*)(P),(LEN))))
#define VERIFY_POINTER1(P) VERIFY_POINTER((P),4)

#define WORDSWAP(a)		((((a)&0xff)<<8) + (((a)&0xff00)>>8))
#define NUM_REDIRECTORS 8

struct ipv4_redirectors
{
  jdk_ipv4_ip_range *range;
	unsigned char dest_a,dest_b,dest_c,dest_d;
	int dest_port;
	bool do_prefix;
};

int load_settings();

bool private_jdk_strncpy( char *dest, const char *src, int maxlen );
inline int private_jdk_strlen( const char *p );
inline void private_jdk_memcpy( void *dest_, const void *src_, int len );

extern "C" int WINAPI my_connect(
						  int sock_fd, 
						  struct sockaddr_in *serv_addr,
						  int addrlen 
						  );

extern "C" void WINAPI simple_connect( DWORD *params );


bool is_fileshare_connection( unsigned char a, unsigned char b, unsigned char c, unsigned char d, int port );


HMODULE ws2_32_handle;

jdk_ipv4_ip_range *allow_range=0;
jdk_ipv4_ip_range *block_range=0;
DWORD block_fileshare=0;

ipv4_redirectors redirectors[NUM_REDIRECTORS] =
{
	{	0, 0,0,0,0, 0, false },
	{	0, 0,0,0,0, 0, false },
	{	0, 0,0,0,0, 0, false },	
	{	0, 0,0,0,0, 0, false },
	
	{	0, 0,0,0,0, 0, false },
	{	0, 0,0,0,0, 0, false },	
	{	0, 0,0,0,0, 0, false },
	{	0, 0,0,0,0, 0, false }	     
};

jdk_win_ver win_ver=jdk_win_unknown;

static CRITICAL_SECTION crit_load_settings;

bool settings_loaded=false;
int run_mode=0;

WS2_SEND_PROC ws2_send;
WS2_IOCTLSOCKET_PROC ws2_ioctlsocket;
WS2_GETLASTERROR_PROC ws2_GetLastError;

HMODULE advapi32_handle;
WIN_REGCLOSEKEY_PROC  win_RegCloseKey;
WIN_REGCREATEKEYEXA_PROC win_RegCreateKeyExA;
WIN_REGQUERYVALUEEXA_PROC	win_RegQueryValueExA;

unsigned char original_raw_code[8];

RealConnectProc do_real_connect = 0;

DWORD hook_connect_reentry=0;
DWORD hook_connect_entry=0;
DWORD simple_connect_hook=0;
DWORD enabled=1;



bool private_jdk_strncpy( char *dest, const char *src, int maxlen ) 
{
	int len=0;
	
	while( *src )
	{	
		if( len>=maxlen-1 )
		{
			*dest='\0';
			return false;
		}		
		
		*dest++ = *src++;
		len++;
	}
	*dest = '\0';
	
	return true;
} 


inline int private_jdk_strlen( const char *p )
{
	const char *q=p;
	while( *q )
		++q;
	return (int)(q-p);
}

inline void private_jdk_memcpy( void *dest_, const void *src_, int len )
{
	unsigned char *dest=(unsigned char *)dest_;
	const unsigned char *src = (const unsigned char *)src_;
	
	for( int i=0; i<len; ++i )
	{
		*dest++=*src++;
	}	
}


/* first 8 bytes of ws2_32.dll's connect() entry point (intel syntax)
 * for winNT: (crash at 0x776ba5ac)
 * 55 = push ebp 
 * 8b ec = mov ebp, esp 
 * 83 ec 14 = sub esp, 0x14
 * 53 = push ebx
 * 8d 45 ec = lea eax,0xffffffec(ebp)
 *
 * for win2000:
 * 55 = push ebp 
 * 8b ec = mov ebp, esp 
 * 83 ec 18 = sub esp, 0x18 
 * 53 = push ebx 
 * 56 = push esi (skip)
 * 
 * For WIN XP:
 *
 * 55 = push ebp
 * 8b ec = mov ebp, esp 
 * 83 ec 18 = sub esp, 0x18 
 * 57 = push edi 
 * 8d = SKIP THIS ONE
 *
 *
 * For WIN XP SP2:
 *
 * 
 * 8b ff = mov edi, edi
 * 55 = push ebp 
 * 8b ec = mov ebp,esp
 * 83 ec 18 = sub esp, 0x18
 * 57 = push edi
 * 8d = skip this one
 *
 *
 * For winME:
 *
 * 55 = push ebp
 * 8b ec = mov ebp, esp
 * 83 ec 10 = sub esp, 0x10
 * 8d 45 fc = lea eax,0xfffffffc(ebp)
 * 
 *
 *
 */
#if JDK_IS_VCPP || JDK_IS_VCPP8

__declspec (naked) void connect_hook_2000(int, void *, int)
{
	_asm
	{
		lea eax,4[esp]
		push eax
		call simple_connect
		push ebp
		mov ebp,esp
		sub esp,24
		push ebx
		push esi
		jmp [hook_connect_reentry]
	}
}
__declspec (naked) void do_real_connect_2000(DWORD,DWORD,DWORD)
{
	_asm
	{
		push ebp
		mov ebp,esp
		sub esp,24
		push ebx
		push esi
		jmp [hook_connect_reentry]
	}
}
__declspec (naked) void connect_hook_xp(int, void *, int)
{
	_asm
	{
		lea eax,4[esp]
		push eax
		call simple_connect
		push ebp
		mov ebp,esp
		sub esp,24
		push edi
		jmp [hook_connect_reentry]
	}
}
__declspec (naked) void do_real_connect_xp(DWORD,DWORD,DWORD)
{
	_asm
	{
		push ebp
		mov ebp,esp
		sub esp,24
		push edi
		jmp [hook_connect_reentry]
	}
}
__declspec (naked) void connect_hook_nt(int, void *, int)
{
	_asm
	{
		lea eax,4[esp]
		push eax
		call simple_connect
		push ebp
		mov ebp,esp
		sub esp,20
		push ebx
    lea eax,0xffffffec[ebp]
		jmp [hook_connect_reentry]
	}
}

__declspec (naked) void do_real_connect_nt(DWORD,DWORD,DWORD)
{
	_asm
	{
		push ebp
		mov ebp,esp
		sub esp,20
		push ebx
    lea eax,0xffffffec[ebp]
		jmp [hook_connect_reentry]
	}
}
__declspec (naked) void connect_hook_me(int, void *, int)
{
	_asm
	{
		lea eax,4[esp]
		push eax
		call simple_connect
		push ebp
		mov ebp,esp
		sub esp,16
    lea eax,0xffffffec[ebp]
		jmp [hook_connect_reentry]
	}
}
__declspec (naked) void do_real_connect_me(DWORD,DWORD,DWORD)
{
	_asm
	{
		push ebp
		mov ebp,esp
		sub esp,16
    lea eax,0xffffffec[ebp]
		jmp [hook_connect_reentry]
	}
}
__declspec (naked) void connect_hook_xpsp2(int, void *, int)
{
	_asm
	{
		lea eax,4[esp]
		push eax
		call simple_connect
    mov edi,edi
		push ebp
    mov ebp,esp
		sub esp,24
		push edi
		jmp [hook_connect_reentry]
	}
}
__declspec (naked) void do_real_connect_xpsp2(DWORD,DWORD,DWORD)
{
	_asm
	{
    mov edi,edi
		push ebp
    mov ebp,esp
		sub esp,24
		push edi
		jmp [hook_connect_reentry]
	}
}

#else
asm(
"   .text\n"
"_connect_hook_2000@12:\n"
"   lea 4(%esp),%eax\n"
"   pushl %eax\n"	
"   call _simple_connect\n"
"	pushl %ebp\n"
"   movl %esp,%ebp\n"
"   subl $0x18, %esp\n"
"   pushl %ebx\n"
"   pushl %esi\n"	
"   jmp *_hook_connect_reentry\n"
	);


asm(
"   .text\n"
"_connect_hook_nt@12:\n"	
"   lea 4(%esp),%eax\n"
"   pushl %eax\n"
"   call _simple_connect\n"
"	pushl %ebp\n"
"   movl %esp,%ebp\n"
"   subl $0x14, %esp\n"
"   pushl %ebx\n"
"   lea  0xffffffec(%ebp),%eax\n"	
"   jmp *_hook_connect_reentry\n"	
	);

asm(
"   .text\n"
"_connect_hook_me@12:\n"
"   lea 4(%esp),%eax\n"
"   pushl %eax\n"	
"   call _simple_connect\n"	
"	pushl %ebp\n"
"   movl %esp,%ebp\n"
"   subl $0x10, %esp\n"
"   lea  0xfffffffc(%ebp),%eax\n"
"   jmp *_hook_connect_reentry\n"
	);

asm(
"   .text\n"
"_connect_hook_xp@12:\n"
"   lea 4(%esp),%eax\n"
"   pushl %eax\n"		
"   call _simple_connect\n"	
"	pushl %ebp\n"
"   movl %esp,%ebp\n"
"   subl $0x18, %esp\n"
"   pushl %edi\n"
"   jmp *_hook_connect_reentry\n"
	);

asm(
"   .text\n"
"_connect_hook_xpsp2@12:\n"
"   lea 4(%esp),%eax\n"
"   pushl %eax\n"		
"   call _simple_connect\n"	
"	  movl %edi,%edi\n"
"   pushl %ebp\n"
"   subl $0x18, %esp\n"
"   pushl %edi\n"
"   jmp *_hook_connect_reentry\n"
	);


asm(
"   .text\n"
"_do_real_connect_2000@12:\n"
"	pushl %ebp\n"
"   movl %esp,%ebp\n"
"   subl $0x18, %esp\n"
"   pushl %ebx\n"
"   pushl %esi\n"	
"   jmp *_hook_connect_reentry\n"
	);


asm(
"   .text\n"
"_do_real_connect_nt@12:\n"
"	pushl %ebp\n"
"   movl %esp,%ebp\n"
"   subl $0x14, %esp\n"
"   pushl %ebx\n"
"   lea  0xffffffec(%ebp),%eax\n"	
"   jmp *_hook_connect_reentry\n"	
	);

asm(
"   .text\n"
"_do_real_connect_me@12:\n"
"	pushl %ebp\n"
"   movl %esp,%ebp\n"
"   subl $0x10, %esp\n"
"   lea  0xfffffffc(%ebp),%eax\n"
"   jmp *_hook_connect_reentry\n"
	);

asm(
"   .text\n"
"_do_real_connect_xp@12:\n"
"	pushl %ebp\n"
"   movl %esp,%ebp\n"
"   subl $0x18, %esp\n"
"   pushl %edi\n"
"   jmp *_hook_connect_reentry\n"
	);

asm(
"   .text\n"
"_do_real_connect_xpsp2@12:\n"
"	  movl %edi,%edi\n"
"   pushl %ebp\n"
"   movl %esp,%ebp\n"
"   subl $0x18, %esp\n"
"   pushl %edi\n"
"   jmp *_hook_connect_reentry\n"
	);
#endif


extern "C" void WINAPI simple_connect( DWORD *params )
{
    int sock_fd = int(params[0]);
	struct sockaddr_in *serv_addr = (struct sockaddr_in *) (params[1]);
	int addrlen = int(params[2]);
		
	load_settings();

	unsigned char *a= (&serv_addr->sin_addr.S_un.S_un_b.s_b1);
	unsigned char *b= (&serv_addr->sin_addr.S_un.S_un_b.s_b2);
	unsigned char *c= (&serv_addr->sin_addr.S_un.S_un_b.s_b3);
	unsigned char *d= (&serv_addr->sin_addr.S_un.S_un_b.s_b4);
	
	if( serv_addr && VERIFY_POINTER1(serv_addr) && serv_addr->sin_family==AF_INET )
	{		
		int port = WORDSWAP(serv_addr->sin_port);
				
		if( port>=65534 && *a==0 && *b==0 && *c==0 && *d==0 )
		{
			enabled=port&1;
		}
		else 
		if( enabled )
		{
			// is it not in the allow range?
			if( !(allow_range && allow_range->is_ip_in_range(*a,*b,*c,*d,port )) )
			{
				// is it in the block range?
				if( block_range && block_range->is_ip_in_range(*a,*b,*c,*d,port ) )
				{
					// yes, redirect to 0.0.0.0:0
					*a=0;
					*b=0;
					*c=0;
					*d=0;
					serv_addr->sin_port=WORDSWAP(0);					
				}
				else
				{
					// are we to redirect it?
					for( int i=0; i<NUM_REDIRECTORS; ++i )
					{
						if( redirectors[i].range && redirectors[i].range->is_ip_in_range(*a,*b,*c,*d,port) )
						{
							// found match.
							*a = redirectors[i].dest_a;
							*b = redirectors[i].dest_b;
							*c = redirectors[i].dest_c;
							*d = redirectors[i].dest_d;
							serv_addr->sin_port = WORDSWAP(redirectors[i].dest_port);
							break;
						}
					}
				}
			}			
		}
	}
		
}

extern "C" int WINAPI my_connect(
						  int sock_fd, 
						  struct sockaddr_in *serv_addr,
						  int addrlen 
						  )
{	
//	MessageBeep(0);	
//	load_settings();
	
	if( serv_addr && VERIFY_POINTER1(serv_addr) && serv_addr->sin_family==AF_INET )
	{
		
		int port = WORDSWAP(serv_addr->sin_port);
		
		unsigned char a=serv_addr->sin_addr.S_un.S_un_b.s_b1;
		unsigned char b=serv_addr->sin_addr.S_un.S_un_b.s_b2;
		unsigned char c=serv_addr->sin_addr.S_un.S_un_b.s_b3;
		unsigned char d=serv_addr->sin_addr.S_un.S_un_b.s_b4;
		
		if( port>=65534 && a==0 && b==0 && c==0 && d==0 )
		{
			enabled=port&1;
			return do_real_connect( sock_fd, serv_addr, addrlen );
		}
		

		if( !enabled )
		{
			return do_real_connect( sock_fd, serv_addr, addrlen );
		}
		
		  
		
		if( allow_range && allow_range->is_ip_in_range(a,b,c,d,port ) )
		{
			return do_real_connect( sock_fd, serv_addr, addrlen );
		}

		if( block_fileshare && is_fileshare_connection(a,b,c,d,port) )
		{
			serv_addr->sin_addr.S_un.S_un_b.s_b1 = 0;
			serv_addr->sin_addr.S_un.S_un_b.s_b2 = 0;
			serv_addr->sin_addr.S_un.S_un_b.s_b3 = 0;
			serv_addr->sin_addr.S_un.S_un_b.s_b4 = 0; 
			serv_addr->sin_port = WORDSWAP(0);		

			return do_real_connect( sock_fd, serv_addr, addrlen );
		}
		if( block_range && block_range->is_ip_in_range(a,b,c,d,port ) )
		{
			serv_addr->sin_addr.S_un.S_un_b.s_b1 = 0;
			serv_addr->sin_addr.S_un.S_un_b.s_b2 = 0;
			serv_addr->sin_addr.S_un.S_un_b.s_b3 = 0;
			serv_addr->sin_addr.S_un.S_un_b.s_b4 = 0; 
			serv_addr->sin_port = WORDSWAP(0);		

			return do_real_connect( sock_fd, serv_addr, addrlen );
		}

		for( int i=0; i<NUM_REDIRECTORS; ++i )
		{
			if( redirectors[i].range && redirectors[i].range->is_ip_in_range(a,b,c,d,port) )
			{
				serv_addr->sin_addr.S_un.S_un_b.s_b1 = redirectors[i].dest_a;
				serv_addr->sin_addr.S_un.S_un_b.s_b2 = redirectors[i].dest_b;
				serv_addr->sin_addr.S_un.S_un_b.s_b3 = redirectors[i].dest_c;
				serv_addr->sin_addr.S_un.S_un_b.s_b4 = redirectors[i].dest_d;
				serv_addr->sin_port = WORDSWAP(redirectors[i].dest_port);
				
				if( redirectors[i].do_prefix )
				{
					// call do_real_connect
					int real_connect_ret;
					real_connect_ret = do_real_connect( sock_fd, serv_addr, addrlen );
					
					// upon success of connect, need to socket write 16 byte header which holds original dest ip and port
					int e=ws2_GetLastError();
					if( real_connect_ret==0 || (real_connect_ret==SOCKET_ERROR && e==WSAEWOULDBLOCK) )
					{
						unsigned char prefix[16];
						for( int i=0; i<16; ++i )
						{
							prefix[i]=0;	
						}
						
						prefix[1] = a;
						prefix[2] = b;
						prefix[3] = c;
						prefix[4] = d;
						prefix[5] = (port&0xff00)>>8;
						prefix[6] = (port&0xff);

						int tosend=16;
						int pos=0;
						int repeat_count=30; // 3 seconds
						
						while(tosend>0)
						{
							int send_count = ws2_send(sock_fd,(char *)prefix, tosend, pos );
							e=ws2_GetLastError();
							
							if( send_count<0 && e!=WSAEWOULDBLOCK && e!=WSAENOTCONN )
							{
								real_connect_ret=-1;
								break;
							}
							if( send_count<=0 )
							{
								send_count=0;
								// just keep hammering every 100 ms!!!
								Sleep(100);	
								if( --repeat_count==0 )
								{
									real_connect_ret=-1;
									break;	
								}
							}
							
							pos+=send_count;
							tosend-=send_count;
						}
					}
					//  return real result from do_real_connect
					return real_connect_ret;
				}
				else
				{
					// just rewrite destination call do_real_connect and return
					return do_real_connect( sock_fd, serv_addr, addrlen );
				}
				
			}			
			
		}
		
	}
	return do_real_connect( sock_fd, serv_addr, addrlen );
}

/*
here are around 130 FILE-SHARING Applications, all of which are readily
available on the Internet.

- Most file-sharing applications are designed to utilise maximum
bandwidth to the detriment of all other    users
- Most file-sharing applications allow downloading/sharing of any file
types, not just mp3, avi etc
- Most file-sharing applications have built in or available CHAT
programs/Chat-Rooms
- Many file-sharing applications can circumvent "normal" firewalls due
to the "user" being able to configure such actions 

I have attached a list of ports & servers + comments on fixing this
problem. 

For schools who have their own UNIX or LINUX Servers, setting the
UNIX/LINUX server as the gateway instead of the CISCO, and then
implementing Firewall Rules (using IPCHAINS, IPF, IPFW) and utilising
the following information will kill off most of these bandwidth hogs...
alternatively SINA/TELSTRA could configure the CISCO's to block all the
following IP RANGES & PORTS:

SERVER IP RANGES TO BLOCK:
--------
MORPHEUS: 206.142.53.0/24

NAPSTER: 208.195.149.0/24 , 64.124.41.0/24, 208.184.216.223/24

WINMX: 209.61.186.0/24 , 64.49.201.0/24

AUDIO GALAXY:64.245.58.0/23

NAPIGATOR:209.25.178.0/24

AIMSTER: 205.188.0.0/16 TCP 53

IMESH: 216.35.208.0/24

SPECIFIC ORTS TO BLOCK:
------

MORPHEUS: 1214 

NAPSTER: 6699, 8888, 8875 

EDONKEY: 4661, 4662, 4663, 4664, 4665

GNUTELLA: 6345, 6346, 6347,6348,6349
(6345-6349 will also block BearShare, ToadNode, Limewire, Gnucleus)

AUDIO GALAXY: 41000-42000

AIMSTER(AOL): 5190

NB#1: ON MANY OF THE ABOVE LISTED PORTS, THE PROTOCOL USED IS HTTP. 
NB#2: TCP/UDP SHOULD BE BLOCKED ON THESE PORTS

SQUID ACL BLOCKS: 
-----------------
GNUTELLA AIMSTER MORPHEUS KAZAA NAPSTER BEAR LIMEWIRE EDONKEY WINMX
IMESH AOL BEARSHARE KAZAACLIENT AUDIOGALAXY BODETELLA BUDDYSHARE CUTEMX
DATASCOURGE DIRECTCONNECT NEOMODUS NEO-MODUS DUODATA FDM FILECAT
FILEFREEDOM FILENAVIGATOR FILETOPIA FLYCODE FREEBASE GNEWTELLA GNOTELLA
GNOLIFE GNUCLEUS  GROKSTER
WRAPSTER YOINK SWAPOO SWAPTOR SWOPTOR TOADNODE TRIPNOSIS SONGSPY
SNIPSNARE SNIPSNAP SPINFRENZY SXCLIENT RIFFSHARE OHAHA ONSHARE OPENNAP
OPENAP PLANETILES RAPIGATOR NAPQUIK NAPQUICK NEWTELLA MP3XCHANGER
MP3EXCHANGER MYNAPSTER MYTELLA N-TELLA NAPASSIST MEDIASHARE MOJONATION
GNUCLEUS IASSIMILATOR GROOVE


SOURCE SITES for FILE SHARING APPS: 
-----------------------------------
bearshare.com  audiogalaxy.com  aimster.com buddyshare.com cutemx.com
datascourge.com neo-modus.com duodate.com  slaughterhouse.com
filecat.com filefreedom.com filetopia.com flycode.com oreality.com
gnotella.nerdherd.com gnutella.wego.com grokster.com spinfrenzy.com
riffshare.com ohaha.com onshare.com
planetfiles.com rapigator.com newtella.com mojonation.com mp3sharing.com
groove.net

NB: SINA SHOULD PROBABLY BE BLOCKING ALL OF THESE SITES!!!!

COMMENTS:
#1: AIMSTER is an AOL FILE SHARING APP WHICH CAN USE OTHER PORTS &
PROTOCOLS INCLUDING NON-HTTP TRAFFIC ON PORT 80. TO STOP AOL/AIMSTER
TOTALLY PROBABLY REQUIRES TOTAL BLOCKING OF ALL AOL WEBSITES +
IMPLEMENTATION OF PORT 80 NON-HTTP BLOCKS (REFER #2 HEREUNDER).

#2: BLOCKING OF ALL NON-HTTP TRAFFIC ON PORT 80 WILL REDUCE
SIGNIFICANTLY, THE FUNCTIONALITY OF MANY FILE SHARING APPS & TUNNELINIG
APPLICATIONS WHICH USE PORT 80 IN AN ATTEMPT TO AVOID
DETECTION/BLOCKING.

#3: BLOCKING OF HTTP TRAFFIC ON PORTS OTHER THAT 80, 3128, 8080, 8000,
443 ETC WILL ALSO BLOCK MANY FILE SHARING APPS. ALSO, BLOCKING OF PORTS
SHOULD BE TCP & UDP             

OK.... thats probably generated a serious case of Information Overload
;-)

PHIL. 
*/

bool is_fileshare_connection( unsigned char a, unsigned char b, unsigned char c, unsigned char d, int port )
{
	// morpheus?
	if( (a==206 && b==142 && c==53) || port==1214  )
		return true;

	// napster?
	if( (a==208 && b==195 && c==149) || port==6699 || port==8888 || port==8875 )
		return true;

	if( ( a==64 && b==124 && c==41) )
		return true;

	if( a==208 && b==184 && c==216 )
		return true;

	// edonky ports
	if( port>=4661 && port<=4665 )
		return true;

	// gnutella and family ports
	if( port>=6345 && port<=6349 )
		return true;

	// winmx?
	if( a==209 && b==61 && c==186 )
		return true;

	// audio galaxy?
	if( (a==64 && b==245 && (c&0xfe)==58) || (port>=41000 && port<=41000) )
		return true;
	
	// napigator
	if( a==209 && b ==25 && c==178 )
		return true;

	// aimster
	if( a==205 && b==188 && port==53 )
		return true;

	if( port==5190 )
		return true;

	// imesh
	if( a==216 && b==35 && c==208 )
		return true;

	return false;
}



#ifdef DEBUG_REDIR
extern "C" WINAPI int dummy_call()
{
	return my_connect(0,0,0);
}

#endif

#ifndef DEBUG_REDIR
int jdk_log( int level, const char *fmt, ... ) 
{
  	return 0;
}

void *operator new(size_t sz)
{
	return (void *)LocalAlloc(LPTR,sz);
}

void operator delete( void *p )
{
	LocalFree((HLOCAL)p);
}

void *operator new[]( size_t sz )
{
	return (void *)LocalAlloc(LPTR,sz);
}

void operator delete[]( void *p )
{
	LocalFree((HLOCAL)p);
}		
#endif



bool registry_get_string( HKEY area, const char *keyname, const char *field, char *result, int max_len )
{
    HKEY key;
    DWORD dummy;
    DWORD e;

    if( (e=win_RegCreateKeyExA(
        area,
        keyname,
        0,
        "",
        0,
        KEY_READ,
        0,
        &key,
        &dummy
        ))!=0 )
    {
#ifdef DEBUG_REDIR_MSG
        LPVOID lpMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            e,
            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
        MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK );
        LocalFree( lpMsgBuf);
#endif
        return false;
    }

	DWORD size=max_len;
	DWORD type;

	if( win_RegQueryValueExA(
			key,
			field,
			0,
			&type,
			(LPBYTE)result,
			&size
		)!=ERROR_SUCCESS || type!=REG_SZ )
	{
        win_RegCloseKey(key);

        return false;
    }
    win_RegCloseKey(key);

    return true;
}

bool registry_get_dword( HKEY area, const char *keyname, const char *field, DWORD *result )
{
    HKEY key;
    DWORD dummy;
    DWORD e;

    if( (e=win_RegCreateKeyExA(
        area,
        keyname,
        0,
        "",
        0,
        KEY_READ,
        0,
        &key,
        &dummy
        ))!=0 )
    {
#ifdef DEBUG_REDIR_MSG
        LPVOID lpMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            e,
            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
        MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK );
        LocalFree( lpMsgBuf);
#endif
        return false;
    }

	DWORD size=sizeof(DWORD);
	DWORD type;

	if( win_RegQueryValueExA(
			key,
			field,
			0,
			&type,
			(LPBYTE)result,
			&size
		)!=ERROR_SUCCESS || type!=REG_DWORD )
	{
        win_RegCloseKey(key);

        return false;

    }
    win_RegCloseKey(key);

    return true;
}




int patch_connect()
{
#ifdef DEBUG_REDIR_MSG
	char str[2048];
#else	
	InitializeCriticalSection(&crit_load_settings);	
#endif	

//	DWORD user_name_len = sizeof( user_name );
//	GetUserNameA( user_name, &user_name_len );

	advapi32_handle = LoadLibrary( "ADVAPI32.DLL" );
	if( !advapi32_handle )
	{
#ifdef DEBUG_REDIR_MSG
		MessageBox( 0, "No Module handle for advapi32.dll", "error", MB_OK );
#endif				
		return 0;
	}
	
	win_RegCloseKey=(WIN_REGCLOSEKEY_PROC)GetProcAddress(advapi32_handle,"RegCloseKey");
	win_RegCreateKeyExA=(WIN_REGCREATEKEYEXA_PROC)GetProcAddress(advapi32_handle,"RegCreateKeyExA");
	win_RegQueryValueExA=(WIN_REGQUERYVALUEEXA_PROC)GetProcAddress(advapi32_handle,"RegQueryValueExA");
	
	if( !win_RegQueryValueExA || !win_RegCloseKey || !win_RegQueryValueExA )
	{
#ifdef DEBUG_REDIR_MSG
		MessageBox( 0, "No procs for advapi32.dll", "error", MB_OK );
#endif				
		return 0;
		
	}
	
	hook_connect_entry=(DWORD)my_connect;
	ws2_32_handle = LoadLibrary( "ws2_32.dll" );
	HMODULE m = ws2_32_handle;
	if( !m )
	{		
#ifdef DEBUG_REDIR_MSG
		MessageBox( 0, "No Module handle for ws2_32.dll", "error", MB_OK );
#endif		
		return 0;
	}
	ws2_send=(WS2_SEND_PROC)GetProcAddress(m,"send");
	ws2_ioctlsocket=(WS2_IOCTLSOCKET_PROC)GetProcAddress(m,"ioctlsocket");
	ws2_GetLastError=(WS2_GETLASTERROR_PROC)GetProcAddress(m,"WSAGetLastError");
	
	
	volatile FARPROC cptr = GetProcAddress( m, "connect" );
	volatile unsigned char *data = (volatile unsigned char *)cptr;
	
	if( !cptr )
	{
#ifdef DEBUG_REDIR_MSG
		MessageBox( 0, "no address for connect", "error", MB_OK  );
#endif		
		return 0;
	}
#ifdef DEBUG_REDIR_MSG	
	sprintf( str, "address of connect is 0x%08lx", (DWORD)data );
	MessageBox( 0, str, "info", MB_OK );	
#endif	
	
	if(	data[0] == 0xff ) // is it patched already?
	{
#ifdef DEBUG_REDIR_MSG	
	sprintf( str, "Already patched!" );
	MessageBox( 0, str, "info", MB_OK );	
#endif			
		return 0;
	}

	win_ver = jdk_win_getver();
	
	DWORD oldProtect;
	if( !VirtualProtect( (void *)cptr, 4096, PAGE_EXECUTE_WRITECOPY, &oldProtect ) )
	{
#ifdef DEBUG_REDIR_MSG
		MessageBox( 0, "First VirtualProtect failed", "error", MB_OK );
#endif		
		if( !VirtualProtect( (void *)cptr, 4096, PAGE_READWRITE, &oldProtect ) )
		{
#ifdef DEBUG_REDIR_MSG
			MessageBox( 0, "Second VirtualProtect failed", "error", MB_OK );
#endif			
			return 0;
		}
	}


	switch( win_ver )
	{
	case jdk_win_unknown:
		return 0;
	case jdk_win_95:
	case jdk_win_98:
	case jdk_win_me:
		return 0;
	case jdk_win_nt4:
		{
			hook_connect_reentry = ((DWORD)(data))+10;
			do_real_connect = (RealConnectProc)do_real_connect_nt;
			simple_connect_hook = (DWORD)connect_hook_nt;
		}
		break;
	case jdk_win_2000:
		{
			hook_connect_reentry = ((DWORD)(data))+8;
			do_real_connect = (RealConnectProc)do_real_connect_2000;	
			simple_connect_hook = (DWORD)connect_hook_2000;		
		}
		break;

	case jdk_win_xp:
		{
			hook_connect_reentry = ((DWORD)(data))+7;
			do_real_connect = (RealConnectProc)do_real_connect_xp;
			simple_connect_hook = (DWORD)connect_hook_xp;				
      break;
		}
	case jdk_win_xpsp2:
		{
			hook_connect_reentry = ((DWORD)(data))+9;
			do_real_connect = (RealConnectProc)do_real_connect_xpsp2;
			simple_connect_hook = (DWORD)connect_hook_xpsp2;				
      break;
		}
		break;
	case jdk_win_server_2003:
    {
      ;
    }
	default:
    {
      ;
    }
	}

#ifdef DEBUG_REDIR_MSG
	sprintf( str,"reentry=%08lx", hook_connect_reentry );

	MessageBox(0,str,"INFO",MB_OK );
	
	sprintf( str, "original: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
				data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9]
			);
	MessageBox( 0, str, "info", MB_OK );
#endif		

	for( int i=0; i<8; ++i )
	{		
	 	original_raw_code[8] = data[i];
	}
	
#if 1
	data[0] = 0xff; /* long jump, indirect */
	data[1] = 0x25;
	DWORD *jmpaddr = (DWORD *)(data+2);
//    *jmpaddr = (DWORD)&hook_connect_entry;
    *jmpaddr = (DWORD)&simple_connect_hook;
#endif	
#ifdef DEBUG_REDIR_MSG
		sprintf( str, "new: %02x %02x %02x %02x %02x %02x %02x %02x",
				data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7] 
			);
	MessageBox( 0, str, "info", MB_OK );
#endif					
	
	DWORD other;
	if( !VirtualProtect( (void *)cptr, 4096, PAGE_EXECUTE_READ, &other ) )
	{
#ifdef DEBUG_REDIR_MSG
			MessageBox( 0, "SecondVirtualProtect failed", "error", MB_OK );
#endif		
		return 0;
	}
	
	
	return 1;	
}


int unpatch_connect()
{
	//char str[2048];
	
	HMODULE m = ws2_32_handle;
	if( !m )
	{
		//MessageBox( 0, "No Module handle for ws2_32.dll", "error", MB_OK );
		return 0;
	}
	
	volatile FARPROC cptr = GetProcAddress( m, "connect" );
	volatile unsigned char *data = (volatile unsigned char *)cptr;
	
	if( !cptr )
	{
		//MessageBox( 0, "no address for connect", "error", MB_OK  );
		return 0;
	}
	//sprintf( str, "address of connect is 0x%08lx", (DWORD)data );
	//MessageBox( 0, str, "info", MB_OK );
	
	DWORD oldProtect;
	if( !VirtualProtect( (void *)cptr, 4096, PAGE_EXECUTE_WRITECOPY, &oldProtect ) )
	{
		if( !VirtualProtect( (void *)cptr, 4096, PAGE_READWRITE, &oldProtect ) )
		{
			DeleteCriticalSection( &crit_load_settings );					
			return 0;
		}	   	
	}   	

	for( int i=0; i<8; ++i )
	{		
	 	data[i] = original_raw_code[8];
	}

	
	DWORD other;
	if( !VirtualProtect( (void *)cptr, 4096, oldProtect, &other ) )
	{
	//	MessageBox( 0, "SecondVirtualProtect failed", "error", MB_OK );
		DeleteCriticalSection( &crit_load_settings );				
		return 0;
	}
	
	FreeLibrary(ws2_32_handle);
	FreeLibrary(advapi32_handle);
	DeleteCriticalSection( &crit_load_settings );		
	
	return 1;	
}

const char * my_atoi( int *val, const char *p )
{
	bool valid=false;
	
	while( *p==' ' || *p==':' || *p=='.' || *p=='\t' || *p=='\r' || *p=='\n' )
	{
		++p;
	}
	
	*val=0;
	
	while( *p>='0' && *p<='9' )
	{
		*val=*val*10;
		*val += *p-'0';
		++p;
		valid=true;
	}
	
	return valid ? p : 0;
}

void read_ipv4( const char *p, unsigned char *a, unsigned char *b, unsigned char *c, unsigned char *d, int *port )
{
	int a_=0,b_=0,c_=0,d_=0;
	
	p=my_atoi(&a_,p);
	if( p )
		p=my_atoi(&b_,p);
	if( p )
		p=my_atoi(&c_,p);
   	if( p )
		p=my_atoi(&d_,p);
	if( p )
		p=my_atoi(port,p);
	
	*a =a_;
	*b =b_;
	*c =c_;
	*d =d_;
	
}


int load_settings()
{
	EnterCriticalSection( &crit_load_settings );
	if( settings_loaded )
	{
		LeaveCriticalSection( &crit_load_settings );
		return true;	
	}

	char line[4096];

 	if( registry_get_string( HKEY_LOCAL_MACHINE, IF2K_MINI_REDIR_REGISTRY_LOCATION, "redir_allow", line, sizeof(line)-1 ) )
	{
		allow_range = new jdk_ipv4_ip_range( line );	
	}
  else
  {
    allow_range = new jdk_ipv4_ip_range( "127.0.0.1/0.255.255.255 192.168.0.0/0.0.255.255 176.16.0.0/0.31.255.255 10.0.0.0/0.255.255.255" );	
  }

 	if( registry_get_string( HKEY_LOCAL_MACHINE, IF2K_MINI_REDIR_REGISTRY_LOCATION, "redir_block", line, sizeof(line)-1 ) )
	{
		block_range = new jdk_ipv4_ip_range( line );
	}

	registry_get_dword( HKEY_LOCAL_MACHINE, IF2K_MINI_REDIR_REGISTRY_LOCATION, "redir_blockfileshare", &block_fileshare );

	char redirect_key[32];
	char redirect_prefix[32];
	char redirect_destination[32];
	
	private_jdk_strncpy( redirect_key, "redir_redirect#", 32 );
	private_jdk_strncpy( redirect_prefix, "redir_redirect_prefix#", 32 );
	private_jdk_strncpy( redirect_destination, "redir_redirect_dest#", 32 );

  int count_of_actual_read = 0;

	for( int i=0; i<=NUM_REDIRECTORS; ++i )
	{
		redirect_key[private_jdk_strlen(redirect_key)-1] = i+'1';
		redirect_prefix[private_jdk_strlen(redirect_prefix)-1] = i+'1';
		redirect_destination[private_jdk_strlen(redirect_destination)-1] = i+'1';
		
	 	if( registry_get_string( HKEY_LOCAL_MACHINE, IF2K_MINI_REDIR_REGISTRY_LOCATION, redirect_key, line, sizeof(line)-1 ) )
		{
      count_of_actual_read++;
			redirectors[i].range = new jdk_ipv4_ip_range( line );
		}

	 	if( registry_get_string( HKEY_LOCAL_MACHINE, IF2K_MINI_REDIR_REGISTRY_LOCATION, redirect_destination, line, sizeof(line)-1 ) )
		{
			read_ipv4( 
					  line, 
					  &redirectors[i].dest_a,
					  &redirectors[i].dest_b,
					  &redirectors[i].dest_c,
					  &redirectors[i].dest_d,
					  &redirectors[i].dest_port
					  );
			
			if( redirectors[i].dest_a == 0 && redirectors[i].dest_b==0 &&
			    redirectors[i].dest_c == 0 && redirectors[i].dest_d==0
			   )
			{
				redirectors[i].dest_a = 127;
				redirectors[i].dest_d = 1;
			}
			
		}
		
	 	if( registry_get_string( HKEY_LOCAL_MACHINE, IF2K_MINI_REDIR_REGISTRY_LOCATION, redirect_prefix, line, sizeof(line)-1 ) )
		{
			if( *line=='1' )
			{
				redirectors[i].do_prefix=true;
			}
		}
		
	}

 		if( count_of_actual_read==0 )
    {
       redirectors[0].range = 
         new jdk_ipv4_ip_range(
         "0.0.0.0/255.255.255.255:80 0.0.0.0/255.255.255.255:8000 0.0.0.0/255.255.255.255:8080 0.0.0.0/255.255.255.255:3128"
         );
       redirectors[0].dest_a = 127;
       redirectors[0].dest_b = 0;
       redirectors[0].dest_c = 0;
       redirectors[0].dest_d = 1;
       redirectors[0].dest_port = 8000;
       redirectors[0].do_prefix = 0;
    }
 
	settings_loaded=true;
	LeaveCriticalSection( &crit_load_settings );
	
	return 1;
}

extern "C" void __declspec(dllexport) mode( DWORD num, DWORD *vals )
{
	if( num>=1 )
	{
		enabled = *vals;
		if( enabled==0 )
		{
//			MessageBox(0,"if2redir disabled","if2redir.dll", MB_OK );
		}
	}
}

extern "C" __declspec(dllexport) void if2k_redir_dll( void )
{
}


extern "C" BOOL WINAPI DllMain(
							IN HINSTANCE hinstDll,
							IN DWORD fdwReason,
							LPVOID lpvReserved
							)
{
	
	switch (fdwReason) {
		
		case DLL_PROCESS_ATTACH:
		patch_connect();
		break;
		
		case DLL_THREAD_ATTACH:
		
		break;
		
		case DLL_THREAD_DETACH:
		
		break;
		
		case DLL_PROCESS_DETACH:
		unpatch_connect();
		break;
	}
	
	
	return(TRUE);
}


#endif
