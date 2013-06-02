CC=g++

SRC=recogBMP.c
TARGET=recbmp

INCLUDEDIR=-I./fann
LDFLAG=-L./fann
LIB=-lfloatfann
LIB+=-lm

$(TARGET):$(SRC)
	$(CC) $(INCLUDEDIR) $(SRC) -o $(TARGET) $(LDFLAG) $(LIB)
clean:
	rm $(TARGET)
