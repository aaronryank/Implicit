OBJ    := simplestack.o execute.o
CC     := gcc
CFLAGS := -O3 -g -Wall
EXE    := simplestack

.PHONY: clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXE) -lm

clean:
	rm -f $(OBJ)