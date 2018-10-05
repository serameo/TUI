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
SOURCES  = $(SRC_DIR)/m_tui.c \
           $(SRC_DIR)/m_tdc.c \
           $(SRC_DIR)/m_tmsgbx.c \
           $(SRC_DIR)/m_tstc.c \
           $(SRC_DIR)/m_tedt.c \
           $(SRC_DIR)/m_tbtn.c \
           $(SRC_DIR)/m_tlb.c \
           $(SRC_DIR)/m_tlctl.c \
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


