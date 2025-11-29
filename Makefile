ROOT        = .
BUILD_DIR   = $(ROOT)/build
INCLUDE	    = -I$(ROOT)/include
NAME        = gb
CC          = gcc
CFLAGS	    = -Wall -g
RM          = rm -f
MKDIR	    = mkdir -p
MAKE        = make
LIB         = -lraylib
OUTPUT      = $(BUILD_DIR)/$(NAME)
SRC         = \
              $(ROOT)/debugger/debugger.c \
              $(ROOT)/devices/sm83/sm83.c \
              $(ROOT)/devices/sm83/interrupt.c \
              $(ROOT)/devices/sm83/decoder.c \
              $(ROOT)/devices/sm83/isa.c \
              $(ROOT)/devices/timer.c \
              $(ROOT)/devices/memory.c \
              $(ROOT)/devices/ppu.c \
              $(ROOT)/devices/joypad.c \
              $(ROOT)/emu/platform/mm.c \
              $(ROOT)/emu/platform/io.c \
              $(ROOT)/emu/platform/render/raylib.c \
              $(ROOT)/emu/gb/emulator.c \
              $(ROOT)/emu/gb/main.c \

OBJ         = $(SRC:.c=.o)

.PHONY:all
all:    $(NAME)
		$(MAKE) -C $(ROOT)/debugger

build:
	$(MKDIR) $(BUILD_DIR)

$(OBJ): %.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $^ -o $@

$(NAME):    build $(OBJ)
	$(CC) $(INCLUDE) $(LIB) -o $(OUTPUT) $(OBJ)

.PHONY:clean
clean:
	$(RM) $(OBJ)
	$(MAKE) -C $(ROOT)/debugger clean

re: clean all
