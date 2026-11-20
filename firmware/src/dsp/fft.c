#include "fft.h"

volatile int16_t fft_sample_ring[FFT_SAMPLE_RING_SIZE];
volatile float fft_frame_re[FFT_FRAME_SIZE / 2];
volatile float fft_frame_im[FFT_FRAME_SIZE / 2];

static volatile uint16_t last = 0u;
volatile uint16_t start_index = 0u;
volatile uint8_t hop_ready = 0u;
 

void fft_ring_push(int16_t x){

    if((last + 1) % 1024 == 0){
        hop_ready = 1u;
        start_index += 1024u; 
    }

    if(last > 4095u) {
        last = 0u;
        start_index = 0u;
    }
    fft_sample_ring[last] = x;
    ++last;

}



static void bit_reverse(float *re, float *im, uint16_t n){
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) {
            float t;
            t = re[i]; re[i] = re[j]; re[j] = t;
            t = im[i]; im[i] = im[j]; im[j] = t;
        }
    }
}

void fft_radix2(float *re, float *im, uint16_t n)
{
    bit_reverse(re, im, n);

    for (int len = 2; len <= n; len <<= 1) {     /* STAGE:  2,4,8..n   */
        int half = len >> 1;
        int step = TW_LEN / len;                   /* twiddle stride     */

        for (int base = 0; base < n; base += len)     /* BLOCK         */
            for (int k = 0; k < half; k++) {          /* BUTTERFLY     */
                int i = base + k, j = i + half;
                float wr = fft_tw_re[k*step], wi = fft_tw_im[k*step];
                float vr = re[j]*wr - im[j]*wi;
                float vi = re[j]*wi + im[j]*wr;
                float ur = re[i], ui = im[i];
                re[i] = ur + vr;  im[i] = ui + vi;
                re[j] = ur - vr;  im[j] = ui - vi;
            }
    }
}

void frame_prepare(const int16_t *ring, uint32_t write_idx,
                   float *re, float *im)
{
    const uint32_t MASK = FFT_SAMPLE_RING_SIZE - 1u;      /* 4095, needs power-of-two size */

    for (uint32_t n = 0; n < FFT_FRAME_SIZE/2; n++) {    /* n = 0 … 2047 */
        uint32_t k_even = 2u*n;                 /* chronological index */
        uint32_t k_odd  = 2u*n + 1u;

        int16_t s_even = ring[(write_idx + k_even) & MASK];
        int16_t s_odd  = ring[(write_idx + k_odd ) & MASK];

        re[n] = (float)s_even * hann_win[k_even];
        im[n] = (float)s_odd  * hann_win[k_odd];
    }
}

void real_unpack_inplace(float *re, float *im)
{
    /* In-place: Z[k] in re/im on entry, X[k] in re/im on exit.
     *
     * X has NC+1 bins but only NC slots exist.  Both X[0] and X[NC] are
     * purely real, so the Nyquist bin is stored in im[0], which would
     * otherwise be a hard zero.  Unpacked layout:
     *
     *     re[0] = X[0].re   (DC)          im[0] = X[NC].re  (Nyquist)
     *     re[k] = X[k].re                 im[k] = X[k].im    k = 1..NC-1
     *
     * Safe in place because every iteration reads both Z[k] and Z[NC-k]
     * into locals before writing either X[k] or X[NC-k]. */

    const float a0 = re[0];
    const float b0 = im[0];
    re[0] = a0 + b0;      /* DC      */
    im[0] = a0 - b0;      /* Nyquist */

    /* k = NC/2 is its own mirror: X = conj(Z). */
    im[N_FFT_COMPLEX / 2u] = -im[N_FFT_COMPLEX / 2u];

    for (uint32_t k = 1u; k < N_FFT_COMPLEX / 2u; k++) {
        const uint32_t m = N_FFT_COMPLEX - k;

        const float zrk = re[k], zik = im[k];
        const float zrm = re[m], zim = im[m];

        const float ar = 0.5f * (zrk + zrm);
        const float ai = 0.5f * (zik - zim);
        const float br = 0.5f * (zik + zim);
        const float bi = 0.5f * (zrm - zrk);

        const float c = unpack_tw_re[k];
        const float s = unpack_tw_im[k];

        const float p = c * br + s * bi;
        const float q = c * bi - s * br;

        re[k] =  ar + p;
        im[k] =  ai + q;
        re[m] =  ar - p;
        im[m] = -ai + q;
    }
}