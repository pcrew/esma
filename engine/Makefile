
.PHONY: clean cleanall

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
CFLAGS += -fPIC

OBJECTS += modules/esma_reactor_epoll.o

all: $(OBJECTS)

clean:
	@rm -fv $(OBJECTS)
cleanall:
	@rm -fv $(OBJECTS)
