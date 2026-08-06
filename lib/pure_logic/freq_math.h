#ifndef __FREQ_MATH_H__
#define __FREQ_MATH_H__
// Frequency grid validation/correction, extracted from main_fun.cpp for
// host-side testing. step: 0 = 5 kHz grid (500), 1 = 6.25 kHz grid (625).
double checkFreqFloatStep(double freq_buf, unsigned char step);
#endif
