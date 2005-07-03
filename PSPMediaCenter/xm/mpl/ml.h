#ifndef ml_h_n31783892389
#define ml_h_n31783892389

#include "../mpl/sys/msg_box.h"
#include "../vbf/vbf.h"
#include "../mpl/mp.h"
#include "../mpl/error.h"

typedef void mpl__md_t;

typedef struct mpl__ml_s {
	int (*load)(void *internal, vbf_t *file, mpl__md_t **md);
	int (*destroy)(void *internal, mpl__md_t *md);

	int (*create_mp)(void *internal, mpl__md_t *md, mpl__msg_box_t *mb, mpl__mp_t **mp);
	int (*destroy_mp)(void *internal, mpl__mp_t **mp);

	int (*set_option)(void *internal, char *option, char *value);
	int (*get_option)(void *internal, char *option, char **value);
	int (*get_proc)(void *internal, char *name, void_func_t *proc);

	void *internal_data;
} mpl__ml_t;

int mpl__ml_load(mpl__ml_t *ml, vbf_t *file, mpl__md_t **md);
int mpl__ml_destroy(mpl__ml_t *ml, mpl__md_t *md);
int mpl__ml_create_mp(mpl__ml_t *ml, mpl__md_t *md, mpl__msg_box_t *mb, mpl__mp_t **mp);
int mpl__ml_destroy_mp(mpl__ml_t *ml, mpl__mp_t **mp);
int mpl__ml_set_option(mpl__ml_t *ml, char *option, char *value);
int mpl__ml_get_option(mpl__ml_t *ml, char *option, char **value);
int mpl__ml_get_proc(mpl__ml_t *ml, char *name, void_func_t *proc);


#endif
