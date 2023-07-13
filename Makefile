
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
-Itests \

C_FLAGS = -Wall

LIBS = -lpthread

all: test_app

test: $(BUILD_DIR)
	$(CC) $(C_FLAGS) $(C_SOURCES) $(C_INCLUDES) $(LIBS) -o $(BUILD_DIR)/tests
	./$(BUILD_DIR)/tests

$(BUILD_DIR):
	-mkdir $@

clean:
	-@$(RM) $(BUILD_DIR)
