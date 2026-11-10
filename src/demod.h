#ifndef DEMOD_H
#define DEMOD_H

/*
Perform FM demodulation on IQ data coming from the SDR

Args:
 - i_buf        : Pointer to a buffer of I samples with values between -1 and 1
 - q_buf        : Pointer to a buffer of Q samples with values between -1 and 1
 - demod_buf    : Pointer to the buffer in which the samples of the demodulated
                  signal should be written
 - buf_size     : Sizes of the buffers i_buf, q_buf and demod_buf (must be equal)
*/
void fm_demod( double* i_buf, double* q_buf, double* demod_buf, int buf_size );


/*
Perform AM demodulation on IQ data coming from the SDR

Args:
 - i_buf        : Pointer to a buffer of I samples with values between -1 and 1
 - q_buf        : Pointer to a buffer of Q samples with values between -1 and 1
 - demod_buf    : Pointer to the buffer in which the samples of the demodulated
                  signal should be written
 - buf_size     : Sizes of the buffers i_buf, q_buf and demod_buf (must be equal)
*/
void am_demod( double* i_buf, double* q_buf, double* demod_buf, int buf_size );

#endif
