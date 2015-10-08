CC = g++ -std=c++11
#CFLAGS = -Wall -O3
CFLAGS = -Wall -pedantic -g

.SUFFIXES:
.SUFFIXES: .cpp .o

INC = include/

OBJ = $(addprefix obj/,image.o pixel.o datatypes.o error.o sort.o)

all: test mextest.mexmaci64

obj/image.o : src/library/image.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC)
obj/pixel.o : src/library/pixel.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC)
obj/datatypes.o : src/library/datatypes.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC)
obj/error.o : src/library/error.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC)
obj/sort.o : src/support/sort.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC)

test: test.cpp $(OBJ)
	$(CC) $(CFLAGS) -o test -I$(INC) $^

mextest.mexmaci64: mextest.cpp $(OBJ)
	$(CC) -bundle $(CFLAGS) -DMATLAB_MEX_FILE -o mextest.mexmaci64 -I$(INC) $^ \
	-I/Applications/MATLAB_R2014b.app/extern/include/ \
	-L/Applications/MATLAB_R2014b.app/bin/maci64 -lmx -lmex

.PHONY: docs
docs:
	doxygen

.PHONY: clean
clean:
	-\rm $(OBJ) test mextest.mexmaci64
	-\rm -r doc/html/*

.PHONY: thoughts
thoughts: DIPthoughts.html
DIPthoughts.pdf : DIPthoughts.md
	pandoc -t latex -o $@ $<
DIPthoughts.html : DIPthoughts.md
	pandoc -t html -o $@ $<
