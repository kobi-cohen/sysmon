CC = gcc
CFLAGS = -Wall -Wextra -g -Iinc

SRC = src
BIN = bin

TARGETS = $(BIN)/mem_sensor \
          $(BIN)/cpu_sensor \
          $(BIN)/proc_sensor \
          $(BIN)/sysmond

all: $(TARGETS)

$(BIN)/mem_sensor: $(SRC)/mem_sensor.c inc/sysmon_ipc.h
	$(CC) $(CFLAGS) $< -o $@

$(BIN)/cpu_sensor: $(SRC)/cpu_sensor.c inc/sysmon_ipc.h
	$(CC) $(CFLAGS) $< -o $@

$(BIN)/proc_sensor: $(SRC)/proc_sensor.c inc/sysmon_ipc.h
	$(CC) $(CFLAGS) $< -o $@

$(BIN)/sysmond: $(SRC)/sysmond.c inc/sysmon_ipc.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

.PHONY: all clean
