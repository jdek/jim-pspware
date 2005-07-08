#ifndef biquad_h_n43287238497789234
#define biquad_h_n43287238497789234

#include "../sys_cfg.h"

/*
usage:

y[n] = a0 * x[n] + a1 * x[n-1] + a2 * x[n-2] +
                   b1 * y[n-1] + b2 * y[n-2]
*/

typedef struct {
    double a0, a1, a2, b1, b2;
} mpl__biquad_coeff_t;

void mpl__fil_lp(mpl__biquad_coeff_t * fil, float omega, float bandwidth);
void mpl__fil_bp(mpl__biquad_coeff_t * fil, float omega, float bandwidth);
void mpl__fil_hp(mpl__biquad_coeff_t * fil, float omega, float bandwidth);
void mpl__fil_no(mpl__biquad_coeff_t * fil, float omega, float bandwidth);
void mpl__fil_peq(mpl__biquad_coeff_t * fil, float omega, float bandwidth, float dbgain);
void mpl__fil_ls(mpl__biquad_coeff_t * fil, float omega, float dbgain, float slope);
void mpl__fil_hs(mpl__biquad_coeff_t * fil, float omega, float dbgain, float slope);

#endif
