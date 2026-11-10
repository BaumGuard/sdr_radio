#ifndef FILTERS_H
#define FILTERS_H

/*
Perform low pass filtering on samples with a cutoff frequency
of 17 kHz (for FM radio)
The filter is implemented as a second degree IIR Butterworth filter
and assumes a sampling frequency of 240 kHz.

Args:
 - buf      : Pointer to the buffer with the samples to filter
              The filter output is written into this buffer as well
 - buf_size : Size of the buffer buf
*/
void lpf_17kHz( double* buf, int buf_size );


/*
Perform low pass filtering on samples with a cutoff frequency
of 5 kHz (for AM radio)
The filter is implemented as a second degree IIR Butterworth filter
and assumes a sampling frequency of 240 kHz.

Args:
 - buf      : Pointer to the buffer with the samples to filter
              The filter output is written into this buffer as well
 - buf_size : Size of the buffer buf
*/
void lpf_5kHz( double* buf, int buf_size );

void bpf( double* buf, int buf_size );

#endif
