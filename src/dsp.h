#ifndef DSP_H
#define DSP_H

#include <rtl-sdr.h>
#include <libairspyhf/airspyhf.h>

/*
Callback function to process a buffer of samples
Performs demodulation, filtering and output of a sample chunk

Args:
 - buf   : Pointer to the buffer of samples
           Format: 8 bit unsigned integers, interleaved IQIQ...
 - len   : Total length of the buffer (with I and Q samples)
 - ctx   : Additional context (Should normally be NULL)
*/
void rtlsdr_on_samples_received( unsigned char* buf, uint32_t len, void* ctx );



/*
Callback function to process a buffer of samples
Performs demodulation, filtering and output of a sample chunk

Args:
 - transfer_fn  : Pointer to the buffer of samples, length of the buffer
                  and further additional information
                  (Defined in /usr/include/libairspyhf/airspyhf.h)

Returns:
 - 0    : Success
*/
int airspyhf_on_samples_received( airspyhf_transfer_t* transfer_fn );



/*
Stop the processing and close all devices used
This function is executed on the signal SIGINT (Ctrl+C)
*/
void stop_processing( int sig );

#endif
