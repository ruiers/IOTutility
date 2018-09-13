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

APP_SRCS = $(foreach d, app, $(wildcard $(addprefix $(d)/*, .c)))
APP_OBJS = $(addsuffix .o, $(basename $(APP_SRCS)))

LIB_UTILITY = libutility.so

BIN_MQTT_CLIENT = mqtt_client.bin
OBJ_MQTT_CLIENT = app/mqtt_client.o

all: clean $(LIB_UTILITY) $(APP_OBJS) 

$(LIB_UTILITY): $(LIB_OBJS)
	$(CC) -o $(LIB_UTILITY) $(LIB_OBJS) $(CFLAGS) -fPIC -shared -lpthread

$(APP_OBJ): $(APP_SRC)
	$(CC) -c $(APP_SRC) $(CFLAGS) -fPIC -shared -lpthread

$(BIN_MQTT_CLIENT): 
	$(CC) -o $(BIN_MQTT_CLIENT) $(OBJ_MQTT_CLIENT) $(CFLAGS) -lpthread -L./ -lutility -Wl,-rpath=.

clean:
	-rm -f $(shell find . -name "*.[o]")
	-rm -f *.bin

