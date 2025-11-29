ROOT        = ..
BUILD_DIR   = $(ROOT)/build
INCLUDE	    = -I$(ROOT)/include
NAME        = debugger
CC          = gcc
CFLAGS	    = -Wall -g
RM          = rm -f
MKDIR	    = mkdir -p
MAKE        = make
OUTPUT      = $(BUILD_DIR)/$(NAME)
SRC         = \
              $(ROOT)/devices/sm83/sm83.c \
              $(ROOT)/devices/sm83/decoder.c \
              $(ROOT)/devices/sm83/interrupt.c \
              $(ROOT)/devices/sm83/isa.c \
              $(ROOT)/devices/memory.c \
              $(ROOT)/devices/timer.c \
              $(ROOT)/emu/platform/mm.c \
              $(ROOT)/emu/platform/io.c \
              $(ROOT)/debugger/debugger.c \
              $(ROOT)/debugger/debugger_main.c \

OBJ         = $(SRC:.c=.o)

.PHONY:all
all:    $(NAME)

build:
	$(MKDIR) $(BUILD_DIR)

$(OBJ): %.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $^ -o $@

$(NAME):    build $(OBJ)
	$(CC) $(INCLUDE) $(LIB) -o $(OUTPUT) $(OBJ)

.PHONY:clean
clean:
	$(RM) $(OBJ)

re: clean all

