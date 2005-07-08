#include <stdio.h>
#include <stdlib.h>
#include "psp.h"

extern "C" {
extern void debug_log( const char* message );

/*
void *malloc(size_t size)
{
	return 0;
}
*/

void free(void *ptr)
{
}

char *strcpy( char *dest, const char *src );
int sprintf( char *buffer, const char *format, ... )
{
	strcpy( buffer, format );
	return 0;
}

int printf( const char *format, ... )
{
}

int puts( const char *string )
{
}

int memcmp(const void *s1, const void *s2, size_t len)
{
	unsigned long int	a0, b0, res;
	int		i;

	i = 0;
	while ( len != 0 ){
		a0 = ((unsigned char*)s1)[i];
		b0 = ((unsigned char*)s2)[i];
		res = a0 - b0;
		if ( res != 0 )
			return res;
		len--;
    }

	return 0;
}

# define op_t	unsigned long int
# define OPSIZ	(sizeof(op_t))

void *memset(void *buf, int ch, size_t n)
{
	int		i;

	if (n == 0) return buf;


	if ( ch ){
		for ( i = 0; i < n; i++ ){
			((char*)buf)[i] = ch;
		}
	} else {
		long int dstp = (long int)buf;

		if ( n >= 8 ){
			size_t	xlen;

			while ( (dstp % OPSIZ) != 0 ){
				((char*)dstp)[0] = 0;
				dstp += 1;
				n -= 1;
			}

			xlen = n / (OPSIZ * 8);
			while ( xlen != 0 ){
				((op_t*)dstp)[0] = 0;
				((op_t*)dstp)[1] = 0;
				((op_t*)dstp)[2] = 0;
				((op_t*)dstp)[3] = 0;
				((op_t*)dstp)[4] = 0;
				((op_t*)dstp)[5] = 0;
				((op_t*)dstp)[6] = 0;
				((op_t*)dstp)[7] = 0;
				dstp += 8 * OPSIZ;
				xlen -= 1;
			}
			n %= OPSIZ * 8;

			xlen = n / OPSIZ;
			while ( xlen != 0 ){
				((op_t*)dstp)[0] = 0;
				dstp += OPSIZ;
				xlen -= 1;
			}
			n %= OPSIZ;
		}

		while ( n != 0 ){
			((char*)dstp)[0] = 0;
			dstp += 1;
			n -= 1;
		}
	}

	return buf;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	int		i;

	if (n == 0) return dest;

	for ( i = 0; i < n; i++ ){
		((char*)dest)[i] = ((char*)src)[i];
	}
	return dest;
}

void* memmove( void* dest, const void* src, size_t n )
{
	unsigned long int dstp = (long int) dest;
	unsigned long int srcp = (long int) src;
	int		i;

	if (n == 0) return dest;


	if ( dstp - srcp >= n ){
		for ( i = 0; i < n; i++ ){
			((char*)dstp)[i] = ((char*)srcp)[i];
		}
	} else {
		for ( i = 0; i < n; i++ ){
			((char*)dstp)[n - i - 1] = ((char*)srcp)[n - i - 1];
		}
	}
	return dest;
}

size_t strlen( const char *string )
{
	int		i;

	for ( i = 0; string[i]; i++ );

	return i;
}

char *strcpy( char *dest, const char *src )
{
	int		i;

	for ( i = 0; src[i]; i++ ){
		dest[i] = src[i];
	}
	dest[i] = 0;

	return dest;
}

char *strncpy( char *dest, const char *src, size_t count )
{
	int		i;

	for ( i = 0; i < count; i++ ){
		dest[i] = src[i];
	}
	dest[i] = 0;

	return dest;
}

char *strcat(char *dest, const char *src)
{
	strcpy( &dest[strlen( dest )], src );

	return dest;
}

char *strchr(const char *s, int c)
{
	int		size;
	int		i;

	size = strlen( s );
	for ( i = 0; i < size; i++ ){
		if ( s[i] == c ){
			return (char*)&s[i];
		}
	}
	return 0;
}

char* strrchr(const char *string, int c)
{
	/* use the asm strchr to do strrchr */
	char* lastmatch;
	char* result;

	/* if char is never found then this will return 0 */
	/* if char is found then this will return the last matched location
	   before strchr returned 0 */

	lastmatch = 0;
	result = strchr(string,c);

	while ((int)result != 0)
	{
		lastmatch=result;
		result = strchr(lastmatch+1,c);
	}

	return lastmatch;
}

int strcmp(const char *p1, const char *p2)
{
	register const unsigned char *s1 = (const unsigned char *) p1;
	register const unsigned char *s2 = (const unsigned char *) p2;
	unsigned char c1, c2;

	do {
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		if (c1 == '\0')
			return c1 - c2;
	} while (c1 == c2);

	return c1 - c2;
}

// add by y
int stricmp(const char *str1, const char *str2)
{
	char c1, c2;
	for(;;){
		c1 = *str1;
		if(c1>=0x61 && c1<=0x7A) c1-=0x20;
		c2 = *str2;
		if(c2>=0x61 && c2<=0x7A) c2-=0x20;
		
		if(c1!=c2)
			return 1;
		else if(c1==0)
			return 0;
		
		str1++; str2++;
	}
}

void strrev(char *s)
{
	char tmp;
	int i;
	int len = strlen(s);
	
	for(i=0; i<len/2; i++){
		tmp = s[i];
		s[i] = s[len-1-i];
		s[len-1-i] = tmp;
	}
}

int atoi(const char *s)
{
	int cnt;
	int num = 0;
	for (cnt = 0; (s[cnt] >= '0') && (s[cnt] <= '9'); cnt++) {
		num = 10 * num + (s[cnt] - '0');
	}
	return num;
}

void itoa(int val, char *s) {
	char *t;
	int mod;

	if(val < 0) {
		*s++ = '-';
		val = -val;
	}
	t = s;

	while(val) {
		mod = val % 10;
		*t++ = (char)mod + '0';
		val /= 10;
	}

	if(s == t)
		*t++ = '0';

	*t = '\0';

	strrev(s);
}

void ustoa(unsigned short val, char *s)
{
	char *t;
	unsigned short mod;
	
	t = s;
	
	while(val) {
		mod = val % 10;
		*t++ = (char)mod + '0';
		val /= 10;
	}

	if(s == t)
		*t++ = '0';

	*t = '\0';

	strrev(s);
}

int strncmp(const char *s1, const char* s2, size_t n)
{
	unsigned char c1 = '\0';
	unsigned char c2 = '\0';

	while (n > 0){
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		if (c1 == '\0' || c1 != c2)
			return c1 - c2;
		n--;
	}

	return c1 - c2;
}

int isupper(int c)
{
	if (c < 'A')
		return 0;

	if (c > 'Z')
		return 0;

	// passed both criteria, so it
	// is an upper case alpha char
	return 1;
}

int islower(int c)
{
	if (c < 'a')
		return 0;

	if (c > 'z')
		return 0;

	// passed both criteria, so it
	// is a lower case alpha char
	return 1;
}

int isalpha(int c)
{
	if ( islower( c ) || isupper( c ) ){
		return 1;
	}
	return 0;
}

int tolower(int c)
{
	if ( isupper( c ) ){
		c += 32;
	}
	return c;
}

int toupper(int c)
{
	if ( islower( c ) ){
		c -= 32;
	}
	return c;
}

int strcasecmp( const char *s1, const char *s2 )
{
	const unsigned char*	p1 = (const unsigned char*)s1;
	const unsigned char*	p2 = (const unsigned char*)s2;
	unsigned char		c1, c2;
	int result;

	if ( p1 == p2 ){
		return 0;
	}

	do {
		c1 = *p1++;
		c2 = *p2++;

        if ( c1 >= 'A' && c1 <= 'Z' ){
            c1 += 0x20;
		}
        if ( c2 >= 'A' && c2 <= 'Z' ){
            c2 += 0x20;
		}

		if ( (result = c1 - c2) ){
			break;
		}
		if ( !c1 ){
			break;
		}
	} while ( 1 );

	return result;
}

int strncasecmp(const char *s1, const char *s2, unsigned n)
{
	const unsigned char*	p1 = (const unsigned char*)s1;
	const unsigned char*	p2 = (const unsigned char*)s2;
	unsigned char		c1, c2;
	int result;

	if ( p1 == p2 || n == 0 ){
		return 0;
	}

	do {
		c1 = *p1++;
		c2 = *p2++;

        if ( c1 >= 'A' && c1 <= 'Z' ){
            c1 += 0x20;
		}
        if ( c2 >= 'A' && c2 <= 'Z' ){
            c2 += 0x20;
		}

		if ( (result = c1 - c2) ){
			break;
		}
		if ( !c1 || --n == 0 ){
			break;
		}
	} while ( 1 );

	return result;
}

int isdigit(int c)
{
	if (c < '0')
		return 0;

	if (c > '9')
		return 0;

	// passed both criteria, so it
	// is a numerical char
	return 1;
}

// -------------------------------
unsigned int         __stdlib_rand_seed = 92384729;

int rand(void)
{
// I don't agree with it...
//  return (__stdlib_rand_seed = ((((__stdlib_rand_seed * 214013) + 2531011) >> 16) & 0xffff));
  unsigned long long t = __stdlib_rand_seed;

  t *= 254124045ull;
  t += 76447ull;
  __stdlib_rand_seed = t;
  // We return a number between 0 and RAND_MAX, which is 2^31-1.
  return (t >> 16) & 0x7FFFFFFF;
}

typedef int		cmp_t(const void *, const void *);
static __inline char	*med3(char *, char *, char *, cmp_t *, void *);
static __inline void	 swapfunc(char *, char *, int, int);

#define min(a, b)	(a) < (b) ? (a) : (b)

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */
#define swapcode(TYPE, parmi, parmj, n) { 		\
	long i = (n) / sizeof (TYPE); 			\
	register TYPE *pi = (TYPE *) (parmi); 		\
	register TYPE *pj = (TYPE *) (parmj); 		\
	do { 						\
		register TYPE	t = *pi;		\
		*pi++ = *pj;				\
		*pj++ = t;				\
        } while (--i > 0);				\
}

