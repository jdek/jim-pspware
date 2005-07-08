#ifndef _MINIMATH_H_
#define _MINIMATH_H_

#ifdef __cplusplus
extern "C" {
#endif

#define sin mysinf
#define sinh mysinf
#define cos mycosf
#define pow mypowf
#define sqrt mysqrtf
#define floorf myfloorf


#define PI      (3.14159265358979323846f)
#define TWOPI (PI+PI)

    static float shift23 = (float) (1 << 23);
    static float OOshift23 = 1.0f / (float) (1 << 23);
    static inline float myfloorf(float i) {	// return largest integer that is less than or equal to i
	float k = (float) ((int) i);
	if (k <= i)
	     return k;
	else
	     return k - 1;
    } static inline float myLog2(float i) {
	float LogBodge = 0.346607f;
	float x;
	float y;
	x = *(int *) &i;
	x *= OOshift23;		//1/pow(2,23);
	x = x - 127;

	y = x - floorf(x);
	y = (y - y * y) * LogBodge;
	return x + y;
    }
    static inline float myPow2(float i) {
	float PowBodge = 0.33971f;
	float x;
	float y = i - floorf(i);
	y = (y - y * y) * PowBodge;

	x = i + 127 - y;
	x *= shift23;		//pow(2,23);
	*(int *) &x = (int) x;
	return x;
    }

    static inline float mypowf(float a, float b) {
	return myPow2(b * myLog2(a));
    }

    static inline float invsqrtf(float x) {
	float xhalf = 0.5f * x;
	int i = *(int *) &x;
	i = 0x5f3759df - (i >> 1);
	x = *(float *) &i;
	x = x * (1.5f - xhalf * x * x);
	return x;
    }

    static inline float mysqrtf(float val) {
	return (1.0f / invsqrtf(val));
    }

#define SIN_ITERATOR 20
    static inline float mysinf(float v) {
	float res, w;
	int t;
	float fac;
	int i = (int) ((v) / (2.0f * PI));
	v -= (float) i *2.0f * PI;

	fac = 1.0f;
	res = 0.0f;
	w = v;
	for (t = 1; t < SIN_ITERATOR;) {
	    res += fac * w;
	    w *= v * v;
	    t++;
	    fac /= t;
	    t++;
	    fac /= t;

	    res -= fac * w;
	    w *= v * v;
	    t++;
	    fac /= t;
	    t++;
	    fac /= t;
	}
	return res;
    }

    static inline float mycosf(float v) {
	float res, w;
	int t;
	float fac;
	int i = (int) ((v) / (2.0f * PI));
	v -= (float) i *2.0f * PI;

	fac = 1.0f;
	res = 0.0f;
	w = 1.0f;
	for (t = 0; t < SIN_ITERATOR;) {
	    res += fac * w;
	    w *= v * v;
	    t++;
	    fac /= t;
	    t++;
	    fac /= t;

	    res -= fac * w;
	    w *= v * v;
	    t++;
	    fac /= t;
	    t++;
	    fac /= t;
	}
	return res;
    }

#ifdef __cplusplus
}
#endif
#endif
