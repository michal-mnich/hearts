CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++20
CXXFLAGSDEBUG = -DDEBUG -g -Og -Wall -Wextra -Wpedantic -std=c++20 -fsanitize=address
DEBUG = 1

ifeq ($(DEBUG), 1)
    CXXFLAGS := $(CXXFLAGSDEBUG)
endif

.PHONY: all clean

all: kierki-klient kierki-serwer

kierki-klient: kierki-klient.o arg_parser.o client.o network_client.o network_common.o error.o protocol_client.o common.o game.o
	$(CXX) $(CXXFLAGS) -lboost_program_options -o $@ $^

kierki-serwer: kierki-serwer.o arg_parser.o server.o network_server.o network_common.o error.o protocol_server.o common.o game.o
	$(CXX) $(CXXFLAGS) -lboost_program_options -o $@ $^

# generated with g++ -MM *.cpp
arg_parser.o: arg_parser.cpp arg_parser.hpp error.hpp
client.o: client.cpp client.hpp arg_parser.hpp network_client.hpp \
 protocol_client.hpp common.hpp error.hpp
common.o: common.cpp common.hpp
error.o: error.cpp error.hpp
game.o: game.cpp game.hpp
kierki-klient.o: kierki-klient.cpp arg_parser.hpp client.hpp \
 network_client.hpp protocol_client.hpp error.hpp network_common.hpp
kierki-serwer.o: kierki-serwer.cpp arg_parser.hpp error.hpp server.hpp \
 game.hpp network_server.hpp protocol_server.hpp common.hpp
network_client.o: network_client.cpp network_client.hpp error.hpp \
 network_common.hpp
network_common.o: network_common.cpp network_common.hpp error.hpp
network_server.o: network_server.cpp network_server.hpp error.hpp \
 network_common.hpp server.hpp arg_parser.hpp game.hpp \
 protocol_server.hpp common.hpp
protocol_client.o: protocol_client.cpp protocol_client.hpp \
 network_client.hpp common.hpp error.hpp network_common.hpp
protocol_server.o: protocol_server.cpp protocol_server.hpp \
 network_server.hpp common.hpp error.hpp network_common.hpp
rules.o: rules.cpp
server.o: server.cpp server.hpp arg_parser.hpp game.hpp \
 network_server.hpp protocol_server.hpp common.hpp error.hpp \
 network_common.hpp

clean:
	rm -f kierki-klient kierki-serwer *.o
