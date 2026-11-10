LDFLAGS=-lm -lrtlsdr -lairspyhf -lasound
CCFLAGS=-Wall

SRC=src
BUILD=build

all: fm_radio

$(BUILD)/sdr.o: $(SRC)/sdr.h $(SRC)/sdr.c
	gcc -c $(SRC)/sdr.c -o $(BUILD)/sdr.o $(CCFLAGS)

$(BUILD)/dsp.o: $(SRC)/dsp.h $(SRC)/dsp.c
	gcc -c $(SRC)/dsp.c -o $(BUILD)/dsp.o $(CCFLAGS)

$(BUILD)/audio.o: $(SRC)/audio.h $(SRC)/audio.c
	gcc -c $(SRC)/audio.c -o $(BUILD)/audio.o $(CCFLAGS)

$(BUILD)/common.o: $(SRC)/common.h $(SRC)/common.c
	gcc -c $(SRC)/common.c -o $(BUILD)/common.o $(CCFLAGS)

$(BUILD)/demod.o: $(SRC)/demod.h $(SRC)/demod.c
	gcc -c $(SRC)/demod.c -o $(BUILD)/demod.o $(CCFLAGS)

$(BUILD)/filters.o: $(SRC)/filters.h $(SRC)/filters.c
	gcc -c $(SRC)/filters.c -o $(BUILD)/filters.o $(CCFLAGS)

$(BUILD)/utils.o: $(SRC)/utils.h $(SRC)/utils.c
	gcc -c $(SRC)/utils.c -o $(BUILD)/utils.o $(CCFLAGS)

$(BUILD)/main.o: $(SRC)/main.c
	gcc -c $(SRC)/main.c -o $(BUILD)/main.o $(CCFLAGS)

$(BUILD)/wavwriter.o: src/wavwriter.h src/wavwriter.c
	gcc -c $(SRC)/wavwriter.c -o $(BUILD)/wavwriter.o


fm_radio: $(BUILD)/sdr.o $(BUILD)/dsp.o $(BUILD)/audio.o $(BUILD)/common.o $(BUILD)/demod.o $(BUILD)/filters.o $(BUILD)/utils.o $(BUILD)/wavwriter.o $(BUILD)/main.o
	gcc $(BUILD)/*.o -o fm_radio $(LDFLAGS) $(CCFLAGS)


clean:
	rm -f $(BUILD)/*.o fm_radio
