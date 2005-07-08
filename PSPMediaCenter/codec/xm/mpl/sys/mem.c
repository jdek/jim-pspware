 /*
    to do
    -----

    - statistics
    - slab allocator? (a lot faster than standard malloc implementation)
  */

#include "../../mpl/sys/mem.h"
#include "../../mpl/sys/critsect.h"
#include <stdlib.h>

static mpl__critical_section_t l_cs;

int mpl__mem_alloc(mpl__mem_size_t size, void **mem)
{
    void *memblock;

//      mpl__cs_enter(&l_cs);
    memblock = malloc(size);
//      mpl__cs_leave(&l_cs);

    if (!memblock) {
	return MPL__ERR_NOMEM;
    }

    *mem = memblock;

    return 0;
}

int mpl__mem_get_size(mpl__mem_size_t * size, void *mem)
{
    return MPL__ERR_GENERIC;
}

int mpl__mem_resize(mpl__mem_size_t newsize, int onlyexpand, void **mem)
{
    return MPL__ERR_GENERIC;
}

int mpl__mem_free(void *mem)
{
    mpl__cs_enter(&l_cs);
    free(mem);
    mpl__cs_leave(&l_cs);

    return 0;
}

int mpl__mem_copy(void *src, void *dst, mpl__mem_size_t size)
{
    char *s, *d;
    int cnt;

    s = (char *) src;
    d = (char *) dst;

    if ((mpl__mem_size_t) src > (mpl__mem_size_t) dst) {
	cnt = 0;

	while (cnt < size) {
	    d[cnt] = s[cnt];
	    cnt++;
	}
    } else {
	while (size--) {
	    d[size] = s[size];
	}
    }

    return 0;
}

int mpl__mem_set_zero(void *mem, mpl__mem_size_t size)
{
    char *m;

    m = (char *) mem;

    while (size--) {
	m[size] = 0;
    }

    return 0;
}

int mpl__mem_init()
{
    if (mpl__cs_construct(&l_cs) < MPL__ERR_GENERIC) {
	return MPL__ERR_GENERIC;
    }

    return 0;
}
