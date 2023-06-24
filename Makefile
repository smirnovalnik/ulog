
CC ?= gcc
RM = rm -fR
ifeq ($(OS),Windows_NT)
  RM = del /q
endif

BUILD_DIR = build

C_SOURCES = \
src/ulog.c \
tests/tests.c

C_INCLUDES = \
-Isrc \
-Itests

C_FLAGS = -Wall

all: tests

tests:
	$(CC) $(C_FLAGS) $(C_SOURCES) $(C_INCLUDES) -o $(BUILD_DIR)/test

$(BUILD_DIR):
	-mkdir $@

clean:
	-@$(RM) $(BUILD_DIR)
