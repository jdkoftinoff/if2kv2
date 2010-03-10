#ifndef _JDK_SCHEME_H
#define _JDK_SCHEME_H
#if 0
#include "jdk_world.h"

#ifndef JDK_SCHEME_USE_JDK_HEAP
#define JDK_SCHEME_USE_JDK_HEAP 1
#endif

#if JDK_SCHEME_USE_JDK_HEAP
#include "jdk_heap.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct jdk_scheme_obj_;
struct jdk_scheme_sexp_;
struct jdk_scheme_dict_;
struct jdk_scheme_env_;
struct jdk_scheme_pool_;
struct jdk_scheme_obj_pool_;
struct jdk_scheme_sexp_pool_;
struct jdk_scheme_dict_pool_;
struct jdk_scheme_env_pool_;
struct jdk_scheme_interpreter_;

int jdk_scheme_dict_xform_from_char_table[128];
char jdk_scheme_dict_xform_to_char_table[36];
/** jdk_scheme_native_proc
 *
 *  All native procedures must be of type jdk_scheme_native_proc.
 *  All builtin functions are implemented as native_procs.
 *	A native proc takes the context - an interpreter, the
 *  current environment we are in, and the list of parameters.
 *  A native proc returns a list of objects.
 */

typedef struct jdk_scheme_sexp_ *(*jdk_scheme_native_proc)(
	struct jdk_scheme_interpreter_ *interp, 
	struct jdk_scheme_env_ *env, 
	struct jdk_scheme_sexp_ *params
	);

typedef void (*jdk_scheme_destruct_proc)(
	struct jdk_scheme_interpreter_ *interp,
	void *self
	);


/** jdk_scheme_pool
 * 
 *  Simple engine for managing a pool of generic objects, allowing
 *  fast allocation and release of these objects.  The first int of each
 *  object must be a reference count.
 */
 
typedef struct jdk_scheme_pool_
{
	struct jdk_scheme_interpreter_ *interp;
	jdk_scheme_destruct_proc destruct_proc;	
	void *buf;
	unsigned char *alloc_flags;	
	int max_items;
	int num_items;
	int item_size;
	int next_free_item;
	int last_garbage_check_position;
} jdk_scheme_pool;


/** jdk_scheme_obj_pool
 *
 *	A pool specialized for scheme_obj objects
 */
 
typedef struct jdk_scheme_obj_pool_
{
	jdk_scheme_pool pool;
} jdk_scheme_obj_pool;

/** jdk_scheme_sexp_pool
 *
 *	A pool specialized for scheme_sexp objects
 */
typedef struct jdk_scheme_sexp_pool_
{
	jdk_scheme_pool pool;
} jdk_scheme_sexp_pool;

/** jdk_scheme_dict_pool
 *
 *	A pool specialized for scheme_dict objects
 */
typedef struct jdk_scheme_dict_pool_
{
	jdk_scheme_pool pool;
} jdk_scheme_dict_pool;

/** jdk_scheme_env_pool
 *
 *	A pool specialized for scheme_env objects
 */
typedef struct jdk_scheme_env_pool_
{
	jdk_scheme_pool pool;
} jdk_scheme_env_pool;


/** jdk_scheme_type
 *
 *  type codes for all types of contents of scheme_obj objects.
 *
 */
 
typedef enum 
{
	jdk_scheme_type_sexp=0,
	jdk_scheme_type_float,
	jdk_scheme_type_int,
	jdk_scheme_type_string,
	jdk_scheme_type_rawdata,
	jdk_scheme_type_lambda,
	jdk_scheme_type_symbol_name,
	jdk_scheme_type_resolved_symbol,
	jdk_scheme_type_native_proc,
	jdk_scheme_type_vecint,
	jdk_scheme_type_vecfloat,
	jdk_scheme_type_vecstring,
	jdk_scheme_type_vecsexp,
	jdk_scheme_type_dict,
	jdk_scheme_type_env
} jdk_scheme_type;

extern char *jdk_scheme_type_names[];

/** jdk_scheme_obj
 *
 *  First entry must be 'int ref_cnt'
 */
 
typedef struct jdk_scheme_obj_
{
	int ref_cnt;
	jdk_scheme_destruct_proc destruct; /* todo: set and use destruct */
	jdk_scheme_type type;
	int len; /* only used for values that are pointed to, sometimes in bytes, in chars, or vec elements */
	union
	{
		float float_value;
		int int_value;
		void *rawdata;
		char *symbol_name;
		char *string_value;
		int *vecint;
		float *vecfloat;
		char **vecstring;
		struct jdk_scheme_sexp_ *vecsexp;
		struct jdk_scheme_dict_ *dict;
		struct jdk_scheme_env_ *env;
		struct jdk_scheme_sexp_ *sexp;
		struct jdk_scheme_sexp_ *resolved_symbol;
		jdk_scheme_native_proc native_proc;
	};
} jdk_scheme_obj;

