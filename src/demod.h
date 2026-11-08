#ifndef DEMOD_H
#define DEMOD_H

void fm_demod( double* i_buf, double* q_buf, double* demod_buf, int buf_size );

void am_demod( double* i_buf, double* q_buf, double* demod_buf, int buf_size );

#endif
