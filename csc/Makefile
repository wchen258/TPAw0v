# CFLAGS = -Iinclude -Wall -lpthread -no-pie
# SRC_FILES := $(wildcard src/*.c)
# O_FILES := $(patsubst src/%.c,src/%.o,$(SRC_FILES))
# MAIN_FILES := start_mp start_etr hello_ETM start_sram start_etm_pmu start_cnt_pmu_event

# # Determine the compiler based on architecture
# ifeq ($(shell uname -m),aarch64)
#     CC = gcc
# else
#     CC = aarch64-linux-gnu-gcc
# endif

# all: $(MAIN_FILES)

# # Target for the final executables
# $(MAIN_FILES): % : $(O_FILES) main/%.o
# 	$(CC) $^ $(CFLAGS) -o $@

# # Generic rule for compiling .c to .o in src/ and main/
# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# # Clean target
# clean:
# 	rm -f $(O_FILES) $(MAIN_FILES:=.o) $(MAIN_FILES)

CFLAGS = -Iinclude -Wall
LDFLAGS = -lpthread -no-pie
SRC_FILES := $(wildcard src/*.c)
O_FILES := $(patsubst src/%.c,src/%.o,$(SRC_FILES))
MAIN_FILES := start_mp start_etr hello_ETM start_sram start_etm_pmu start_cnt_pmu_event

# Determine the compiler based on architecture
ifeq ($(shell uname -m),aarch64)
    CC = gcc
	OBJDUMP = objdump
else
    CC = aarch64-linux-gnu-gcc
	OBJDUMP = aarch64-linux-gnu-objdump
endif

all: $(MAIN_FILES) hello_ETM.dump

# Target for the final executables
$(MAIN_FILES): % : main/%.o $(O_FILES)
	$(CC) $^ $(LDFLAGS) -o $@

# Rule for compiling .c to .o in src/
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for compiling .c to .o in main/
main/%.o: main/%.c
	$(CC) $(CFLAGS) -c $< -o $@

hello_ETM.dump: hello_ETM
	$(OBJDUMP) -d hello_ETM > hello_ETM.dump

# Clean target
clean:
	rm -f $(O_FILES) main/*.o $(MAIN_FILES) hello_ETM.dump
