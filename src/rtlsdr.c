#include "rtlsdr.h"

#include "demod.h"
#include "filters.h"
#include "audio.h"
#include "common.h"
#include "colorprint.h"

#include <rtl-sdr.h>



int init_rtlsdr( uint32_t sdr_index, uint32_t freq, uint32_t sample_rate, int gain ) {
    wavwriter_open( &filter_wav_file, "filter.wav", 1, 240000, F64 );
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



void rtlsdr_on_samples_received( unsigned char* buf, uint32_t len, void* ctx ) {
    uint32_t idx = 0;
    uint32_t current_buf = recv_buf_idx % NUM_INPUT_BUFFERS;

    for( uint32_t n = 0; n < len; n += 2 ) {
        recv_i_buf[current_buf][idx] = (double)( (int)buf[n] - 127 ) / 128.0;
        recv_q_buf[current_buf][idx] = (double)( (int)buf[n + 1] - 127 ) / 128.0;
        idx++;
    }

    recv_buf_idx++;

    switch( modulation ) {
        case FM:
            fm_demod(
                recv_i_buf[current_buf],
                recv_q_buf[current_buf],
                buffer,
                INPUT_BUFFER_SIZE
            );

            lpf_17kHz( buffer, INPUT_BUFFER_SIZE );
            //bpf( buffer, INPUT_BUFFER_SIZE );
            break;

        case AM:
            am_demod(
                recv_i_buf[current_buf],
                recv_q_buf[current_buf],
                buffer,
                INPUT_BUFFER_SIZE
            );

            lpf_5kHz( buffer, INPUT_BUFFER_SIZE );
            break;
    }

    wavwriter_write_samples( filter_wav_file, buffer, INPUT_BUFFER_SIZE );
    output_audio_chunk( buffer, INPUT_BUFFER_SIZE, 1.0 );
}
