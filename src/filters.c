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



void bpf( double* buf, int buf_size ) {
    static double
        x0 = 0.0, x1 = 0.0, x2 = 0.0, x3 = 0.0, x4 = 0.0, x5 = 0.0, x6 = 0.0,
        y0 = 0.0, y1 = 0.0, y2 = 0.0, y3 = 0.0, y4 = 0.0, y5 = 0.0, y6 = 0.0;

    static double
        a1 = -2.8570266491156824, a2 = 4.86529587175914, a3 = -5.020707094599197, a4 = 3.611250430942624, a5 = -1.5664913924010309, a6 = 0.40743436683069173,
        b0 = 0.007395688813095265, b1 = -1.9816124738985983e-18, b2 = -0.022187066439285797, b3 = 0.0, b4 = 0.022187066439285797, b5 = 1.9816124738985983e-18, b6 = -0.007395688813095265;

    for( int i = 0; i < buf_size; i++ ) {
        x6 = x5;
        x5 = x4;
        x4 = x3;
        x3 = x2;
        x2 = x1;
        x1 = x0;
        x0 = buf[i];

        y6 = y5;
        y5 = y4;
        y4 = y3;
        y3 = y2;
        y2 = y1;
        y1 = y0;
        y0 = b0*x0 + b1*x1 + b2*x2 + b3*x3 + b4*x4 + b5*x5 + b6*x6;
        y0 -= a1*y1 + a2*y2 + a3*y3 + a4*y4 + a5*y5 + a6*y6;

        buf[i] = y0;
    }
}
