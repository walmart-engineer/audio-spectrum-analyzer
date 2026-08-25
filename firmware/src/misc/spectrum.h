/*
 * spectrum.h — 24-bar audio spectrum analyser display
 *
 * Layout (landscape 320×240):
 *
 *   ┌──────────┬──────────────────────────────┐  y=0
 *   │ Y-axis   │  24 green bars on black      │
 *   │ labels   │  each 10px wide, 2px gap     │
 *   │  30px    │          290px               │
 *   │          │                              │
 *   ├──────────┴──────────────────────────────┤  y=AXIS_Y (210)
 *   │  X-axis: purple line + bar labels       │
 *   │  30px tall                              │
 *   └────────────────────────────────────────-┘  y=240
 */

#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <stdint.h>

/* ---- bar count -------------------------------------------------------- */
#define SPECTRUM_BARS       24

/* ---- layout geometry -------------------------------------------------- */
#define PLOT_X_ORIGIN       30      /* left edge of plot area              */
#define PLOT_Y_TOP          5       /* top edge of plot area               */
#define AXIS_Y              210     /* Y-coordinate of the X-axis line     */
#define PLOT_HEIGHT         (AXIS_Y - PLOT_Y_TOP)   /* 205 px             */
#define BAR_SLOT_WIDTH      12      /* pixels per bar slot (bar + gap)     */
#define BAR_WIDTH           10      /* actual coloured bar width           */
#define BAR_GAP             2       /* black gap between bars              */
#define BAR_X_START         (PLOT_X_ORIGIN + 1)  /* first bar left edge   */

extern const uint8_t font5x7[10][5];

/* ---- API -------------------------------------------------------------- */

/* Draw the static axes and labels (call once after init) */
void Spectrum_DrawAxes(void);

/* Differential update: only repaints pixels that changed since last call.
   magnitudes[24] = 0..255 per bar.                                       */
void Spectrum_Update(const uint8_t *magnitudes);

#endif
