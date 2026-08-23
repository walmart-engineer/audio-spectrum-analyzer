#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define SYS_CLK 64000000U

#define ADC_BUFFER_SIZE 512u
#define DAC_BUFFER_SIZE (4u * ADC_BUFFER_SIZE) //factor 4 este deja aplicat in dac_push() Nu trebuie aplicat inca odata la dimensiunea totala buddy
#define FFT_SAMPLE_RING_SIZE 4096u
#define FFT_FRAME_SIZE 4096u
#define N_FFT_COMPLEX 2048u

#define PI 3.14159265359f


#endif