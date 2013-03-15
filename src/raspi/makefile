SHELL = /bin/sh
CC = gcc
CFLAGS = -c -Wall -fPIC
DEFINES = __RASPI__

TARGET  = libPIC18FTools.so
OBJ = PICTools.c
OUT = PICTools.o
LIBS = wiringPi

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $< -o $(OUT) -D $(DEFINES) -l$(LIBS)
	$(CC) -shared -o $(TARGET) $(OUT)
	rm ./$(OUT)
	mv  -f ./$(TARGET) ../../lib/$(TARGET)
	
	