CC = gcc
CFLAGS = -Iinclude -Iio -Iclient_core -Iserver_core -Wall
LDFLAGS = -lm

SRCS_SERVER = src/GS.c
SRCS_CLIENT = src/player.c client_core/client_core.c client_core/aux_client.c
SRCS_GENERAL = include/utils.c server_core/game_core.c

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
