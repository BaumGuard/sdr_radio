#ifndef RTLSDR_H
#define RTLSDR_H

#include <stdint.h>

int init_rtlsdr( uint32_t sdr_index, uint32_t sample_rate, int gain );

void rtlsdr_on_samples_received( unsigned char* buf, uint32_t len, void* ctx );

#endif
