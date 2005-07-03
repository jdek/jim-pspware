#ifndef mp_h_n432789237
#define mp_h_n432789237

#include "../mpl/snddev.h"

#define MPL__MP_LOOP_NONE      0
#define MPL__MP_LOOP_FOREVER   -1

#define MPL__MP_MSG_RESTART    0
#define MPL__MP_MSG_END        1

typedef struct mpl__mp_s {
	int (*set_dev)(void *internal_data, mpl__snd_dev_t *dev);

	int (*reset)(void *internal_data);

	int (*play)(void *internal_data);
	int (*stop)(void *internal_data);
	int (*set_loop)(void *internal_data, int loop);

	// typically: 0xXXXYYY (XXX = pattern, YYY = row)
	int (*set_pos)(void *internal_data, int pos);
	int (*get_pos)(void *internal_data);

	int (*set_vol)(void *internal_data, float32 vol);
	int (*get_vol)(void *internal_data, float32 *vol);

	int (*set_option)(void *internal, char *option, char *value);
	int (*get_option)(void *internal, char *option, char **value);
	int (*get_proc)(void *internal, char *name, void_func_t *proc);

	void *internal_data;
} mpl__mp_t;

int mpl__mp_set_dev(mpl__mp_t *mp, mpl__snd_dev_t *dev);
int mpl__mp_reset(mpl__mp_t *mp);
int mpl__mp_play(mpl__mp_t *mp);
int mpl__mp_stop(mpl__mp_t *mp);
int mpl__mp_set_loop(mpl__mp_t *mp, int loop);
int mpl__mp_set_pos(mpl__mp_t *mp, int pos);
int mpl__mp_get_pos(mpl__mp_t *mp);
int mpl__mp_set_vol(mpl__mp_t *mp, float32 vol);
int mpl__mp_get_vol(mpl__mp_t *mp, float32 *vol);
int mpl__mp_set_option(mpl__mp_t *mp, char *option, char *value);
int mpl__mp_get_option(mpl__mp_t *mp, char *option, char **value);
int mpl__mp_get_proc(mpl__mp_t *mp, char *name, void_func_t *proc);

#endif
