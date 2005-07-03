#ifndef snddev_h_n478237842378
#define snddev_h_n478237842378

// notes:
//
// data is always signed (any need for unsigned data?!?)


#include "../sys_cfg.h"
#include "../mpl/error.h"

#define MPL__SND_DEV_FMT_NONLOOPING    0
#define MPL__SND_DEV_FMT_LOOPING       1
#define MPL__SND_DEV_FMT_BIDILOOPING   2

#define MPL__SND_DEV_FMT_8BIT          1
#define MPL__SND_DEV_FMT_16BIT         2
#define MPL__SND_DEV_FMT_24BIT         3
#define MPL__SND_DEV_FMT_32BIT         4
#define MPL__SND_DEV_FMT_FLOAT_32BIT   5

#define MPL__SND_DEV_LATENCY_HIGH      0
#define MPL__SND_DEV_LATENCY_LOW       1

#define MPL__SND_DEV_DIR_FORWARDS      0
#define MPL__SND_DEV_DIR_BACKWARDS     1

typedef struct mpl__snd_dev_output_s {
	int format;
	int channels;
	float32 freq;
	int latency;
} mpl__snd_dev_output_t;

typedef void(*mpl__snd_call_back_func_t)(void *data, int param);

typedef struct mpl__snd_mixer_call_back_s {
	mpl__snd_call_back_func_t	func;
	void						*data;
	int							param;
	float64						period;
} mpl__snd_call_back_t;

typedef struct mpl__snd_dev_smp_s {
	int			format;
	int			channels;
	int			loopmode;
	int			loop;
	int			length;
	float32		vol;
	float32		pan_lr;
	float32		pan_fb;
	float32		freq;
	void		*data;
} mpl__snd_mixer_smp_t;

typedef struct mpl__snd_mixer_s {
	int (*reset)(void *internal);  // stops playing, toasts samples
	int (*pause)(void *internal);  // 1 -> paused | 0 -> unpaused ; doesn't run callback
	int (*stop)(void *internal);

	int (*upload_sample)(void *internal, mpl__snd_mixer_smp_t sample);
	int (*destroy_sample)(void *internal, int handle);

	int (*set_vol)(void *internal, float32 vol, float32 pan_lr, float32 pan_fb);
	int (*get_vol)(void *internal, float32 *vol, float32 *pan_lr, float32 *pan_fb);

	int (*get_output_options)(void *internal, mpl__snd_dev_output_t *opt);
	int (*get_latency)(void *internal);   // ms

	int (*get_num_channels)(void *internal);
	int (*get_num_active_channels)(void *internal);
	int (*get_free_channel)(void *internal);
	int (*is_channel_active)(void *internal, int ch);

	int (*set_channel_vol)(void *internal, int ch, float32 vol, float32 pan_lr, float32 pan_fb);
	int (*set_channel_freq)(void *internal, int ch, float32 freq);
	int (*set_channel_pos)(void *internal, int ch, float64 pos, int dir);
	int (*play_channel)(void *internal, int ch, int handle, float32 freq, float32 vol, float32 pan_lr, float32 pan_fb, float64 pos, int dir);
	int (*stop_channel)(void *internal, int ch);

	/*
		getoption(...)

		"ID"
		"VERSION"
		"AUTHOR"
		"EXTENSIONS" -> e.g. "EXT_MMX EXT_EQ EXT_REVERB"....
	*/
	int (*set_option)(void *internal, char *option, char *value);
	int (*get_option)(void *internal, char *option, char **value);
	int (*get_proc)(void *internal, char *name, void_func_t *proc);

	int (*set_call_back)(void *internal, mpl__snd_call_back_t call_back);

	void *internal_data;
} mpl__snd_mixer_t;

typedef struct mpl__snd_mixer_opt_s {
	int			ch_num;   // min number of channels
	int			smp_num;  // min number of samples
	int			latency;
} mpl__snd_mixer_opt_t;

typedef struct mpl__snd_stream_format_s {
	int			format;
	int			channels;
	float32		freq;
} mpl__snd_stream_format_t;

typedef struct mpl__snd_stream_opt_s {
	mpl__snd_stream_format_t	format;
	int							latency;
} mpl__snd_stream_opt_t;

