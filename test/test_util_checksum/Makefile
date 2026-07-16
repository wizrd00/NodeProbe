CC = pcc

BUILD_DIR := build
INCLUDE_DIR := ../../include/
LIB_DIR := ../../library
UNITY_DIR := ../libunity

ifeq ($(CC), pcc)
CFLAGS := -std=c99 -g -O3 -Wc,-Werror=implicit-function-declaration,-Werror=missing-prototypes,-Werror=pointer-sign,-Werror=sign-compare,-Werror=strict-prototypes,-Werror=shadow -pthread
UNITY_LIB_FLAGS := -Wl,--library-path=$(LIB_DIR),--library=unity-musl,-rpath=$(LIB_DIR)
else ifeq ($(CC), gcc)
CFLAGS := -std=gnu99 -O3 -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wcast-align -Wconversion -Wsign-conversion -Wshadow -Wno-switch -pthread
UNITY_LIB_FLAGS := -L$(LIB_DIR) -lunity-glibc -Wl,-rpath=$(LIB_DIR)
else
$(error unsupported compiler : $(CC))
endif

TEST_SRC := test.c
TEST_OBJ := $(BUILD_DIR)/test.o

CHECKSUM_HDR := ../../include/utils/checksum.h

test.elf : $(TEST_OBJ) $(CHECKSUM_HDR)
	@echo linking object files $^
	$(CC) $(CFLAGS) -o $@ $< $(UNITY_LIB_FLAGS)

$(TEST_OBJ) : $(TEST_SRC)
	@echo compiling $^
	$(CC) -c $(CFLAGS) -I$(UNITY_DIR) -I$(INCLUDE_DIR) -o $@ $^

clean :
	rm -rf build/* test.elf
