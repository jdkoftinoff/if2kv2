#include "wnworld.h"
#include "jdk_util.h"

#include "wnextdll.h"


WNExternalDLL * WNExternalDLL::head=0;


WNExternalDLL::WNExternalDLL(
							 char *fname_,
							 bool abort_on_fail_
							 )
:
fname( fname_ ),
lib_instance(0),
abort_on_fail( abort_on_fail_ )
{
	// insert self into linked list
	
	next=head;
	prev=0;
	head=this;
}

WNExternalDLL::~WNExternalDLL()
{
	// remove self from linked list
	
	
	if( prev )
	{
		prev->next=next;
	}
	
	if( next )
	{
		next->prev=prev;
	}
	
	if( this==head )
	{
		head=0;
	}
}


bool WNExternalDLL::LoadDLL()
{
	if( (int)lib_instance<(int)HINSTANCE_ERROR )
	{
		lib_instance=LoadLibrary( fname );
	}
	
	if( (int)lib_instance<(int)HINSTANCE_ERROR && abort_on_fail)
	{
		char buf[1024];
		jdk_sprintf( buf, "Error loading DLL: '%s'", fname );
		MessageBox( 0, buf, "ERROR", MB_OK );
	}
	
	
	return IsLoaded();
}


void	WNExternalDLL::FreeDLL()
{
	if( IsLoaded() )
	{
		FreeLibrary( lib_instance );
		lib_instance=0;
	}
	
}


bool	WNExternalDLL::LoadAllDLLs()
{
	WNExternalDLL *cur = head;
	
	while( cur )
	{
		if( !cur->LoadDLL() )
		  if( cur->abort_on_fail )
		{
			return false;
		}
		
		cur=cur->next;
	}
	return true;
}


void WNExternalDLL::FreeAllDLLs()
{
	WNExternalDLL *cur=head;
	
	while( cur )
	{
		cur->FreeDLL();
		
		cur=cur->next;
	}
}



