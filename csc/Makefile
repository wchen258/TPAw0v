CFLAGS = -Iinclude -Wall -lpthread -no-pie
SRC_FILES := $(wildcard src/*.c)
O_FILES := $(patsubst src/%.c,src/%.o,$(SRC_FILES))
MAIN_FILES := start_mp start_etr hello_ETM start_sram start_etm_pmu start_cnt_pmu_event

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