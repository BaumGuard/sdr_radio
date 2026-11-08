#include "utils.h"

#include "common.h"

#include <string.h>


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
