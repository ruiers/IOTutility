CC = gcc

INC_PATH ?= -Iinc/ -Iinc/linux
CFLAGS = -fPIC $(INC_PATH)

OSTYPE = $(shell uname)
ifneq (${OSTYPE}, Linux)
CFLAGS += -DHOLYBSD
else
CFLAGS += -DOS_LINUX -DFN_DEBUG
endif

LIB_PTHREAD = -lpthread
LIBS_LD = $(LIB_PTHREAD)

LIB_SRCS = $(foreach d, lib/linux, $(wildcard $(addprefix $(d)/*, .c)))
LIB_OBJS = $(addsuffix .o, $(basename $(LIB_SRCS)))

LIB_UTILITY = libutility.so
BIN_THREAD_TEST = thread_test.bin
OBJ_THREAD_TEST = thread_test.o

all: clean $(LIB_UTILITY) $(BIN_THREAD_TEST)

$(LIB_UTILITY): $(LIB_OBJS)
	$(CC) -o $(LIB_UTILITY) $(LIB_OBJS) $(CFLAGS) -fPIC -shared -lpthread

$(BIN_THREAD_TEST): $(OBJ_THREAD_TEST)
	$(CC) -o $(BIN_THREAD_TEST) $(OBJ_THREAD_TEST) $(CFLAGS) -lpthread -L./ -lutility -Wl,-rpath=.

clean:
	-rm -f $(shell find . -name "*.[o]")
	-rm -f *.bin

