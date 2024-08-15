CFLAGS = -Iinclude -Wall
SRC_FILES := $(shell find src/*.c)
O_FILES := $(SRC_FIELSi:.c=.o)
CC=g++

all: r5

debug:
	gcc -DDEBUG $(CFLAG) $(SRC_FILES) main/start.cpp -o start

r5: $(O_FILES)
	$(CC) -DR5 $(O_FILES) $(CFLAGS) main/start.cpp -o start
	$(CC) -DR5 $(O_FILES) $(CFLAGS) main/start_mp.cpp -o start_mp

mp: $(SRC_FILES) main/start_mp.cpp
	$(CC) -DR5 $(SRC_FILES) $(CFLAGS) main/start_mp.cpp -o start_mp 

mig:
	g++ $(CFLAG) $(SRC_FILES) main/start.cpp -o start

bench:
	g++ -Iinclude $(SRC_FILES) main/bench.cpp -Wall -o bench

cc:
	g++ $(CFLAG) -no-pie $(SRC_FILES) main/pmcc.c -o pmcc
	objdump -d pmcc > pmcc.dp



clean:
	rm -rf start
	rm -rf bench
	rm -rf pmcc
	rm -rf start_mp
