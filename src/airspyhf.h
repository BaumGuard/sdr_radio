#ifndef AIRSPYHF_H
#define AIRSPYHF_H

#include <libairspyhf/airspyhf.h>
#include <stdint.h>
#include <pthread.h>

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

#endif
