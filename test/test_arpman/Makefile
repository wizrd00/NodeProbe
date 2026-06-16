CC = pcc

BUILD_DIR := build
FAKE_DIR := fake
INCLUDE_DIR := ../../include/
LIB_DIR := ../../library
UNITY_DIR := ../libunity

ifeq ($(CC), pcc)
CFLAGS := -std=c99 -O3 -Wc,-Werror=implicit-function-declaration,-Werror=missing-prototypes,-Werror=pointer-sign,-Werror=sign-compare,-Werror=strict-prototypes,-Werror=shadow -pthread
UNITY_LIB_FLAGS := -Wl,--library-path=$(LIB_DIR),--library=unity,-rpath=$(LIB_DIR)
else ifeq ($(CC), gcc)
CFLAGS := -std=gnu99 -O3 -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wcast-align -Wconversion -Wsign-conversion -Wshadow -Wno-switch -pthread
UNITY_LIB_FLAGS := -L$(LIB_DIR) -lunity -Wl,-rpath=$(LIB_DIR)
else
$(error unsupported compiler : $(CC))
endif

TEST_SRC := test.c
TEST_OBJ := $(BUILD_DIR)/test.o

REAL_ARPMAN_SRC := ../../source/arpman.c
FAKE_ARPMAN_SRC := fake/fake_arpman.c
REAL_ARPMAN_HDR := ../../include/arpman.h
FAKE_ARPMAN_HDR := fake/fake_arpman.h
ARPMAN_OBJ := $(BUILD_DIR)/arpman.o

FAKE_SOCKET_SRC := fake/fake_socket.c
FAKE_SOCKET_OBJ := $(BUILD_DIR)/fake_socket.o

FAKE_POLL_SRC := fake/fake_poll.c
FAKE_POLL_OBJ := $(BUILD_DIR)/fake_poll.o

test.elf : $(ARPMAN_OBJ) $(FAKE_SOCKET_OBJ) $(FAKE_POLL_OBJ) $(TEST_OBJ)
	@echo linking object files $^
	$(CC) $(CFLAGS) $(UNITY_LIB_FLAGS) -o $@ $^

$(TEST_OBJ) : $(TEST_SRC)
	@echo compiling $^
	$(CC) -c $(CFLAGS) -I$(FAKE_DIR) -I$(UNITY_DIR) -I$(INCLUDE_DIR) -o $@ $^

$(ARPMAN_OBJ) : $(FAKE_ARPMAN_SRC) $(FAKE_ARPMAN_HDR)
	@echo compiling $<
	$(CC) -c $(CFLAGS) -I$(FAKE_DIR) -I$(INCLUDE_DIR) -o $@ $<

$(FAKE_ARPMAN_SRC) : $(REAL_ARPMAN_SRC)
	@cp $< $@

$(FAKE_ARPMAN_HDR) : $(REAL_ARPMAN_HDR)
	@cp $< $@
	@sed -e "s/include <poll\.h>/include <fake_poll.h>/" -e "s/include <sys\/socket\.h>/include <fake_socket\.h>/" $< > $@

$(FAKE_SOCKET_OBJ) : $(FAKE_SOCKET_SRC)
	@echo compiling $<
	$(CC) -c $(CFLAGS) -I$(FAKE_DIR) -I$(INCLUDE_DIR) -o $@ $<

$(FAKE_POLL_OBJ) : $(FAKE_POLL_SRC)
	@echo compiling $<
	$(CC) -c $(CFLAGS) -I$(FAKE_DIR) -I$(INCLUDE_DIR) -o $@ $<

memcheck : test.elf
	@echo start checking test.elf for any memory bug
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./$^

clean :
	rm -rf build/* test.elf