typedef struct jdk_scheme_sexp_
{
	int ref_cnt;
	struct jdk_scheme_obj_ *contents;
	struct jdk_scheme_sexp_ *next;
} jdk_scheme_sexp;

typedef struct jdk_scheme_dict_
{
	int ref_cnt;
	struct jdk_scheme_dict_ *table[36]; 	/* just enough for [0-9] and [a-z] case insensitive, in that order */
	struct jdk_scheme_sexp_ *obj;
} jdk_scheme_dict;

typedef struct jdk_scheme_env_
{
	int ref_cnt;
	struct jdk_scheme_dict_ *globals;
	struct jdk_scheme_env_ *caller;
	struct jdk_scheme_sexp_ *params;
	struct jdk_scheme_dict_ *locals;
	struct jdk_scheme_sexp_ *result;
} jdk_scheme_env;

typedef struct jdk_scheme_interpreter_
{
#if JDK_SCHEME_USE_JDK_HEAP
	jdk_heap heap;	
	int heap_allocated;	
#endif	
	struct jdk_scheme_dict_pool_ *dict_pool;
	struct jdk_scheme_sexp_pool_ *sexp_pool;
	struct jdk_scheme_env_pool_ *env_pool;
	struct jdk_scheme_obj_pool_ *obj_pool;
	
	struct jdk_scheme_env_ *main_env;
	struct jdk_scheme_sexp_ *empty_value;
	struct jdk_scheme_sexp_ *true_value;
	
} jdk_scheme_interpreter;

typedef struct jdk_scheme_init_params_
{
	int num_dict;
	int num_sexp;
	int num_env;
	int num_obj;
#if JDK_SCHEME_USE_JDK_HEAP
	void *heap_start;
	int heap_size;
#endif
} jdk_scheme_init_params;


/* memory allocation */

void * jdk_scheme_malloc( 
	jdk_scheme_interpreter *self, 
	int bytecount 
	);
	
void jdk_scheme_free( 
	jdk_scheme_interpreter *self, 
	void *o 
	);


/* generic pool functions */

void jdk_scheme_pool_create(
	jdk_scheme_interpreter *interp,
	jdk_scheme_pool *self,
	int num_items,
	int item_size,
	jdk_scheme_destruct_proc destruct_proc
	);
	
void jdk_scheme_pool_kill(
	jdk_scheme_pool *self 
	);
	
void *jdk_scheme_pool_alloc(
	jdk_scheme_pool *self
	);
	
void jdk_scheme_pool_free(
	jdk_scheme_pool *self,
	void *p
	);	
	
void jdk_scheme_pool_inc_ref(
	jdk_scheme_pool *self,
	void *p
	);	

void jdk_scheme_pool_dec_ref(
	jdk_scheme_pool *self,
	void *p
	);	

void jdk_scheme_pool_collect_garbage(
	jdk_scheme_pool *self,
	int iterations
	);
	
void *jdk_scheme_pool_get_addr(
	jdk_scheme_pool *self,
	int item
	);

int jdk_scheme_pool_get_num(
	jdk_scheme_pool *self,
	void *p
	);

/* jdk_scheme_obj pool functions */	

jdk_scheme_obj_pool *jdk_scheme_obj_pool_create( 
	jdk_scheme_interpreter *interp, 
	int num 
	);

void jdk_scheme_obj_pool_kill(
	jdk_scheme_obj_pool *self 
	);

jdk_scheme_obj *jdk_scheme_obj_pool_alloc(
	jdk_scheme_obj_pool *self
	);

void jdk_scheme_obj_pool_free(
	jdk_scheme_obj_pool *self,
	jdk_scheme_obj *o
	);

void jdk_scheme_obj_pool_inc_ref(
	jdk_scheme_obj_pool *self,
	void *p
	);	

void jdk_scheme_obj_pool_dec_ref(
	jdk_scheme_obj_pool *self,
	void *p
	);	
	
void jdk_scheme_obj_pool_collect_garbage(
	jdk_scheme_obj_pool *self,
	int iterations
	);


/* jdk_scheme_sexp pool functions */	

jdk_scheme_sexp_pool *jdk_scheme_sexp_pool_create( 
	jdk_scheme_interpreter *interp,
	int num
	);
	
void jdk_scheme_sexp_pool_kill(
	jdk_scheme_sexp_pool *self 
	);	

jdk_scheme_sexp *jdk_scheme_sexp_pool_alloc(
	jdk_scheme_sexp_pool *self
	);

void jdk_scheme_sexp_pool_free(
	jdk_scheme_sexp_pool *self,
	jdk_scheme_sexp *o
	);

void jdk_scheme_sexp_pool_inc_ref(
	jdk_scheme_sexp_pool *self,
	void *p
	);	

void jdk_scheme_sexp_pool_dec_ref(
	jdk_scheme_sexp_pool *self,
	void *p
	);	

void jdk_scheme_sexp_pool_collect_garbage(
	jdk_scheme_sexp_pool *self,
	int iterations
	);

