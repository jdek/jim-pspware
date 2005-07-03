#include "../mpl/mx.h"

int
mpl__mixer_set_output_options(mpl__mixer_t *mx, mpl__snd_dev_output_t *opt, int latency)
{
	return mx->set_output_options(mx->internal_data, opt, latency);
}

int
mpl__mixer_set_options(mpl__mixer_t *mx, mpl__mixer_opt_t *opt)
{
	return mx->set_options(mx->internal_data, opt);
}

int
mpl__mixer_get_options(mpl__mixer_t *mx, mpl__mixer_opt_t *opt)
{
	return mx->get_options(mx->internal_data, opt);
}

int
mpl__mixer_get_proc(mpl__mixer_t *mx, char *name, void_func_t *func)
{
	return mx->get_proc(mx->internal_data, name, func);
}

int 
mpl__mixer_get_interface(mpl__mixer_t *mx, mpl__snd_mixer_t **mixer)
{
	return mx->get_interface(mx->internal_data, mixer);
}

int
mpl__mixer_set_vol(mpl__mixer_t *mx, float32 vol, float32 pan_lr, float32 pan_fb)
{
	return mx->set_vol(mx->internal_data, vol, pan_lr, pan_fb);
}

int
mpl__mixer_mix(mpl__mixer_t *mx, void *buffer, int length)
{
	return mx->mix(mx->internal_data, buffer, length);
}

int
mpl__mixer_get_num_active_channels(mpl__mixer_t *mx)
{
	return mx->get_num_active_channels(mx->internal_data);
}
