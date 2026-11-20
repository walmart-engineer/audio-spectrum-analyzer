#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Systems define

#define SYS_CLK 64000000U

// Array Sizes

#define ADC_BUFFER_SIZE 512u
#define DAC_BUFFER_SIZE (4u * ADC_BUFFER_SIZE) //factor 4 este deja aplicat in dac_push() Nu trebuie aplicat inca odata la dimensiunea totala buddy
#define FFT_SAMPLE_RING_SIZE 4096u
#define FFT_FRAME_SIZE 4096u

// DSP Constants

#define N_FFT_COMPLEX 2048u

// Math

#define PI 3.14159265359f

// Spectrum


#define BIN_HZ          11.71875f
#define NBANDS          24u
#define BANDS_PER_OCT   2.4f
#define BAND_F_LO       20.0f
#define BAND_F_HI       (BAND_F_LO * 1024.0f)       /* 20480 Hz, derived   */
 
/* Below this frequency a third-octave band is narrower than one FFT bin,
 * so the bottom bands are resolution-limited, not true third-octaves.
 * f * (2^(1/3) - 1) >= BIN_HZ  ->  f >= BIN_HZ / 0.259921 = 45.1 Hz */
#define BAND_TRUE_F_LO  (BIN_HZ / 0.334840f)

/* ---- display scaling ------------------------------------------------- */
#define DB_MIN          (-60.0f)
#define DB_MAX          (0.0f)
#define DB_OFFSET       (0.0f)   /* calibrate: full-scale sine -> 0 dB     */
#define BAR_MAX_PX      200u
#define DECAY_PX_HOP    3.0f

#endif