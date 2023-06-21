CC	= gcc
CPP	= g++
FLAGS	= -Wall -O0 -pg -g -rdynamic
INCLUDES = -I$(METISX_API_PATH)src -Isrc 
LIBS = -lm -lz -lpthread -Wl,-R$(METISX_API_PATH)lib -Wl,--no-as-needed -L$(METISX_API_PATH)lib -lmetisxAPI -lpython3.8

ifeq ($(shell uname -s),Linux)
	LIBS += -lrt
endif

all: sort_test
sort_test : sort_test.o
	$(CPP)  sort_test.o -o sort_test $(LIBS)

sort_test.o: sort_test.cpp
	$(CPP) $(INCLUDES) -c sort_test.cpp -o sort_test.o $(FLAGS)

clean:
	rm -f *.o sort_test
