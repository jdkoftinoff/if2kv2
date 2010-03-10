#include "jdk_world.h"

#include "jdk_thread.h"
#include "jdk_log.h"
#include "jdk_error.h"




jdk_thread_pool::jdk_thread_pool( int num_threads, jdk_thread_factory &factory_ ) :
  pool( num_threads ),
  factory( factory_ ),
  shared( factory_.create_thread_shared() ),
  actual_created()
{		
  for( int i=0; i<num_threads; ++i )
  {
    jdk_thread *t = factory.create_thread( *shared, i );
    if( t )
    {			  				
      pool.add( t );
    }
    else
    {
      JDK_THROW_ERROR("Error creating thread", "");
    }			
  }		
}


jdk_thread_pool::~jdk_thread_pool()
{
  please_stop();
  if( !wait_for_stop(2) )
  {
    jdk_log( JDK_LOG_INFO, "Killing stuck threads" );
    kill_all();
  }
  pool.clear();
  delete shared;
}


bool jdk_thread_pool::run()
{
  if( !shared->run() )
  {
    JDK_THROW_ERROR("Error starting thread shared objects", "");			
  }
  for( int i=0; i<pool.getnum(); ++i )
  {
    jdk_thread *t = pool.get(i);
    if( t )
    {
      if( !t->run() )
      {
        JDK_THROW_ERROR("Error starting thread", "");			
      }
    }			
  }	
  return true;
}

bool jdk_thread_pool::update()
{
  return shared->update();
}

int jdk_thread_pool::count_started()
{
  int count=0;
  
  for( int i=0; i<pool.getnum(); ++i )
  {
    jdk_thread *t = pool.get(i);
    if( t && t->thread_hasstarted() )
    {
      count++;
    }			
  }					
  return count;
}


void jdk_thread_pool::please_stop()
{
  for( int i=0; i<pool.getnum(); ++i )
  {
    jdk_thread *t = pool.get(i);
    if( t )
    {
      t->thread_pleasestop();	
    }			
  }				
}

bool jdk_thread_pool::wait_for_stop(int timeout_in_seconds)
{
  int i=0;		
  int timeout_count=0;
  
  do
  {						
    for( i=0; i<pool.getnum(); ++i )
    {
      jdk_thread *t = pool.get(i);
      if( t && t->thread_hasstarted() && !t->thread_isdone()  )
      {
        break;
      }			
    }
#if JDK_IS_WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
    timeout_count++;
    if( timeout_in_seconds!=0 && timeout_count>timeout_in_seconds )
    {
      break;
    }
    
  } while( i!=pool.getnum() );
  
  return i==pool.getnum();
}

void jdk_thread_pool::kill_all()
{
  for( int i=0; i<pool.getnum(); ++i )
  {
    jdk_thread *t = pool.get(i);
    if( t && !t->thread_isdone()  )
    {
      t->thread_cancel();
    }			
  }
}

