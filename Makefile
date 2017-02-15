ARCHIVE := $(shell uname -m)
BIT32 := i686
BIT64 := x86_64
FPIC=
ifeq ($(ARCHIVE),$(BIT64))
        FPIC=-fPIC
endif

ifndef BUILD_DIR
BUILD_DIR := ./bin
endif
NULL := $(shell mkdir -pv $(BUILD_DIR))

CC = g++
CFLAGS = -Wall -O -g -I/usr/local/include/  
LDFLAGS = -static -L. -lstdc++

SERV := $(BUILD_DIR)/Serv
CLI  := $(BUILD_DIR)/Cli

SERVER_DIR = ./server
CLIENT_DIR = ./client

CLIENT_SRC=$(wildcard $(CLIENT_DIR)/*.c $(CLIENT_DIR)/*.cpp)
CLIENT_OBJS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o, $(wildcard $(CLIENT_SRC))))

SERVER_SRC=$(wildcard $(SERVER_DIR)/*.c $(SERVER_DIR)/*.cpp)
SERVER_OBJS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o, $(wildcard $(SERVER_SRC))))

all: $(SERV) $(CLI) 


$(SERV):$(SERVER_OBJS)
	g++ -o $@ $^ $(LDFLAGS)

$(CLI):$(CLIENT_OBJS)
	g++ -o $@ $^ $(LDFLAGS)

%.o: %.c	
	g++ $(CFLAGiS) -c $< -o $@ $(INCLUDES)	

.PHONY: clean

clean:
	rm -rf $(SERVER_DIR)/*.o  $(CLIENt_DIR)/*.o $(BUILD_DIR)

