BUILDDIR = ../build
SRC = \
	  test_sm83.c \
	  ../src/sm83/debugger.c \
	  ../src/sm83/decoder.c \
	  ../src/sm83/sm83.c \
	  ../src/sm83/sm83_isa.c \
	  ../src/alloc.c \
	  ../src/list.c \
	  ../src/fs.c \

OBJ = $(SRC:.c=.o)
INCLUDE = -I../include
CC = gcc
CFLAGS = -g -Wall -DSM83_TEST
RM = rm -f
MKDIR = -p
NAME = $(BUILDDIR)/sm83-test
LIB = -lcjson

.PHONY:all
all: $(NAME)

$(OBJ): %.o: %.c
	$(CC) $(INCLUDE) $(LIB) $(CFLAGS) -c $^ -o $@

$(NAME): $(OBJ)
	$(CC) $(INCLUDE) $(LIB) $(CFLAGS) $(OBJ) -o $(NAME)

.PHONY:clean
clean:
	$(RM) $(OBJ)

re: clean all
