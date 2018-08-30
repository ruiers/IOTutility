CC = gcc

INC_PATH ?= inc/
CFLAGS = -fPIC -Iinc

OSTYPE = $(shell uname)
ifneq (${OSTYPE}, Linux)
CFLAGS += -DHOLYBSD
else
CFLAGS += -DOS_LINUX -DFN_DEBUG
endif

LIB_PTHREAD = -lpthread
LIBS_LD = $(LIB_PTHREAD)

NS_SRCS = $(foreach d, sys/linux, $(wildcard $(addprefix $(d)/*, .c)))
NS_OBJS = $(addsuffix .o, $(basename $(NS_SRCS)))

LIB_UTILITY = libutility.so
BIN_THREAD_TEST = thread_test.bin
OBJ_THREAD_TEST = thread_test.o

all: clean $(LIB_UTILITY) $(BIN_THREAD_TEST)

$(LIB_UTILITY): $(NS_OBJS)
	$(CC) -o $(LIB_UTILITY) $(NS_OBJS) -fPIC -shared -lpthread

$(BIN_THREAD_TEST): $(OBJ_THREAD_TEST)
	$(CC) -o $(BIN_THREAD_TEST) $(OBJ_THREAD_TEST) -lpthread -L./ -lutility -Wl,-rpath=.

clean:
	-rm -f *.o *.so
	-rm -f *.bin

