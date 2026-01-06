CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -Isrc
TARGET = test_allocator

SRC_DIR = src
TEST_DIR = tests

SRCS = $(SRC_DIR)/buddy_allocator.c $(SRC_DIR)/allocator_factory.c $(TEST_DIR)/test_basic.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

debug: CFLAGS += -DDEBUG -O0
debug: clean $(TARGET)