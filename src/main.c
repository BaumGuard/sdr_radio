#include <rtl-sdr.h>
#include <libairspyhf/airspyhf.h>
#include <stdio.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include <signal.h>
#include <string.h>

#include "wavwriter/wavwriter.h"

#define SQRT_2  1.414213562
#define STR_EQUAL( str1, str2 )     strcmp( str1, str2 ) == 0

/*-----------------------------------------------------------------*/

// SDR/RADIO SETTINGS
#define RADIO_FREQUENCY 100700000
#define SDR_INDEX 0
#define SDR_SAMPLE_RATE 240000
#define AIRSPYHF_SAMPLE_RATE 192000
#define SDR_GAIN 450

// AUDIO OUTPUT SETTINGS
#define AUDIO_SAMPLE_RATE 48000


// PROGRAM SETTINGS
#define NUM_INPUT_BUFFERS 8
#define INPUT_BUFFER_SIZE 131072
#define AIRSPYHF_INPUT_BUFFER_SIZE 2048
#define AUDIO_BUFFER_SIZE 2048

// CONSTANTS
#define TWOPI 6.283185307


enum Modulations {
    FM,
    AM
};

enum SDRs {
    RTLSDR,
    AIRSPYHF
};


uint32_t radio_freq = RADIO_FREQUENCY;
bool record_audio = false;
char audio_filepath[256];
int modulation = FM;
int sdr_type = AIRSPYHF;

int downsampling_factor = 5;

/*-----------------------------------------------------------------*/

// BUFFERS

double
    recv_i_buf[NUM_INPUT_BUFFERS][INPUT_BUFFER_SIZE],
    recv_q_buf[NUM_INPUT_BUFFERS][INPUT_BUFFER_SIZE];

double
    airspyhf_recv_i_buf[NUM_INPUT_BUFFERS][AIRSPYHF_INPUT_BUFFER_SIZE],
    airspyhf_recv_q_buf[NUM_INPUT_BUFFERS][AIRSPYHF_INPUT_BUFFER_SIZE];

double buffer[INPUT_BUFFER_SIZE];

uint32_t recv_buf_idx  = 0;

//  AUDIO OUTPUT
rtlsdr_dev_t* sdr;
airspyhf_device_t* airspyhf;
snd_pcm_t* playback_handle;
wav_file_t* wavfile;
FILE* audio_file;

