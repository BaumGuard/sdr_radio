#include "filters.h"

#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <complex.h>


double a[3], b[3];


void butterworth_iir_coefficients( double fc, double fs, double* a, double* b ) {
    // Frequency prewarping
    double Fc = fs / M_PI * tan( M_PI * fc / fs );

    // Position of the first pole
    double theta = M_PI / 4.0;
    complex double p = -sin( theta ) + I*cos( theta );
    p *= TWO_PI * Fc;

    // s to z domain transform
    p = (1.0 + p / (2.0*fs)) / (1.0 - p / (2.0*fs));

    // IIR coefficients
    a[0] =  1.0,
    a[1] = -2.0 * creal(p),
    a[2] = creal(p)*creal(p) + cimag(p)*cimag(p),

    b[0] = 1.0,
    b[1] = 2.0,
    b[2] = 1.0;

    // Gain normalization
    double k = (a[0] + a[1] + a[2]) / (b[0] + b[1] + b[2]);
    b[0] *= k;
    b[1] *= k;
    b[2] *= k;
}


void filter_forward(
    double* src_buf, double* dest_buf, int buf_size, double* a, double* b )
{
    static double
        x0 = 0.0, x1 = 0.0, x2 = 0.0,
        y0 = 0.0, y1 = 0.0, y2 = 0.0;

        for( uint32_t i = 0; i < buf_size; i++ ) {
            x2 = x1;
            x1 = x0;
            x0 = src_buf[i];

            y2 = y1;
            y1 = y0;
            y0 = b[0]*x0 + b[1]*x1 + b[2]*x2;
            y0 -= a[1]*y1 + a[2]*y2;

            dest_buf[i] = y0;
        }
}
