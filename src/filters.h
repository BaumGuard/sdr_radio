#ifndef FILTERS_H
#define FILTERS_H


// Arrays for the IIR coefficients
extern double a[3], b[3];


/*------------------------------------------------------------------------*/


/*
Calculate the coefficients of a low pass filter implemented as a
2nd degree Butterworth IIR filter

Args:
 - fc   : Cutoff frequency in Hz
 - fs   : Sampling frequency in Hz
 - a    : Array of the denominator coefficients (size 3)
 - b    : Array of the numerator coefficients (size 3)
*/
void butterworth_iir_coefficients( double fc, double fs, double* a, double* b );


/*
Send samples through the IIR filter

Args:
 - src_buf  : Array of the samples to be filtered
 - dest_buf : Array of the output samples
 - buf_size : Size of the source and destination buffer
 - a        : Array of the denominator coefficients (size 3)
 - b        : Array of the numerator coefficients (size 3)
*/
void filter_forward(
    double* src_buf, double* dest_buf, int buf_size, double* a, double* b );

#endif
