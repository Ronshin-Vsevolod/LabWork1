PROJECT = editorbmp

LIBPROJECT = $(PROJECT).a

CXX = g++

A = ar

AFLAGS = rsv

CCXFLAGS = -I. -std=c++17 -Werror -Wpedantic -Wall -g -fPIC

LDXXFLAGS = $(CCXFLAGS) -L. -l:$(LIBPROJECT)

DEPS=$(wildcard *.h)

OBJ= Main.o EditorBMP.o

.PHONY: default

default: all;

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CCXFLAGS)

$(LIBPROJECT): $(OBJ)
	$(A) $(AFLAGS) $@ $^

$(PROJECT): Main.o $(LIBPROJECT)
	$(CXX) -o $@ Main.o $(LDXXFLAGS)

all: $(PROJECT)

.PHONY: clean

clean:
	rm -f *.o

cleanall: clean
	rm -f $(PROJECT)
	rm -f $(LIBPROJECT)
