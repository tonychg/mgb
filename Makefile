SRC_SUBDIR = src
BUILD_DIR = build

SRC = \
      $(SRC_SUBDIR)/sm83/debugger.c \
      $(SRC_SUBDIR)/sm83/decoder.c \
      $(SRC_SUBDIR)/sm83/sm83.c \
      $(SRC_SUBDIR)/sm83/sm83_isa.c \
      $(SRC_SUBDIR)/cpu.c \
      $(SRC_SUBDIR)/opcodes.c \
      $(SRC_SUBDIR)/op_helpers.c \
      $(SRC_SUBDIR)/decoder.c \
      $(SRC_SUBDIR)/list.c \
      $(SRC_SUBDIR)/fs.c \
      $(SRC_SUBDIR)/alloc.c \
      $(SRC_SUBDIR)/cartridge.c \
      $(SRC_SUBDIR)/memory.c \
      $(SRC_SUBDIR)/cli.c \
      $(SRC_SUBDIR)/gb.c \
      $(SRC_SUBDIR)/tests.c \
      $(SRC_SUBDIR)/video.c \
      $(SRC_SUBDIR)/render.c \
      $(SRC_SUBDIR)/timer.c \
      $(SRC_SUBDIR)/main.c \

INCLUDE	    = -Iinclude
NAME        = $(BUILD_DIR)/gb
TEST	    = $(BUILD_DIR)/test.bin
CC          = gcc
CFLAGS	    = -Wall
OBJ         = $(SRC:.c=.o)
RM          = rm -f
MKDIR	    = mkdir -p
LIB         = -lraylib -lcjson -lpthread
MAKE        = make

.PHONY:all
all:    $(NAME)
	$(MAKE) -C src/sm83
	$(MAKE) -C tests
test: CFLAGS = -Wall -g -DTEST

build:
	$(MKDIR) $(BUILD_DIR)

$(OBJ): %.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $^ -o $@

$(OBJ_TEST):
	$(CC) $(INCLUDE) $(CFLAGS_TEST) -c $^ -o $@

$(NAME):    build $(OBJ)
	$(CC) $(INCLUDE) $(LIB) -o $(NAME) $(OBJ)

test:	clean build $(OBJ)
	$(CC) $(INCLUDE) $(LIB) $(CFLAGS) $(OBJ) -o $(NAME)
	@$(NAME)

.PHONY:clean
clean:
	$(RM) $(OBJ)
	$(MAKE) -C src/sm83 clean
	$(MAKE) -C tests clean

re: clean all
