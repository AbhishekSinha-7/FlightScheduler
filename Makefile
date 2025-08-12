# Makefile for Windows using MinGW
CC = g++
CFLAGS = -Wall -O2
LDFLAGS = 
TARGET = flight_simulation.exe
SOURCES = flight_simulation.cpp
OBJECTS = flight_simulation.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES)

clean:
	del $(TARGET) $(OBJECTS)

.PHONY: all clean