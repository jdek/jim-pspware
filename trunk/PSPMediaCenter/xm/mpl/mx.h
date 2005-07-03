#ifndef mx_h_n7238478432789423
#define mx_h_n7238478432789423

#include "../mpl/snddev.h"

#define MPL__MX_MEM_AGGRESSIVE   0
#define MPL__MX_MEM_LOW          1

#define MPL__MX_HQ               0  // high quality
#define MPL__MX_LQ               1  // low quality

typedef struct mpl__mixer_opt_s {
	int ch_num, smp_num, mem_usage, qual;
} mpl__mixer_opt_t;

typedef struct mpl__mixer_s {
	int (*set_output_options)(void *internal, mpl__snd_dev_output_t *opt, int latency);
	int (*set_options)(void *internal, mpl__mixer_opt_t *opt);
	int (*get_options)(void *internal, mpl__mixer_opt_t *opt);
	int (*get_proc)(void *internal, char *name, void_func_t *func);
	int (*get_interface)(void *internal, mpl__snd_mixer_t **mixer);

	int (*set_vol)(void *internal, float32 vol, float32 pan_lr, float32 pan_fb);

	int (*mix)(void *internal, void *buffer, int length);

	int (*get_num_active_channels)(void *internal);

	void *internal_data;
} mpl__mixer_t;

int mpl__mixer_set_output_options(mpl__mixer_t *mx, mpl__snd_dev_output_t *opt, int latency);
int mpl__mixer_set_options(mpl__mixer_t *mx, mpl__mixer_opt_t *opt);
int mpl__mixer_get_options(mpl__mixer_t *mx, mpl__mixer_opt_t *opt);
int mpl__mixer_get_proc(mpl__mixer_t *mx, char *name, void_func_t *func);
int mpl__mixer_get_interface(mpl__mixer_t *mx, mpl__snd_mixer_t **mixer);
int mpl__mixer_set_vol(mpl__mixer_t *mx, float32 vol, float32 pan_lr, float32 pan_fb);
int mpl__mixer_mix(mpl__mixer_t *mx, void *buffer, int length);
int mpl__mixer_get_num_active_channels(mpl__mixer_t *mx);

#endif
