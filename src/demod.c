#include "demod.h"

#include <math.h>
#include <stdint.h>

#include "common.h"


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
