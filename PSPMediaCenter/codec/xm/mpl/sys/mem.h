#ifndef mem_h_n78923478924
#define mem_h_n78923478924

#include "../../sys_cfg.h"
#include "../../mpl/error.h"

typedef int mpl__mem_size_t;

int mpl__mem_init();
int mpl__mem_alloc(mpl__mem_size_t size, void **mem);
int mpl__mem_get_size(mpl__mem_size_t * size, void *mem);
int mpl__mem_resize(mpl__mem_size_t newsize, int onlyexpand, void **mem);
int mpl__mem_free(void *mem);
int mpl__mem_copy(void *src, void *dst, mpl__mem_size_t size);
int mpl__mem_set_zero(void *mem, mpl__mem_size_t size);

#endif
