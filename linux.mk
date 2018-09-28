#
# File Name: makefile
# Author: Seree Rakwong
# Date: 28-NOV-2017
#

TARGET = tui_test_linux
CC     = gcc
CFLAGS = -g -Wall
LFLAGS = -lm -lcurses

.PHONY: default all clean

default: $(TARGET)
all: default

BIN_DIR  = ./bin
SRC_DIR  = ./src
INC_DIR  = ./include
SOURCES  = $(SRC_DIR)/tui.c \
           $(SRC_DIR)/tdc.c \
           $(SRC_DIR)/tmsgbx.c \
           $(SRC_DIR)/tstc.c \
           $(SRC_DIR)/tedt.c \
           $(SRC_DIR)/tbtn.c \
           $(SRC_DIR)/tlb.c \
           $(SRC_DIR)/tlctl.c \
           $(SRC_DIR)/tui_test.c


OBJECTS  = $(SOURCES:.c=.o)
INCLUDES = -I$(INC_DIR)
DEFINES  = -D__UNIX__ -D__LINUX__ -D__USE_CURSES__


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^

clean:
	rm -f src/*.o $(TARGET)* tui_test_*


