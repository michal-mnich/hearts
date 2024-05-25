CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++20 -lboost_program_options
CXXFLAGSDEBUG = -DDEBUG -g -Og -Wall -Wextra -Wpedantic -std=c++20 -fsanitize=address -lboost_program_options
DEBUG = 1

ifeq ($(DEBUG), 1)
    CXXFLAGS := $(CXXFLAGSDEBUG)
endif

.PHONY: all clean

all: kierki-klient kierki-serwer

kierki-klient: kierki-klient.o arg_parser.o client.o network_client.o network_common.o error.o protocol.o common.o
	$(CXX) $(CXXFLAGS) -o $@ $^

kierki-serwer: kierki-serwer.o arg_parser.o server.o network_server.o network_common.o error.o protocol.o common.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# generated with g++ -MM *.cpp
arg_parser.o: arg_parser.cpp arg_parser.hpp error.hpp
client.o: client.cpp client.hpp network_client.hpp protocol.hpp error.hpp
common.o: common.cpp common.hpp
error.o: error.cpp error.hpp
kierki-klient.o: kierki-klient.cpp arg_parser.hpp client.hpp \
 network_client.hpp protocol.hpp error.hpp network_common.hpp
kierki-serwer.o: kierki-serwer.cpp arg_parser.hpp error.hpp server.hpp \
 network_server.hpp protocol.hpp
network_client.o: network_client.cpp network_client.hpp error.hpp \
 network_common.hpp
network_common.o: network_common.cpp network_common.hpp error.hpp
network_server.o: network_server.cpp network_server.hpp error.hpp \
 network_common.hpp server.hpp protocol.hpp
protocol.o: protocol.cpp protocol.hpp error.hpp network_common.hpp
rules.o: rules.cpp
server.o: server.cpp server.hpp network_server.hpp protocol.hpp error.hpp \
 common.hpp

clean:
	rm -f kierki-klient kierki-serwer *.o
