CC = gcc
TARGET = shell_sched
FLAGS = -W
FLAGS += -Wall
FLAGS += -Werror
FLAGS += -pedantic
FLAGS += -Wno-unused-parameter
FLAGS += -Wno-error=unused-result
FLAGS += -Wno-implicit-fallthrough

RELEASE_FLAGS += $(FLAGS)

DEBUG_FLAGS += $(FLAGS)
DEBUG_FLAGS += -DENABLE_DEBUG
DEBUG_FLAGS += -fsanitize=address,undefined
DEBUG_FLAGS += -g
DEBUG_FLAGS += -O0

OUT_DIR = out
RELEASE_DIR = $(OUT_DIR)/release
DEBUG_DIR = $(OUT_DIR)/debug

DEBUG_BIN_DIR = $(DEBUG_DIR)/bin
RELEASE_BIN_DIR = $(RELEASE_DIR)/bin
SRC_DIR = shell_sched
SRC = $(shell find $(SRC_DIR) -name '*.c')

OBJ_RELEASE_DIR = $(RELEASE_DIR)/obj
OBJS_RELEASE=$(patsubst $(SRC_DIR)/%.c,$(OBJ_RELEASE_DIR)/%.o, $(SRC))

OBJ_DEBUG_DIR = $(DEBUG_DIR)/obj
OBJS_DEBUG=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DEBUG_DIR)/%.o, $(SRC))

$(TARGET): $(OBJS_RELEASE)
	@mkdir -p $(RELEASE_BIN_DIR)
	$(CC) -o $(RELEASE_BIN_DIR)/$@ $^ $(RELEASE_FLAGS) -I .

$(OBJ_RELEASE_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(RELEASE_FLAGS) -o $@ -c $< -I .

$(OBJ_DEBUG_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(DEBUG_FLAGS) -o $@ -c $< -I .

release: $(TARGET)

test:
	@echo "Building test cases..."
	@mkdir -p out/test
	gcc test/test01.c -o out/test/test01
	gcc test/test02.c -o out/test/test02

clean:
	@echo "Clean up environment..."
	@echo "-----------------------"
	rm -rf $(OUT_DIR)
	@echo "-----------------------"
	@echo "Environment cleaned!"

format:
	@echo "Applying Format..."
	@find . -type f \( -name "*.c" \) -exec clang-format -i {} \;
	@find . -type f \( -name "*.h" -o -name "*.h" \) -exec clang-format -i {} \;
	@echo "Done! "

.PHONY: all clean