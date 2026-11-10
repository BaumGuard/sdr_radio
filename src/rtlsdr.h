#ifndef RTLSDR_H
#define RTLSDR_H

#include <stdint.h>

/*
Initialize an RTL-SDR device
This does not start the reception, it just configures parameters
like the SDR, the receiving frequency, the sample rate and the gain

Args:
 - sdr_index    : SDR index (can be obtained using rtl_test and should
                  normally be 0 if no other RTL-SDR is connected)
 - freq         : Receiving frequency in Hz
 - sample_rate  : Sampling frequency (normally 240000 Hz)
 - gain         : Receiver gain in tenths of decibels (normally 450)
                  The possible gains can be obtained with the function
                  rtlsdr_get_tuner_gains() in /usr/include/rtl-sdr.h

Returns:
 -  0    : Success
 - -1    : Error
*/
int init_rtlsdr( uint32_t sdr_index, uint32_t freq, uint32_t sample_rate, int gain );



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

#endif