/* jdk_scheme_dict pool functions */	

jdk_scheme_dict_pool *jdk_scheme_dict_pool_create(
	jdk_scheme_interpreter *interp,
	int num
	);
	
void jdk_scheme_dict_pool_kill(	
	jdk_scheme_dict_pool *self 
	);		

jdk_scheme_dict *jdk_scheme_dict_pool_alloc(
	jdk_scheme_dict_pool *self
	);

void jdk_scheme_dict_pool_free(
	jdk_scheme_dict_pool *self,
	jdk_scheme_dict *o
	);

void jdk_scheme_dict_pool_inc_ref(
	jdk_scheme_dict_pool *self,
	void *p
	);	

void jdk_scheme_dict_pool_dec_ref(
	jdk_scheme_dict_pool *self,
	void *p
	);	

void jdk_scheme_dict_pool_collect_garbage(
	jdk_scheme_dict_pool *self,
	int iterations
	);


/* jdk_scheme_env pool functions */	
		
jdk_scheme_env_pool *jdk_scheme_env_pool_create(
	jdk_scheme_interpreter *interp,
	int num
	);
	
void jdk_scheme_env_pool_kill(
	jdk_scheme_env_pool *self 
	);		
	
jdk_scheme_env *jdk_scheme_env_pool_alloc(
	jdk_scheme_env_pool *self
	);

void jdk_scheme_env_pool_free(
	jdk_scheme_env_pool *self,
	jdk_scheme_env *o
	);
	
void jdk_scheme_env_pool_inc_ref(
	jdk_scheme_env_pool *self,
	void *p
	);	

void jdk_scheme_env_pool_dec_ref(
	jdk_scheme_env_pool *self,
	void *p
	);	
	
void jdk_scheme_env_pool_collect_garbage(
	jdk_scheme_env_pool *self,
	int iterations
	);


/* jdk_scheme_obj functions */

/* various creation functions */
jdk_scheme_obj *jdk_scheme_obj_create_sexp( jdk_scheme_interpreter *interp );
jdk_scheme_obj *jdk_scheme_obj_create_float( jdk_scheme_interpreter *interp, float val );
jdk_scheme_obj *jdk_scheme_obj_create_int( jdk_scheme_interpreter *interp, int val );
jdk_scheme_obj *jdk_scheme_obj_create_string( jdk_scheme_interpreter *interp, const char *s );
jdk_scheme_obj *jdk_scheme_obj_create_rawdata( jdk_scheme_interpreter *interp, void *raw, int length );
jdk_scheme_obj *jdk_scheme_obj_create_lambda( jdk_scheme_interpreter *interp, jdk_scheme_sexp *sexp );
jdk_scheme_obj *jdk_scheme_obj_create_resolved_symbol( jdk_scheme_interpreter *interp, jdk_scheme_sexp *sexp );
jdk_scheme_obj *jdk_scheme_obj_create_symbol_name( jdk_scheme_interpreter *interp, const char *fname );
jdk_scheme_obj *jdk_scheme_obj_create_native_proc( jdk_scheme_interpreter *interp, jdk_scheme_native_proc proc );
jdk_scheme_obj *jdk_scheme_obj_create_vecint( jdk_scheme_interpreter *interp, int initial_size );
jdk_scheme_obj *jdk_scheme_obj_create_vecfloat( jdk_scheme_interpreter *interp, int initial_size );
jdk_scheme_obj *jdk_scheme_obj_create_vecstring( jdk_scheme_interpreter *interp, int initial_size );
jdk_scheme_obj *jdk_scheme_obj_create_vecsexp( jdk_scheme_interpreter *interp, int initial_size );
jdk_scheme_obj *jdk_scheme_obj_create_dict( jdk_scheme_interpreter *interp );
jdk_scheme_obj *jdk_scheme_obj_create_env( jdk_scheme_interpreter *interp );

void jdk_scheme_obj_inc_ref(
	jdk_scheme_interpreter *interp,
	jdk_scheme_obj *self
	);

void jdk_scheme_obj_dec_ref(
	jdk_scheme_interpreter *interp,
	jdk_scheme_obj *self
	);
	
void jdk_scheme_obj_free(
	jdk_scheme_interpreter *interp,
	jdk_scheme_obj *self
	);

void jdk_scheme_obj_destruct(
	struct jdk_scheme_interpreter_ *interp,
	void *self
	);

/* sexp functions */

/* allocate an sexp with specified object as contents. may be null for empty list */

jdk_scheme_sexp *jdk_scheme_sexp_create(
	jdk_scheme_interpreter *interp,
	jdk_scheme_obj *contents
	);
	
jdk_scheme_sexp *jdk_scheme_sexp_create_next(
	jdk_scheme_interpreter *interp,
	jdk_scheme_sexp *sexp,
	jdk_scheme_obj *contents
	);

void jdk_scheme_sexp_inc_ref(
	jdk_scheme_interpreter *interp,
	jdk_scheme_sexp *self
	);

