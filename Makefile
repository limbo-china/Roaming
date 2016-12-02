BIN = ./bin/roam
CC = gcc
CFLAGS = -Wall -g
INC = -I ./base -I ./src -I ./rabbit
SRC =  $(wildcard base/*.c) $(wildcard src/*.c) $(wildcard rabbit/*.c)
LIB = -lpthread -lm ./rabbit/librabbitmq.a
#OBJ =

all: $(BIN)

$(BIN):$(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(INC) $(LIB)

.PHONY:clean
clean:
	rm -rf $(BIN)
