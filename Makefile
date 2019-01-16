CC = g++
PWD = $(shell pwd)

TARGET = ftpserver
CFLAGS = -std=c++11 -g -Wall
INCLUDES = -I./src -I./include
SRCS := $(shell find . -name "*.cpp")
LIBS = -pthread 
LOG := log

$(TARGET) : $(SRCS) 
	$(CC) $(CFLAGS) $^ $(LIBS) $(INCLUDES) -o $@ 2>$(LOG)
	@echo "Build success..."

.PHONY : clean
clean:
	rm -rf $(TARGET) $(LOG)
	
