#ifndef UTILS_H
#define UTILS_H

/*
Parse commandline arguments and set the variables in the section
SETTINGS in common.h accordingly

Args:
 - argc : Number of commandline arguments
 - argv : Array of the commandline arguments as strings
          (the first one is at index 1)


Supported arguments:
 -f <freq>     Receiving frequency
                - In Hertz (a positive integer) : 100700000
                - In Kilohertz (a positive integer with a k at the end) : 5865k
                - In Megahertz (a positive integer or float with an M at the end) : 100.7M

-r <path>      Record the audio as a WAV file (e.g. -r audio.wav)
*/
void parse_arguments( int argc, char* argv[] );

#endif
