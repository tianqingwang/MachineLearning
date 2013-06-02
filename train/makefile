export LD_LIBRARY_PATH=./fann
INCLUDEDIR=-I../fann/
LDFLAG=-L../fann/

LIB=-lfloatfann
LIB+=-lm

SRC=train_nn.c
OBJ=train_nn

CC=gcc

$(OBJ):$(SRC)
	$(CC) $(INCLUDEDIR) $(SRC) -o $(OBJ) $(LDFLAG) $(LIB) 