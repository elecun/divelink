
# Makefile for Divelink Application
# Author : Byunghun Hwang <bh.hwang@iae.re.kr>
# Usage : make ARCH=x86_64

# Makefile

OS := $(shell uname)

#Set Architecutre
ARCH := x86_64

#Compilers
CC := g++
GCC := gcc
LD_LIBRARY_PATH += -L./lib/x86_64 -L/usr/lib/x86_64-linux-gnu
OUTDIR		= ./
BUILDDIR		= ../bin/
INCLUDE_DIR = -I./ -I../../include/ -I/usr/include
#LD_LIBRARY_PATH += -L/usr/local/lib -L./lib/x86_64 -L/usr/lib/x86_64-linux-gnu


# OS
ifeq ($(OS),Linux) #for Linux
	LDFLAGS = -Wl,--export-dynamic -Wl,-rpath=$(LD_LIBRARY_PATH)
	LDLIBS = -lrt -pthread -lboost_thread -lboost_system -lmosquitto
	GTEST_LDLIBS = -lgtest
endif

$(shell mkdir -p $(OUTDIR))
$(shell mkdir -p $(BUILDDIR))

#if release(-O3), debug(-O0)
CXXFLAGS = -O3 -fPIC -Wall -std=c++17 -D__cplusplus=201703L -DSINGLE_HEADER

#custom definitions
CXXFLAGS += -D__MAJOR__=0 -D__MINOR__=0 -D__REV__=1
RM	= rm -rf

#directories

INCLUDE_FILES = ../include/
SOURCE_FILES = ./
INSTALL_DIR = /usr/local/bin/

# Make
pubsub_shm:	$(BUILDDIR)pubsub.o
		$(CC) $(LDFLAGS) $(LD_LIBRARY_PATH) -o $(BUILDDIR)$@ $^ $(LDLIBS)

$(BUILDDIR)pubsub.o:	$(SOURCE_FILES)pubsub.cpp
					$(CC) $(CXXFLAGS) $(INCLUDE_DIR) -c $^ -o $@

all : pubsub_shm

deploy : FORCE
	cp $(BINDIR)
clean : FORCE 
		$(RM) $(BUILDDIR)*.o
FORCE : 