#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

/*
Initialize the ALSA audio output interface

Args:
 - interface    : Name of the ALSA audio interface (e.g. pulse)
 - sample_rate  : Sampling frequency of the audio output (preferably 48000 Hz)
 - channels     : Number of channels (1 -> Mono, 2 -> Stereo)
 - sample_type  : Datatype of the samples (Defined in /usr/include/alsa/asoundlib.h)
                  Preferably SND_PCM_FORMAT_S16_LE
*/
void init_alsa_audio_output(
    char* interface, uint32_t sample_rate, int channels, int sample_type
);


/*
Output a chunk of audio samples over the ALSA interface

Args
 - buf            : Pointer to a buffer of samples as double values between -1 and 1
 - buf_size       : Size of the buffer (should not be larger than 1024)
 - amplify_factor : Factor by which each sample should be multiplied in order to
                    amplify or attenuate the volume
*/
void output_audio_chunk( double* buf, int buf_size, double amplify_factor );

#endif
