#include "../mpl/snddev.h"

// dev

int mpl__snd_dev_set_vol(mpl__snd_dev_t * dev, float32 vol, float32 pan_lr, float32 pan_fb)
{
    return dev->set_vol(dev->internal_data, vol, pan_lr, pan_fb);
}

int mpl__snd_dev_get_vol(mpl__snd_dev_t * dev, float32 * vol, float32 * pan_lr, float32 * pan_fb)
{
    return dev->get_vol(dev->internal_data, vol, pan_lr, pan_fb);
}

int mpl__snd_dev_set_output_options(mpl__snd_dev_t * dev, mpl__snd_dev_output_t opt)
{
    return dev->set_output_options(dev->internal_data, opt);
}

int mpl__snd_dev_get_output_options(mpl__snd_dev_t * dev, mpl__snd_dev_output_t * opt)
{
    return dev->get_output_options(dev->internal_data, opt);
}

int mpl__snd_dev_set_latency(mpl__snd_dev_t * dev, int latency)
{
    return dev->set_latency(dev->internal_data, latency);
}

int mpl__snd_dev_get_latency(mpl__snd_dev_t * dev)
{
    return dev->get_latency(dev->internal_data);
}

int mpl__snd_dev_set_option(mpl__snd_dev_t * dev, char *option, char *value)
{
    return dev->set_option(dev->internal_data, option, value);
}

int mpl__snd_dev_get_option(mpl__snd_dev_t * dev, char *option, char **value)
{
    return dev->get_option(dev->internal_data, option, value);
}

int mpl__snd_dev_get_proc(mpl__snd_dev_t * dev, char *name, void_func_t * proc)
{
    return dev->get_proc(dev->internal_data, name, proc);
}

int mpl__snd_dev_create_mixer(mpl__snd_dev_t * dev, mpl__snd_mixer_t ** mixer, mpl__snd_mixer_opt_t opt)
{
    return dev->create_mixer(dev->internal_data, mixer, opt);
}

int mpl__snd_dev_destroy_mixer(mpl__snd_dev_t * dev, mpl__snd_mixer_t * mixer)
{
    return dev->destroy_mixer(dev->internal_data, mixer);
}

int mpl__snd_dev_create_stream(mpl__snd_dev_t * dev, mpl__snd_stream_t ** stream, mpl__snd_stream_opt_t opt)
{
    return dev->create_stream(dev->internal_data, stream, opt);
}

int mpl__snd_dev_destroy_stream(mpl__snd_dev_t * dev, mpl__snd_stream_t * stream)
{
    return dev->destroy_stream(dev->internal_data, stream);
}

// mixer

int mpl__snd_mixer_reset(mpl__snd_mixer_t * mixer)
{
    return mixer->reset(mixer->internal_data);
}

int mpl__snd_mixer_pause(mpl__snd_mixer_t * mixer)
{
    return mixer->pause(mixer->internal_data);
}

int mpl__snd_mixer_stop(mpl__snd_mixer_t * mixer)
{
    return mixer->stop(mixer->internal_data);
}

int mpl__snd_mixer_upload_sample(mpl__snd_mixer_t * mixer, mpl__snd_mixer_smp_t sample)
{
    return mixer->upload_sample(mixer->internal_data, sample);
}

int mpl__snd_mixer_destroy_sample(mpl__snd_mixer_t * mixer, int handle)
{
    return mixer->destroy_sample(mixer->internal_data, handle);
}

int mpl__snd_mixer_set_vol(mpl__snd_mixer_t * mixer, float32 vol, float32 pan_lr, float32 pan_fb)
{
    return mixer->set_vol(mixer->internal_data, vol, pan_lr, pan_fb);
}

int mpl__snd_mixer_get_vol(mpl__snd_mixer_t * mixer, float32 * vol, float32 * pan_lr, float32 * pan_fb)
{
    return mixer->get_vol(mixer->internal_data, vol, pan_lr, pan_fb);
}

int mpl__snd_mixer_get_output_options(mpl__snd_mixer_t * mixer, mpl__snd_dev_output_t * opt)
{
    return mixer->get_output_options(mixer->internal_data, opt);
}

int mpl__snd_mixer_get_latency(mpl__snd_mixer_t * mixer)
{
    return mixer->get_latency(mixer->internal_data);
}

int mpl__snd_mixer_get_num_channels(mpl__snd_mixer_t * mixer)
{
    return mixer->get_num_channels(mixer->internal_data);
}

int mpl__snd_mixer_get_num_active_channels(mpl__snd_mixer_t * mixer)
{
    return mixer->get_num_active_channels(mixer->internal_data);
}

