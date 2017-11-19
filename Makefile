OBJ    := simplestack.o execute.o itoa.o
CC     := gcc
CFLAGS := -O3 -g -Wall
EXE    := simplestack

.PHONY: clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXE) -lm

clean:
	rm -f $(OBJ)