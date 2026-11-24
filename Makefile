CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = src
BIN = bin

TARGETS = $(BIN)/mem_sensor \
          $(BIN)/cpu_sensor \
          $(BIN)/proc_sensor \
          $(BIN)/sysmond

all: $(TARGETS)

$(BIN)/mem_sensor: $(SRC)/mem_sensor.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN)/cpu_sensor: $(SRC)/cpu_sensor.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN)/proc_sensor: $(SRC)/proc_sensor.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN)/sysmond: $(SRC)/sysmond.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

.PHONY: all clean
