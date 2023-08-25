CC := gcc
CFLAGS := -Wall -Wextra -g
ccflags-y := -std=gnu11
MAIN_SRC := main.c src/queue.c src/helpers.c src/consumer.c src/producer.c src/sig_handler.c
MAIN_BIN := main

.PHONY: all clean

all: main

main:
	$(CC) $(CFLAGS) $(MAIN_SRC) -o $(MAIN_BIN) -lpthread -lrt

clean:
	rm -f $(MAIN_BIN)

