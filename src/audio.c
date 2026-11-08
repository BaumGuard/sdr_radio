#include "audio.h"

#include "common.h"
#include "wavwriter/wavwriter.h"

#include <alsa/asoundlib.h>


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
