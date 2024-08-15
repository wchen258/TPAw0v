CC=gcc

all: deformat.o
	$(CC) -g -o deformat deformat.o

deformat.o: deformat.c
	$(CC) -g -c deformat.c

clean:
	rm deformat deformat.o
	rm trc_*.dat trc_*.out trc_*.hum

.PHONY: all clean
