#ifndef BANDS_H
#define BANDS_H

#include <math.h>
#include "config.h"

/* Build the bin-range table.  Call once at boot, before bands_compute(). */
void bands_init(void);
 
/* Sum |X[k]|^2 over each band's bin range.
 * xr/xi are the NC+1 real-spectrum bins from real_unpack().
 * energy[] receives NBANDS values (linear power, not dB). */
void bands_compute(const float *xr, const float *xi, float *energy);
 
/* Convert band energies to bar heights in pixels, applying dB scaling,
 * clamping, and fast-attack / slow-decay ballistics.
 * bar[] persists between calls — it holds the decay state. */
void bands_to_bars(const float *energy, float *bar);
 
/* Read-only access to the edge table, for tests and diagnostics. */
const uint16_t *bands_edges(void);
 
/* How many low bands are narrower than one FFT bin and so had their edges
 * forced apart.  Those bands are resolution-limited: they do not cover
 * their nominal third-octave range.  Zero means the whole display is
 * honest third-octave. */
uint8_t bands_narrow_count(void);



#endif