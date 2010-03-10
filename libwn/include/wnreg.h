#ifndef __WNREG_H
#define __WNREG_H




class WNRegister
{
public:
	WNRegister(
		LPCSTR 	classname_,
		LPCSTR	menuname_,
		LPCSTR	iconname_ 	= IDI_APPLICATION,
		LPCSTR	cursorname_ 	= IDC_ARROW,
		LONG 	background_ 	= COLOR_WINDOW,
		UINT	style_ 		= CS_DBLCLKS,
		int	wextra_		= 0,
                WNDPROC	proc_		= 0
		);
	~WNRegister();
	
	static	BOOL	RegisterAll();
	
protected:

	LPCSTR	classname;
	UINT	style;
	LPCSTR	menuname;
	LPCSTR 	iconname;
	LPCSTR	cursorname;
	BOOL	registered;
	LONG	background;
	WNDPROC	proc;
        int	wextra;

	WNRegister *next;	
	static WNRegister *top;
private:

};

#endif
