#include "jdk_world.h"
#include "jdk_heap.h"

#if JDK_HAS_HEAP
#ifdef __cplusplus
extern "C" {
#endif
  
#ifndef JDK_HEAP_DEFAULT_ERROR
#define JDK_HEAP_DEFAULT_ERROR 1
#endif
  
#if JDK_HEAP_DEFAULT_ERROR
  inline void jdk_heap_error(const char *,...)
  {
  }
  
  inline void jdk_heap_debug(const char *,...)
  {
  }
#endif
  
  void  jdk_heap_init(
    struct jdk_heap_ *heap,
    void *base,
    int32 size_in_words,
    struct jdk_heap_ *fallback
    )
  {
    // align heap size to be 8 byte aligned
    
    size_in_words &= ~7;
    heap->first = (struct jdk_heap_block_ *)base;
    heap->size_in_words = size_in_words;
    
    heap->first->next=0;
    heap->first->prev=0;
    heap->first->size_in_words = -(size_in_words-(int32)(sizeof( struct jdk_heap_block_ )/sizeof(int32)));
    heap->last_free=heap->first;
    
    heap->fallback = fallback;
    heap->error_proc = 0;
  }
  
  void *jdk_heap_alloc(
    struct jdk_heap_ *heap,
    int32 numwords
    )
  {
    void *ptr=0;
    
    // align requested size to be 8 byte aligned
    numwords = (numwords+7)&(~7);
    
    /* try allocate from requested heap */
    
    ptr = jdk_heap_alloc_(heap,numwords);
    
    /* if it couldnt be allocated from requested heap, try the fallback heap */
    
    if( !ptr )
    {
      if( heap->fallback )
      {
        ptr = jdk_heap_alloc_(heap->fallback, numwords );
      }
      
      
      /* if it still couldnt be allocated, call the heaps error handler */
      
      if( !ptr )
      {
        /* do we have an error handler procedure? */
        if( heap->error_proc )
        {
          /* yes, call it. It may find some memory or not, or may not return */
          ptr = heap->error_proc( heap, numwords );
        }
        
        
        /* if we STILL have no pointer then this allocation fails */
        
        if( !ptr )
        {
          jdk_heap_error( "[ERROR] Memory alloc failed" );
        }
        
      }
      else
      {
        jdk_heap_error("WARNING: Using second heap" );
      }
    }
    
    return ptr;
  }
  
  
  void  jdk_heap_free(
    struct jdk_heap_ *heap,
    void *ptr
    )
  {
    /* check to see if ptr is within heap - it may be in fallback heap */
    
    jdk_uint64 start_range;
    jdk_uint64 end_range;
    
    /* ignore null pointers */
    
    if( !ptr )
    {
      return;
    }
    
    start_range = (jdk_uint64)heap->first;
    end_range = start_range + (jdk_uint64)heap->size_in_words * sizeof(int32);
    
    /* is ptr within first heap? */
    if( (jdk_uint64)ptr >= start_range &&
        (jdk_uint64)ptr < end_range
      )
    {
      /* yes, free from first heap */
      
      jdk_heap_free_( heap, ptr );
      return;
    }
    else
    {
      /* no, free from fallback heap if it exists and is within range */
      
      if( heap->fallback )
      {
        start_range = (jdk_uint64)heap->fallback->first;
        end_range = start_range + (jdk_uint64)heap->fallback->size_in_words * sizeof(int32);
        
        if( (uint32)ptr >= start_range &&
            (uint32)ptr < end_range )
        {
          jdk_heap_free_( heap->fallback, ptr );
          return;
        }
        
      }
      
    }
    
    /* we were given a bad pointer to free */
    
    jdk_heap_error( "WARNING: Bad param to jdk_heap_free()" );
    
  }
  
  
  
  void *jdk_heap_alloc_(
    struct jdk_heap_ *heap,
    int32 numwords
    )
  {
    struct jdk_heap_block_ *cur = heap->last_free;
    struct jdk_heap_block_ *usable = 0;
    void *ptr;
    
    /* find a free memory block that is big enough */
    
    if( !cur )
      cur=heap->first;
    
    if( numwords==0)
      numwords=1;
    
    
    do
    {
      if( cur->size_in_words<0 && -cur->size_in_words >= numwords ) /* JEM */
      {
        /* found a free memory block big enough! */
        usable=cur;
        break;
      }
      cur=cur->next;
    } while(cur);
    
    /* if cur is 0, we couldn't find ANY free blocks big enough */
    
    if( cur==0 )
    {
      /* We need to pack all free memory */
      
      cur=heap->first;
      while( cur )
      {
        jdk_heap_pack( heap, cur );
        cur=cur->next;
      }
      
      /* now try find a free block that may be large enough */
      cur=heap->first;
      do
      {
        if( cur->size_in_words<0 && -cur->size_in_words >= numwords ) /* JEM */
        {
          /* found a free memory block big enough! */
          usable=cur;
          break;
        }
        cur=cur->next;
      } while(cur);
      
      /* if cur is still 0, then we cannot handle this memory request */
      
      if( cur==0 )
      {
        return 0;
      }
    }
    
    /*
     * we have found a big enough block. Check to see if we should
     * split this block into two blocks so we don't waste memory
     *
     * Only split the block if the block is more than 32 words larger
     * than the requested amount
     */
    
    
    usable->size_in_words = -usable->size_in_words;       /* mark the block as not free. */
    
    if( usable->size_in_words > (int32)(numwords + 32*sizeof(int32)) ) /* JEM */
    {
      int32 orig_size = usable->size_in_words;
      struct jdk_heap_block_ *orig_next = usable->next;
      
      /* yes, we should split the block into two free blocks */
      
      
      /* Make the block perfectly sized */
      usable->size_in_words = numwords;
      
      /* calculate the position of the next block */
      
      usable->next = (struct jdk_heap_block_ *)(
        (char *)usable +
        numwords*sizeof(int32) +
        sizeof( struct jdk_heap_block_));
      
      /* put the links in properly */
      
      usable->next->next = orig_next;
      usable->next->prev = usable;
      if( orig_next )
        orig_next->prev=usable->next;
      
      /* figure out how big the left over block is */
      
      usable->next->size_in_words = - (orig_size - numwords - (int32)(sizeof(struct jdk_heap_block_)/sizeof(int32)) );
      
      
    }
    
    heap->last_free=usable->next;
    
    ptr = (void *)(((char *)usable)+sizeof( struct jdk_heap_block_ ));
    
    /* return the data section of the block to the caller */
    return ptr;
    
  }
  
  
  
  void  jdk_heap_free_(
    struct jdk_heap_ *heap,
    void *ptr
    )
  {
    struct jdk_heap_block_ *block;
    int32 sz=0;
    
    if( ptr==0 )
    {
      return;
    }
    /* subtract the MemBlock header size from the pointer given to us
     * to get the MemBlock header address
     */
    
    block = (struct jdk_heap_block_ *)(((char *)ptr) - (sizeof(struct jdk_heap_block_)));
    
    /*
     * mark it as free by changing the size to negative size
     */
    
    if( block->size_in_words>0 )
    {
      sz=block->size_in_words;
      block->size_in_words = -sz;
    }
    
    /*
     * call jdk_heap_pack() to pack it and any free blocks before and after it
     */
    
    jdk_heap_pack( heap, block );
    
  }
  
  
  void  jdk_heap_pack(
    struct jdk_heap_ *heap,
    struct jdk_heap_block_ *block
    )
  {
    struct jdk_heap_block_ *first, *cur, *last;
    
    cur=block;
    
    /* block had better be already free before calling this! */
    
    if( cur->size_in_words>0 )
      return;
    
    /*
     * search backwards for contiguous free blocks
     */
    
    
    while( cur->prev && cur->prev->size_in_words<0 )
    {
      cur=cur->prev;
    }
    
    first=cur;
    
    /* update the heaps last free block cache so it does not get
    ** fucked
    */
    
    heap->last_free= first;
    
    /* search forward for contiguous free blocks */
    
    cur=block;
    while( cur->next && cur->next->size_in_words<0 )
    {
      cur=cur->next;
    }
    
    last=cur;
    
    /* if there is only only lonely block of free memory,
     * there is nothing to do.
     */
    
    if( first==last )
    {
      return;
    }
    
    /*
     * calculate size of reclaimed space
     * It is the difference between the first block
     * address and the last block address, plus the
     * size of the last block.
     * This value is then the size of the whole
     * free area including the MemBlock header on the
     * first block. We subtract this from our total
     * and set the first block size to the negative
     * of that, since it is a free block
     */
    
    
    first->size_in_words = - (
      ((uint64)last - (uint64)first)/(uint64)sizeof(int32)
      + (-last->size_in_words)
      );
    
    /* fix links */
    
    first->next = last->next;
    if( last->next )
      last->next->prev = first;
  }
  
  
  
  void  jdk_heap_reset(
    struct jdk_heap_ *heap
    )
  {
    heap->first->next=0;
    heap->first->prev=0;
    heap->first->size_in_words=-(heap->size_in_words - (int32)(sizeof(struct jdk_heap_block_)/sizeof(int32)));
  }
  
  
  void  jdk_heap_report(
    struct jdk_heap_ *heap,
    int32 *free_mem,
    int32 *used_mem,
    int32 *free_chunks,
    int32 *used_chunks,
    int32 *largest_free,
    int32 *largest_used
    )
  {
    int32 fch=0, uch=0, f=0, u=0;
    struct jdk_heap_block_ *cur;
    
    cur=heap->first;
    
    *largest_free = 0;
    *largest_used = 0;
    
    while( cur )
    {
      int32 sz = cur->size_in_words;
      
      if( sz < 0 )
      {
        f += -sz;
        ++fch;
        if (-sz > *largest_free) *largest_free = -sz;
      }
      else
      {
        u += sz;
        ++uch;
        if (sz > *largest_used) *largest_used = sz;
      }
      
      cur=cur->next;
    }
    
    *free_chunks=fch;
    *used_chunks=uch;
    *free_mem=f;
    *used_mem=u;
  }
  
  
#if defined(JDK_HEAP_TEST)
  
  void memory_print( jdk_heap *heap )
  {
    int32 free_block_num		= 0;
    int32 free_block_space		= 0;
    int32 used_block_num		= 0;
    int32 used_block_space		= 0;
    
    
    jdk_heap_report(
      heap,
      &free_block_space,
      &used_block_space,
      &free_block_num,
      &used_block_num
      );
    
    printf("free_block_num:%d free_block_space:%d",
           free_block_num, free_block_space);
    printf("used_block_num:%d used_block_space:%d",
           used_block_num, used_block_space);
    
  }
  
  
  int main( int argc, char **argv )
  {
    int ret=0;
    struct jdk_heap_ heap;
    static char raw_heap[10000*4];
    char *p[256];
    int i;
    
    jdk_heap_init( &heap, raw_heap, sizeof( raw_heap_ )/sizeof(int32), 0 );
    
    printf( "\nStarted\n" );
    memory_print( &heap );
    
    for( i=0; i<256; ++i )
    {
      p[i] = (char *)jdk_heap_alloc( &heap, 20 );
    }
    
    printf( "\nallocated %d words\n", i * 20 );
    memory_print( &heap );
    
    for( i=0; i<16; i++ )
    {
      printf( "\nptr[i]=%08x", p[i] );
    }
    
    
    for( i=0; i<128; i++ )
    {
      int rot = ((i<<4)&0xf0) + ((i>>4)&0x0f);
      jdk_heap_free( &heap, p[rot] );
    }
    
    printf( "\nfreed half, scattered\n" );
    memory_print( &heap );
    
    for( ; i<256; i++ )
    {
      int rot = ((i<<4)&0xf0) + ((i>>4)&0x0f);
      jdk_heap_free( &heap, p[rot] );
    }
    
    printf( "\nfreed second half, scattered\n" );
    memory_print( &heap );
    
    
    return ret;
  }
  
#endif
  
#ifdef __cplusplus
}
#endif
#endif
