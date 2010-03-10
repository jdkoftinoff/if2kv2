#ifndef _JDK_QUEUE_H
# define _JDK_QUEUE_H

template <class T>
class jdk_queue
{
  explicit jdk_queue( const jdk_queue & );
  const jdk_queue & operator = (const jdk_queue & );
public:
   inline explicit jdk_queue( int max_sz_pwr_2=4096 ) :
   next_in(0),
   next_out(0),
   max_size(max_sz_pwr_2),
   mask(max_sz_pwr_2-1),
   buf(new T[max_sz_pwr_2])
   {	   
   }  
   
   ~jdk_queue()
   {
	   delete [] buf;
   }
   
   void clear()
   {
	   next_in=0;
	   next_out=0;
   }

   int get_data_length() const
   {
       return (next_in - next_out)&mask;
   }

   int get_available_space() const
   {
     int n = max_size - get_data_length()-1;
     if(n<0)
     {
       n=0;
     }
     return n;
   }

   bool can_put( int sz )
   {
     return (max_size - get_data_length()) >=sz;
   }

   bool can_put()
   {
	   return ( (next_in+1)&mask ) != next_out;
   }


   bool can_get()
   {
	   return (next_out!=next_in);
   }
   
   void put( const T &o )
   {
	   buf[next_in]=o;
	   next_in = (next_in+1)&mask;
   }

   void put_array( const T *src, int count )
   {
     for( int i=0; i<count; ++i )
     {
	   buf[next_in]=src[i];
	   next_in = (next_in+1)&mask;
     }
   }
   
   T peek() const
   {
	   return buf[next_out];
   }
   
   void skip( int count=1)
   {
	   next_out = (next_out+count)&mask;
   }

   void get_array( T *dest, int count ) const
   {
     for( int i=0; i<count; ++i )
     {
       dest[i] = buf[next_out];
       next_out = (next_out+1)&mask;
     }
   }

   const T * get_array1_chunk() const
   {
       return &buf[next_out];
   }

   int get_array1_size() const
   {
     if( next_out<=next_in )
     {
       return next_in - next_out;
     }
     else
     {
       return max_size - next_out;
     }
   }

   const T * get_array2_chunk() const
   {
       return &buf[0];
   }

   int get_array2_size() const
   {
     if( next_out<=next_in )
     {
       return 0;
     }
     else
     {
       return next_in;
     }
   }

   volatile unsigned int next_in;
   volatile unsigned int next_out;	
   unsigned int max_size;
   unsigned int mask;
   T *buf;
};

typedef jdk_queue<char> jdk_char_queue;


#endif
