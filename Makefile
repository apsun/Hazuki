BUILD_DIR = build
INCLUDE_DIR = include
HAZUKI_DIR = src/hazuki
TEST_DIR = src/test
MKDIR = mkdir -p
CFLAGS = -std=c99 -O3 -I$(INCLUDE_DIR) -Wall -Wextra -pedantic
OUTPUT_HAZUKI = libhazuki.a
OUTPUT_TEST = test

.PHONY: all builddir hazuki test clean

all: hazuki test

builddir:
	$(MKDIR) $(BUILD_DIR)

utils.o: builddir
	$(CC) $(CFLAGS) -c $(HAZUKI_DIR)/utils.c -o $(BUILD_DIR)/utils.o

vector.o: builddir utils.o
	$(CC) $(CFLAGS) -c $(HAZUKI_DIR)/vector.c -o $(BUILD_DIR)/vector.o

map.o: builddir utils.o
	$(CC) $(CFLAGS) -c $(HAZUKI_DIR)/map.c -o $(BUILD_DIR)/map.o

test_utils.o: builddir utils.o
	$(CC) $(CFLAGS) -c $(TEST_DIR)/test_utils.c -o $(BUILD_DIR)/test_utils.o

test_vector.o: builddir utils.o vector.o
	$(CC) $(CFLAGS) -c $(TEST_DIR)/test_vector.c -o $(BUILD_DIR)/test_vector.o

test_map.o: builddir utils.o map.o
	$(CC) $(CFLAGS) -c $(TEST_DIR)/test_map.c -o $(BUILD_DIR)/test_map.o

test_main.o: builddir test_utils.o test_vector.o test_map.o
	$(CC) $(CFLAGS) -c $(TEST_DIR)/test_main.c -o $(BUILD_DIR)/test_main.o

hazuki: builddir utils.o vector.o map.o
	$(AR) $(ARFLAGS) $(BUILD_DIR)/$(OUTPUT_HAZUKI) \
		$(BUILD_DIR)/utils.o \
		$(BUILD_DIR)/vector.o \
		$(BUILD_DIR)/map.o

test: builddir utils.o vector.o map.o test_utils.o test_vector.o test_map.o test_main.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(OUTPUT_TEST) \
		$(BUILD_DIR)/utils.o \
		$(BUILD_DIR)/vector.o \
		$(BUILD_DIR)/map.o \
		$(BUILD_DIR)/test_utils.o \
		$(BUILD_DIR)/test_vector.o \
		$(BUILD_DIR)/test_map.o \
		$(BUILD_DIR)/test_main.o

clean:
	$(RM) $(BUILD_DIR)/*.o $(BUILD_DIR)/$(OUTPUT_HAZUKI) $(BUILD_DIR)/$(OUTPUT_TEST)
