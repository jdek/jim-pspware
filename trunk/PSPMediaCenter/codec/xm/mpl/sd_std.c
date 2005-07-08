#include "../mpl/sd_std.h"
#include "../mpl/sys/mem.h"

static int cmd_set_vol(void *data, float32 vol, float32 pan_lr, float32 pan_fb);
static int cmd_get_vol(void *data, float32 * vol, float32 * pan_lr, float32 * pan_fb);
static int cmd_set_output_options(void *data, mpl__snd_dev_output_t opt);
static int cmd_get_output_options(void *data, mpl__snd_dev_output_t * opt);
static int cmd_set_latency(void *data, int latency);
static int cmd_get_latency(void *data);
static int cmd_set_option(void *data, char *option, char *value);
static int cmd_get_option(void *data, char *option, char **value);
static int cmd_get_proc(void *data, char *name, void_func_t * proc);
static int cmd_create_mixer(void *data, mpl__snd_mixer_t ** mixer, mpl__snd_mixer_opt_t opt);
static int cmd_destroy_mixer(void *data, mpl__snd_mixer_t * mixer);
static int cmd_create_stream(void *data, mpl__snd_stream_t ** stream, mpl__snd_stream_opt_t opt);
static int cmd_destroy_stream(void *data, mpl__snd_stream_t * stream);

static int cmd_set_vol(void *data, float32 vol, float32 pan_lr, float32 pan_fb)
{
    return MPL__ERR_GENERIC;
}

static int cmd_get_vol(void *data, float32 * vol, float32 * pan_lr, float32 * pan_fb)
{
    return MPL__ERR_GENERIC;
}

static int cmd_set_output_options(void *data, mpl__snd_dev_output_t opt)
{
    return MPL__ERR_GENERIC;
}

static int cmd_get_output_options(void *data, mpl__snd_dev_output_t * opt)
{
    return MPL__ERR_GENERIC;
}

static int cmd_set_latency(void *data, int latency)
{
    return MPL__ERR_GENERIC;
}

static int cmd_get_latency(void *data)
{
    return MPL__ERR_GENERIC;
}

static int cmd_set_option(void *data, char *option, char *value)
{
    return MPL__ERR_GENERIC;
}

static int cmd_get_option(void *data, char *option, char **value)
{
    return MPL__ERR_GENERIC;
}

static int cmd_get_proc(void *data, char *name, void_func_t * proc)
{
    return MPL__ERR_GENERIC;
}

static int cmd_create_mixer(void *data, mpl__snd_mixer_t ** mixer, mpl__snd_mixer_opt_t opt)
{
    return MPL__ERR_GENERIC;
}

static int cmd_destroy_mixer(void *data, mpl__snd_mixer_t * mixer)
{
    return MPL__ERR_GENERIC;
}

static int cmd_create_stream(void *data, mpl__snd_stream_t ** stream, mpl__snd_stream_opt_t opt)
{
    return MPL__ERR_GENERIC;
}

static int cmd_destroy_stream(void *data, mpl__snd_stream_t * stream)
{
    return MPL__ERR_GENERIC;
}

int
mpl__sd_std_create(mpl__snd_dev_t ** sd, int (*mx_create) (mpl__mixer_t ** mixer),
		   void (*mx_destroy) (mpl__mixer_t * mixer))
{
    int result;

    result = mpl__mem_alloc(sizeof(mpl__snd_dev_t), (void **) sd);

    if (result < MPL__ERR_OK) {
	return result;
    }

    (*sd)->create_mixer = cmd_create_mixer;
    (*sd)->create_stream = cmd_create_stream;
    (*sd)->destroy_mixer = cmd_destroy_mixer;
    (*sd)->destroy_stream = cmd_destroy_stream;
    (*sd)->get_latency = cmd_get_latency;
    (*sd)->get_option = cmd_get_option;
    (*sd)->get_output_options = cmd_get_output_options;
    (*sd)->get_proc = cmd_get_proc;
    (*sd)->get_vol = cmd_get_vol;
    (*sd)->set_latency = cmd_set_latency;
    (*sd)->set_option = cmd_set_option;
    (*sd)->set_output_options = cmd_set_output_options;
    (*sd)->set_vol = cmd_set_vol;

    return MPL__ERR_GENERIC;
}

void mpl__sd_std_destroy(mpl__snd_dev_t * sd)
{
}