typedef struct mpl__snd_stream_s {
	int (*set_vol)(void *internal, float32 vol, float32 pan_lr, float32 pan_fb);
	int (*get_vol)(void *internal, float32 *vol, float32 *pan_lr, float32 *pan_fb);

	int (*get_output_options)(void *internal, mpl__snd_dev_output_t *opt);
	int (*set_input_format)(void *internal, mpl__snd_stream_format_t *opt);
	int (*get_input_format)(void *internal, mpl__snd_stream_format_t *opt);
	int (*get_latency)(void *internal);   // ms
	int (*get_buffer_size)(void *internal); // ms

	int (*write)(void *internal, void *data, int length);

	int (*set_option)(void *internal, char *option, char *value);
	int (*get_option)(void *internal, char *option, char **value);
	int (*get_proc)(void *internal, char *name, void_func_t *proc);

	int (*set_call_back)(void *internal, mpl__snd_call_back_t call_back);

	void *internal_data;
} mpl__snd_stream_t;

typedef struct mpl__snd_dev_s {
	int (*set_vol)(void *internal, float32 vol, float32 pan_lr, float32 pan_fb);
	int (*get_vol)(void *internal, float32 *vol, float32 *pan_lr, float32 *pan_fb);

	int (*set_output_options)(void *internal, mpl__snd_dev_output_t opt);
	int (*get_output_options)(void *internal, mpl__snd_dev_output_t *opt);

	int (*set_latency)(void *internal, int latency);
	int (*get_latency)(void *internal);   // ms

	int (*set_option)(void *internal, char *option, char *value);
	int (*get_option)(void *internal, char *option, char **value);
	int (*get_proc)(void *internal, char *name, void_func_t *proc);

	int (*create_mixer)(void *internal, mpl__snd_mixer_t **mixer, mpl__snd_mixer_opt_t opt);
	int (*destroy_mixer)(void *internal, mpl__snd_mixer_t *mixer);

	int (*create_stream)(void *internal, mpl__snd_stream_t **stream, mpl__snd_stream_opt_t opt);
	int (*destroy_stream)(void *internal, mpl__snd_stream_t *stream);

	void *internal_data;
} mpl__snd_dev_t;

// dev

int mpl__snd_dev_set_vol(mpl__snd_dev_t *dev, float32 vol, float32 pan_lr, float32 pan_fb);
int mpl__snd_dev_get_vol(mpl__snd_dev_t *dev, float32 *vol, float32 *pan_lr, float32 *pan_fb);
int mpl__snd_dev_set_output_options(mpl__snd_dev_t *dev, mpl__snd_dev_output_t opt);
int mpl__snd_dev_get_output_options(mpl__snd_dev_t *dev, mpl__snd_dev_output_t *opt);
int mpl__snd_dev_set_latency(mpl__snd_dev_t *dev, int latency);
int mpl__snd_dev_get_latency(mpl__snd_dev_t *dev);
int mpl__snd_dev_set_option(mpl__snd_dev_t *dev, char *option, char *value);
int mpl__snd_dev_get_option(mpl__snd_dev_t *dev, char *option, char **value);
int mpl__snd_dev_get_proc(mpl__snd_dev_t *dev, char *name, void_func_t *proc);
int mpl__snd_dev_create_mixer(mpl__snd_dev_t *dev, mpl__snd_mixer_t **mixer, mpl__snd_mixer_opt_t opt);
int mpl__snd_dev_destroy_mixer(mpl__snd_dev_t *dev, mpl__snd_mixer_t *mixer);
int mpl__snd_dev_create_stream(mpl__snd_dev_t *dev, mpl__snd_stream_t **stream, mpl__snd_stream_opt_t opt);
int mpl__snd_dev_destroy_stream(mpl__snd_dev_t *dev, mpl__snd_stream_t *stream);

// mixer

