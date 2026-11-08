#include <rtl-sdr.h>
#include <libairspyhf/airspyhf.h>
#include <stdio.h>

#include <signal.h>
#include <pthread.h>

#include "rtlsdr.h"
#include "airspyhf.h"
#include "audio.h"
#include "wavwriter/wavwriter.h"
#include "utils.h"
#include "common.h"


void stop_processing( int sig ) {
    printf( "[INFO] Stop signal received - Stopping the reception\n" );

    if( record_audio )
        wavwriter_close( wavfile );

    switch( sdr_type ) {
        case RTLSDR:
            rtlsdr_cancel_async( rtlsdr );
            rtlsdr_close( rtlsdr );
            break;

        case AIRSPYHF:
            airspyhf_stop( airspyhf );
            airspyhf_close( airspyhf );
            pthread_cond_signal( &airspyhf_running );
            break;
    }

    snd_pcm_close( playback_handle );
}

/*
void read_iq_file( char* filename, void (*sample_cb)(unsigned char*, uint32_t, void*) ) {
    FILE* iq_file = fopen( filename, "rb" );

    unsigned char iq_buf[INPUT_BUFFER_SIZE*2];

    unsigned char current_value;
    while( true ) {
        for( uint32_t i = 0; i < INPUT_BUFFER_SIZE*2; i++ ) {
            current_value = getc(iq_file);
            if( !feof(iq_file) ) {
                iq_buf[i] = current_value;
            }
            else {
                return;
            }
        }
        on_samples_received( iq_buf, INPUT_BUFFER_SIZE*2, NULL );
    }

    fclose( iq_file );
}
*/




int main( int argc, char* argv[] ) {
    parse_arguments( argc, argv );

    if( radio_freq <= 30000000 ) {
        modulation = AM;
    }

    if( record_audio )
        wavwriter_open( &wavfile, audio_filepath, 1, AUDIO_SAMPLE_RATE, S16 );

    signal( SIGINT, stop_processing );

    init_alsa_audio_output( "pulse", AUDIO_SAMPLE_RATE, 1, SND_PCM_FORMAT_S16_LE );



    switch( sdr_type ) {
        case RTLSDR:
            if( init_rtlsdr(SDR_INDEX, SDR_SAMPLE_RATE, SDR_GAIN) ) {
                return -1;
            }
            rtlsdr_set_center_freq( rtlsdr, radio_freq );

            printf( "[INFO] Opening RTL-SDR\n" );
            if( radio_freq <= 30000000 )
                printf( "[INFO] Receiving on %d MHz\n", radio_freq / 1000 );
            else
                printf( "[INFO] Receiving on %.3f MHz\n", (double)radio_freq / 1000000.0 );

            rtlsdr_read_async( rtlsdr, rtlsdr_on_samples_received, NULL, 0, 0 );
            break;

        case AIRSPYHF:
            downsampling_factor = 4;

            if( init_airspyhf(radio_freq, AIRSPYHF_SAMPLE_RATE, 0) ) {
                return -1;
            }

            printf( "[INFO] Opening AirspyHF\n" );
            if( radio_freq <= 30000000 )
                printf( "[INFO] Receiving on %d MHz\n", radio_freq / 1000 );
            else
                printf( "[INFO] Receiving on %.3f MHz\n", (double)radio_freq / 1000000.0 );

            airspyhf_start( airspyhf, airspyhf_on_samples_received, NULL );

            while( pthread_cond_wait(&airspyhf_running, &airspyhf_mutex) );
            pthread_mutex_unlock( &airspyhf_mutex );
    }

    return 0;
}
