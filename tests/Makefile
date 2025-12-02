DESTINATION = ..
PROGRAM = test
CFLAGS = -Wall -g
LIB = -lcriterion
SRC = \
	  $(DESTINATION)/devices/ppu.c \
	  $(DESTINATION)/devices/joypad.c \
	  test.c

include $(DESTINATION)/Makefile.common

test: all
	$(BUILD_DIR)/test
