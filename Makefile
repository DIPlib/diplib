CC = g++ -std=c++11
#CFLAGS = -Wall -O3
CFLAGS = -Wall -pedantic -g -Wno-sign-compare

.SUFFIXES:
.SUFFIXES: .cpp .o

INC = include/

OBJ = $(addprefix obj/,image.o image_manip.o image_indexing.o datatypes.o error.o sort.o)

all: test mextest.mexmaci64

obj/image.o : src/library/image.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC)
obj/image_manip.o : src/library/image_manip.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC)
obj/image_indexing.o : src/library/image_indexing.cpp
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
