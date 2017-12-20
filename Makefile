OBJ    := src/main.o src/execute.o src/commands.o src/itoa.o
CC     := gcc
CFLAGS := -O3 -g -Wall
EXE    := simplestack

.PHONY: clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXE) -lm

clean:
	rm -f $(OBJ)
