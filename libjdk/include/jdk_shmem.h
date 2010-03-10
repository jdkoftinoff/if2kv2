#ifndef _JDK_SHMEM_H
#define _JDK_SHMEM_H

#include "jdk_thread.h"

#if JDK_IS_WIN32

class jdk_shmem
{
 public:
	jdk_shmem(const char *mapname,size_t sz);
	virtual ~jdk_shmem();
	
	void *getmem() 
	{
		 return mem;
	}
	
	const void *getmem() const
	{
		return mem;
	}
	
 protected:
	void *mem;
	HANDLE map;
};

#define jdk_increment(a) InterlockedIncrement(a)
#define jdk_decrement(a) InterlockedDecrement(a)

#elif JDK_IS_UNIX

class jdk_shmem
{
  explicit jdk_shmem( const jdk_shmem & );
  const jdk_shmem & operator = ( const jdk_shmem & );
 public:
	jdk_shmem(const char *mapname,size_t sz);
	virtual ~jdk_shmem();
	
	void *getmem() 
	{
		 return mem;
	}
	
	const void *getmem() const
	{
		return mem;
	}
	
 protected:
	void *mem;
};

#define jdk_increment(a) (++(a))
#define jdk_decrement(a) (--(a))

#endif


template <class T> class jdk_shmem_obj : protected jdk_shmem
{
 public:
	jdk_shmem_obj(const char *mapname ) : jdk_shmem( mapname, sizeof(T) )
	{
	}
	

	T *get() 
	{
		 return (T *)mem;
	}
	
	const T *get() const
	{
		return (T *)mem;
	}
	
};




#endif
