CC := gcc
CFLAGS := -Wall -Wextra -Wno-unused-parameter -O2
LDFLAGS := -pthread

.PHONY: all server client init clean

all: server client

server: server.c structures.h functions/*.h
	$(CC) $(CFLAGS) -o server server.c $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LDFLAGS)

init: functions/setAdmin.c structures.h
	$(CC) $(CFLAGS) -o setAdmin functions/setAdmin.c $(LDFLAGS)
	./setAdmin
	@echo "Data files initialized."

clean:
	rm -f server client setAdmin
