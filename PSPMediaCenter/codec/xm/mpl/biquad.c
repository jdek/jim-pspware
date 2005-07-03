#include "../mpl/biquad.h"
#include <math.h>

#include "minimath.h"

#define LN2O2 0.34657359027997265470861606072909


void
mpl__biquad_lp(mpl__biquad_coeff_t *biquad, float32 omega, float32 bandwidth)
{
	float64 sn, cs, al, a0, a1, a2, b0, b1, b2, ooa0;

	sn = sin(omega);
	cs = cos(omega);
	al = sn * sinh(LN2O2 * bandwidth * omega / sn);

	b0 = (1 - cs) * 0.5;
	b1 = 1 - cs;
	b2 = (1 - cs) * 0.5;
	a0 = 1 + al;
	a1 = -2 * cs;
	a2 = 1 - al;

	ooa0 = 1.0f / a0;

	biquad->a0 = b0 * ooa0;
	biquad->a1 = b1 * ooa0;
	biquad->a2 = b2 * ooa0;
	biquad->b1 = -a1 * ooa0;
	biquad->b2 = -a2 * ooa0;
}

void
mpl__biquad_bp(mpl__biquad_coeff_t *biquad, float32 omega, float32 bandwidth)
{
	float64 sn, cs, al, a0, a1, a2, b0, b1, b2, ooa0;

	sn = sin(omega);
	cs = cos(omega);
	al = sn * sinh(LN2O2 * bandwidth * omega / sn);

	b0 = al;
	b1 = 0;
	b2 = -al;
	a0 = 1 + al;
	a1 = -2 * cs;
	a2 = 1 - al;

	ooa0 = 1.0f / a0;

	biquad->a0 = b0 * ooa0;
	biquad->a1 = b1 * ooa0;
	biquad->a2 = b2 * ooa0;
	biquad->b1 = -a1 * ooa0;
	biquad->b2 = -a2 * ooa0;
}

void
mpl__biquad_hp(mpl__biquad_coeff_t *biquad, float32 omega, float32 bandwidth)
{
	float64 sn, cs, al, a0, a1, a2, b0, b1, b2, ooa0;

	sn = sin(omega);
	cs = cos(omega);
	al = sn * sinh(LN2O2 * bandwidth * omega / sn);

	b0 = (1 + cs) * 0.5;
	b1 = -(1 + cs);
	b2 = (1 + cs) * 0.5;
	a0 = 1 + al;
	a1 = -2 * cs;
	a2 = 1 - al;

	ooa0 = 1.0f / a0;

	biquad->a0 = b0 * ooa0;
	biquad->a1 = b1 * ooa0;
	biquad->a2 = b2 * ooa0;
	biquad->b1 = -a1 * ooa0;
	biquad->b2 = -a2 * ooa0;
}

void
mpl__biquad_no(mpl__biquad_coeff_t *biquad, float32 omega, float32 bandwidth)
{
	float64 sn, cs, al, a0, a1, a2, b0, b1, b2, ooa0;

	sn = sin(omega);
	cs = cos(omega);
	al = sn * sinh(LN2O2 * bandwidth * omega / sn);

	b0 = 1;
	b1 = -2 * cs;
	b2 = 1;
	a0 = 1 + al;
	a1 = b1;
	a2 = 1 - al;

	ooa0 = 1.0f / a0;

	biquad->a0 = b0 * ooa0;
	biquad->a1 = b1 * ooa0;
	biquad->a2 = b2 * ooa0;
	biquad->b1 = -a1 * ooa0;
	biquad->b2 = -a2 * ooa0;
}

void
mpl__biquad_peq(mpl__biquad_coeff_t *biquad, float32 omega, float32 bandwidth, float32 dbgain)
{
	float64 sn, cs, al, a, a0, a1, a2, b0, b1, b2, ooa, ooa0;

	sn = sin(omega);
	cs = cos(omega);
	a  = pow(10, dbgain * 0.025);
	ooa = 1.0 / a;
	al = sn * sinh(LN2O2 * bandwidth * omega / sn);

	b0 = 1 + al * a;
	b1 = -2 * cs;
	b2 = 1 - al * a;
	a0 = 1 + al * ooa;
	a1 = b1;
	a2 = 1 - al * ooa;

	ooa0 = 1.0f / a0;

	biquad->a0 = b0 * ooa0;
	biquad->a1 = b1 * ooa0;
	biquad->a2 = b2 * ooa0;
	biquad->b1 = -a1 * ooa0;
	biquad->b2 = -a2 * ooa0;
}

void
mpl__biquad_ls(mpl__biquad_coeff_t *biquad, float32 omega, float32 dbgain, float32 slope)
{
	float64 sn, cs, be, a, a0, a1, a2, b0, b1, b2, ooa0, ap1, am1;

	sn  = sin(omega);
	cs  = cos(omega);
	a   = pow(10, dbgain * 0.025);
	ap1 = a + 1;
	am1 = a - 1;
	be  = sqrt((a * a + 1) / slope - am1 * am1);

	b0 = a * (ap1 - am1 * cs + be * sn);
	b1 = 2* a * (am1 - ap1 * cs);
	b2 = a * (ap1 - am1 * cs - be * sn);
	a0 = ap1 + am1 * cs + be * sn;
	a1 = -2 * (am1 + ap1 * cs);
	a2 = ap1 + am1 * cs - be * sn;

	ooa0 = 1.0f / a0;

	biquad->a0 = b0 * ooa0;
	biquad->a1 = b1 * ooa0;
	biquad->a2 = b2 * ooa0;
	biquad->b1 = -a1 * ooa0;
	biquad->b2 = -a2 * ooa0;
}

void
mpl__biquad_hs(mpl__biquad_coeff_t *biquad, float32 omega, float32 dbgain, float32 slope)
{
	float64 sn, cs, be, a, a0, a1, a2, b0, b1, b2, ooa0, ap1, am1;

	sn  = sin(omega);
	cs  = cos(omega);
	a   = pow(10, dbgain * 0.025);
	ap1 = a + 1;
	am1 = a - 1;
	be  = sqrt((a * a + 1) / slope - am1 * am1);

	b0 = a * (ap1 + am1 * cs + be * sn);
	b1 = -2 * a * (am1 + ap1 * cs);
	b2 = a * (ap1 + am1 * cs - be * sn);
	a0 = ap1 - am1 * cs + be * sn;
	a1 = 2 * (am1 - ap1 * cs);
	a2 = ap1 - am1 * cs - be * sn;

	ooa0 = 1.0f / a0;

	biquad->a0 = b0 * ooa0;
	biquad->a1 = b1 * ooa0;
	biquad->a2 = b2 * ooa0;
	biquad->b1 = -a1 * ooa0;
	biquad->b2 = -a2 * ooa0;
}
