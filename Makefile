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

BIN_THREAD_TEST = thread_test.bin
OBJ_THREAD_TEST = thread_test.o

all: $(BIN_THREAD_TEST)

$(OBJ_THREAD): 
	$(CC) -c $(SRC_THREAD) -I$(INC_PATH) $(LIBS_LD)

$(OBJ_CONNECT): 
	$(CC) -c $(SRC_CONNECT) -I$(INC_PATH) $(LIBS_LD)

$(OBJ_CACHE): 
	$(CC) -c $(SRC_CACHE) -I$(INC_PATH) $(LIBS_LD)

$(BIN_THREAD_TEST): $(OBJ_THREAD_TEST) $(OBJ_THREAD) $(OBJ_CONNECT) $(OBJ_CACHE)
	$(CC) -o $(BIN_THREAD_TEST) $(OBJ_THREAD_TEST) $(OBJ_THREAD) $(OBJ_CONNECT) $(OBJ_CACHE) $(LIBS_LD) 

clean:
	rm *.o 
	rm *.bin

