#include "jdk_world.h"
#include "jdk_base64.h"

static char base64_encode_map[64] =
{
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'
};

#if 0
static signed char base64_decode_map[128] =
{
  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,    // 0x00-0x0f
  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,    // 0x10-0x1f
  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,    // 0x20-0x2f
  52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,    // 0x30-0x3f
  -1,-1,1,2,   3,4,5,6,     7,8,9,10,    11,12,13,14,    // 0x40-0x4f
  15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,    // 0x50-0x5f
  -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,    // 0x60-0x6f
  41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1    // 0x70-0x7f
};
#endif

bool jdk_base64_encode( const void *inp, size_t insize, char *output, size_t out_len )
{
  //  * Copyright (C) 2001, Andrew Francis and Daniel Stenberg - from libcurl
  unsigned char ibuf[3];
  unsigned char obuf[4];
  size_t i;
  int inputparts;
  char *base64data;
  
  const char *indata = (const char *)inp;
  
  if(0 == insize)
    insize = strlen(indata);
  
  base64data = output;
  
  while(insize > 0) {
    for (i = inputparts = 0; i < 3; i++) { 
      if(*indata) {
        inputparts++;
        ibuf[i] = *indata;
        indata++;
        insize--;
      }
      else
        ibuf[i] = 0;
    }
    
    obuf [0] = (ibuf [0] & 0xFC) >> 2;
    obuf [1] = ((ibuf [0] & 0x03) << 4) | ((ibuf [1] & 0xF0) >> 4);
    obuf [2] = ((ibuf [1] & 0x0F) << 2) | ((ibuf [2] & 0xC0) >> 6);
    obuf [3] = ibuf [2] & 0x3F;
    
    switch(inputparts) {
    case 1: /* only one byte read */
      jdk_sprintf(output, "%c%c==", 
              base64_encode_map[obuf[0]],
              base64_encode_map[obuf[1]]);
      break;
    case 2: /* two bytes read */
      jdk_sprintf(output, "%c%c%c=", 
              base64_encode_map[obuf[0]],
              base64_encode_map[obuf[1]],
              base64_encode_map[obuf[2]]);
      break;
    default:
      jdk_sprintf(output, "%c%c%c%c", 
              base64_encode_map[obuf[0]],
              base64_encode_map[obuf[1]],
              base64_encode_map[obuf[2]],
              base64_encode_map[obuf[3]] );
      break;
    }
    output += 4;
    if( size_t(output-base64data)>out_len-4 )
    {
      return false;	  
    }
    
  }
  *output=0;
  
  return strlen(base64data) > 0; /* return the length of the new data */
}

static void decodeQuantum(unsigned char *dest, const char *src)
{
  //  * Copyright (C) 2001, Andrew Francis and Daniel Stenberg - from libcurl	
  unsigned int x = 0;
  int i;
  for(i = 0; i < 4; i++) {
    if(src[i] >= 'A' && src[i] <= 'Z')
      x = (x << 6) + (unsigned int)(src[i] - 'A' + 0);
    else if(src[i] >= 'a' && src[i] <= 'z')
      x = (x << 6) + (unsigned int)(src[i] - 'a' + 26);
    else if(src[i] >= '0' && src[i] <= '9') 
      x = (x << 6) + (unsigned int)(src[i] - '0' + 52);
    else if(src[i] == '+')
      x = (x << 6) + 62;
    else if(src[i] == '/')
      x = (x << 6) + 63;
  }
  
  dest[2] = (unsigned char)(x & 255); x >>= 8;
  dest[1] = (unsigned char)(x & 255); x >>= 8;
  dest[0] = (unsigned char)(x & 255); x >>= 8;
}

bool jdk_base64_decode( const void *src_, size_t inlen, unsigned char *out, size_t outlen )
{
  //  * Copyright (C) 2001, Andrew Francis and Daniel Stenberg - from libcurl	
  int length = 0;
  int equalsTerm = 0;
  int i;
  unsigned char lastQuantum[3];
  const char *src=(const char*)src_;
  
  while((src[length] != '=') && src[length])
    length++;
  while(src[length+equalsTerm] == '=')
    equalsTerm++;
  
  for(i = 0; i < length/4 - 1; i++) 
  {
    decodeQuantum(out, src);
    out += 3; src += 4;
  }
  
  decodeQuantum(lastQuantum, src);
  for(i = 0; i < 3 - equalsTerm; i++) 
    out[i] = lastQuantum[i];
  
  return (length * 3 / 4) - equalsTerm > 0;	
}



