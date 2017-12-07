
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)
#include <string.h>
#include <elf.h>

typedef struct _ring_trace_t
{
   char                  eyec[8];
    /*                    12345678 */
#define RING_TRACE_EYEC "%RTRACE%"
    char                 id[16];       /* id follows eyec */
    char                 time[32];
    void		*nxt;
    void		*addr;
    char		*dump;
    char		*text;
    int                  size;
    int                  type;
#define RING_TRACE_TEXT        0x0001
#define RING_TRACE_DUMP        0x0002
    int                  dump_len;
    int			 filler1;
    pthread_t            thread;
} ring_trace_t;

typedef struct _ring_trace_64le
{
   char                  eyec[8];
    /*                    12345678 */
#define RING_TRACE_EYEC "%RTRACE%"
    char                 id[16];       /* id follows eyec */
    char                 time[32];
    long                 nxt;
    long                 addr;
    long                 dump;
    long                 text;
	int                  size;
    int                  type;
#define RING_TRACE_TEXT        0x0001
#define RING_TRACE_DUMP        0x0002
    int                  dump_len;
	int									filler1;
    pthread_t            thread;
} ring_trace_64le_t;

typedef struct _ring_trace_64be
{
   char                  eyec[8];
    /*                    12345678 */
#define RING_TRACE_EYEC "%RTRACE%"
    char                 id[16];       /* id follows eyec */
    char                 time[32];
    long                 nxt;
    long                 addr;
    long                 dump;
    long                 text;
    int                  size;
    int                  type;
#define RING_TRACE_TEXT        0x0001
#define RING_TRACE_DUMP        0x0002
    int                  dump_len;
    int			filler1;
    pthread_t            thread;
} ring_trace_64be_t;


typedef struct _ring_trace_32le
{
   char                  eyec[8];
    /*                    12345678 */
#define RING_TRACE_EYEC "%RTRACE%"
    char                 id[16];       /* id follows eyec */
    char                 time[32];
    int                  nxt;
    int                  addr;
    int                  dump;
    int                  text;
    int                  size;
    int                  type;
#define RING_TRACE_TEXT        0x0001
#define RING_TRACE_DUMP        0x0002
    int                  dump_len;
    int			 filler1;
    int                  thread;
} ring_trace_32le_t;

typedef struct _ring_trace_32be
{
   char                  eyec[8];
    /*                    12345678 */
#define RING_TRACE_EYEC "%RTRACE%"
    char                 id[16];       /* id follows eyec */
    char                 time[32];
    int                  nxt;
    int                  addr;
    int                  dump;
    int                  text;
    int                  size;
    int                  type;
#define RING_TRACE_TEXT        0x0001
#define RING_TRACE_DUMP        0x0002
    int                  dump_len;
    int			 filler1;
    int                  thread;
} ring_trace_32be_t;

int print_rt_text( ring_trace_t *);
int print_rt_dump( ring_trace_t *);
int dump( char *text, char *addr, size_t len);
// void dump( unsigned char *p, unsigned char *, int len);
int swap_byte_order( void *p, size_t size);


Elf64_Ehdr	*elf_header;

