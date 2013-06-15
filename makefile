CC=g++

SRC=recogBMP.c gif.c
FEATURE_SRC=featureExtract.c

TARGET=recdigit
#FEATURETARGET=fExtract
INCLUDEDIR=-I./fann
#LDFLAG=-L./fann
#LIB=-lfloatfann
LIB+=-lm

#all:$(TARGET) $(FEATURETARGET)
all:$(TARGET)
$(TARGET):$(SRC)
	$(CC) $(INCLUDEDIR) $(SRC) -o $(TARGET) $(LDFLAG) $(LIB)
#$(FEATURETARGET):$(FEATURE_SRC)
#	$(CC) $(INCLUDEDIR) $(FEATURE_SRC) -o $(FEATURETARGET) $(LDFLAG) $(LIB)
clean:
	rm $(TARGET) $(FEATURETARGET)
