#ifndef TWIDDLE_H
#define TWIDDLE_H

#include "config.h"

/* FFT_N is the COMPLEX length (2048); tables hold FFT_N/2 angles each. */
#define TW_LEN  (FFT_FRAME_SIZE / 2)

extern const float fft_tw_re[TW_LEN];      /* cos(-2*pi*k/2048) */
extern const float fft_tw_im[TW_LEN];      /* sin(-2*pi*k/2048) */

extern const float unpack_tw_re[TW_LEN];   /* cos(-2*pi*k/4096) */
extern const float unpack_tw_im[TW_LEN];   /* sin(-2*pi*k/4096) */

#endif