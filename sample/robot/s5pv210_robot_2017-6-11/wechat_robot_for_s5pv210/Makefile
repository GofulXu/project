vpath %.c = src/
vpath %.h = inc/
vpath %.so= lib/

CC=arm-none-linux-gnueabi-gcc

ELF = wechat 

CPPFLAGS += -Iinc
CPPFLAGS += -Iinc/jpg
CPPFLAGS += -Iinc/sound

LDFLAGS += -Llib
LDFLAGS += -lts 
LDFLAGS += -ljpeg 
LDFLAGS += -lv4l2
LDFLAGS += -lasound
LDFLAGS += -lpthread
LDFLAGS += -Wl,-rpath=.
LDFLAGS += -Wl,-rpath=./lib
LDFLAGS +=  -lmsc -lrt -ldl -lm

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst %.c, %.o, $(SRCS))
OBJS := $(filter-out src/v4l2.o, $(OBJS))

$(ELF):$(OBJS)
	$(CC) -o $@ $^ $(CPPFLAGS) $(LDFLAGS)
	-@$(RM) src/*.o 

clean:
	-@$(RM) src/*.o $(ELF)