int mpl__snd_mixer_get_free_channel(mpl__snd_mixer_t * mixer)
{
    return mixer->get_free_channel(mixer->internal_data);
}

int mpl__snd_mixer_is_channel_active(mpl__snd_mixer_t * mixer, int ch)
{
    return mixer->is_channel_active(mixer->internal_data, ch);
}

int mpl__snd_mixer_set_channel_vol(mpl__snd_mixer_t * mixer, int ch, float32 vol, float32 pan_lr, float32 pan_fb)
{
    return mixer->set_channel_vol(mixer->internal_data, ch, vol, pan_lr, pan_fb);
}

int mpl__snd_mixer_set_channel_freq(mpl__snd_mixer_t * mixer, int ch, float freq)
{
    return mixer->set_channel_freq(mixer->internal_data, ch, freq);
}

int mpl__snd_mixer_set_channel_pos(mpl__snd_mixer_t * mixer, int ch, float pos, int dir)
{
    return mixer->set_channel_pos(mixer->internal_data, ch, pos, dir);
}

int
mpl__snd_mixer_play_channel(mpl__snd_mixer_t * mixer, int ch, int handle, float32 freq, float32 vol, float32 pan_lr,
			    float32 pan_fb, float64 pos, int dir)
{
    return mixer->play_channel(mixer->internal_data, ch, handle, freq, vol, pan_lr, pan_fb, pos, dir);
}

int mpl__snd_mixer_stop_channel(mpl__snd_mixer_t * mixer, int ch)
{
    return mixer->stop_channel(mixer->internal_data, ch);
}

int mpl__snd_mixer_set_option(mpl__snd_mixer_t * mixer, char *option, char *value)
{
    return mixer->set_option(mixer->internal_data, option, value);
}

int mpl__snd_mixer_get_option(mpl__snd_mixer_t * mixer, char *option, char **value)
{
    return mixer->get_option(mixer->internal_data, option, value);
}

int mpl__snd_mixer_get_proc(mpl__snd_mixer_t * mixer, char *name, void_func_t * proc)
{
    return mixer->get_proc(mixer->internal_data, name, proc);
}

int mpl__snd_mixer_set_call_back(mpl__snd_mixer_t * mixer, mpl__snd_call_back_t call_back)
{
    return mixer->set_call_back(mixer->internal_data, call_back);
}

// stream

int mpl__snd_stream_set_vol(mpl__snd_stream_t * stream, float32 vol, float32 pan_lr, float32 pan_fb)
{
    return stream->set_vol(stream->internal_data, vol, pan_lr, pan_fb);
}

int mpl__snd_stream_get_vol(mpl__snd_stream_t * stream, float32 * vol, float32 * pan_lr, float32 * pan_fb)
{
    return stream->get_vol(stream->internal_data, vol, pan_lr, pan_fb);
}

int mpl__snd_stream_get_output_options(mpl__snd_stream_t * stream, mpl__snd_dev_output_t * opt)
{
    return stream->get_output_options(stream->internal_data, opt);
}

int mpl__snd_stream_set_input_format(mpl__snd_stream_t * stream, mpl__snd_stream_format_t * opt)
{
    return stream->set_input_format(stream->internal_data, opt);
}

int mpl__snd_stream_get_input_format(mpl__snd_stream_t * stream, mpl__snd_stream_format_t * opt)
{
    return stream->get_input_format(stream->internal_data, opt);
}

int mpl__snd_stream_get_latency(mpl__snd_stream_t * stream)
{
    return stream->get_latency(stream->internal_data);
}

int mpl__snd_stream_get_buffer_size(mpl__snd_stream_t * stream)
{
    return stream->get_buffer_size(stream->internal_data);
}

int mpl__snd_stream_set_option(mpl__snd_stream_t * stream, char *option, char *value)
{
    return stream->set_option(stream->internal_data, option, value);
}

int mpl__snd_stream_get_option(mpl__snd_stream_t * stream, char *option, char **value)
{
    return stream->get_option(stream->internal_data, option, value);
}

int mpl__snd_stream_get_proc(mpl__snd_stream_t * stream, char *name, void_func_t * proc)
{
    return stream->get_proc(stream->internal_data, name, proc);
}

int mpl__snd_stream_write(mpl__snd_stream_t * stream, void *data, int length)
{
    return stream->write(stream->internal_data, data, length);
}

int mpl__snd_stream_set_call_back(mpl__snd_stream_t * stream, mpl__snd_call_back_t call_back)
{
    return stream->set_call_back(stream->internal_data, call_back);
}
