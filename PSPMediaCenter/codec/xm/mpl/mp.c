#include "../mpl/mp.h"

int mpl__mp_set_dev(mpl__mp_t * mp, mpl__snd_dev_t * dev)
{
    return mp->set_dev(mp->internal_data, dev);
}

int mpl__mp_reset(mpl__mp_t * mp)
{
    return mp->reset(mp->internal_data);
}

int mpl__mp_play(mpl__mp_t * mp)
{
    return mp->play(mp->internal_data);
}

int mpl__mp_stop(mpl__mp_t * mp)
{
    return mp->stop(mp->internal_data);
}

int mpl__mp_set_loop(mpl__mp_t * mp, int loop)
{
    return mp->set_loop(mp->internal_data, loop);
}

int mpl__mp_set_pos(mpl__mp_t * mp, int pos)
{
    return mp->set_pos(mp->internal_data, pos);
}

int mpl__mp_get_pos(mpl__mp_t * mp)
{
    return mp->get_pos(mp->internal_data);
}

int mpl__mp_set_vol(mpl__mp_t * mp, float32 vol)
{
    return mp->set_vol(mp->internal_data, vol);
}

int mpl__mp_get_vol(mpl__mp_t * mp, float32 * vol)
{
    return mp->get_vol(mp->internal_data, vol);
}

int mpl__mp_set_option(mpl__mp_t * mp, char *option, char *value)
{
    return mp->set_option(mp->internal_data, option, value);
}

int mpl__mp_get_option(mpl__mp_t * mp, char *option, char **value)
{
    return mp->get_option(mp->internal_data, option, value);
}

int mpl__mp_get_proc(mpl__mp_t * mp, char *name, void_func_t * proc)
{
    return mp->get_proc(mp->internal_data, name, proc);
}
