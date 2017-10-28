CC = g++
PWD = $(shell pwd)

TARGET = ftpserver
CFLAGS = -I$(PWD) -I$(PWD)/FtpService -std=c++11 -g -Wall
SRC := $(shell find . -name "*.cpp")
LIBS = 
LOG := log

$(TARGET) : $(SRC) 
	$(CC) $(CFLAGS) $^ -o $@ 2>$(LOG) $(LIBS)
	@echo "Build success..."

.PHONY : clean
clean:
	rm -rf $(TARGET) $(LOG)
	
