#ifndef AIRSPYHF_H
#define AIRSPYHF_H

#include <libairspyhf/airspyhf.h>
#include <stdint.h>
#include <pthread.h>

extern pthread_mutex_t airspyhf_mutex;
extern pthread_cond_t airspyhf_running;

int init_airspyhf( uint32_t freq, uint32_t sample_rate, int gain );

int airspyhf_on_samples_received( airspyhf_transfer_t* transfer_fn );

#endif
