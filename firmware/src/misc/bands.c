#include "bands.h"
 
/* band b owns bins [band_edge[b], band_edge[b+1]) */
static uint16_t band_edge[NBANDS + 1u];
 
/* Number of bands whose nominal width is below one FFT bin.  Set by
 * bands_init(); report it rather than hiding the limitation. */
static uint8_t band_narrow_count;
 
void bands_init(void)
{
    /* Third-octave: ratio is fixed by the octave subdivision, not fitted to
     * a target top frequency.  r = 2^(1/3) = 1.259921.  Thirty bands from
     * 20 Hz therefore land on 20 * 2^10 = 20480 Hz exactly. */
    const float r = powf(2.0f, 1.0f / BANDS_PER_OCT);
 
    band_narrow_count = 0u;
 
    float f = BAND_F_LO;
    for (uint32_t b = 0u; b <= NBANDS; b++) {
        int32_t bin = (int32_t)(f / BIN_HZ + 0.5f);   /* round to nearest */
 
        if (bin < 1)           bin = 1;               /* never include DC  */
        if (bin > (int32_t)N_FFT_COMPLEX) bin = (int32_t)N_FFT_COMPLEX;
 
        band_edge[b] = (uint16_t)bin;
 
        if (b < NBANDS && f < BAND_TRUE_F_LO) {
            band_narrow_count++;
        }
        f *= r;
    }
 
    /* At 11.72 Hz resolution a third-octave band is narrower than one bin
     * below ~45 Hz, so the lowest few edges collapse onto the same bin.
     * Forcing them apart keeps every band non-empty, but those bands then
     * cover their bin's frequency range rather than their nominal
     * third-octave range.  This is a resolution limit of NFFT, not
     * something the band table can fix -- see band_narrow_count. */
    for (uint32_t b = 1u; b <= NBANDS; b++) {
        if (band_edge[b] <= band_edge[b - 1u]) {
            band_edge[b] = (uint16_t)(band_edge[b - 1u] + 1u);
        }
        if (band_edge[b] > (uint16_t)N_FFT_COMPLEX) {
            band_edge[b] = (uint16_t)N_FFT_COMPLEX;
        }
    }
}
 
uint8_t bands_narrow_count(void)
{
    return band_narrow_count;
}
 
void bands_compute(const float *xr, const float *xi, float *energy)
{
    /* Sum of |X[k]|^2, not of |X[k]|.  Energy is additive (Parseval);
     * magnitude is not, so summing magnitudes overstates wide bands.
     * Sum rather than average, so a band genuinely carrying broadband
     * content reads louder than a single-bin band holding one tone.
     *
     * No sqrtf here — 10*log10(mag^2) == 20*log10(mag), so the root folds
     * into the dB conversion for free. */
    for (uint32_t b = 0u; b < NBANDS; b++) {
        const uint32_t k0 = band_edge[b];
        const uint32_t k1 = band_edge[b + 1u];
 
        float sum = 0.0f;
        for (uint32_t k = k0; k < k1; k++) {
            const float rr = xr[k];
            const float ii = xi[k];
            sum += rr * rr + ii * ii;
        }
        energy[b] = sum;
    }
}
 
void bands_to_bars(const float *energy, float *bar)
{
    const float scale = (float)BAR_MAX_PX / (DB_MAX - DB_MIN);
 
    for (uint32_t b = 0u; b < NBANDS; b++) {
        /* +1e-20 keeps log10f finite during digital silence. */
        const float db = 10.0f * log10f(energy[b] + 1e-20f) + DB_OFFSET;
 
        float lvl = (db - DB_MIN) * scale;
        if (lvl < 0.0f)                    lvl = 0.0f;
        if (lvl > (float)BAR_MAX_PX)       lvl = (float)BAR_MAX_PX;
 
        /* Fast attack, slow release.  Without this the bars flicker at the
         * hop rate and read as broken even when the numbers are correct. */
        if (lvl > bar[b]) {
            bar[b] = lvl;
        } else {
            bar[b] -= DECAY_PX_HOP;
            if (bar[b] < 0.0f) bar[b] = 0.0f;
        }
    }
}
 
const uint16_t *bands_edges(void)
{
    return band_edge;
}