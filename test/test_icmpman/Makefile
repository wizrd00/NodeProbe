CC = pcc

BUILD_DIR := build
FAKE_DIR := fake
INCLUDE_DIR := ../../include/
LIB_DIR := ../../library
UNITY_DIR := ../libunity

ifeq ($(CC), pcc)
CFLAGS := -std=c99 -O3 -Wc,-Werror=implicit-function-declaration,-Werror=missing-prototypes,-Werror=pointer-sign,-Werror=sign-compare,-Werror=strict-prototypes,-Werror=shadow -pthread
UNITY_LIB_FLAGS := -Wl,--library-path=$(LIB_DIR),--library=unity-musl,-rpath=$(LIB_DIR)
else ifeq ($(CC), gcc)
CFLAGS := -std=gnu99 -O3 -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wcast-align -Wconversion -Wsign-conversion -Wshadow -Wno-switch -pthread
UNITY_LIB_FLAGS := -L$(LIB_DIR) -lunity-glibc -Wl,-rpath=$(LIB_DIR)
else
$(error unsupported compiler : $(CC))
endif

TEST_SRC := test.c
TEST_OBJ := $(BUILD_DIR)/test.o

REAL_ICMPMAN_SRC := ../../source/icmpman.c
FAKE_ICMPMAN_SRC := fake/fake_icmpman.c
REAL_ICMPMAN_HDR := ../../include/icmpman.h
FAKE_ICMPMAN_HDR := fake/fake_icmpman.h
ICMPMAN_OBJ := $(BUILD_DIR)/icmpman.o

FAKE_SOCKET_SRC := fake/fake_socket.c
FAKE_SOCKET_OBJ := $(BUILD_DIR)/fake_socket.o

FAKE_POLL_SRC := fake/fake_poll.c
FAKE_POLL_OBJ := $(BUILD_DIR)/fake_poll.o

test.elf : $(ICMPMAN_OBJ) $(FAKE_SOCKET_OBJ) $(FAKE_POLL_OBJ) $(TEST_OBJ)
	@echo linking object files $^
	$(CC) $(CFLAGS) -o $@ $^ $(UNITY_LIB_FLAGS)

$(TEST_OBJ) : $(TEST_SRC)
	@echo compiling $^
	$(CC) -c $(CFLAGS) -I$(FAKE_DIR) -I$(UNITY_DIR) -I$(INCLUDE_DIR) -o $@ $^

$(ICMPMAN_OBJ) : $(FAKE_ICMPMAN_SRC) $(FAKE_ICMPMAN_HDR)
	@echo compiling $<
	$(CC) -c $(CFLAGS) -I$(FAKE_DIR) -I$(INCLUDE_DIR) -o $@ $<

$(FAKE_ICMPMAN_SRC) : $(REAL_ICMPMAN_SRC)
	@cp $< $@

$(FAKE_ICMPMAN_HDR) : $(REAL_ICMPMAN_HDR)
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
