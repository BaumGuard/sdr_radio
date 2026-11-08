LDFLAGS=-lm -lrtlsdr -lairspyhf -lasound
CCFLAGS=-Wall

SRC=src
BUILD=build

all: fm_radio

$(BUILD)/airspyhf.o: $(SRC)/airspyhf.h $(SRC)/airspyhf.c
	gcc -c $(SRC)/airspyhf.c -o $(BUILD)/airspyhf.o $(CCFLAGS)

$(BUILD)/audio.o: $(SRC)/audio.h $(SRC)/audio.c
	gcc -c $(SRC)/audio.c -o $(BUILD)/audio.o $(CCFLAGS)

$(BUILD)/common.o: $(SRC)/common.h $(SRC)/common.c
	gcc -c $(SRC)/common.c -o $(BUILD)/common.o $(CCFLAGS)

$(BUILD)/demod.o: $(SRC)/demod.h $(SRC)/demod.c
	gcc -c $(SRC)/demod.c -o $(BUILD)/demod.o $(CCFLAGS)

$(BUILD)/filters.o: $(SRC)/filters.h $(SRC)/filters.c
	gcc -c $(SRC)/filters.c -o $(BUILD)/filters.o $(CCFLAGS)

$(BUILD)/rtlsdr.o: $(SRC)/rtlsdr.h $(SRC)/rtlsdr.c
	gcc -c $(SRC)/rtlsdr.c -o $(BUILD)/rtlsdr.o $(CCFLAGS)

$(BUILD)/utils.o: $(SRC)/utils.h $(SRC)/utils.c
	gcc -c $(SRC)/utils.c -o $(BUILD)/utils.o $(CCFLAGS)

$(BUILD)/main.o: $(SRC)/main.c
	gcc -c $(SRC)/main.c -o $(BUILD)/main.o $(CCFLAGS)

$(BUILD)/wavwriter.o: src/wavwriter/wavwriter.h src/wavwriter/wavwriter.c
	gcc -c $(SRC)/wavwriter/wavwriter.c -o $(BUILD)/wavwriter.o


fm_radio: $(BUILD)/airspyhf.o $(BUILD)/audio.o $(BUILD)/common.o $(BUILD)/demod.o $(BUILD)/filters.o $(BUILD)/rtlsdr.o $(BUILD)/utils.o $(BUILD)/wavwriter.o $(BUILD)/main.o
	gcc $(BUILD)/*.o -o fm_radio $(LDFLAGS) $(CCFLAGS)


clean:
	rm -f $(BUILD)/*.o fm_radio
