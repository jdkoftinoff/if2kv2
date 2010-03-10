#ifndef _JDK_HEAP_H
#define _JDK_HEAP_H

#ifdef __cplusplus
extern "C" {
#endif


#ifndef JDK_HEAP_PAD_TO_QUAD
#define JDK_HEAP_PAD_TO_QUAD 0
#endif

struct jdk_heap_;
struct jdk_heap_block_;

typedef void * (*jdk_heap_proc)(
						struct  jdk_heap_ *heap,
						uint32 size_in_words
						);


void jdk_heap_error(const char *,...);
void jdk_heap_debug(const char *,...);

typedef struct jdk_heap_
{
	struct jdk_heap_block_ *first;
	int32 size_in_words;
	struct jdk_heap_block_ *last_free;
	struct jdk_heap_ *fallback;
	jdk_heap_proc error_proc;
#if JDK_HEAP_PAD_TO_QUAD
	int32 _pad;
#endif
} jdk_heap;

typedef struct jdk_heap_block_
{
	struct jdk_heap_block_ *next;
	struct jdk_heap_block_ *prev;
	int32 size_in_words; /* negative means free */
#if JDK_HEAP_PAD_TO_QUAD	
	int32 pad_;
#endif
} jdk_heap_block_;



void  jdk_heap_init(struct jdk_heap_ *heap, void *base, int32 size_in_words, struct jdk_heap_ *fallback );
void *jdk_heap_alloc(struct jdk_heap_ *heap,int32 numwords);
void  jdk_heap_free(struct jdk_heap_ *heap,void *ptr);

void  jdk_heap_reset(struct jdk_heap_ *heap);
void  jdk_heap_pack(struct jdk_heap_ *heap, struct jdk_heap_block_ *first );
void  jdk_heap_report(
			struct jdk_heap_ *heap,
			int32 *free_mem,
			int32 *used_mem,
			int32 *free_chunks,
			int32 *used_chunks,
			int32 *largest_free,
			int32 *largest_used
			);

void *jdk_heap_alloc_(struct jdk_heap_ *heap,int32 numwords);
void  jdk_heap_free_(struct jdk_heap_ *heap,void *ptr);

#define JDK_HEAP_BYTES_TO_WORDS(a) (((a)+sizeof(int32)-1)/sizeof(int32))

#ifdef __cplusplus
}
#endif

#endif
