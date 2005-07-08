#ifndef dsp_std_h_n43782784923
#define dsp_std_h_n43782784923

#include "../mpl/snddev.h"

int mpl__dsp_add(void *src, void *dst, int size, int ch_num, float32 vol, int format);
int mpl__dsp_sub(void *src, void *dst, int size, int ch_num, float32 vol, int format);
int mpl__dsp_move(void *src, void *dst, int size, int ch_num, float32 vol, int format);
int mpl__dsp_conv_format(void *src, void *dst, int src_format, int dst_format, int size, int ch_num, float32 vol);

#endif
