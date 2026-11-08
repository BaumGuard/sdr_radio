#include "common.h"

uint32_t radio_freq = RADIO_FREQUENCY;
bool record_audio = false;
char audio_filepath[256];
int modulation = FM;
int sdr_type = AIRSPYHF;

int downsampling_factor = 5;




// BUFFERS

double
    recv_i_buf[NUM_INPUT_BUFFERS][INPUT_BUFFER_SIZE],
    recv_q_buf[NUM_INPUT_BUFFERS][INPUT_BUFFER_SIZE];

double
    airspyhf_recv_i_buf[NUM_INPUT_BUFFERS][AIRSPYHF_INPUT_BUFFER_SIZE],
    airspyhf_recv_q_buf[NUM_INPUT_BUFFERS][AIRSPYHF_INPUT_BUFFER_SIZE];

double buffer[INPUT_BUFFER_SIZE];

uint32_t recv_buf_idx  = 0;

// SDR
rtlsdr_dev_t* rtlsdr;
airspyhf_device_t* airspyhf;

// AUDIO
snd_pcm_t* playback_handle;
wav_file_t* wavfile;
FILE* audio_file;
