#include "filters.h"

#include <stdint.h>

void lpf_17kHz( double* buf, int buf_size ) {
    static double
        x0 = 0.0, x1 = 0.0, x2 = 0.0,
        y0 = 0.0, y1 = 0.0, y2 = 0.0;

    static double
        a1 = -1.38389, a2 = 0.533252,
        b0 = 0.0373403, b1 = 0.0746806, b2 = 0.0373403;

    for( uint32_t i = 0; i < buf_size; i++ ) {
        x2 = x1;
        x1 = x0;
        x0 = buf[i];

        y2 = y1;
        y1 = y0;
        y0 = b0*x0 + b1*x1 + b2*x2;
        y0 -= a1*y1 + a2*y2;

        buf[i] = y0;
    }
}


void lpf_5kHz( double* buf, int buf_size ) {
    static double
        x0 = 0.0, x1 = 0.0, x2 = 0.0,
        y0 = 0.0, y1 = 0.0, y2 = 0.0;

    static double
        a1 = -1.81534, a2 = 0.831006,
        b0 = 0.00391613, b1 = 0.00783225, b2 = 0.00391613;

    for( uint32_t i = 0; i < buf_size; i++ ) {
        x2 = x1;
        x1 = x0;
        x0 = buf[i];

        y2 = y1;
        y1 = y0;
        y0 = b0*x0 + b1*x1 + b2*x2;
        y0 -= a1*y1 + a2*y2;

        buf[i] = y0;
    }
}
