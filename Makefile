CC = gcc
CFLAGS = -Iinclude -Iio -Wall
LDFLAGS = -lm

SRCS_SERVER = src/GS.c
SRCS_CLIENT = src/player.c
SRCS_GENERAL = include/utils.c io/udp_io.c

EXEC_SERVER = server.o
EXEC_CLIENT = player.o

all: $(EXEC_SERVER) $(EXEC_CLIENT)

$(EXEC_SERVER): $(SRCS_SERVER)
	$(CC) $(CFLAGS) $(SRCS_SERVER) $(SRCS_GENERAL) -o $(EXEC_SERVER) $(LDFLAGS)

$(EXEC_CLIENT): $(SRCS_CLIENT)
	$(CC) $(CFLAGS) $(SRCS_CLIENT) $(SRCS_GENERAL) -o $(EXEC_CLIENT) $(LDFLAGS)

clean:
	rm -f $(EXEC_SERVER) $(EXEC_CLIENT)

.PHONY: all clean