void jdk_scheme_sexp_dec_ref(
	jdk_scheme_interpreter *interp,
	jdk_scheme_sexp *self
	);

/* free the sexp list */
 
void jdk_scheme_sexp_free(
	jdk_scheme_interpreter *interp,
	jdk_scheme_sexp *self
	);

void jdk_scheme_sexp_destruct(
	struct jdk_scheme_interpreter_ *interp,
	void *self
	);

/* jdk_scheme_dict functions */

jdk_scheme_dict * jdk_scheme_dict_create( 
	jdk_scheme_interpreter *interp 
	);

void jdk_scheme_dict_inc_ref(
	jdk_scheme_interpreter *interp,
	jdk_scheme_dict *self
	);

void jdk_scheme_dict_dec_ref(
	jdk_scheme_interpreter *interp,
	jdk_scheme_dict *self
	);
	
void jdk_scheme_dict_free( 
	jdk_scheme_interpreter *interp, 
	jdk_scheme_dict *self 
	);

void jdk_scheme_dict_destruct( 
	struct jdk_scheme_interpreter_ *interp, 
	void *self 
	);
			
void jdk_scheme_dict_add( 
	jdk_scheme_interpreter *interp, 
	jdk_scheme_dict *self, 
	const char *name, 
	jdk_scheme_sexp *obj 
	);
	
void jdk_scheme_dict_remove( 
	jdk_scheme_interpreter *interp, 
	jdk_scheme_dict *self, 
	const char *name 
	);
	
jdk_scheme_sexp * jdk_scheme_dict_find( 
	jdk_scheme_interpreter *interp, 
	jdk_scheme_dict *self, 
	const char *name 
	);
	
void jdk_scheme_dict_dump(
	jdk_scheme_interpreter *interp,
	jdk_scheme_dict *self
	);

/* jdk_scheme_env functions */

jdk_scheme_env * jdk_scheme_env_create( 
	jdk_scheme_interpreter *interp 
	);
	
void jdk_scheme_env_inc_ref(
	jdk_scheme_interpreter *interp,
	jdk_scheme_env *self
	);

void jdk_scheme_env_dec_ref(
	jdk_scheme_interpreter *interp,
	jdk_scheme_env *self
	);
		
void jdk_scheme_env_free( 
	jdk_scheme_interpreter *interp, 
	jdk_scheme_env *self 
	);
	
void jdk_scheme_env_destruct( 
	struct jdk_scheme_interpreter_ *interp, 
	void *self 
	);	
	
jdk_scheme_sexp * jdk_scheme_env_find_sexp( 
	jdk_scheme_interpreter *interp, 
	jdk_scheme_env *self, 
	const char *name 
	);

jdk_scheme_sexp * jdk_scheme_env_resolve_symbol(
	jdk_scheme_interpreter *interp,
	jdk_scheme_env *self,
	const char *name
	);


/* scheme_interpreter functions */


jdk_scheme_interpreter *jdk_scheme_interpreter_create( 
	jdk_scheme_init_params *init_params /* TODO: create empty list and true list */
	);

void jdk_scheme_interpreter_kill( 
	jdk_scheme_interpreter *self 
	);

void jdk_scheme_interpreter_def_global_native_proc(
	jdk_scheme_interpreter *self,
	const char *symbol_name,
	jdk_scheme_native_proc *native_proc
	);
	
void jdk_scheme_interpreter_def_global_sexp(
	jdk_scheme_interpreter *self,
	const char *symbol_name,
	jdk_scheme_sexp *sexp
	);

void jdk_scheme_interpreter_def_global_string(
	jdk_scheme_interpreter *self,
	const char *symbol_name,
	const char *v
	);

void jdk_scheme_interpreter_def_global_float(
	jdk_scheme_interpreter *self,
	const char *symbol_name,
	float v
	);



/* the evaluator */

jdk_scheme_sexp *jdk_scheme_eval( 
	jdk_scheme_interpreter *interp, 
	jdk_scheme_env *self, 
	jdk_scheme_sexp *list 
	);


/* the builtin functions */

#define JDK_SCHEME_NATIVE(A) \
struct jdk_scheme_sexp_ *jdk_scheme_native_proc_##A( \
	struct jdk_scheme_interpreter_ *interp, \
	struct jdk_scheme_env_ *self, \
	struct jdk_scheme_sexp_ *list \
	)

JDK_SCHEME_NATIVE(collectgarbage);
JDK_SCHEME_NATIVE(trace);
JDK_SCHEME_NATIVE(typeof);
JDK_SCHEME_NATIVE(lambda);
JDK_SCHEME_NATIVE(define);
JDK_SCHEME_NATIVE(quote);
JDK_SCHEME_NATIVE(macro);
JDK_SCHEME_NATIVE(let);
JDK_SCHEME_NATIVE(begin);
JDK_SCHEME_NATIVE(while);
JDK_SCHEME_NATIVE(repeat);
JDK_SCHEME_NATIVE(module);
JDK_SCHEME_NATIVE(usemodule);
JDK_SCHEME_NATIVE(getenv);

