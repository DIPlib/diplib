CC = g++ -std=c++11
#CFLAGS = -Wall -O3
CFLAGS = -Wall -pedantic -g -Wno-sign-compare

MATLABDIR = /Applications/MATLAB_R2016a.app

.SUFFIXES:
.SUFFIXES: .cpp .o

INC = include/

OBJ = $(addprefix obj/,image.o image_manip.o image_indexing.o datatypes.o error.o sort.o numeric.o)

all: tests

# src/library:
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

# src/support:
obj/sort.o : src/support/sort.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC)
obj/numeric.o : src/support/numeric.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC)

# tests:
.PHONY: tests
tests: testout/test_image testout/test_options testout/mextest.mexmaci64

# cmdline tests:
testout/test_image: test/test_image.cpp $(OBJ)
	$(CC) $(CFLAGS) -o $@ -I$(INC) $^
testout/test_options: test/test_options.cpp $(OBJ)
	$(CC) $(CFLAGS) -o $@ -I$(INC) $^

# mex tests:
testout/mextest.mexmaci64: test/mextest.cpp $(OBJ)
	$(CC) -bundle $(CFLAGS) -DMATLAB_MEX_FILE -o $@ -I$(INC) $^ \
	-I$(MATLABDIR)/extern/include/ -L$(MATLABDIR)/bin/maci64 -lmx -lmex

# docs:
.PHONY: docs
docs:
	doxygen

# thoughts:
.PHONY: thoughts
thoughts: DIPthoughts.html
DIPthoughts.pdf : DIPthoughts.md
	pandoc -t latex -o $@ $<
DIPthoughts.html : DIPthoughts.md
	pandoc -t html -o $@ $<

# clean:
.PHONY: clean
clean:
	-\rm -f $(OBJ) testout/test_image testout/test_options testout/mextest.mexmaci64
	-\rm -rf doc/html/*
