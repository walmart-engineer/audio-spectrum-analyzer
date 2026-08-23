#ifndef WINDOW_H
#define WINDOW_H

#include "config.h"

/* Window spans the REAL frame (4096), not the complex FFT length. */
#define WIN_LEN  FFT_FRAME_SIZE

extern const float hann_win[WIN_LEN];

/* Coherent gain 0.5 -> multiply single-tone magnitudes by 2.0 */
#define WIN_AMP_CORR   2.0f
/* ENBW 1.5 bins -> for noise-density work, divide power by 1.5 */
#define WIN_ENBW_BINS  1.5f

#endif