#define SWAPINIT(a, es) swaptype = ((char *)a - (char *)0) % sizeof(long) || \
	es % sizeof(long) ? 2 : es == sizeof(long)? 0 : 1;

static __inline void
swapfunc(char *a, char *b, int n, int swaptype)
{
	if(swaptype <= 1)
		swapcode(long, a, b, n)
	else
		swapcode(char, a, b, n)
}

#define swap(a, b)					\
	if (swaptype == 0) {				\
		long t = *(long *)(a);			\
		*(long *)(a) = *(long *)(b);		\
		*(long *)(b) = t;			\
	} else						\
		swapfunc((char*)a, b, es, swaptype)

#define vecswap(a, b, n) 	if ((n) > 0) swapfunc((char*)a, b, n, swaptype)

#define	CMP(t, x, y) (cmp((x), (y)))

static __inline char *
med3(char *a, char *b, char *c, cmp_t *cmp, void *thunk
)
{
	return CMP(thunk, a, b) < 0 ?
	       (CMP(thunk, b, c) < 0 ? b : (CMP(thunk, a, c) < 0 ? c : a ))
              :(CMP(thunk, b, c) > 0 ? b : (CMP(thunk, a, c) < 0 ? a : c ));
}

#define	thunk NULL
void
qsort(void *a, size_t n, size_t es, cmp_t *cmp)
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	int d, r, swaptype, swap_cnt;