JDK_SCHEME_NATIVE(zip);
JDK_SCHEME_NATIVE(map);
JDK_SCHEME_NATIVE(apply);

JDK_SCHEME_NATIVE(first);
JDK_SCHEME_NATIVE(second);
JDK_SCHEME_NATIVE(third);
JDK_SCHEME_NATIVE(fourth);
JDK_SCHEME_NATIVE(next);

JDK_SCHEME_NATIVE(clone);
JDK_SCHEME_NATIVE(dump);
JDK_SCHEME_NATIVE(parse);

JDK_SCHEME_NATIVE(not);
JDK_SCHEME_NATIVE(lt);	
JDK_SCHEME_NATIVE(lteq);
JDK_SCHEME_NATIVE(eq);
JDK_SCHEME_NATIVE(gteq);
JDK_SCHEME_NATIVE(gt);

JDK_SCHEME_NATIVE(mul);
JDK_SCHEME_NATIVE(div);
JDK_SCHEME_NATIVE(add);
JDK_SCHEME_NATIVE(muladd);
JDK_SCHEME_NATIVE(sub);
JDK_SCHEME_NATIVE(mod);

JDK_SCHEME_NATIVE(makevecint);
JDK_SCHEME_NATIVE(makevecfloat);
JDK_SCHEME_NATIVE(makevecstring);
JDK_SCHEME_NATIVE(makevecsexp);
JDK_SCHEME_NATIVE(makedict);

JDK_SCHEME_NATIVE(vecnot);
JDK_SCHEME_NATIVE(veclt);
JDK_SCHEME_NATIVE(veclteq);
JDK_SCHEME_NATIVE(veceq);
JDK_SCHEME_NATIVE(vecgteq);
JDK_SCHEME_NATIVE(vecmul);
JDK_SCHEME_NATIVE(vecdiv);
JDK_SCHEME_NATIVE(vecadd);
JDK_SCHEME_NATIVE(vecmuladd);
JDK_SCHEME_NATIVE(vecsub);
JDK_SCHEME_NATIVE(vecmod);

JDK_SCHEME_NATIVE(vecsum);
JDK_SCHEME_NATIVE(vecproduct);

JDK_SCHEME_NATIVE(vecget);
JDK_SCHEME_NATIVE(vecset);
JDK_SCHEME_NATIVE(veccount);

void jdk_scheme_register_builtins(
	jdk_scheme_interpreter *interp
	);	

inline jdk_scheme_obj *jdk_scheme_first( jdk_scheme_sexp *o )  { return o->contents; }
inline jdk_scheme_sexp *jdk_scheme_next( jdk_scheme_sexp *o ) { return o->next; }

extern int jdk_scheme_dict_xform_from_char_table[128];
extern char jdk_scheme_dict_xform_to_char_table[36];
void jdk_scheme_dict_xform_init( void );


/* inline versions of functions */
#ifndef JDK_INLINE
#define JDK_INLINE __inline
#endif


JDK_INLINE void * jdk_scheme_malloc( 
	jdk_scheme_interpreter *self, 
	int bytecount 
	)
{
#if JDK_SCHEME_USE_JDK_HEAP
	return jdk_heap_alloc( &self->heap, bytecount );
#else
	return malloc( bytecount );
#endif
}

	
JDK_INLINE void jdk_scheme_free( 
	jdk_scheme_interpreter *self, 
	void *o 
	)
{
#if JDK_SCHEME_USE_JDK_HEAP
	jdk_heap_free( &self->heap, o );
#else	
	free( o );
#endif
}


	


JDK_INLINE void jdk_scheme_pool_create(
	jdk_scheme_interpreter *interp,
	jdk_scheme_pool *self,
	int num_items,
	int item_size,
	jdk_scheme_destruct_proc destruct_proc
	)
{
	int i;
	self->interp = interp;
	self->destruct_proc = destruct_proc;
	self->buf = jdk_scheme_malloc( interp, num_items * item_size );
	self->alloc_flags = (unsigned char *)jdk_scheme_malloc( interp, num_items );
	for( i=0; i<num_items; ++i )
		self->alloc_flags[i] = 0;
	self->next_free_item=0;
	self->last_garbage_check_position=0;
	self->max_items = num_items;
	self->item_size = item_size;
	self->num_items = 0;
}

