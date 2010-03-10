#ifndef _JDK_BINDIR_H
#define _JDK_BINDIR_H

struct jdk_bindir
{
	char *group;
	char *name;
	unsigned char *data;
	size_t length;
    struct jdk_bindir *next;
};

extern struct jdk_bindir *jdk_bindir_top;
extern struct jdk_bindir *jdk_bindir_bottom;


struct jdk_bindir *jdk_bindir_find( const char *group, const char *name );

static inline void jdk_bindir_register( struct jdk_bindir *d )
{
	if( jdk_bindir_top==0 )
	{
		jdk_bindir_top=d;
	}
	else
	{
		jdk_bindir_bottom->next = d;	
	}

	jdk_bindir_bottom=d;
	
	while( jdk_bindir_bottom->next )
	{
		jdk_bindir_bottom = jdk_bindir_bottom->next;		
	} 	
}


#endif











