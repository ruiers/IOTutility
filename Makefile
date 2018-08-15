CC = gcc

INC_PATH ?= inc/
CFLAGS = -std=c99

OSTYPE = $(shell uname)
ifneq (${OSTYPE}, Linux)
CFLAGS += -DHOLYBSD
else
CFLAGS += -DOS_LINUX -DFN_DEBUG
endif

LIB_PTHREAD = -lpthread
LIBS_LD = $(LIB_PTHREAD)


OBJ_THREAD = thread.o
SRC_THREAD = sys/linux/thread.c

OBJ_CONNECT = connect.o
SRC_CONNECT = sys/linux/connect.c

OBJ_CACHE = cache.o
SRC_CACHE = sys/linux/cache.c

LIB_UTILITY = libutility.so
BIN_THREAD_TEST = thread_test.bin
OBJ_THREAD_TEST = thread_test.o

all: $(LIB_UTILITY) $(BIN_THREAD_TEST)

$(OBJ_THREAD): 
	$(CC) -c $(SRC_THREAD) -I$(INC_PATH) $(LIBS_LD) -fPIC

$(OBJ_CONNECT): 
	$(CC) -c $(SRC_CONNECT) -I$(INC_PATH) $(LIBS_LD) -fPIC

$(OBJ_CACHE): 
	$(CC) -c $(SRC_CACHE) -I$(INC_PATH) $(LIBS_LD) -fPIC

$(LIB_UTILITY): $(OBJ_THREAD) $(OBJ_CONNECT) $(OBJ_CACHE)
	$(CC) -o $(LIB_UTILITY) $(OBJ_THREAD) $(OBJ_CONNECT) $(OBJ_CACHE) $(LIBS_LD) -fPIC -shared

$(BIN_THREAD_TEST): $(OBJ_THREAD_TEST)
	$(CC) -o $(BIN_THREAD_TEST) $(OBJ_THREAD_TEST) $(LIBS_LD) -L./ -lutility -Wl,-rpath=.

clean:
	rm *.o *.so
	rm *.bin

