
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ALIGNED_MALLOC_DEBUG

void	*aligned_malloc( size_t, size_t);
void	aligned_free( void *);

int main( void)
{
    void 	*x;
    int		i;
    
    for( i = 0; i < 10; i++)
    {
    	x = aligned_malloc( 1000*i, 64);
    	aligned_free( x);
    }
    
    return 0;
}


void *aligned_malloc( size_t size, size_t aligment)
{
	size_t	px;
	char 	*p, *pa;
	
#ifdef ALIGNED_MALLOC_DEBUG
	printf( "aligned_malloc(size=%d, aligment=%d) -->\n", size, aligment);
#endif

	p = ( char *) malloc( size + aligment + 1 + sizeof( p));
	
	pa = ( char *) ((( size_t) p + aligment + 1 + sizeof( p)) & ~(aligment-1));

#ifdef ALIGNED_MALLOC_DEBUG
	printf( "original = 0x%p\n", p);
	printf( "new      = 0x%p\n", pa);
#endif

	// save original pointer	
	memmove( pa - sizeof( void *), &p, sizeof( p));	
	
#ifdef ALIGNED_MALLOC_DEBUG
	printf( "aligned_malloc <-- (rslt=0x%p)\n", pa);
#endif
	
	return ( void *) pa;
}

void aligned_free( void *pa)
{
	void *p;

#ifdef ALIGNED_MALLOC_DEBUG
	printf( "free(0x%p) -->\n", pa);
#endif
	
	memmove( &p, (( char *) pa) - sizeof( void *), sizeof( void *));

#ifdef ALIGNED_MALLOC_DEBUG
	printf( "free at 0x%p\n", p);
#endif
	
	free( p);

#ifdef ALIGNED_MALLOC_DEBUG
	printf( "free <-- (OK)\n");
#endif
	
	return;
}
