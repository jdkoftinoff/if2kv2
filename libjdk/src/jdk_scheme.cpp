#include "jdk_world.h"
#include "jdk_scheme.h"
#if 0
#ifdef __cplusplus
extern "C" {
#endif
  
  char *jdk_scheme_type_names[] =
  {
    "sexp",
    "float",
    "int",
    "string",
    "rawdata",
    "lambda",
    "symbol",
    "resolved",
    "native",
    "vecint",
    "vecfloat",
    "vecstring",
    "vecsexp",
    "dict",
    "env"
  };
  
  extern int jdk_scheme_dict_xform_from_char_table[128];
  extern char jdk_scheme_dict_xform_to_char_table[36];

  void jdk_scheme_env_pool_inc_ref(
                                   jdk_scheme_env_pool *self,
                                   void *p
                                   )
  {
    
  }
  
  void jdk_scheme_env_pool_dec_ref(
                                   jdk_scheme_env_pool *self,
                                   void *p
                                   )
  {
  }
                
  
  inline int jdk_scheme_dict_xform_from_char_calc( char c )
  {
    int v;
    if( c>='0' && c<='9' )
    {
      v=c;
    }
    else
    {
      v= (c&0x1f)+10;
    }
    return v;
  }
  
  inline char jdk_scheme_dict_xform_to_char_calc( int v )
  {
    char c;
    if( v>=0 && v<=9 )
    {
      c=v+'0';
    }
    else
    {
      c=v+'a'-10;
    }
    return c;
  }
  
  void jdk_scheme_dict_xform_init( void )
  {
    int i;
    for( i=0; i<128; ++i )
    {
      jdk_scheme_dict_xform_from_char_table[i] = 
        jdk_scheme_dict_xform_from_char_calc(i);
    }
    
    for( i=0; i<36; ++i )
    {
      jdk_scheme_dict_xform_to_char_table[i] =
        jdk_scheme_dict_xform_to_char_calc( i );
    }	
  }
  
#undef JDK_SCHEME_NATIVE
#define JDK_SCHEME_NATIVE(A) \
struct jdk_scheme_sexp_ *jdk_scheme_native_proc_##A( \
	struct jdk_scheme_interpreter_ *interp, \
	struct jdk_scheme_env_ *self, \
	struct jdk_scheme_sexp_ *list \
	)
  
  
  JDK_SCHEME_NATIVE(collectgarbage)
  {
    int i=100;
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    
    jdk_scheme_env_pool_collect_garbage(	interp->env_pool, i );
    jdk_scheme_dict_pool_collect_garbage( interp->dict_pool, i );
    jdk_scheme_sexp_pool_collect_garbage( interp->sexp_pool, i );
    jdk_scheme_obj_pool_collect_garbage( interp->obj_pool, i );
    
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  JDK_SCHEME_NATIVE(trace)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  JDK_SCHEME_NATIVE(typeof)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  JDK_SCHEME_NATIVE(lambda)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(define)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(quote)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(macro)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(let)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(begin)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(while)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(repeat)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(module)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(usemodule)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(getenv)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  JDK_SCHEME_NATIVE(zip)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(map)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(apply)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  JDK_SCHEME_NATIVE(first)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(second)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(third)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(fourth)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(next)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  JDK_SCHEME_NATIVE(clone)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(dump)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(parse)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  JDK_SCHEME_NATIVE(not)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(lt)
    
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(lteq)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(eq)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(gteq)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(gt)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(mul)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(div)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(add)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(muladd)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(sub)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(mod)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  JDK_SCHEME_NATIVE(makevecint)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(makevecfloat)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(makevecstring)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(makevecsexp)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(makedict)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  JDK_SCHEME_NATIVE(vecnot)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(veclt)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(veclteq)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(veceq)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(vecgteq)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(vecmul)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(vecdiv)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(vecadd)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(vecmuladd)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(vecsub)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(vecmod)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  JDK_SCHEME_NATIVE(vecsum)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(vecproduct)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  JDK_SCHEME_NATIVE(vecget)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(vecset)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  JDK_SCHEME_NATIVE(veccount)
  {
    jdk_scheme_sexp_dec_ref( interp, list ); /* throw away any input parameters */
    jdk_scheme_sexp_inc_ref( interp, interp->empty_value );
    return interp->empty_value;
  }
  
  
#ifdef __cplusplus
}
#endif

#endif
