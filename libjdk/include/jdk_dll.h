
#ifndef _JDK_DLL_H
#define _JDK_DLL_H

//
// class jdk_dll_manager is the simple ELF C++ Library manager.
//
// It tries to dynamically load the specified shared library
// when it is construted.
//
// You should call LastError() before doing anything.  If it 
// returns NULL there is no error.
//


class jdk_dll_manager
{
 public:
	jdk_dll_manager( const char *fname );
	virtual ~jdk_dll_manager();


	bool get_symbol( void **, const char *sym_name );

	const char *last_error() 
	{
		 return err;
	}
	
 protected:
#if JDK_IS_WIN32 && !JDK_IS_WINE
	HINSTANCE h;
	char err[1024];
#elif JDK_IS_UNIX || JDK_IS_WINE
	void *h;
	const char *err;	
#else   
#error specify platform
#endif

};


//
// class jdk_dll_factory_base is the base class used for the DLLFactory
// template class.  
// 
// It inherits from the jdk_dll_manager class and must be constructed with
// the file name of the shared library and the function name within that
// library which will create the desired C++ factory class.
// If you do not provide func_name to the constructor, it defaults to
// the undecorated "C" symbol "factory0"
//
// factory_func will be set to a pointer to the requested factory creator 
// function.  If there was an error linking to the shared library,
// factory_func will be 0.
//
// You can call 'last_error()' to find the error message that occurred.
//
//

class jdk_dll_factory_base : public jdk_dll_manager
{
 public:
	jdk_dll_factory_base(
		       const char *fname,
		       const char *func_name=0
		       );
		
	virtual ~jdk_dll_factory_base();
	
	void * (*factory_func)(void);	
};


//
// The DLLFactory template class inherits from jdk_dll_factory_base.
// The constructor takes the file name of the shared library
// and the undecorated "C" symbol name of the factory creator
// function.  The factory creator function in your shared library
// MUST either return a pointer to an object that is a subclass
// of 'T' or it must return 0.
//
// If everything is cool, then 'factory' will point to the
// requested factory class.  If not, it will be 0.
//
// Since the jdk_dll_factory template ultimately inherits jdk_dll_manager,
// you can call last_error() to get any error code information
//
// The created factory is OWNED by the jdk_dll_factory class.  
// The created factory will get deleted when the jdk_dll_factory class
// is deleted, because the DLL will get unloaded as well.
//

template <class T>
class jdk_dll_factory : public jdk_dll_factory_base
{
 public:
	jdk_dll_factory(
		   const char *fname,
		   const char *func_name=0
		   ) : jdk_dll_factory_base( fname, func_name )
	{
		if( factory_func )
		  factory = (T *)factory_func();
		else 
		  factory = 0;
	}
	
	~jdk_dll_factory()
	{
		delete factory;
	}

	T *factory;
};






#endif
