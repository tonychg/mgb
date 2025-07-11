BUILDDIR = ../../build
SRC = \
	  debugger.c \
	  debugger_cli.c \
	  decoder.c \
	  sm83.c \
	  sm83_isa.c \
	  ../alloc.c \
	  ../fs.c \

OBJ = $(SRC:.c=.o)
INCLUDE = -I../../include
CC = gcc
CFLAGS = -g -Wall -DSM83_DEBUGGER
RM = rm -f
MKDIR = -p
NAME = $(BUILDDIR)/sm83-debugger

.PHONY:all
all: $(NAME)

$(OBJ): %.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $^ -o $@

$(NAME): $(OBJ)
	$(CC) $(INCLUDE) $(CFLAGS) $(OBJ) -o $(NAME)

.PHONY:clean
clean:
	$(RM) $(OBJ)

re: clean all
