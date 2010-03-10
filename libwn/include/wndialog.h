#ifndef __WNDIALOG_H
#define __WNDIALOG_H


#include "wnwindow.h"
#include "wnwlist.h"



#define WSJK_DIALOG	(WS_MINIMIZEBOX | WS_OVERLAPPED | WS_SYSMENU )




class WNDialog : public WNWindow
{
public:
	WNDialog( LPCSTR rsrcname, LPCSTR clsname );
	virtual	~WNDialog();
	
	virtual	bool	Create();
	
protected:
	
	virtual	LPARAM	KeyDownMsg( WPARAM vk, LPARAM kd );		// WM_KEYDOWN
	
	LPCSTR 	dlg_resource_name;
	
	HWND	GetDialogItem( long id );
	
private:
	
	WNWList dialog_items;
	int 	dlg_w, dlg_h;
	HWND	saved_focus_item;
};

inline HWND WNDialog::GetDialogItem( long id )
{
	return dialog_items.FindHWND(id);
}



#endif


