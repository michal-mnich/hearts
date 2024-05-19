CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++20 -lboost_program_options
CXXFLAGSDEBUG = -DDEBUG -g -Og -Wall -Wextra -Wpedantic -std=c++20 -fsanitize=address -lboost_program_options
DEBUG = 1

ifeq ($(DEBUG), 1)
    CXXFLAGS := $(CXXFLAGSDEBUG)
endif

.PHONY: all clean

all: kierki-klient kierki-serwer

kierki-klient: kierki-klient.o arg_parser.o
	$(CXX) $(CXXFLAGS) -o $@ $^

kierki-serwer: kierki-serwer.o arg_parser.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# generated with g++ -MM *.cpp
arg_parser.o: arg_parser.cpp arg_parser.hpp
kierki-klient.o: kierki-klient.cpp arg_parser.hpp
kierki-serwer.o: kierki-serwer.cpp arg_parser.hpp

clean:
	rm -f kierki-klient kierki-serwer *.o
