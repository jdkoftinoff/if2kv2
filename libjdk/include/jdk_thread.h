#ifndef _JDK_THREAD_H
#define _JDK_THREAD_H

#include "jdk_world.h"
#include "jdk_list.h"
#include "jdk_log.h"

#define JDK_HAS_THREADS 1

#if JDK_IS_WIN32 
#define JDK_IS_WIN32_THREADS JDK_HAS_THREADS
#define JDK_IS_UNIX_THREADS 0
#else
#define JDK_IS_WIN32_THREADS 0
#define JDK_IS_UNIX_THREADS JDK_HAS_THREADS
#endif


#if JDK_HAS_THREADS|| JDK_FAKE_THREAD

#if JDK_IS_UNIX_THREADS
# define JDK_THREAD_RETVAL void *

# if JDK_HAS_THREADS
#  define JDK_CURTHREADID ((jdk_thrid_t) pthread_self())
# else
#  define JDK_CURTHREADID ((jdk_thrid_t) getpid())
# endif

#elif JDK_IS_BEOS
# define JDK_THREAD_RETVAL thread_id
# define JDK_CURTHREADID (0)

#elif JDK_IS_WIN32_THREADS
#undef JDK_HAS_THREADS
#define JDK_HAS_THREADS 1
#define JDK_FAKE_THREAD 0
# define JDK_THREAD_RETVAL void
# define JDK_CURTHREADID ((jdk_thrid_t)GetCurrentThread())

#else
# error
#endif

#if JDK_IS_WIN32
typedef jdk_uint64 jdk_thrid_t;
#else
typedef jdk_uint64 jdk_thrid_t;
#endif

class jdk_thread;
class jdk_mutexbase;
    class jdk_mutex;
    class jdk_recursivemutex;
class jdk_mutexsection;
class jdk_thread_factory;
class jdk_thread_shared;
class jdk_thread_pool_base;
class jdk_thread_pool;

class jdk_mutexbase
{
public:
    explicit jdk_mutexbase()
    {
    }
    	
    virtual ~jdk_mutexbase()
	{
	}
		
	virtual void    get() = 0;
	virtual bool    tryget() = 0;	
	virtual void    release() = 0;
private:
    jdk_mutexbase( const jdk_mutexbase & );    // not allowed
    const jdk_mutexbase & operator = ( const jdk_mutexbase & ); // not allowed
};


class jdk_mutex : public jdk_mutexbase
{
public:
	explicit jdk_mutex( const char *name_=0 );
    ~jdk_mutex();
		
	void    get();	
	bool    tryget();	
	void    release();
protected:
	const char *name;
private:

#if JDK_IS_BEOS
	sem_id my_mutex;
	int32 ben_val;
#elif JDK_IS_WIN32_THREADS
	CRITICAL_SECTION crit;
#elif !JDK_FAKE_THREAD	
	pthread_mutex_t my_mutex;
#endif


	jdk_mutex( const jdk_mutex & ); // not allowed
	const jdk_mutex & operator = ( const jdk_mutex & ); // not allowed	
};



class jdk_recursivemutex : public jdk_mutex
{
public:
	explicit jdk_recursivemutex( const char *name_=0 );
    ~jdk_recursivemutex();

	void    get();
	bool    tryget();
	void    release();

private:

	volatile jdk_thrid_t owner_id;
	volatile int owner_count;
	jdk_mutex guard;
	int owner_pid; // for debugging easier

	jdk_recursivemutex( const jdk_recursivemutex & ); // not allowed
	const jdk_recursivemutex & operator = ( const jdk_recursivemutex & ); // not allowed	
};


class jdk_mutexsection
{
public:
#if 0
	explicit jdk_mutexsection( jdk_mutexbase *s ) : mutblock( *s )
	{
		mutblock.get();
	}
#endif
	explicit jdk_mutexsection( jdk_mutexbase &s ) : mutblock( s )
	{
		mutblock.get();
	}

	~jdk_mutexsection()
	{
	    mutblock.release();
	}

private:
	jdk_mutexbase &mutblock;
	
	jdk_mutexsection( const jdk_mutexsection & ); // not allowed
	const jdk_mutexsection & operator = ( const jdk_mutexsection & ); // not allowed

};

#define jdk_synchronized(a) jdk_mutexsection _jdk_mutexsection__0(a)
#define jdk_synchronized1(a) jdk_mutexsection _jdk_mutexsection__1(a)
#define jdk_synchronized2(a) jdk_mutexsection _jdk_mutexsection__2(a)

//------------------------------------------------------------------------------------

class jdk_thread
{
public:
	 		jdk_thread();
	virtual ~jdk_thread();

	// call thread_setinitialstack before calling threadrun() 
	// if you want a non-default sized stack	
	void    thread_setinitialstack( long stksize_ )
	{
	    stksize = stksize_;	
	}

	// call thread_plansuicide() to tell the thread to 
	// delete itself upon thread completion.  If you plan to use this
	// feature you must always allocate your threads via new()
	// and once you call run() to start it the you can never use the
	// pointer to the thread anymore - it may be deleted already,
	// even if run() returns false.

	void	thread_plansuicide(bool f=true)
	{
		suicide = f;
	}

	// call thread_usefork() to tell the thread to actually be a forked process
	// instead of just a thread.
	
	void	thread_usefork( bool f=true )
	{
		usefork=f;
	}
	
		
	// call run() once only to start the thread running	
	bool   	run();	

	// call blockrun() once only to start the procedure without spawning thread or process
	// it will not return until the main() ends.
	bool   	blockrun();	
	
	jdk_thrid_t   thread_getid() const
	{
		return my_id;
	}	
	
