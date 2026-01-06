CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -Isrc
LDFLAGS = -lm
TARGET = allocator_demo

SRC_DIR = src
TEST_DIR = tests

SRCS = $(SRC_DIR)/buddy_allocator.c \
       $(SRC_DIR)/slab_allocator.c \
       $(SRC_DIR)/allocator_factory.c

TEST_SRCS = $(TEST_DIR)/simple_test.c

OBJS = $(SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

# Manual compilation if needed
manual:
	$(CC) $(CFLAGS) -c src/buddy_allocator.c -o src/buddy_allocator.o
	$(CC) $(CFLAGS) -c src/slab_allocator.c -o src/slab_allocator.o
	$(CC) $(CFLAGS) -c src/allocator_factory.c -o src/allocator_factory.o
	$(CC) $(CFLAGS) -c tests/simple_test.c -o tests/simple_test.o
	$(CC) $(CFLAGS) -o $(TARGET) src/*.o tests/*.o $(LDFLAGS)