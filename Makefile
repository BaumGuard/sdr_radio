LDFLAGS=-lm -lrtlsdr -lasound
CCFLAGS=-Wall

fm_radio: src/main.c
	gcc src/main.c -o fm_radio $(LDFLAGS) $(CCFLAGS)
