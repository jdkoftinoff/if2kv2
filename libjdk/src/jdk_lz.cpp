#include "jdk_world.h"
#include "jdk_lz.h"

/******************************************************************************/


#define FLAG_BYTES    4     /* Number of bytes used by copy flag. */
#define FLAG_COMPRESS 0     /* Signals that compression occurred. */
#define FLAG_COPY     1     /* Signals that a copyover occurred.  */
#define PS *p++!=*s++  /* Body of inner unrolled matching loop.         */
#define ITEMMAX 16     /* Maximum number of bytes in an expanded item.  */


static inline void jdk_lz_copy( const unsigned char *p_src, unsigned char *p_dst, size_t len, unsigned char prexorvalue ) /* Fast copy routine.             */
{
  while (len--) *p_dst++=((*p_src++)^prexorvalue);
}


/******************************************************************************/

void jdk_lz_compress( 
  const unsigned char *p_src_first,
  size_t src_len,
  unsigned char *p_dst_first,
  size_t *p_dst_len,
  unsigned char xorvalue
  )
/* Input  : Specify input block using p_src_first and src_len.          */
/* Input  : Point p_dst_first to the start of the output zone (OZ).     */
/* Input  : Point p_dst_len to a uint32 to receive the output length.    */
/* Input  : Input block and output zone must not overlap.               */
/* Output : Length of output block written to *p_dst_len.               */
/* Output : Output block in Mem[p_dst_first..p_dst_first+*p_dst_len-1]. */
/* Output : May write in OZ=Mem[p_dst_first..p_dst_first+src_len+256-1].*/
/* Output : Upon completion guaranteed *p_dst_len<=src_len+FLAG_BYTES.  */
{
  const unsigned char *p_src=p_src_first;
  unsigned char *p_dst=p_dst_first;
  const unsigned char *p_src_post=p_src_first+src_len;
  unsigned char *p_dst_post=p_dst_first+src_len;
  const unsigned char *p_src_max1=p_src_post-ITEMMAX,*p_src_max16=p_src_post-16*ITEMMAX;
  const unsigned char *hash[4096];
  unsigned char *p_control; 
  size_t control=0,control_bits=0;
  
  if( src_len==0 )
  {
    *p_dst_len = 0;
    return;
  }
  
  *p_dst=FLAG_COMPRESS; 
  p_dst+=FLAG_BYTES; 
  p_control=p_dst; 
  p_dst+=2;
  
  while (true)
  {
    const unsigned char *p,*s; 
    uint16 unroll=16;
    size_t len,index; 
    size_t offset;
    
    if (p_dst>p_dst_post) goto overrun;
    
    if (p_src>p_src_max16)
    {
      unroll=1;
      if (p_src>p_src_max1)
      {
        if (p_src==p_src_post) break; 
        goto literal;
      }
    }
    
  begin_unrolled_loop:
    
    index=((40543*((((p_src[0]<<4)^p_src[1])<<4)^p_src[2]))>>4) & 0xFFF;
    
    p=hash[index]; 
    hash[index]=s=p_src; 
    offset=s-p;
    
    if (offset>4095 || p<p_src_first || offset==0 || PS || PS || PS)
    {
    literal: *p_dst++=*p_src++; control>>=1; control_bits++;
    }		
    else
    {
      (void) (PS || PS || PS || PS || PS || PS || PS ||
              PS || PS || PS || PS || PS || PS || s++); 
      
      len=s-p_src-1;
      
      *p_dst++=(unsigned char)(((offset&0xF00)>>4)+(len-1)); 
      *p_dst++=(unsigned char)(offset&0xFF);
      p_src+=len; 
      control=(control>>1)|0x8000; 
      control_bits++;
    }
    
    /*end_unrolled_loop:*/ 
    if (--unroll) goto begin_unrolled_loop;
    
    if (control_bits==16)
    {
      *p_control=(unsigned char)(control&0xFF); 
      *(p_control+1)=(unsigned char)(control>>8);
      p_control=p_dst; 
      p_dst+=2; 
      control=control_bits=0;
    }
    
  }
  
  control>>=16-control_bits;
  *p_control++=(unsigned char)(control&0xFF); 
  *p_control++=(unsigned char)(control>>8);
  if (p_control==p_dst) 
    p_dst-=2;
  *p_dst_len=p_dst-p_dst_first;
  
  if( xorvalue !=0 )
  {
    for( size_t i=0; i<*p_dst_len; ++i )
    {
      p_dst_first[i] ^= xorvalue;
    }
  }
  
  return;
  
overrun: 
  jdk_lz_copy(p_src_first,p_dst_first+FLAG_BYTES,src_len,0);
  *p_dst_first=FLAG_COPY; 
  *p_dst_len=src_len+FLAG_BYTES;
  
  if( xorvalue !=0 )
  {
    for( size_t i=0; i<*p_dst_len; ++i )
    {
      p_dst_first[i] ^= xorvalue;
    }
  }
  
}


/******************************************************************************/

void jdk_lz_decompress(
  const unsigned char * p_src_first,
  size_t src_len,
  unsigned char *p_dst_first,
  size_t * p_dst_len,
  unsigned char xorvalue
  )
/* Input  : Specify input block using p_src_first and src_len.          */
/* Input  : Point p_dst_first to the start of the output zone.          */
/* Input  : Point p_dst_len to a uint32 to receive the output length.    */
/* Input  : Input block and output zone must not overlap. User knows    */
/* Input  : upperbound on output block length from earlier compression. */
/* Input  : In any case, maximum expansion possible is eight times.     */
/* Output : Length of output block written to *p_dst_len.               */
/* Output : Output block in Mem[p_dst_first..p_dst_first+*p_dst_len-1]. */
/* Output : Writes only  in Mem[p_dst_first..p_dst_first+*p_dst_len-1]. */
{
  uint16 controlbits=0, control=0;
  const unsigned char *p_src=p_src_first+FLAG_BYTES;
  unsigned char *p_dst=p_dst_first;
  const unsigned char *p_src_post=p_src_first+src_len;
  
  if( src_len==0 )
  {
    *p_dst_len = 0;
    return;
  }
  
  
  if ( ((*p_src_first) ^ xorvalue)==FLAG_COPY )
  {
    jdk_lz_copy(p_src_first+FLAG_BYTES,p_dst_first,src_len-FLAG_BYTES,xorvalue);
    *p_dst_len=src_len-FLAG_BYTES; 
    return;
  }
  
  while (p_src!=p_src_post)
  {
    if (controlbits==0)
    {
      control=(*p_src++) ^ xorvalue; 
      control|=((*p_src++)^xorvalue)<<8; 
      controlbits=16;
    }
    
    if (control&1)
    {
      size_t offset,len; 
      unsigned char *p;
      offset=(((*p_src) ^ xorvalue)&0xF0)<<4; 
      len=1+(((*p_src++) ^ xorvalue)&0xF);
      offset+=((*p_src++) ^ xorvalue)&0xFF; 
      p=p_dst-offset;
      while (len--) 
        *p_dst++=(*p++);
    }		
    else
      *p_dst++=((*p_src++) ^ xorvalue);
    
    control>>=1; 
    controlbits--;
  }
  
  *p_dst_len=p_dst-p_dst_first;
}

