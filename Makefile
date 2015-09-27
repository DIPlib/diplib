CC = g++ -std=c++11
#CFLAGS = -Wall -O3
CFLAGS = -Wall -pedantic -g

.SUFFIXES:
.SUFFIXES: .cpp .o

INC = include/

OBJ = $(addprefix obj/,image.o sort.o datatypes.o error.o)

obj/%.o : src/%.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC)

all: test mextest.mexmaci64

test: test.cpp $(OBJ)
	$(CC) $(CFLAGS) -o test -I$(INC) $^

mextest.mexmaci64: mextest.cpp $(OBJ)
	$(CC) -bundle $(CFLAGS) -DMATLAB_MEX_FILE -o mextest.mexmaci64 -I$(INC) $^ \
	-I/Applications/MATLAB_R2014b.app/extern/include/ \
	-L/Applications/MATLAB_R2014b.app/bin/maci64 -lmx -lmex

.PHONY: clean
clean:
	rm $(OBJ) test
