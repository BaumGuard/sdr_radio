#include <rtl-sdr.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

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

double demod_buf[NUM_INPUT_BUFFERS][INPUT_BUFFER_SIZE];

int16_t lpf_buf[AUDIO_BUFFER_SIZE];

uint32_t
    recv_buf_idx  = 0,
    demod_buf_idx = 0,
    lpf_buf_idx   = 0;


// THREADING

bool flag_receiver_running = false;
bool flag_audioplayback_running = false;

bool flag_samples_received = false;
bool flag_demodulated = false;

pthread_mutex_t buffer_mutex;
pthread_mutex_t demod_buffer_mutex;
pthread_mutex_t lpf_mutex;

pthread_cond_t cond_samples_received;
pthread_cond_t cond_demod;


//  AUDIO OUTPUT

snd_pcm_t* playback_handle;
FILE* audio_file;


void on_samples_received( unsigned char* buf, uint32_t len, void* ctx ) {
    pthread_mutex_lock( &buffer_mutex );
    uint32_t idx = 0;
    uint32_t current_buf = recv_buf_idx % NUM_INPUT_BUFFERS;

    for( uint32_t n = 0; n < len; n += 2 ) {
        recv_i_buf[current_buf][idx] = (double)( (int)buf[n] - 127 ) / 128.0;
        recv_q_buf[current_buf][idx] = (double)( (int)buf[n + 1] - 127 ) / 128.0;
        idx++;
    }

    recv_buf_idx++;

    pthread_mutex_unlock( &buffer_mutex );
    flag_samples_received = true;
    pthread_cond_signal( &cond_samples_received );
}


void* start_receiver( void* ptr ) {
    rtlsdr_dev_t* sdr = (rtlsdr_dev_t*) ptr;
    flag_receiver_running = true;

    printf( "[INFO] Receiving on %.1f MHz\n", (float)RADIO_FREQUENCY / 1000000.0 );
    rtlsdr_read_async( sdr, on_samples_received, NULL, 0, 0 );
}

void* fm_demod( void* ptr ) {
    double
        prev_phase = 0.0, current_phase = 0.0, phase_diff = 0.0;

    uint32_t current_buf = 0;

    printf( "[INFO] FM demodulator running\n" );

    double index_float = 0.0;
    uint32_t index_int = 0;
    double step = (double)SDR_SAMPLE_RATE / (double)AUDIO_SAMPLE_RATE;

    while( flag_receiver_running ) {

        pthread_mutex_lock( &buffer_mutex );
        pthread_mutex_lock( &demod_buffer_mutex );

        while( !flag_samples_received ) {
            pthread_cond_wait( &cond_samples_received, &buffer_mutex );
        }
        current_buf = demod_buf_idx % NUM_INPUT_BUFFERS;

        for( uint32_t i = 0; i < INPUT_BUFFER_SIZE; i++ ) {
            current_phase = atan2( recv_q_buf[current_buf][i], recv_i_buf[current_buf][i] );
            phase_diff = current_phase - prev_phase;
            prev_phase = current_phase;

            if( phase_diff > M_PI ) {
                phase_diff -= TWOPI;
            }
            else if ( phase_diff < -M_PI ) {
                phase_diff += TWOPI;
            }

            phase_diff /= M_PI;
            demod_buf[current_buf][i] = phase_diff;
        }

        demod_buf_idx++;

        flag_samples_received = false;
        flag_demodulated = true;
        pthread_mutex_unlock( &buffer_mutex );
        pthread_mutex_unlock( &demod_buffer_mutex );

        pthread_cond_signal( &cond_demod );
    }
}

void* low_pass_filter( void* ptr ) {
    double
        x0 = 0.0, x1 = 0.0, x2 = 0.0,
        y0 = 0.0, y1 = 0.0, y2 = 0.0;

    uint32_t demod_current_buf = 0;

    uint32_t idx = 0;

    while( flag_receiver_running ) {
        demod_current_buf = demod_buf_idx % NUM_INPUT_BUFFERS;

        pthread_mutex_lock( &lpf_mutex );
        while( !flag_demodulated ) {
            pthread_cond_wait( &cond_demod, &lpf_mutex );
        }
        //printf( "[DEBUG] RECEIVE=%d DEMOD=%d\n", recv_buf_idx, demod_buf_idx );

        for( uint32_t i = 0; i < INPUT_BUFFER_SIZE; i++ ) {
            x2 = x1;
            x1 = x0;
            x0 = demod_buf[demod_current_buf][i];

            y2 = y1;
            y1 = y0;
            y0 = 0.0373403 * x0 + 0.0746806 * x1 + 0.0373403 * x2;
            y0 -= -1.38389 * y1 + 0.533252 * y2;

            if( idx % 5 == 0 ) {
                if ( lpf_buf_idx >= AUDIO_BUFFER_SIZE ) {
                    lpf_buf_idx = 0;
                    snd_pcm_writei( playback_handle, lpf_buf, AUDIO_BUFFER_SIZE );
                    //fwrite( lpf_buf, AUDIO_BUFFER_SIZE, 2, audio_file );
                }
                lpf_buf[lpf_buf_idx++] = (int16_t)(y0 * INT16_MAX);
            }

            idx++;

        }

        flag_demodulated = false;
        pthread_mutex_unlock( &lpf_mutex );
    }
}


void audio_output() {
    snd_pcm_hw_params_t* hw_params;

    snd_pcm_open( &playback_handle, "pulse", SND_PCM_STREAM_PLAYBACK, 0 );
    snd_pcm_hw_params_malloc( &hw_params );
    snd_pcm_hw_params_any( playback_handle, hw_params );
    //snd_pcm_hw_params_set_access( playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED );
    snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    uint32_t rate = 48000;
    snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, 0);
    snd_pcm_hw_params_set_channels( playback_handle, hw_params, 1 );
    snd_pcm_hw_params( playback_handle, hw_params );
    snd_pcm_hw_params_free( hw_params );
    snd_pcm_prepare( playback_handle );

    flag_audioplayback_running = true;
}


int main() {
    uint32_t sdr_index = SDR_INDEX;

    rtlsdr_dev_t* sdr;
    int status = rtlsdr_open( &sdr, sdr_index );
    if( status < 0 ) {
        fprintf( stderr, "[ERROR] No SDR found\n" );
        return -1;
    }

    status = rtlsdr_set_sample_rate( sdr, SDR_SAMPLE_RATE );
    if( status ) {
        fprintf( stderr, "[ERROR] Could not set sample rate\n" );
        return -1;
    }

    status = rtlsdr_set_tuner_gain( sdr, SDR_GAIN );
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

    audio_file = fopen("out.raw", "wb");

    pthread_mutex_init( &buffer_mutex, NULL );
    pthread_cond_init( &cond_samples_received, NULL );
    pthread_mutex_init( &demod_buffer_mutex, NULL );
    pthread_cond_init( &cond_demod, NULL );
    pthread_mutex_init( &lpf_mutex, NULL );

    audio_output();
    pthread_t receiver_thread, demod_thread, lpf_thread;
    pthread_create( &receiver_thread, NULL, start_receiver, (void*)sdr );
    pthread_create( &demod_thread, NULL, fm_demod, NULL );
    pthread_create( &lpf_thread, NULL, low_pass_filter, NULL );

    pthread_join( receiver_thread, NULL );
    pthread_join( demod_thread, NULL );
    pthread_join( lpf_thread, NULL );


    rtlsdr_close( sdr );
    return 0;
}
