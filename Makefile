CC = gcc

INC_PATH ?= inc/
CFLAGS = -std=c89

OSTYPE = $(shell uname)
ifneq (${OSTYPE}, Linux)
CFLAGS += -DHOLYBSD
else
CFLAGS += -DOS_LINUX
endif

LIB_PTHREAD = -lpthread
LIBS_LD = $(LIB_PTHREAD)


OBJ_THREAD = thread.o
SRC_THREAD = sys/linux/thread.c

BIN_THREAD_TEST = thread_test.bin
OBJ_THREAD_TEST = thread_test.o

all: $(BIN_THREAD_TEST)

$(OBJ_THREAD): 
	$(CC) -c $(SRC_THREAD) -I$(INC_PATH) $(LIBS_LD)

$(BIN_THREAD_TEST): $(OBJ_THREAD_TEST) $(OBJ_THREAD)
	$(CC) -o $(BIN_THREAD_TEST) $(OBJ_THREAD_TEST) $(OBJ_THREAD) $(LIBS_LD) 

clean:
	rm *.o 
	rm *.bin

