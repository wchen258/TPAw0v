SRC_FILES := $(shell find src/*.c)

all: ctrace

ctrace: $(SRC_FILES)
	$(CC) -o ctrace $(SRC_FILES) -I headers/ -Wall -g

clean:
	rm -f ctrace
