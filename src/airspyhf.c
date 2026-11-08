#include "airspyhf.h"

#include "demod.h"
#include "filters.h"
#include "audio.h"
#include "common.h"

#include <stdio.h>


pthread_mutex_t airspyhf_mutex;
pthread_cond_t airspyhf_running;


int init_airspyhf( uint32_t freq, uint32_t sample_rate, int gain ) {
    pthread_mutex_init( &airspyhf_mutex, NULL );
    pthread_cond_init( &airspyhf_running, NULL );

    pthread_mutex_lock( &airspyhf_mutex );

    int status;
    status = airspyhf_open( &airspyhf );

    switch( status ) {
        case AIRSPYHF_ERROR:
            fprintf( stderr, "[ERROR] No SDR connected\n" );
            return status;

        case AIRSPYHF_UNSUPPORTED:
            fprintf( stderr, "[ERROR] SDR not supported\n" );
            return status;
    }

    status = airspyhf_set_hf_agc( airspyhf, gain );
    if( status ) {
        fprintf( stderr, "[ERROR] AirspyHF: Failed to enable AGC\n" );
        return status;
    }

    status = airspyhf_set_samplerate( airspyhf, sample_rate );
    if( status ) {
        fprintf( stderr, "[ERROR] AirspyHF: Invalid sample rate %d Hz\n", sample_rate );
    }

    status = airspyhf_set_freq( airspyhf, freq );
    if( status ) {
        fprintf( stderr, "[ERROR] Airspy: Invalid frequency %d Hz\n", freq );
    }

    // airspyhf_set_lib_dsp( airspyhf, 1 );

    return 0;
}



int airspyhf_on_samples_received( airspyhf_transfer_t* transfer_fn ) {
    uint32_t current_buf = recv_buf_idx % NUM_INPUT_BUFFERS;

    for( int i = 0; i < transfer_fn->sample_count; i++ ) {
        airspyhf_recv_i_buf[current_buf][i] = (double) transfer_fn->samples[i].re;
        airspyhf_recv_q_buf[current_buf][i] = (double) transfer_fn->samples[i].im;
    }

    switch( modulation ) {
        case FM:
            fm_demod( airspyhf_recv_i_buf[current_buf], airspyhf_recv_q_buf[current_buf], buffer, AIRSPYHF_INPUT_BUFFER_SIZE );
            lpf_17kHz( buffer, AIRSPYHF_INPUT_BUFFER_SIZE );
            output_audio_chunk( buffer, AIRSPYHF_INPUT_BUFFER_SIZE, 1.0 );
            break;

        case AM:
            am_demod( airspyhf_recv_i_buf[current_buf], airspyhf_recv_q_buf[current_buf], buffer, AIRSPYHF_INPUT_BUFFER_SIZE );
            lpf_5kHz( buffer, AIRSPYHF_INPUT_BUFFER_SIZE );
            output_audio_chunk( buffer, AIRSPYHF_INPUT_BUFFER_SIZE, 128.0 );
            break;
    }



    recv_buf_idx++;

    return 0;
}
