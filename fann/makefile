
CC=gcc
SRC=floatfann.c
OBJ=floatfann.o
LIB_OBJECT=libfloatfann.so

.PHONY:all  clean

all:$(LIB_OBJECT)

$(LIB_OBJECT):$(OBJ)
	$(CC) $(OBJ) -fPIC -shared -o $(LIB_OBJECT)

$(OBJ):$(SRC)
	$(CC) -fPIC -c $(SRC)

clean:
	rm *.o *.so
