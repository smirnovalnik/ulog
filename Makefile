
CC ?= gcc
RM = rm -fR
ifeq ($(OS),Windows_NT)
  RM = del /q
endif

BUILD_DIR ?= build

C_SOURCES = \
src/ulog.c \
tests/tests.c \

C_INCLUDES = \
-Isrc \
-Itemplates \
-Itests \

C_FLAGS = -std=c99  -Wall -Wpedantic -Wextra -fpic

LIBS = -lpthread

all: test

test: $(BUILD_DIR)
	$(CC) $(C_FLAGS) $(C_SOURCES) $(C_INCLUDES) $(LIBS) -o $(BUILD_DIR)/tests
	./$(BUILD_DIR)/tests

bench: $(BUILD_DIR)
	$(CC) $(C_FLAGS) -O2 src/ulog.c tests/bench.c $(C_INCLUDES) $(LIBS) -o $(BUILD_DIR)/bench
	./$(BUILD_DIR)/bench

$(BUILD_DIR):
	-mkdir $@

clean:
	-@$(RM) $(BUILD_DIR)
