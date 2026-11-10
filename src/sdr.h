#ifndef SDR_H
#define SDR_H

#include <pthread.h>
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




// Conditional variable to terminate the reception in
// the function stop_processing()
extern pthread_mutex_t airspyhf_mutex;
extern pthread_cond_t airspyhf_running;

/*
Initialize the AirspyHF SDR
This does not start the reception it just configures parameters
like the receiving frequency, the sample rate and the gain

Args:
 - freq         : Receiving frequency in Hz
 - sample_rate  : Sampling frequency in Hz (normally 192000)
                  Possible values:
                   - 192000
                   - 256000
                   - 384000
                   - 456000
                   - 768000
                   - 912000
 - gain         : Use AGC
                  Possible values:
                   - 0 -> AGC off
                   - 1 -> AGC on

Returns (Defined in /usr/include/libairspyhf/airspyhf.h):
 -  0 : Success

 - -1 (AIRSPYHF_ERROR)
 - -2 (AIRSPYHF_UNSUPPORTED)
*/
int init_airspyhf( uint32_t freq, uint32_t sample_rate, int gain );

#endif