	// thread_hasstarted() returns true only if the actual thread has
	// started executing.
	
	bool    thread_hasstarted() const
	{
		return started;
	}

    bool	thread_pleasestop()
    {
    	please_stop = true;
		return true;
    }

	// thread_isdone() returns true either if main() returned or
	// the thread was killed

	bool    thread_isdone() const
	{
	    return done;
	}

	void	thread_cancel()
	{
		if( started && !done )
		{		   	
#if JDK_IS_MACOSX || JDK_FAKE_THREAD 
		    kill( SIGTERM, (pid_t)my_id );
#elif JDK_IS_CYGWIN
		    pthread_kill( (pthread_t *)&my_id, SIGTERM );
#elif JDK_IS_UNIX_THREADS
		    pthread_cancel( (pthread_t)my_id );
#elif JDK_IS_WIN32_THREADS
		    TerminateThread( (HANDLE)my_id, 1 );
#endif

		}

		done = true;
	}


protected:

	// premain() is called from the thread context, just before main() is called
    virtual void premain();

	// main() is where your main thread routine lives
	virtual void main() = 0;

	// postmain() is called after the main() finished or when exit() is called.
	// postmain() is called from the thread context.
    virtual void postmain();

	// thread_prerun() is called from the parent's thread context, just before
	// the thread is spawned.  if prerun() returns false, the thread will
	// not be created.
    virtual bool thread_prerun();

    // thread_exit() can be called from your thread to nicely abort the thread.
    // after threadexit() is called, postmain() will be called to clean up.
	virtual void thread_exit();


	// sleep for specified number of seconds
	void	sleep( int s );

	// sleep for specified number of milliseconds
	void	sleepms( int s );

    volatile bool please_stop;


private:
	jdk_thrid_t my_id;
	long stksize;
	volatile bool done;
	volatile bool started;
	bool suicide;
	bool usefork;


#if JDK_HAS_THREADS
	static  JDK_THREAD_RETVAL threaded_start( void *);
#if JDK_IS_UNIX_THREADS
	pthread_attr_t attr;
	pthread_t thr;
#endif
#endif
#if JDK_HAS_FORK
	void forked_start();
#endif


private:
	jdk_thread( const jdk_thread &o ); // not allowed
	const jdk_thread &operator = ( const jdk_thread & ); // not allowed
};

class jdk_thread_shared
{
public:
    jdk_thread_shared() {}
    virtual bool run() = 0;
    virtual bool update() = 0;
    
    virtual ~jdk_thread_shared() {}
};


class jdk_thread_factory
{
public:
	jdk_thread_factory()
	{
	}

	virtual ~jdk_thread_factory()
	{
	}

	virtual jdk_thread *create_thread( jdk_thread_shared &shared, int my_id ) = 0;

	virtual jdk_thread_pool_base *create_thread_pool() = 0;

    virtual jdk_thread_shared *create_thread_shared() = 0;

private:
    jdk_thread_factory( const jdk_thread_factory & );    // not allowed
    const jdk_thread_factory &operator = ( const jdk_thread_factory & ); // not allowed
};

class jdk_thread_pool_base
{
public:

	jdk_thread_pool_base() {}

	virtual ~jdk_thread_pool_base()	{}

	virtual bool run() = 0;
    virtual bool update() = 0;
	virtual int count_started() = 0;
	virtual void please_stop() = 0;
	virtual bool wait_for_stop(int timeout_in_seconds=0) = 0;
	virtual void kill_all() = 0;


private:
	jdk_thread_pool_base( const jdk_thread_pool_base & );
	const jdk_thread_pool_base & operator = ( const jdk_thread_pool_base & );
};


class jdk_thread_pool : public jdk_thread_pool_base
{
public:
	jdk_thread_pool( int num_threads, jdk_thread_factory &factory_ );
	virtual ~jdk_thread_pool();

	bool run();
    bool update();
	int count_started();
	void please_stop();
	bool wait_for_stop(int timeout_in_seconds);
	void kill_all();

protected:
	jdk_list<jdk_thread> pool;
	jdk_thread_factory &factory;
	jdk_thread_shared *shared;
	int actual_created;

private:
	jdk_thread_pool( const jdk_thread_pool & );
	const jdk_thread_pool & operator = ( const jdk_thread_pool );
};

#else

class jdk_mutexbase
{
public:
    jdk_mutexbase()
    {
    }
    	
    ~jdk_mutexbase()
	{
	}
		
	void    get() 
	{
	}
	
	bool    tryget()
	{
		return true;
	}
	
	void    release()
	{
	}
	
private:
    jdk_mutexbase( const jdk_mutexbase & );    // not allowed
    const jdk_mutexbase & operator = ( const jdk_mutexbase & ); // not allowed
};

#define jdk_mutex jdk_mutexbase
#define jdk_recursivemutex jdk_mutexbase

class jdk_mutexsection
{
public:
	explicit jdk_mutexsection( jdk_mutexbase *s )
	{
	}

	explicit jdk_mutexsection( jdk_mutexbase &s )
	{
	}

	~jdk_mutexsection()
	{
	}

private:
	
	jdk_mutexsection( const jdk_mutexsection & ); // not allowed
	const jdk_mutexsection & operator = ( const jdk_mutexsection & ); // not allowed

};

#define jdk_synchronized(a) jdk_mutexsection _jdk_mutexsection__0(a)
#define jdk_synchronized1(a) jdk_mutexsection _jdk_mutexsection__1(a)
#define jdk_synchronized2(a) jdk_mutexsection _jdk_mutexsection__2(a)


#endif


//------------------------------------------------------------------------------------


#endif
