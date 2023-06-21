CC	= gcc
CPP	= g++
FLAGS	= -Wall -O0 -pg -g -rdynamic
INCLUDES = -I$(METISX_API_PATH)src -Isrc 
LIBS = -lm -lz -lpthread -Wl,-R$(METISX_API_PATH)lib -Wl,--no-as-needed -L$(METISX_API_PATH)lib -lmetisxAPI -lpython3.8

ifeq ($(shell uname -s),Linux)
	LIBS += -lrt
endif

all: echo_test
echo_test : echo_test.o
	$(CPP)  echo_test.o -o echo_test $(LIBS)

echo_test.o: echo_test.cpp
	$(CPP) $(INCLUDES) -c echo_test.cpp -o echo_test.o $(FLAGS)

clean:
	rm -f *.o echo_test
