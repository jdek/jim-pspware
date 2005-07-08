#include "../mpl/ml.h"

int mpl__ml_load(mpl__ml_t * ml, vbf_t * file, mpl__md_t ** md)
{
    return ml->load(ml->internal_data, file, md);
}

int mpl__ml_destroy(mpl__ml_t * ml, mpl__md_t * md)
{
    return ml->destroy(ml->internal_data, md);
}

int mpl__ml_create_mp(mpl__ml_t * ml, mpl__md_t * md, mpl__msg_box_t * mb, mpl__mp_t ** mp)
{
    return ml->create_mp(ml->internal_data, md, mb, mp);
}

int mpl__ml_destroy_mp(mpl__ml_t * ml, mpl__mp_t ** mp)
{
    return ml->destroy_mp(ml->internal_data, mp);
}

int mpl__ml_set_option(mpl__ml_t * ml, char *option, char *value)
{
    return ml->set_option(ml->internal_data, option, value);
}

int mpl__ml_get_option(mpl__ml_t * ml, char *option, char **value)
{
    return ml->get_option(ml->internal_data, option, value);
}

int mpl__ml_get_proc(mpl__ml_t * ml, char *name, void_func_t * proc)
{
    return ml->get_proc(ml->internal_data, name, proc);
}
