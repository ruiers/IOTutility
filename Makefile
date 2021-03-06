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
APPS = $(basename $(APP_SRCS))

LIB_UTILITY = libutility.so

all: clean $(LIB_UTILITY) $(APP_OBJ)

$(LIB_UTILITY): $(LIB_OBJS)
	$(CC) -o $(LIB_UTILITY) $(LIB_OBJS) $(CFLAGS) -fPIC -shared -lpthread

$(APP_OBJ): $(APP_SRC)
	$(CC) -c $(APP_SRC) $(CFLAGS)

apps: $(APP_OBJS)
	for app_obj in $(APPS) ; do \
	app_bin=$$app_obj.bin ; \
	$(CC) -o $$app_bin $$app_obj.o $(CFLAGS) -lpthread -L./ -lutility -Wl,-rpath=. ; \
	done

clean:
	-rm -f $(shell find . -name "*.[o]")
	-rm -f *.bin

prepare:
	-find . -name "*.c" | xargs  astyle --style=allman
	-find . -name "*.h" | xargs  astyle --style=allman
	-find . -name "*.c" | xargs dos2unix
	-find . -name "*.h" | xargs dos2unix

finish:
	-find . -name "*.c" | xargs dos2unix
	-find . -name "*.h" | xargs dos2unix
	-find . -name "*.c" | xargs chmod a+w
	-find . -name "*.h" | xargs chmod a+w