void fm_demod( double* i_buf, double* q_buf, double* demod_buf, int buf_size ) {
    static double
        prev_phase = 0.0, current_phase = 0.0, phase_diff = 0.0;

    for( uint32_t i = 0; i < buf_size; i++ ) {
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

void am_demod( double* i_buf, double* q_buf, double* demod_buf, int buf_size ) {
    for( int i = 0; i < buf_size; i++ ) {
        demod_buf[i] = sqrt( i_buf[i]*i_buf[i] + q_buf[i]*q_buf[i] ) / SQRT_2;
    }
}

void lpf_17kHz( double* buf, int buf_size ) {
    static double
        x0 = 0.0, x1 = 0.0, x2 = 0.0,
        y0 = 0.0, y1 = 0.0, y2 = 0.0;

    static double
        a1 = -1.38389, a2 = 0.533252,
        b0 = 0.0373403, b1 = 0.0746806, b2 = 0.0373403;

    for( uint32_t i = 0; i < buf_size; i++ ) {
        x2 = x1;
        x1 = x0;
        x0 = buf[i];

        y2 = y1;
        y1 = y0;
        y0 = b0*x0 + b1*x1 + b2*x2;
        y0 -= a1*y1 + a2*y2;

        buf[i] = y0;
    }
}


void lpf_5kHz( double* buf, int buf_size ) {
    static double
        x0 = 0.0, x1 = 0.0, x2 = 0.0,
        y0 = 0.0, y1 = 0.0, y2 = 0.0;

    static double
        a1 = -1.81534, a2 = 0.831006,
        b0 = 0.00391613, b1 = 0.00783225, b2 = 0.00391613;

    for( uint32_t i = 0; i < buf_size; i++ ) {
        x2 = x1;
        x1 = x0;
        x0 = buf[i];

        y2 = y1;
        y1 = y0;
        y0 = b0*x0 + b1*x1 + b2*x2;
        y0 -= a1*y1 + a2*y2;

        buf[i] = y0;
    }
}


int16_t audio_chunk[AUDIO_BUFFER_SIZE];
uint32_t audio_idx = 0;
uint32_t audio_buf_idx = 0;
void output_audio_chunk( double* buf, int buf_size, double amplify_factor ) {
    for( uint32_t i = 0; i < buf_size; i++ ) {
        if( audio_idx % downsampling_factor == 0 ) {
            audio_chunk[audio_buf_idx++] = (int16_t)( buf[i] * amplify_factor * INT16_MAX );
        }
        audio_idx++;

        if( audio_buf_idx >= AUDIO_BUFFER_SIZE ) {
            snd_pcm_writei( playback_handle, audio_chunk, AUDIO_BUFFER_SIZE );

            if( record_audio )
                wavwriter_write_samples( wavfile, audio_chunk, AUDIO_BUFFER_SIZE );
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

    switch( modulation ) {
        case FM:
            fm_demod( recv_i_buf[current_buf], recv_q_buf[current_buf], buffer, INPUT_BUFFER_SIZE );
            lpf_17kHz( buffer, INPUT_BUFFER_SIZE );
            break;

        case AM:
            am_demod( recv_i_buf[current_buf], recv_q_buf[current_buf], buffer, INPUT_BUFFER_SIZE );
            lpf_5kHz( buffer, INPUT_BUFFER_SIZE );
            break;
    }

    output_audio_chunk( buffer, INPUT_BUFFER_SIZE, 1.0 );
}


int on_airspyhf_samples_received( airspyhf_transfer_t* transfer_fn ) {
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
            output_audio_chunk( buffer, AIRSPYHF_INPUT_BUFFER_SIZE, 12.0 );
            break;
    }



    recv_buf_idx++;

    return 0;
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

    status = rtlsdr_reset_buffer( sdr );
    if( status ) {
        fprintf( stderr, "[ERROR] Resetting buffer\n" );
        return -1;
    }

    return 0;
}


int init_airspyhf( uint32_t freq, uint32_t sample_rate, int gain ) {
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

    return 0;
}


void stop_processing( int sig ) {
    printf( "[INFO] Stop signal received - Stopping the reception\n" );
    rtlsdr_cancel_async( sdr );

    if( record_audio )
        wavwriter_close( wavfile );

    if( sdr_type == AIRSPYHF ) {
        airspyhf_stop( airspyhf );
        airspyhf_close( airspyhf );
    }

    snd_pcm_close( playback_handle );
}


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


void parse_arguments( int argc, char* argv[] ) {
    for( int i = 1; i < argc; i++ ) {
        if( STR_EQUAL(argv[i], "-f") ) {
            int arglen = strlen( argv[i+1] );

            switch( argv[i+1][arglen-1] ) {
                case 'M':
                    argv[i+1][arglen-1] = '\0';
                    radio_freq = (int)( atof(argv[2]) * 1000000.0 );
                    break;

                case 'k':
                    argv[i+1][arglen-1] = '\0';
                    radio_freq = (int)( atof(argv[2]) * 1000.0 );
                    break;

                default:
                    radio_freq = atoi( argv[2] );
                    break;
            }
        }

        if( STR_EQUAL(argv[i], "-r") ) {
            record_audio = true;
            strncpy( audio_filepath, argv[i+1], 256 );
        }
    }
}


int main( int argc, char* argv[] ) {
    parse_arguments( argc, argv );

    if( radio_freq <= 30000000 ) {
        modulation = AM;
    }

    if( record_audio )
        wavwriter_open( &wavfile, audio_filepath, 1, AUDIO_SAMPLE_RATE, S16 );

    signal( SIGINT, stop_processing );

    init_alsa_audio_output( "pulse", AUDIO_SAMPLE_RATE, 1, SND_PCM_FORMAT_S16_LE );

    printf( "[INFO] Receiving on %.3f MHz\n", (double)radio_freq / 1000000.0 );

    switch( sdr_type ) {
        case RTLSDR:
            init_rtlsdr( SDR_INDEX, SDR_SAMPLE_RATE, SDR_GAIN );
            rtlsdr_set_center_freq( sdr, radio_freq );
            rtlsdr_read_async( sdr, on_samples_received, NULL, 0, 0 );
            break;

        case AIRSPYHF:
            downsampling_factor = 4;
            init_airspyhf( radio_freq, AIRSPYHF_SAMPLE_RATE, 0 );
            airspyhf_start( airspyhf, on_airspyhf_samples_received, NULL );
            getchar();
    }

    return 0;
}
