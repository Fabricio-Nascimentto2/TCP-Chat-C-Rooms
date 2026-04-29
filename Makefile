CC = gcc
CFLAGS = -Wall -pthread -Iserver/include -Icommon/include

SRC = \
	client/client.c \
	server/src/main.c \
	server/src/broadcast.c \
	server/src/handler.c \
	server/src/commands.c \
	server/common/util/utils.c

OBJ = $(SRC:.c=.o)

IP = 192.168.1.7
PORT = 8996

server: server/logs $(OBJ)
	$(CC) $(CFLAGS) -o server/server $(OBJ)

server/logs:
	mkdir -p server/logs

run: server
	./server/server $(IP) $(PORT)

clean:
	rm -f $(OBJ) server/server