JDK_INLINE void jdk_scheme_pool_kill(
	jdk_scheme_pool *self 
	)
{
	int i;
	
	/* call destructors for all items still allocated, and free them */
	if( self->destruct_proc )
	{
		for( i=0; i<self->max_items; ++i )
		{
			if( self->alloc_flags[i] )
			{
				self->alloc_flags[i] = 0;
				self->destruct_proc( 
					self->interp, 
					(void *)( (char *)self->buf + (i * self->item_size))
					);
			}
		}
	}

	jdk_scheme_free( self->interp, self );
}
	
	
JDK_INLINE void *jdk_scheme_pool_alloc(
	jdk_scheme_pool *self
	)
{
	void *result = 0;
	
	/* do we already know which item is free? */
	if( self->next_free_item==-1 )
	{
		/* no, unfortunately we have to do an exhaustive search */
		int i;
		for( i=0; i<self->max_items; ++i )
		{
			if( self->alloc_flags[i] == 0 )
			{
				self->next_free_item=i;
				break;
			}
		}
	}
	
	/* do we have a free item? */
	if( self->next_free_item )
	{
		int i = self->next_free_item;
		/* yes, calculate the address and mark it used */
		result = (void *)( (char *)self->buf + (i * self->item_size));		
		self->alloc_flags[ i ] = 1;
		
		++i;
		if( i==self->max_items )
			i=0;
			
		if( self->alloc_flags[i]==0 )
			self->next_free_item=i; 	 /* handy, next free item WAS next */
		else
			self->next_free_item=-1;	 /* dunno where next free item is yet */			
	}
	
	return result;
}
	
JDK_INLINE void jdk_scheme_pool_free(
	jdk_scheme_pool *self,
	void *p
	)
{
	uint32 pc = (uint32)p;
	uint32 top = (uint32)self->buf;
	int i = (pc - top) / self->item_size;
	
	if( self->alloc_flags[i] )
	{
		self->alloc_flags[i] = 0;
		
		/* call the destructor for this object */
		if( self->destruct_proc )
		{
			self->destruct_proc( 
					self->interp, 
					p
					);		
		}
		
		/* if we did not already know a next free item, use this one as it */
		if( self->next_free_item==-1 )
			self->next_free_item = i;
	}
}
	
JDK_INLINE void jdk_scheme_pool_inc_ref(
	jdk_scheme_pool *self,
	void *p
	)
{
	uint32 pc = (uint32)p;
	uint32 top = (uint32)self->buf;
	int item = (pc - top) / self->item_size;
	
	if( self->alloc_flags[item] )
	{
		unsigned int *refcnt;
		refcnt = (unsigned int *)p;
		++(*refcnt);	
	}
}

JDK_INLINE void jdk_scheme_pool_dec_ref(
	jdk_scheme_pool *self,
	void *p
	)
{
	uint32 pc = (uint32)p;
	uint32 top = (uint32)self->buf;
	int i = (pc - top) / self->item_size;
	
	if( self->alloc_flags[i] )
	{
		unsigned int *refcnt = (unsigned int *)((char *)self->buf + (self->item_size * i));
		--(*refcnt);
		if( *refcnt==0 )
		{
			self->alloc_flags[i] = 0;
		
			/* call the destructor for this object */
			if( self->destruct_proc )
			{
				self->destruct_proc( 
						self->interp, 
						p
						);		
			}
		
			/* if we did not already know a next free item, use this one as it */
			if( self->next_free_item==-1 )
				self->next_free_item = i;		
		}	
	}
}

JDK_INLINE void jdk_scheme_pool_collect_garbage(
	jdk_scheme_pool *self,
	int iterations
	)
{
	int i;
	int pos = self->last_garbage_check_position;
	unsigned int *refcnt=0;
	
	if( iterations > self->max_items )
	{
		iterations = self->max_items;
	}
			
	for( i=0; i<iterations; ++i )
	{
		/* if the garbage check position is allocated,
		 * and the refcnt is 0, then free the item
		 */
		
		if( self->alloc_flags[pos] )
		{
			refcnt = (unsigned int *)((char *)self->buf + (self->item_size * pos));
			if( *refcnt==0 )
			{
				self->alloc_flags[pos] = 0;
			}
		}
		
		++pos;
		if( pos==self->max_items )
			pos=0;
	}
}


JDK_INLINE void *jdk_scheme_pool_get_addr(
	jdk_scheme_pool *self,
	int item
	)
{
	return ((char *)self->buf + (self->item_size * item));
}

JDK_INLINE int jdk_scheme_pool_get_num(
	jdk_scheme_pool *self,
	void *p
	)
{
	uint32 pc = (uint32)p;
	uint32 top = (uint32)self->buf;
	int item = (pc - top) / self->item_size;
	return item;
}


/* jdk_scheme_obj pool functions */	

JDK_INLINE jdk_scheme_obj_pool *jdk_scheme_obj_pool_create( 
	jdk_scheme_interpreter *interp, 
	int num 
	)
{
	jdk_scheme_obj_pool *self = (jdk_scheme_obj_pool *)jdk_scheme_malloc( interp, sizeof( jdk_scheme_obj_pool ) );
	
	jdk_scheme_pool_create(
		interp,
		&self->pool,
		num,
		sizeof( jdk_scheme_obj ),
		jdk_scheme_obj_destruct
		);
	return self;
}

