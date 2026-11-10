#include "dsp.h"

#include "sdr.h"
#include "demod.h"
#include "filters.h"
#include "audio.h"
#include "common.h"
#include "wavwriter.h"

#include <pthread.h>
#include <stdio.h>


void rtlsdr_on_samples_received( unsigned char* buf, uint32_t len, void* ctx ) {
    uint32_t idx = 0;
    uint32_t current_buf = recv_buf_idx % NUM_INPUT_BUFFERS;

    for( uint32_t n = 0; n < len; n += 2 ) {
        rtlsdr_recv_i_buf[current_buf][idx] = (double)( (int)buf[n] - 127 ) / 128.0;
        rtlsdr_recv_q_buf[current_buf][idx] = (double)( (int)buf[n + 1] - 127 ) / 128.0;
        idx++;
    }

    recv_buf_idx++;

    switch( modulation ) {
        case FM:
            fm_demod(
                rtlsdr_recv_i_buf[current_buf],
                rtlsdr_recv_q_buf[current_buf],
                dsp_buffer,
                INPUT_BUFFER_SIZE
            );

            lpf_17kHz( dsp_buffer, dsp_buffer, INPUT_BUFFER_SIZE );
            break;

        case AM:
            am_demod(
                rtlsdr_recv_i_buf[current_buf],
                rtlsdr_recv_q_buf[current_buf],
                dsp_buffer,
                INPUT_BUFFER_SIZE
            );

            lpf_5kHz( dsp_buffer, INPUT_BUFFER_SIZE );
            break;
    }

    //wavwriter_write_samples( filter_wav_file, diff_band_buf, INPUT_BUFFER_SIZE );
    output_audio_chunk( dsp_buffer, INPUT_BUFFER_SIZE, 1.0 );
}


int airspyhf_on_samples_received( airspyhf_transfer_t* transfer_fn ) {
    uint32_t current_buf = recv_buf_idx % NUM_INPUT_BUFFERS;

    for( int i = 0; i < transfer_fn->sample_count; i++ ) {
        airspyhf_recv_i_buf[current_buf][i] = (double) transfer_fn->samples[i].re;
        airspyhf_recv_q_buf[current_buf][i] = (double) transfer_fn->samples[i].im;
    }

    switch( modulation ) {
        case FM:
            fm_demod(
                airspyhf_recv_i_buf[current_buf],
                airspyhf_recv_q_buf[current_buf],
                dsp_buffer,
                AIRSPYHF_INPUT_BUFFER_SIZE
            );

            lpf_17kHz( dsp_buffer, dsp_buffer, AIRSPYHF_INPUT_BUFFER_SIZE );
            output_audio_chunk( dsp_buffer, AIRSPYHF_INPUT_BUFFER_SIZE, 1.0 );
            break;

        case AM:
            am_demod(
                airspyhf_recv_i_buf[current_buf],
                airspyhf_recv_q_buf[current_buf],
                dsp_buffer,
                AIRSPYHF_INPUT_BUFFER_SIZE
            );

            lpf_5kHz( dsp_buffer, AIRSPYHF_INPUT_BUFFER_SIZE );
            output_audio_chunk( dsp_buffer, AIRSPYHF_INPUT_BUFFER_SIZE, 128.0 );
            break;
    }



    recv_buf_idx++;

    return 0;
}



void stop_processing( int sig ) {
    printf( "[INFO] Stop signal received - Stopping the reception\n" );

    if( record_audio )
        wavwriter_close( wavfile );

    switch( sdr_type ) {
        case RTLSDR:
            rtlsdr_cancel_async( rtlsdr );
            //rtlsdr_close( rtlsdr );
            break;

        case AIRSPYHF:
            airspyhf_stop( airspyhf );
            airspyhf_close( airspyhf );
            pthread_cond_signal( &airspyhf_running );
            break;
    }

    snd_pcm_close( playback_handle );
    //wavwriter_close( filter_wav_file );
}