int mpl__snd_mixer_reset(mpl__snd_mixer_t *mixer);  // stops playing, toasts samples
int mpl__snd_mixer_pause(mpl__snd_mixer_t *mixer);  // 1 -> paused | 0 -> unpaused ; doesn't run callback
int mpl__snd_mixer_stop(mpl__snd_mixer_t *mixer);
int mpl__snd_mixer_upload_sample(mpl__snd_mixer_t *mixer, mpl__snd_mixer_smp_t sample);
int mpl__snd_mixer_destroy_sample(mpl__snd_mixer_t *mixer, int handle);
int mpl__snd_mixer_set_vol(mpl__snd_mixer_t *mixer, float32 vol, float32 pan_lr, float32 pan_fb);
int mpl__snd_mixer_get_vol(mpl__snd_mixer_t *mixer, float32 *vol, float32 *pan_lr, float32 *pan_fb);
int mpl__snd_mixer_get_output_options(mpl__snd_mixer_t *mixer, mpl__snd_dev_output_t *opt);
int mpl__snd_mixer_get_latency(mpl__snd_mixer_t *mixer);   // ms
int mpl__snd_mixer_get_num_channels(mpl__snd_mixer_t *mixer);
int mpl__snd_mixer_get_num_active_channels(mpl__snd_mixer_t *mixer);
int mpl__snd_mixer_get_free_channel(mpl__snd_mixer_t *mixer);
int mpl__snd_mixer_is_channel_active(mpl__snd_mixer_t *mixer, int ch);
int mpl__snd_mixer_set_channel_vol(mpl__snd_mixer_t *mixer, int ch, float32 vol, float32 pan_lr, float32 pan_fb);
int mpl__snd_mixer_set_channel_freq(mpl__snd_mixer_t *mixer, int ch, float32 freq);
int mpl__snd_mixer_set_channel_pos(mpl__snd_mixer_t *mixer, int ch, float64 pos, int dir);
int mpl__snd_mixer_play_channel(mpl__snd_mixer_t *mixer, int ch, int handle, float32 freq, float32 vol, float32 pan_lr, float32 pan_fb, float64 pos, int dir);
int mpl__snd_mixer_stop_channel(mpl__snd_mixer_t *mixer, int ch);
int mpl__snd_mixer_set_option(mpl__snd_mixer_t *mixer, char *option, char *value);
int mpl__snd_mixer_get_option(mpl__snd_mixer_t *mixer, char *option, char **value);
int mpl__snd_mixer_get_proc(mpl__snd_mixer_t *mixer, char *name, void_func_t *proc);
int mpl__snd_mixer_set_call_back(mpl__snd_mixer_t *mixer, mpl__snd_call_back_t call_back);

// stream

int mpl__snd_stream_set_vol(mpl__snd_stream_t *stream, float32 vol, float32 pan_lr, float32 pan_fb);
int mpl__snd_stream_get_vol(mpl__snd_stream_t *stream, float32 *vol, float32 *pan_lr, float32 *pan_fb);
int mpl__snd_stream_get_output_options(mpl__snd_stream_t *stream, mpl__snd_dev_output_t *opt);
int mpl__snd_stream_set_input_format(mpl__snd_stream_t *stream, mpl__snd_stream_format_t *opt);
int mpl__snd_stream_get_input_format(mpl__snd_stream_t *stream, mpl__snd_stream_format_t *opt);
int mpl__snd_stream_get_latency(mpl__snd_stream_t *stream);   // ms
int mpl__snd_stream_get_buffer_size(mpl__snd_stream_t *stream); // ms
int mpl__snd_stream_set_option(mpl__snd_stream_t *stream, char *option, char *value);
int mpl__snd_stream_get_option(mpl__snd_stream_t *stream, char *option, char **value);
int mpl__snd_stream_get_proc(mpl__snd_stream_t *stream, char *name, void_func_t *proc);
int mpl__snd_stream_write(mpl__snd_stream_t *stream, void *data, int length);
int mpl__snd_stream_set_call_back(mpl__snd_stream_t *stream, mpl__snd_call_back_t call_back);


//////////////////////////////////////////////////////////////
// extensions

#define MPL__EXT_SND_MIXER_SUSTAIN_LOOP      "EXT_SUSTAIN_LOOP"
#define MPL__FUNC_SND_MIXER_SET_SUSTAIN_LOOP "mpl__snd_mixer_set_sustain_loop"
#define MPL__FUNC_SND_MIXER_END_SUSTAIN      "mpl__snd_mixer_end_sustain"

typedef int (*mpl__snd_mixer_set_sustain_loop_t)(mpl__snd_mixer_t *sndmx, int handle, int begin, int end, int mode);
typedef int (*mpl__snd_mixer_end_sustain_t)(mpl__snd_mixer_t *sndmx, int ch);


#endif