JDK_INLINE void jdk_scheme_obj_pool_kill(
	jdk_scheme_obj_pool *self 
	)
{
	jdk_scheme_pool_kill( &self->pool );
	jdk_scheme_free( self->pool.interp, self );
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_pool_alloc(
	jdk_scheme_obj_pool *self
	)
{
	return (jdk_scheme_obj *)jdk_scheme_pool_alloc( &self->pool );
}

JDK_INLINE void jdk_scheme_obj_pool_free(
	jdk_scheme_obj_pool *self,
	jdk_scheme_obj *o
	)
{
	jdk_scheme_pool_free( &self->pool, o );
}


JDK_INLINE void jdk_scheme_obj_pool_inc_ref(
	jdk_scheme_obj_pool *self,
	void *p
	)
{
	jdk_scheme_pool_inc_ref( &self->pool, p );
}	

JDK_INLINE void jdk_scheme_obj_pool_dec_ref(
	jdk_scheme_obj_pool *self,
	void *p
	)
{
	jdk_scheme_pool_dec_ref( &self->pool, p );
}	


void jdk_scheme_obj_pool_collect_garbage(
	jdk_scheme_obj_pool *self,
	int iterations
	)
{
	jdk_scheme_pool_collect_garbage( &self->pool, iterations );
}


/* jdk_scheme_sexp pool functions */	

JDK_INLINE jdk_scheme_sexp_pool *jdk_scheme_sexp_pool_create( 
	jdk_scheme_interpreter *interp, 
	int num 
	)
{
	jdk_scheme_sexp_pool *self = (jdk_scheme_sexp_pool *)jdk_scheme_malloc( interp, sizeof( jdk_scheme_sexp_pool ) );
	
	jdk_scheme_pool_create(
		interp,
		&self->pool,
		num,
		sizeof( jdk_scheme_sexp ),
		jdk_scheme_sexp_destruct
		);
	return self;
}

JDK_INLINE void jdk_scheme_sexp_pool_kill(
	jdk_scheme_sexp_pool *self 
	)
{
	jdk_scheme_pool_kill( &self->pool );
	jdk_scheme_free( self->pool.interp, self );
}

JDK_INLINE jdk_scheme_sexp *jdk_scheme_sexp_pool_alloc(
	jdk_scheme_sexp_pool *self
	)
{
	return (jdk_scheme_sexp *)jdk_scheme_pool_alloc( &self->pool );
}

JDK_INLINE void jdk_scheme_sexp_pool_free(
	jdk_scheme_sexp_pool *self,
	jdk_scheme_sexp *o
	)
{
	jdk_scheme_pool_free( &self->pool, o );
}


JDK_INLINE void jdk_scheme_sexp_pool_inc_ref(
	jdk_scheme_sexp_pool *self,
	void *p
	)
{
	jdk_scheme_pool_inc_ref( &self->pool, p );
}	

JDK_INLINE void jdk_scheme_sexp_pool_dec_ref(
	jdk_scheme_sexp_pool *self,
	void *p
	)
{
	jdk_scheme_pool_dec_ref( &self->pool, p );
}	


void jdk_scheme_sexp_pool_collect_garbage(
	jdk_scheme_sexp_pool *self,
	int iterations
	)
{
	jdk_scheme_pool_collect_garbage( &self->pool, iterations );
}


/* jdk_scheme_dict pool functions */	

JDK_INLINE jdk_scheme_dict_pool *jdk_scheme_dict_pool_create( 
	jdk_scheme_interpreter *interp, 
	int num 
	)
{
	jdk_scheme_dict_pool *self = (jdk_scheme_dict_pool *)jdk_scheme_malloc( interp, sizeof( jdk_scheme_dict_pool ) );
	
	jdk_scheme_pool_create(
		interp,
		&self->pool,
		num,
		sizeof( jdk_scheme_dict ),
		jdk_scheme_dict_destruct
		);
	return self;
}

JDK_INLINE void jdk_scheme_dict_pool_kill(
	jdk_scheme_dict_pool *self 
	)
{
	jdk_scheme_pool_kill( &self->pool );
	jdk_scheme_free( self->pool.interp, self );

}

JDK_INLINE jdk_scheme_dict *jdk_scheme_dict_pool_alloc(
	jdk_scheme_dict_pool *self
	)
{
	return (jdk_scheme_dict *)jdk_scheme_pool_alloc( &self->pool );
}

JDK_INLINE void jdk_scheme_dict_pool_free(
	jdk_scheme_dict_pool *self,
	jdk_scheme_dict *o
	)
{
	jdk_scheme_pool_free( &self->pool, o );
}


JDK_INLINE void jdk_scheme_dict_pool_inc_ref(
	jdk_scheme_dict_pool *self,
	void *p
	)
{
	jdk_scheme_pool_inc_ref( &self->pool, p );
}	

JDK_INLINE void jdk_scheme_dict_pool_dec_ref(
	jdk_scheme_dict_pool *self,
	void *p
	)
{
	jdk_scheme_pool_dec_ref( &self->pool, p );
}	


void jdk_scheme_dict_pool_collect_garbage(
	jdk_scheme_dict_pool *self,
	int iterations
	)
{
	jdk_scheme_pool_collect_garbage( &self->pool, iterations );
}


/* jdk_scheme_env pool functions */	

JDK_INLINE jdk_scheme_env_pool *jdk_scheme_env_pool_create( 
	jdk_scheme_interpreter *interp, 
	int num 
	)
{
	jdk_scheme_env_pool *self = (jdk_scheme_env_pool *)jdk_scheme_malloc( interp, sizeof( jdk_scheme_env_pool ) );
	
	jdk_scheme_pool_create(
		interp,
		&self->pool,
		num,
		sizeof( jdk_scheme_env ),
		jdk_scheme_env_destruct
		);
	return self;
}

JDK_INLINE void jdk_scheme_env_pool_kill(
	jdk_scheme_env_pool *self 
	)
{
	jdk_scheme_pool_kill( &self->pool );
	jdk_scheme_free( self->pool.interp, self );
}

JDK_INLINE jdk_scheme_env *jdk_scheme_env_pool_alloc(
	jdk_scheme_env_pool *self
	)
{
	return (jdk_scheme_env *)jdk_scheme_pool_alloc( &self->pool );
}

JDK_INLINE void jdk_scheme_env_pool_free(
	jdk_scheme_env_pool *self,
	jdk_scheme_env *o
	)
{
	jdk_scheme_pool_free( &self->pool, o );
}


JDK_INLINE void jdk_scheme_env_pool_inc_ref(
	jdk_scheme_env_pool *self,
	void *p
	)
{
	jdk_scheme_pool_inc_ref( &self->pool, p );
}	

JDK_INLINE void jdk_scheme_env_pool_dec_ref(
	jdk_scheme_env_pool *self,
	void *p
	)
{
	jdk_scheme_pool_dec_ref( &self->pool, p );
}	


JDK_INLINE void jdk_scheme_env_pool_collect_garbage(
	jdk_scheme_env_pool *self,
	int iterations
	)
{
	jdk_scheme_pool_collect_garbage( &self->pool, iterations );
}



JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_sexp( jdk_scheme_interpreter *interp )
{
	jdk_scheme_obj *o = jdk_scheme_obj_pool_alloc( interp->obj_pool );
	o->type = jdk_scheme_type_sexp;
	o->len=0;
	o->ref_cnt=0;
	o->sexp=0;
	return o;	
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_float( jdk_scheme_interpreter *interp, float val )
{
	jdk_scheme_obj *o = jdk_scheme_obj_pool_alloc( interp->obj_pool );
	o->type = jdk_scheme_type_float;
	o->len=0;
	o->ref_cnt=0;
	o->float_value=0.0f;
	return o;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_int( jdk_scheme_interpreter *interp, int val )
{
	jdk_scheme_obj *o = jdk_scheme_obj_pool_alloc( interp->obj_pool );
	o->type = jdk_scheme_type_int;
	o->len=0;
	o->ref_cnt=0;
	o->int_value=0;
	return o;	
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_string( jdk_scheme_interpreter *interp, const char *s )
{
	jdk_scheme_obj *o = jdk_scheme_obj_pool_alloc( interp->obj_pool );
	o->type = jdk_scheme_type_string;
	o->len=strlen(s);
	o->ref_cnt=0;
	o->string_value = (char *)jdk_scheme_malloc( interp, o->len );
	memcpy( o->string_value, s, o->len );
	return o;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_rawdata( jdk_scheme_interpreter *interp, void *raw, int length )
{
	jdk_scheme_obj *o = jdk_scheme_obj_pool_alloc( interp->obj_pool );
	o->type = jdk_scheme_type_rawdata;
	o->len=length;
	o->ref_cnt=0;
	o->rawdata = raw;
	return o;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_lambda( jdk_scheme_interpreter *interp, jdk_scheme_sexp *sexp )
{
	return 0;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_resolved_symbol( jdk_scheme_interpreter *interp, jdk_scheme_sexp *sexp )
{
	return 0;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_symbol_name( jdk_scheme_interpreter *interp, const char *fname )
{
	return 0;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_native_proc( jdk_scheme_interpreter *interp, jdk_scheme_native_proc proc )
{
	return 0;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_vecint( jdk_scheme_interpreter *interp, int initial_size )
{
	return 0;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_vecfloat( jdk_scheme_interpreter *interp, int initial_size )
{
	return 0;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_vecstring( jdk_scheme_interpreter *interp, int initial_size )
{
	return 0;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_vecsexp( jdk_scheme_interpreter *interp, int initial_size )
{
	return 0;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_dict( jdk_scheme_interpreter *interp )
{
	return 0;
}

JDK_INLINE jdk_scheme_obj *jdk_scheme_obj_create_env( jdk_scheme_interpreter *interp )
{
	return 0;
}

#ifdef __cplusplus
}
#endif
#endif
#endif

