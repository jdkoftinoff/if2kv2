
#include "wnworld.h"

#include "wnwlist.h"
#include "wnwindow.h"

#ifndef DEBUG_WNWLIST
# define DEBUG_WNWLIST 0
#endif

#if !DEBUG_WNWLIST
# undef DBG
# define DBG(a)
#endif


WNWListLink::WNWListLink()
{
	ENTER( "WNWListLink::WNWListLink()" );
	
	window=0;
	id=-1;
	hwnd=0;
	
	prev=0;
	next=0;
}

WNWListLink::~WNWListLink()
{
	ENTER( "WNWListLink::~WNWListLink()" );
	
	prev=0;
	next=0;
}


WNWList::WNWList()
{
	head=0;
	num=0;
}


WNWList::~WNWList()
{
	WNWListLink *cur = head;
	WNWListLink *next;
	
	
	while( cur )
	{
		next=cur->next;
		delete cur;
		cur=next;
	}
	
}


bool WNWList::Add( WNWindow *w, long id )
{
	ENTER( "WNWList::Add(WNWindow*,long)" );
	
	WNWListLink *l = new WNWListLink;
	
	if( l )
	{
		l->window=w;
		l->id=id;
		l->hwnd=0;
		l->prev=0;
		l->next=head;
		head=l;
		if( l->next )
		  l->next->prev = l;
		++num;
		return true;
	}
	else
	{
		return false;
	}
}


bool WNWList::Add( HWND hwnd, long id )
{
	ENTER( "WNWList::Add(HWND,long)" );
	
	WNWListLink *l = new WNWListLink;
	
	if( l )
	{
		l->window=0;
		l->id=id;
		l->hwnd=hwnd;
		l->prev=0;
		l->next=head;
		if( l->next )
		  l->next->prev = l;
		head=l;
		++num;
		return true;
	}
	else
	{
		return false;
	}
}



bool	WNWList::Remove( WNWindow *w )
{
	ENTER( "WNWList::Remove(WNWindow*)" );
	
	WNWListLink *l = FindLink( w );
	
	if( l )
	{
		if( l==head )
		  head=l->next;
		
		if( l->prev )
		  l->prev->next = l->next;
		
		if( l->next )
		  l->next->prev = l->prev;
		
		delete l;
		--num;
		return true;
	}
	else
	{
		return false;
	}
}



bool WNWList::Remove( HWND h )
{
	ENTER( "WNWList::Remove(HWND)" );
	
	WNWListLink *l = FindLink( h );
	
	if( l )
	{
		if( l==head )
		  head=l->next;
		
		if( l->prev )
		  l->prev->next = l->next;
		
		if( l->next )
		  l->next->prev = l->prev;
		
		delete l;
		--num;
		return true;
	}
	else
	{
		return false;
	}
	
}

bool WNWList::Remove( long id )
{
	ENTER( "WNWList::Remove(long)" );
	WNWListLink *l = FindLink( id );
	
	if( l )
	{
		if( l==head )
		  head=l->next;
		
		if( l->prev )
		  l->prev->next = l->next;
		
		if( l->next )
		  l->next->prev = l->prev;
		
		delete l;
		--num;
		return true;
	}
	else
	{
		return false;
	}
	
}


WNWindow *WNWList::Find( HWND h )
{
	ENTER( "WNWList::Find(HWND)" );
	WNWListLink *cur = head;
	
	while( cur )
	{
		if( cur->window )
		{
			if( cur->window->GetHWND() == h )
			{
				return cur->window;
			}
			
		}
		
		cur = cur->next;
		
	}
	
	return 0;
	
}


WNWindow *WNWList::Find( long id )
{
	ENTER( "WNWList::Find(long)" );
	WNWListLink *cur = head;
	
	while( cur )
	{
		if( cur->id == id )
		{
			return cur->window;
			
		}
		
		cur = cur->next;
		
	}
	
	return 0;
	
}


long WNWList::Find( WNWindow *w )
{
	ENTER( "WNWList::Find(WNWindow*)" );
	WNWListLink *cur = head;
	
	while( cur )
	{
		if( cur->window==w )
		{
			return cur->id;
		}
		
		cur = cur->next;
		
	}
	
	return -1;
}




WNWListLink *WNWList::FindLink( WNWindow *w )
{
	ENTER( "WNWList::FindLink(WNWindow*)" );
	
	WNWListLink *cur = head;
	
	while( cur )
	{
		if( cur->window==w )
		{
			return cur;
		}
		
		cur = cur->next;
		
	}
	
	return 0;
	
}


WNWListLink *WNWList::FindLink( HWND h )
{
	ENTER( "WNWList::FindLink(HWND)" );
	
	WNWListLink *cur = head;
	
	while( cur )
	{
		if( cur->window )
		{
			if( cur->window->GetHWND() == h )
			{
				return cur;
			}
			
		}
		
		cur = cur->next;
		
	}
	
	return 0;
	
}


WNWListLink *WNWList::FindLink( long id )
{
	ENTER( "WNWList::Find(long)" );
	
	WNWListLink *cur = head;
	
	while( cur )
	{
		if( cur->id == id )
		{
			return cur;
			
		}
		
		cur = cur->next;
		
	}
	
	return 0;
	
}


HWND WNWList::FindHWND( long id )
{
	ENTER( "WNWList::FindHWND(long)" );
	
	WNWListLink *cur = head;
	
	while( cur )
	{
		if( cur->id == id )
		{
			if( cur->hwnd==0 && cur->window )
			  return cur->window->GetHWND();
			else
			  return cur->hwnd;
			
		}
		
		cur = cur->next;
		
	}
	
	return 0;
	
}


void	WNWList::SendAll( int msg, WPARAM w, LPARAM l )
{
	ENTER( "WNWList::SendAll()" );
	
	
	if( msg==WM_CLOSE)
	{
		CloseAll();
		return;
	}
	
	if( msg==WM_DESTROY )
	{
		DestroyAll();
		return;
	}
	
	
	WNWListLink *cur = head;
	
	while( cur )
	{
		HWND hwnd=cur->hwnd;
		
		if( !hwnd && cur->window )
		{
			hwnd=cur->window->GetHWND();
		}
		
		if( hwnd )
		{
			SendMessage( hwnd, msg, w, l );
		}
		
		cur = cur->next;
		
	}
	
}


bool	WNWList::CloseAll()
{
	ENTER( "WNWList::CloseAll()" );
	
	HWND hwnd;
	HWND nhwnd;
	
	while(head)
	{
		hwnd=head->hwnd;
		if( !hwnd && head->window )
		  hwnd=head->window->GetHWND();
		
		if( hwnd )
		{
			SendMessage( hwnd, WM_CLOSE,0,0 ); 
		}
		else
		{
			// this should not happen
			return true; // return true so main window can quit
		}
		
		if( head )
		{
			nhwnd=head->hwnd;
			if( !nhwnd && head->window )
			  nhwnd=head->window->GetHWND();
			
			if( nhwnd==hwnd )
			  return false;
		}
	}
	
	return true;
}


void	WNWList::DestroyAll()
{
	ENTER( "WNWList::DestroyAll()" );
	
	HWND hwnd;
	WNWListLink *cur;
	
	cur=head;
	
	while(cur)
	{
		hwnd=head->hwnd;
		if( !hwnd && head->window )
		  hwnd=head->window->GetHWND();
		
		if( hwnd )
		{
			DestroyWindow( hwnd );
		}
		else
		{
			cur=cur->next;
		}
		
	}
	
}




