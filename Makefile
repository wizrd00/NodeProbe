CC = pcc

LIBC := $(shell ldd --version 2>&1 | grep -qi musl && echo musl || echo glibc)

SRC_DIR := source
INC_DIR := include
BIN_DIR := binary
LIB_DIR := library

ifeq ($(LIBC), musl)
NODEPROBE := libnodeprobe-musl.so
else ifeq ($(LIBC), glibc)
NODEPROBE := libnodeprobe-glibc.so
else
$(error unsupported libc : $(LIBC))
endif

LOGMAN_MACROS := -DLOG_WARNN -DLOG_ERROR
DEBUG_LOGMAN_MACROS := -DLOG_TRACE -DLOG_DEBUG -DLOG_WARNN -DLOG_ERROR

ifeq ($(CC), pcc)
CFLAGS := -std=c99 -O3 -fPIC -shared -Wc,-Werror=implicit-function-declaration,-Werror=missing-prototypes,-Werror=pointer-sign,-Werror=sign-compare,-Werror=strict-prototypes,-Werror=shadow -pthread
ifeq ($(wildcard $(LIB_DIR)/*), $())
LIB_FLAGS :=
else
LIB_FLAGS := -Wl,--library-path=$(LIB_DIR),-rpath=$(LIB_DIR)
endif
else ifeq ($(CC), gcc)
SPECIAL_OPTS = -DARPHRD_ETHER=1
CFLAGS := -std=c99 -O3 -fPIC -shared -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wcast-align -Wconversion -Wsign-conversion -Wshadow -Wno-switch -pthread $(SPECIAL_OPTS)
ifeq ($(wildcard $(LIB_DIR)/*), $())
LIG_FLAGS :=
else
LIB_FLAGS := -L$(LIB_DIR) -Wl,-rpath=$(LIB_DIR)
endif
else
$(error unsupported compiler : $(CC))
endif

DEBUG_CFLAGS := $(CFLAGS) -g

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
HDR_FILES := $(wildcard $(INC_DIR)/*.h)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SRC_FILES))

INCLUDE_FLAGS := -I$(INC_DIR)

POINTER_SYM := "\e[91m->\e[0m"

$(NODEPROBE) : $(BIN_DIR) $(OBJ_FILES) $(HDR_FILES)
	@/usr/bin/echo -e $(POINTER_SYM) "\e[96mlinking modules into" $@ "\e[0m"
	$(CC) $(CFLAGS) $(LOGMAN_MACROS) -o $@ $(OBJ_FILES) $(LIB_FLAGS)
	@/usr/bin/echo -e $(POINTER_SYM) "\e[93mstrip" $@ "\e[0m"
	@strip $@

debug : $(BIN_DIR) $(OBJ_FILES) $(HDR_FILES)
	@/usr/bin/echo -e $(POINTER_SYM) "\e[96mlinking modules into" $(NODEPROBE) "\e[0m"
	$(CC) $(DEBUG_CFLAGS) $(DEBUG_LOGMAN_MACROS) -o $(NODEPROBE) $(OBJ_FILES) $(LIB_FLAGS)

$(BIN_DIR) :
	@mkdir -p $(BIN_DIR)

$(BIN_DIR)/%.o : $(SRC_DIR)/%.c $(INC_DIR)/%.h
	@/usr/bin/echo -e $(POINTER_SYM) "\e[93mcompiling module" $< "\e[0m"
	$(CC) -c $(CFLAGS) $(LIB_FLAGS) $(INCLUDE_FLAGS) -o $@ $<

memcheck : $(NODEPROBE)
	@echo start checking test.elf for any memory bug
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./$^

clean :
	rm $(wildcard $(BIN_DIR)/*.o)
	if [ -f $(NODEPROBE) ]; then rm $(NODEPROBE); fi;

clear : clean
