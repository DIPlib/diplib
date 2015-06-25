CC = g++ -std=c++11
#CFLAGS = -Wall -O3
CFLAGS = -Wall -g

.SUFFIXES:
.SUFFIXES: .cpp .o

INC = include/

OBJ = $(addprefix obj/,image.o sort.o datatypes.o error.o)

obj/%.o : src/%.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC)

all: test.cpp $(OBJ)
	$(CC) $(CFLAGS) -o test -I$(INC) $^

.PHONY: clean
clean:
	rm $(OBJ) test
