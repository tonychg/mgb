DESTINATION = ..
PROGRAM = test
CFLAGS = -Wall -g
LIB = -lcriterion
SRC = \
	  $(DESTINATION)/mgb/memory.c \
	  $(DESTINATION)/mgb/video.c \
	  $(DESTINATION)/mgb/joypad.c \
	  test.c

include $(DESTINATION)/Makefile.common

test: all
	$(BUILD_DIR)/test
