CC = gcc
CFLAGS = -Wall -Wextra

SRC_DIR = $(PWD)
BUILD_DIR = $(PWD)

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
EXECUTABLE = LP25

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lcrypto -lssl

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)/*.o $(EXECUTABLE)

.PHONY: clean
