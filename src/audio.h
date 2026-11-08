#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

void init_alsa_audio_output( char* interface, uint32_t sample_rate, int channels, int sample_type );

void output_audio_chunk( double* buf, int buf_size, double amplify_factor );

#endif
