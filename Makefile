CC = gcc
BUILD_DIR = build/$(notdir $(CC))

CFLAGS = -D_XOPEN_SOURCE=500 -Iinclude -std=c11 -g -Wall -Werror

LINKER_FLAGS =

vpath %.c src

.PHONY: all directories check clean

all: directories slur slur-repl

directories: $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $<

slur: $(BUILD_DIR)/editor.o
	$(CC) $^ $(LINKER_FLAGS) -o $@

slur-repl: $(BUILD_DIR)/repl.o
	$(CC) $^ $(LINKER_FLAGS) -o $@

check:
	scan-build make

clean:
	rm $(BUILD_DIR)/*.o -f
