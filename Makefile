LDFLAGS=-lm -lrtlsdr -lairspyhf -lasound
CCFLAGS=-Wall

all: fm_radio

src/cfft/cfft.o: src/cfft/fft.h src/cfft/fft.c
	gcc -c -g src/cfft/fft.c -o src/cfft/fft.o

src/wavwriter/wavwriter.o: src/wavwriter/wavwriter.h src/wavwriter/wavwriter.c
	gcc -c -g src/wavwriter/wavwriter.c -o src/wavwriter/wavwriter.o

fm_radio: src/main.c src/wavwriter/wavwriter.o src/cfft/fft.o
	gcc -g src/main.c src/wavwriter/wavwriter.o src/cfft/fft.o -o fm_radio $(LDFLAGS) $(CCFLAGS)
