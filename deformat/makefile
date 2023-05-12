CC=gcc

all: deformat.o
	$(CC) -o deformat deformat.o

sep: deformat
	../c_code/ctrace trc_0.out > trc_0.hum
	../c_code/ctrace trc_1.out > trc_1.hum
	../c_code/ctrace trc_2.out > trc_2.hum
	../c_code/ctrace trc_3.out > trc_3.hum

clean:
	rm deformat deformat.o
	rm trc_*.dat trc_*.out trc_*.hum