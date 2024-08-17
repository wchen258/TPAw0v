# CFLAGS = -Iinclude -Wall -lpthread
# SRC_FILES := $(wildcard src/*.c)
# O_FILES := $(patsubst src/%.c,src/%.o,$(SRC_FILES))

# # Determine the compiler based on architecture
# ifeq ($(shell uname -m),aarch64)
#     CC=gcc
# else
#     CC=aarch64-linux-gnu-gcc
# endif

# all: mp hello etr

# # Target for the final executable
# mp: $(O_FILES) main/start_mp.o
# 	$(CC) $^ $(CFLAGS) -o start_mp 

# # Target for the final executable start_etr
# etr: $(O_FILES) main/start_etr.o
# 	$(CC) $^ $(CFLAGS) -o start_etr

# # Target for the final executable hello_ETM
# hello: $(O_FILES) main/hello_ETM.o
# 	$(CC) -no-pie $^ $(CFLAGS) -o hello_ETM

# # Generic rule for compiling .c to .o
# src/%.o: src/%.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# # Rule for compiling start_mp.c
# main/start_mp.o: main/start_mp.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# # Rule for compiling start_etr.c
# main/start_etr.o: main/start_etr.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# # Rule for compiling hello_ETM.c
# main/hello_ETM.o: main/hello_ETM.c
# 	$(CC) $(CFLAGS) -no-pie -c $< -o $@

# # Clean target
# clean:
# 	rm -f $(O_FILES) main/start_mp.o main/hello_ETM.o main/start_etr.o
# 	rm -f start_mp start_etr hello_ETM


CFLAGS = -Iinclude -Wall -lpthread -no-pie
SRC_FILES := $(wildcard src/*.c)
O_FILES := $(patsubst src/%.c,src/%.o,$(SRC_FILES))
MAIN_FILES := start_mp start_etr hello_ETM start_sram

# Determine the compiler based on architecture
ifeq ($(shell uname -m),aarch64)
    CC = gcc
else
    CC = aarch64-linux-gnu-gcc
endif

all: $(MAIN_FILES)

# Target for the final executables
$(MAIN_FILES): % : $(O_FILES) main/%.o
	$(CC) $^ $(CFLAGS) -o $@

# Generic rule for compiling .c to .o in src/ and main/
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(O_FILES) $(MAIN_FILES:=.o) $(MAIN_FILES)