#ifndef __WNWLIST_H
#define __WNWLIST_H

class WNWListLink;
class WNWList;
class WNWindow;


class WNWListLink
{
	public:
	WNWListLink();
	~WNWListLink();
	
	WNWindow *window;
	long	id;
	HWND 	hwnd;
	
	WNWListLink *prev;
	WNWListLink *next;
};

class WNWList
{
	public:
	WNWList();
	virtual	~WNWList();
	
	bool	Add( WNWindow *w, long id );
	bool	Add( HWND hwnd, long id );
	
	bool	Remove( WNWindow *w );
	bool	Remove( HWND h );
	bool	Remove( long id );
	
	
	long	Find( WNWindow * w );
	WNWindow *Find( HWND h );
	WNWindow *Find( long id );
	
	HWND	FindHWND( long id );
	
	void	SendAll( int msg, WPARAM w, LPARAM l );
	
	bool	CloseAll();
	void	DestroyAll();
	
	int	GetNumber();
	
	WNWListLink *FindLink( WNWindow *w );
	WNWListLink *FindLink( HWND h );
	WNWListLink *FindLink( long id );
	
	WNWListLink *head;
	protected:
	int num;
};

inline	int	WNWList::GetNumber()
{
	return num;
}


#endif
