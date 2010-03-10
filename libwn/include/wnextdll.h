#ifndef __WNEXTDLL_H
#define __WNEXTDLL_H




class WNExternalDLL
{
public:
	WNExternalDLL( char *fname, bool abort_on_fail=TRUE );
	~WNExternalDLL();

	bool	IsLoaded();
	operator HINSTANCE();


	bool LoadDLL();
	void	FreeDLL();

	static 	bool	LoadAllDLLs();
	static 	void	FreeAllDLLs();


protected:
	char	*fname;	
	HINSTANCE lib_instance;
	bool	abort_on_fail;


	static 	WNExternalDLL *head;
		WNExternalDLL *next;
		WNExternalDLL *prev;
};

inline bool WNExternalDLL::IsLoaded()
{
	return (int)lib_instance > (int)HINSTANCE_ERROR;
}

inline WNExternalDLL::operator HINSTANCE()
{
	return lib_instance;
}




#endif