int main( int argc, char *argv[])
{
    char 			filename[256], *addr, *p;
    int				fd, i, bit32, le;
    struct stat 	sb;
    size_t			length;
    ring_trace_t	srt;

    if ( argc < 2)
    {
	printf( "rt_core - RingTrace extractor\n");
	printf( "usage:\n\trt_core dumpfile\n");
	return 16;
    }

    strcpy( filename, argv[1]);

    printf( "Extract MemoryTrace from corefile %s\n", filename);
    printf( "================================================================\n");

    printf( "opening '%s'\n", filename);

    fd = open( filename, O_RDONLY);
    if (fd == -1) { perror("open"); exit( 16); }

    if (fstat(fd, &sb) == -1) { perror( "fstat");  exit( 16); }

    length = sb.st_size;

    /* offset for mmap() must be page aligned */
    addr = mmap( NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if ( addr == MAP_FAILED) { perror( "mmap"); exit( 16); }
/*
    printf( "mapped @ 0x%p, size = %d\n", addr, length);
    printf( "sizeof( int)       = %d\n", sizeof( int));
    printf( "sizeof( long)      = %d\n", sizeof( long));
    printf( "sizeof( void *)    = %d\n", sizeof( void *));
    printf( "sizeof( pthread_t) = %d\n", sizeof( pthread_t));
    printf( "================================================================\n");

    printf( "ELF header check\n");
    printf( "Magic byte 0 = %.2X\n", *addr);
    for ( i = 1; i < 4; i++)
	printf( "Magic byte %d = %.2X(%c)\n", i, addr[i], addr[i]);
    bit32 = ( addr[4] == ELFCLASS32) ? 1 : 0;
    printf( "Word size    = %.2X -> %s-bit\n", addr[4], ( bit32) ? "32" : "64");
    le = ( addr[5] == ELFDATA2LSB) ? 1 : 0;
    printf( "Byte order   = %.2X -> %s endian\n", addr[5], ( le) ? "little" : "BIG");

    printf( "================================================================\n");
*/
    bit32 = 1;
    le = 0;
    ring_trace_t	*rt = &srt;
    ring_trace_32le_t	*rt32le;
    ring_trace_32be_t	*rt32be;
    ring_trace_64le_t	*rt64le;
    ring_trace_64be_t	*rt64be;

    long		prev_id, id, entries;

    for( prev_id = 0, p = addr, i = 0, entries = 0; i < length; i++, p++, prev_id = id)
    {
	if ( memcmp( p, "%RTRACE%", 8)) continue;

	rt32le = ( ring_trace_32le_t *) p;
	rt64le = ( ring_trace_64le_t *) p;
	rt32be = ( ring_trace_32be_t *) p;
	rt64be = ( ring_trace_32be_t *) p;

	if ( !isdigit( rt64le->id[0])) continue;

	memcpy( rt->eyec, rt64le->eyec, sizeof( rt->eyec));
	memcpy( rt->id, rt64le->id, sizeof( rt->id));
	memcpy( rt->time, rt64le->time, sizeof( rt->time));
	rt->type = rt64le->type;
	sscanf( rt->id, "%X", &id);
	if ( prev_id + 1 != id && prev_id != 0)
		printf( "!!!! not in right order, perhaps new RT block !!!!\n");

	if ( le)
	{
		if ( bit32)
		{
  			memcpy( &rt->addr, &rt32le->addr, sizeof( rt32le->addr));
  			rt->size = ( long) rt32le->size;
  			rt->type = ( long) rt32le->type;
  			rt->dump_len = rt32le->dump_len;
  			rt->dump = (( char *) ( rt32le + 1));
  			rt->text = rt->dump + rt->dump_len;
  			memcpy( &rt->thread, &rt32le->thread, sizeof( rt32le->thread));
  		} else 
  		{
  			memcpy( rt, rt64le, sizeof( *rt));
  			rt->dump = (( char *) ( rt64le + 1));
  			rt->text = rt->dump + rt->dump_len;
  		}
  	}
	else
	{
		if ( bit32)
		{
			ring_trace_32be_t w;
			ring_trace_32be_t *wrt32be = &w;
			memcpy( wrt32be, rt32be, sizeof( *rt32be));
			swap_byte_order( &wrt32be->addr, sizeof( rt32be->addr));
			memcpy( &rt->addr, &wrt32be->addr, sizeof( rt32be->addr));
			swap_byte_order( &wrt32be->size, sizeof( rt32be->size));
			swap_byte_order( &wrt32be->type, sizeof( rt32be->type));
			swap_byte_order( &wrt32be->dump_len, sizeof( rt32be->dump_len));
			rt->size = ( long) wrt32be->size;
			rt->type = ( long) wrt32be->type;
			rt->dump_len = wrt32be->dump_len;
			rt->dump = (( char *) ( rt32be + 1));
			rt->text = rt->dump + rt->dump_len;
			memcpy( &rt->thread, &rt32be->thread, sizeof( rt32be->thread));
		} else
		{
			ring_trace_64be_t w;
			ring_trace_64be_t *wrt64be = &w;
			memcpy( wrt64be, rt64be, sizeof( *rt64be));
			swap_byte_order( &wrt64be->addr, sizeof( rt64be->addr));
			memcpy( &rt->addr, &wrt64be->addr, sizeof( rt64be->addr));
			swap_byte_order( &wrt64be->size, sizeof( rt64be->size));
			swap_byte_order( &wrt64be->type, sizeof( rt64be->type));
			swap_byte_order( &wrt64be->dump_len, sizeof( rt64be->dump_len));
			rt->size = ( long) wrt64be->size;
			rt->type = ( long) wrt64be->type;
			rt->dump_len = wrt64be->dump_len;
			rt->dump = (( char *) ( rt64be + 1));
			rt->text = rt->dump + rt->dump_len;
			memcpy( &rt->thread, &rt64be->thread, sizeof( rt64be->thread));
		}
	}

	entries++;

	if ( rt->type == RING_TRACE_TEXT) 
	    print_rt_text( rt);
	else
	    print_rt_dump( rt);
    }

    printf( "================================================================\n");
    printf( "extracetd %d entries\n", entries);

	return 0;
}

int print_rt_text( ring_trace_t *rt)
{
	printf( "%.16lX - %s - %s\n", rt->thread, rt->time, rt->text);
		
	return 0;
}

int print_rt_dump( ring_trace_t *rt)
{
    int i;
    char s1[256], s2[256], s3[256], s4[2];
    unsigned char *orig_addr = rt->addr;
    unsigned char *p         = rt->dump;

    // dump text
    printf( "%.16lX - %s - DUMP(%s) (@%p, size=%d(0x%X))\n", rt->thread, rt->time, rt->text, rt->addr, rt->dump_len, rt->dump_len);

	sprintf( s1, "%.16lX - \t\t\t%.8p-%.5X: ", rt->thread, orig_addr, 0);
	sprintf( s2, "\0");
    s4[1] = '\0';

	for ( i = 0; i < rt->dump_len; i++)
    {
  		sprintf( s3, "%.2X", *p);
    	strcat( s1, s3);
    	if ( ( i%4  == 3) &&
      	     ( i%16 != 15)) strcat( s1, " ");
    	s4[0] = ( isprint( *p)) ? *p : '.';
    	strcat( s2, s4);
    	p ++;
    	orig_addr ++;
    	if ( i%16 == 15)
  		{
      	    printf( "%s  >%s<\n", s1, s2);
	    	sprintf( s1, "%.16lX - \t\t\t%.8p-%.5X: ", rt->thread, orig_addr, i + 1);
  	  		sprintf( s2, "\0");
    	}
  	}

	if ( strlen( s2) != 0)
	{
		// strcat( s1, "                                                             ");
		// s1[55] = 0;
	    printf( "%-75.75s  >%s<\n", s1, s2);
	}

	return 0;
}

int dump( char *text, char *addr, size_t len)
{
    int i;
    char s1[256], s2[256], s3[256], s4[2];
    unsigned char *orig_addr = addr;
    unsigned char *p         = addr;

    // dump text
    printf( "dumping '%s' ( @%p, size=%d(0x%X))\n", text, addr, len);

    sprintf( s2, "\0");
    s4[1] = '\0';

    for ( i = 0; i < len; i++)
    {
	sprintf( s3, "%.2X", *p);
	strcat( s1, s3);
	if ( ( i%4  == 3) &&
      	     ( i%16 != 15)) strcat( s1, " ");
    	s4[0] = ( isprint( *p)) ? *p : '.';
    	strcat( s2, s4);
    	p ++;
    	orig_addr ++;
    	if ( i%16 == 15)
  		{
      	    printf( "%s  >%s<\n", s1, s2);
      	    sprintf( s1, "%.8p-%.5X: ", orig_addr, i + 1);
	    sprintf( s2, "\0");
    	}
  	}

	if ( strlen( s2) != 0)
	{
	    printf( "%-59.59s  >%s<\n", s1, s2);
	}

	return 0;
}

int swap_byte_order( void *p, size_t size)
{
    unsigned char *x = ( unsigned char *) p, w;
    int i;

    // printf( "sbo( %p, size=%d)\n", p, size);

    for( i = 0; i < size/2; i++)
    {
	w = x[i];
	x[i] = x[size-i-1];
	x[size-i-1] = w;
    }

    return 0;
}