loop:	SWAPINIT(a, es);
	swap_cnt = 0;
	if (n < 7) {
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7) {
		pl = (char*)a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40) {
			d = (n / 8) * es;
			pl = med3(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3(pm - d, pm, pm + d, cmp, thunk);
			pn = med3(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3(pl, pm, pn, cmp, thunk);
	}
	swap(a, pm);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (r = CMP(thunk, pb, a)) <= 0) {
			if (r == 0) {
				swap_cnt = 1;
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (r = CMP(thunk, pc, a)) >= 0) {
			if (r == 0) {
				swap_cnt = 1;
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swap(pb, pc);
		swap_cnt = 1;
		pb += es;
		pc -= es;
	}
	if (swap_cnt == 0) {  /* Switch to insertion sort */
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		return;
	}

	pn = (char *)a + n * es;
	r = min(pa - (char *)a, pb - pa);
	vecswap(a, pb - r, r);
	r = min(pd - pc, pn - pd - es);
	vecswap(pb, pn - r, r);
	if ((r = pb - pa) > es)
		qsort(a, r / es, es, cmp);
	if ((r = pd - pc) > es) {
		/* Iterate rather than recurse to save stack space */
		a = pn - r;
		n = r / es;
		goto loop;
	}
}
// -------------------------------

FILE *fopen(const char *path, const char *mode)
{
	int		psp_mode;
	int		fd;

	switch ( mode[0] ){
	case 'r':
		psp_mode = PSP_O_RDONLY;
		break;

	case 'w':
		psp_mode = PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC;
		break;

	case 'a':
		psp_mode = PSP_O_RDWR | PSP_O_APPEND;
		break;
	}

	fd = sceIoOpen( path, psp_mode, 0777 );
	if ( fd < 0 ){
		return NULL;
	}

	return (FILE*)fd;
}

int fclose(FILE *fp)
{
	sceIoClose( (int)fp );

	return 0;
}

int fgetc(FILE *fp)
{
	char	c;

	sceIoRead( (int)fp, &c, sizeof(char) );

	return (int)c;
}

size_t fread(void *buf, size_t size, size_t n, FILE *fp)
{
	int		ret;

	ret = sceIoRead( (int)fp, buf, size * n );

	return ret / size;
}

size_t fwrite(const void *buf, size_t size, size_t n, FILE *fp)
{
	sceIoWrite( (int)fp, (void*)buf, size * n );

	return n;
}

};


#define SLASH_STR "/"
#define SLASH_CHAR '/'

void _makepath( char *path, const char *drive, const char *dir, const char *fname, const char *ext )
{
	if ( drive && *drive ){
		*path       = *drive;
		*(path + 1) = ':';
		*(path + 2) = 0;
	} else {
		*path = 0;
	}
	
	if ( dir && *dir ){
		strcat( path, dir );
		if ( strlen( dir ) != 1 || *dir != '\\' ){
			strcat( path, SLASH_STR );
		}
	}
	
	if ( fname ){
		strcat( path, fname );
	}
	if ( ext && *ext ){
		strcat( path, "." );
		strcat( path, ext );
	}
}

void _splitpath( const char *path, char *drive, char *dir, char *fname, char *ext )
{
	if ( *path && *(path + 1) == ':' ){
		*drive = toupper( *path );
		path += 2;
	} else {
		*drive = 0;
	}

	char*	slash = strrchr( path, SLASH_CHAR );
	if ( !slash ){
		slash = strrchr( path, '/' );
	}
	char*	dot = strrchr( path, '.' );
	if ( dot && slash && dot < slash ){
		dot = NULL;
	}

	if ( !slash ){
		if ( *drive ){
			strcpy( dir, "\\" );
		} else {
			strcpy( dir, "" );
		}
		strcpy( fname, path );
		if ( dot ){
			*(fname + (dot - path)) = 0;
			strcpy( ext, dot + 1 );
		} else {
			strcpy( ext, "" );
		}
	} else {
		if ( *drive && *path != '\\' ){
			strcpy( dir, "\\" );
			strcat( dir, path );
			*(dir + (slash - path) + 1) = 0;
		} else {
			strcpy( dir, path );
			if ( (slash - path) == 0 ){
				*(dir + 1) = 0;
			} else {
				*(dir + (slash - path)) = 0;
			}
		}

		strcpy( fname, slash + 1 );
		if ( dot ){
			*(fname + (dot - slash) - 1) = 0;
			strcpy( ext, dot + 1 );
		} else {
			strcpy( ext, "" );
		}
	}
}

