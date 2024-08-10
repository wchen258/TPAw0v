CFLAGS = -Iinclude -Wall -lpthread
SRC_FILES := $(wildcard src/*.c)
O_FILES := $(patsubst src/%.c,src/%.o,$(SRC_FILES))

# Determine the compiler based on architecture
ifeq ($(shell uname -m),aarch64)
    CC=gcc
else
    CC=aarch64-linux-gnu-gcc
endif

# Target for the final executable
mp: $(O_FILES) main/start_mp.o
	$(CC) -DR5 $^ $(CFLAGS) -o start_mp 

# Generic rule for compiling .c to .o
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for compiling start_mp.c
main/start_mp.o: main/start_mp.c
	$(CC) $(CFLAGS) -DR5 -c $< -o $@

# Clean target
clean:
	rm -f $(O_FILES) main/start_mp.o
	rm -f start_mp
