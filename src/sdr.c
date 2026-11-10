#include "sdr.h"

#include "common.h"
#include "colorprint.h"

#include <rtl-sdr.h>
#include <libairspyhf/airspyhf.h>


int init_rtlsdr( uint32_t sdr_index, uint32_t freq, uint32_t sample_rate, int gain ) {
    //wavwriter_open( &filter_wav_file, "filter.wav", 1, 240000, F64 );
    int status = rtlsdr_open( &rtlsdr, sdr_index );
    if( status < 0 ) {
        color_fprintln( stderr, COLOR_RED, "[ERROR] No SDR found" );
        return -1;
    }

    status = rtlsdr_set_sample_rate( rtlsdr, sample_rate );
    if( status ) {
        color_fprintln( stderr, COLOR_RED, "[ERROR] Could not set sample rate" );
        return -1;
    }

    status = rtlsdr_set_tuner_gain( rtlsdr, gain );
    if ( status ) {
        color_fprintln( stderr, COLOR_RED, "[ERROR] Gain could not be set" );
        return -1;
    }

    status = rtlsdr_reset_buffer( rtlsdr );
    if( status ) {
        color_fprintln( stderr, COLOR_RED, "[ERROR] Resetting buffer" );
        return -1;
    }

    status = rtlsdr_set_center_freq( rtlsdr, freq );
    if( status ) {
        color_fprintln( stderr, COLOR_RED, "[ERROR] Invalid frequency %d Hz", freq );
        return -1;
    }

    return 0;
}



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
            color_fprintln( stderr, COLOR_RED, "[ERROR] No SDR connected" );
            return status;

        case AIRSPYHF_UNSUPPORTED:
            color_fprintln( stderr, COLOR_RED, "[ERROR] SDR not supported" );
            return status;
    }

    status = airspyhf_set_hf_agc( airspyhf, gain );
    if( status ) {
        color_fprintln( stderr, COLOR_RED, "[ERROR] AirspyHF: Failed to enable AGC" );
        return status;
    }

    status = airspyhf_set_samplerate( airspyhf, sample_rate );
    if( status ) {
        color_fprintln(
            stderr, COLOR_RED,
            "[ERROR] AirspyHF: Invalid sample rate %d Hz", sample_rate
        );
    }

    status = airspyhf_set_freq( airspyhf, freq );
    if( status ) {
        color_fprintln(
            stderr, COLOR_RED,
            "[ERROR] Airspy: Invalid frequency %d Hz", freq
        );
    }

    // airspyhf_set_lib_dsp( airspyhf, 1 );

    return 0;
}
