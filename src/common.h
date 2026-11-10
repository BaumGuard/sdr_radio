#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <rtl-sdr.h>
#include <libairspyhf/airspyhf.h>
#include <alsa/asoundlib.h>

#include "wavwriter.h"

#define true 1
#define false 0

#define SQRT_2  1.414213562
#define STR_EQUAL( str1, str2 )     strcmp( str1, str2 ) == 0

// CONSTANTS
#define TWOPI 6.283185307



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
#define AUDIO_BUFFER_SIZE 1024



// SETTINGS
extern uint32_t radio_freq;
extern bool record_audio;
extern char audio_filepath[256];
extern int modulation;
extern int sdr_type;

extern int downsampling_factor;


enum Modulations {
    FM,
    AM
};

enum SDRs {
    NOSDR,
    RTLSDR,
    AIRSPYHF
};



// BUFFERS
extern double
    rtlsdr_recv_i_buf[NUM_INPUT_BUFFERS][INPUT_BUFFER_SIZE],
    rtlsdr_recv_q_buf[NUM_INPUT_BUFFERS][INPUT_BUFFER_SIZE];

extern double
    airspyhf_recv_i_buf[NUM_INPUT_BUFFERS][AIRSPYHF_INPUT_BUFFER_SIZE],
    airspyhf_recv_q_buf[NUM_INPUT_BUFFERS][AIRSPYHF_INPUT_BUFFER_SIZE];

extern double dsp_buffer[INPUT_BUFFER_SIZE];

extern uint32_t recv_buf_idx;


// SDR
extern rtlsdr_dev_t* rtlsdr;
extern airspyhf_device_t* airspyhf;

// AUDIO
extern snd_pcm_t* playback_handle;
extern wav_file_t* wavfile;
extern FILE* audio_file;




extern wav_file_t* filter_wav_file;

#endif
