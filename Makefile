SRC_SUBDIR = src
BUILD_DIR = build

SRC = \
      $(SRC_SUBDIR)/list.c \
      $(SRC_SUBDIR)/fs.c \
      $(SRC_SUBDIR)/alloc.c \
      $(SRC_SUBDIR)/cartridge.c \
      $(SRC_SUBDIR)/memory.c \
      $(SRC_SUBDIR)/cpu.c \
      $(SRC_SUBDIR)/opcodes.c \
      $(SRC_SUBDIR)/op_helpers.c \
      $(SRC_SUBDIR)/cli.c \
      $(SRC_SUBDIR)/gb.c \
      $(SRC_SUBDIR)/decoder.c \
      $(SRC_SUBDIR)/tests.c \
      $(SRC_SUBDIR)/video.c \
      $(SRC_SUBDIR)/render.c \
      $(SRC_SUBDIR)/thread.c \
      $(SRC_SUBDIR)/timer.c \
      $(SRC_SUBDIR)/main.c \

NAME        = $(BUILD_DIR)/gb
TEST	    = $(BUILD_DIR)/test.bin
DEBUG       = $(BUILD_DIR)/debug.bin
CC          = gcc
CFLAGS	    = -g -Wall
OBJ         = $(SRC:.c=.o)
RM          = rm -f
MKDIR	    = mkdir -p
LIB         = -lraylib -lcjson -lpthread

.PHONY:all
all:    clean $(NAME)
test: CFLAGS = -Wall -g -DTEST

build:
	$(MKDIR) $(BUILD_DIR)

$(OBJ): %.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(OBJ_TEST):
	$(CC) $(CFLAGS_TEST) -c $^ -o $@

$(NAME):    build $(OBJ)
	$(CC) $(LIB) -o $(NAME) $(OBJ)

debug:  clean build $(OBJ)
	$(CC) $(LIB) $(CFLAGS) $(OBJ) -o $(DEBUG)

test:	clean build $(OBJ)
	$(CC) $(LIB) $(CFLAGS) $(OBJ) -o $(TEST)
	./$(TEST)

.PHONY:clean
clean:
	$(RM) $(OBJ)

re: clean test all
