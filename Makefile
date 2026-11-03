LDFLAGS=-lm -lrtlsdr -lasound

fm_radio: src/main.c
	gcc main.c -o fm_radio $(LDFLAGS)
