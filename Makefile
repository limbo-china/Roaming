BIN = ./bin/roam
CC = gcc
CFLAGS = -Wall -g
INC = -I ./base -I ./src
SRC =  $(wildcard base/*.c) $(wildcard src/*.c)
LIB = -lpthread -lm
#OBJ =

all: $(BIN)

$(BIN):$(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(INC) $(LIB)

.PHONY:clean
clean:
	rm -rf $(BIN)
