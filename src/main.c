#include <rtl-sdr.h>
#include <stdio.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include <signal.h>

/*-----------------------------------------------------------------*/

// SDR/RADIO SETTINGS
#define RADIO_FREQUENCY 90700000
#define SDR_INDEX 0
#define SDR_SAMPLE_RATE 240000
#define SDR_GAIN 450

// AUDIO OUTPUT SETTINGS
#define AUDIO_SAMPLE_RATE 48000


// PROGRAM SETTINGS
#define NUM_INPUT_BUFFERS 8
#define INPUT_BUFFER_SIZE 131072
#define AUDIO_BUFFER_SIZE 1024

// CONSTANTS
#define TWOPI 6.283185307


/*-----------------------------------------------------------------*/

// BUFFERS

double
    recv_i_buf[NUM_INPUT_BUFFERS][INPUT_BUFFER_SIZE],
    recv_q_buf[NUM_INPUT_BUFFERS][INPUT_BUFFER_SIZE];
double buffer[INPUT_BUFFER_SIZE];

uint32_t recv_buf_idx  = 0;

//  AUDIO OUTPUT
rtlsdr_dev_t* sdr;
snd_pcm_t* playback_handle;
FILE* audio_file;



void fm_demod( double* i_buf, double* q_buf, double* demod_buf ) {
    static double
        prev_phase = 0.0, current_phase = 0.0, phase_diff = 0.0;

    for( uint32_t i = 0; i < INPUT_BUFFER_SIZE; i++ ) {
        current_phase = atan2( q_buf[i], i_buf[i] );
        phase_diff = current_phase - prev_phase;
        prev_phase = current_phase;

        if( phase_diff > M_PI ) {
            phase_diff -= TWOPI;
        }
        else if ( phase_diff < -M_PI ) {
            phase_diff += TWOPI;
        }

        phase_diff /= M_PI;
        demod_buf[i] = phase_diff;
    }
}

void low_pass_filter( double* buf ) {
    static double
        x0 = 0.0, x1 = 0.0, x2 = 0.0,
        y0 = 0.0, y1 = 0.0, y2 = 0.0;

    for( uint32_t i = 0; i < INPUT_BUFFER_SIZE; i++ ) {
        x2 = x1;
        x1 = x0;
        x0 = buf[i];

        y2 = y1;
        y1 = y0;
        y0 = 0.0373403 * x0 + 0.0746806 * x1 + 0.0373403 * x2;
        y0 -= -1.38389 * y1 + 0.533252 * y2;

        buf[i] = y0;
    }
}

int16_t audio_chunk[AUDIO_BUFFER_SIZE];
uint32_t audio_idx = 0;
uint32_t audio_buf_idx = 0;
void output_audio_chunk( double* buf ) {
    for( uint32_t i = 0; i < INPUT_BUFFER_SIZE; i++ ) {
        if( audio_idx % 5 == 0 ) {
            audio_chunk[audio_buf_idx++] = (int16_t)( buf[i] * INT16_MAX );
        }
        audio_idx++;

        if( audio_buf_idx >= AUDIO_BUFFER_SIZE ) {
            snd_pcm_writei( playback_handle, audio_chunk, AUDIO_BUFFER_SIZE );
            audio_buf_idx = 0;
        }
    }
}



void on_samples_received( unsigned char* buf, uint32_t len, void* ctx ) {
    uint32_t idx = 0;
    uint32_t current_buf = recv_buf_idx % NUM_INPUT_BUFFERS;

    for( uint32_t n = 0; n < len; n += 2 ) {
        recv_i_buf[current_buf][idx] = (double)( (int)buf[n] - 127 ) / 128.0;
        recv_q_buf[current_buf][idx] = (double)( (int)buf[n + 1] - 127 ) / 128.0;
        idx++;
    }

    recv_buf_idx++;

    fm_demod( recv_i_buf[current_buf], recv_q_buf[current_buf], buffer );
    low_pass_filter( buffer );
    output_audio_chunk( buffer );
}


void init_alsa_audio_output( char* interface, uint32_t sample_rate, int channels, int sample_type ) {
    snd_pcm_hw_params_t* hw_params;

    snd_pcm_open( &playback_handle, interface, SND_PCM_STREAM_PLAYBACK, 0 );
    snd_pcm_hw_params_malloc( &hw_params );
    snd_pcm_hw_params_any( playback_handle, hw_params );
    snd_pcm_hw_params_set_format(playback_handle, hw_params, sample_type );
    snd_pcm_hw_params_set_rate_near( playback_handle, hw_params, &sample_rate, 0 );
    snd_pcm_hw_params_set_channels( playback_handle, hw_params, channels );
    snd_pcm_hw_params( playback_handle, hw_params );
    snd_pcm_hw_params_free( hw_params );
    snd_pcm_prepare( playback_handle );
}

int init_rtlsdr( uint32_t sdr_index, uint32_t sample_rate, int gain ) {
    int status = rtlsdr_open( &sdr, sdr_index );
    if( status < 0 ) {
        fprintf( stderr, "[ERROR] No SDR found\n" );
        return -1;
    }

    status = rtlsdr_set_sample_rate( sdr, sample_rate );
    if( status ) {
        fprintf( stderr, "[ERROR] Could not set sample rate\n" );
        return -1;
    }

    status = rtlsdr_set_tuner_gain( sdr, gain );
    if ( status ) {
        fprintf( stderr, "[ERROR] Gain could not be set\n" );
        return -1;
    }

    status = rtlsdr_set_center_freq( sdr, RADIO_FREQUENCY );
    if( status ) {
        fprintf( stderr, "[ERROR] Center frequency could not be set\n" );
        return -1;
    }

    status = rtlsdr_reset_buffer( sdr );
    if( status ) {
        fprintf( stderr, "[ERROR] Resetting buffer\n" );
        return -1;
    }

    return 0;
}


void stop_processing( int sig ) {
    printf( "[INFO] Stop signal received - Stopping the reception\n" );
    rtlsdr_cancel_async( sdr );
    //rtlsdr_close( sdr );
    snd_pcm_close( playback_handle );
}


int main() {
    //audio_file = fopen("out.raw", "wb");
    signal( SIGINT, stop_processing );

    init_rtlsdr( SDR_INDEX, SDR_SAMPLE_RATE, SDR_GAIN );
    init_alsa_audio_output( "pulse", AUDIO_SAMPLE_RATE, 1, SND_PCM_FORMAT_S16_LE );

    rtlsdr_read_async( sdr, on_samples_received, NULL, 0, 0 );

    return 0;
}
