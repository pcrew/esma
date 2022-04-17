
.PHONY: clean cleanall

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
CFLAGS += -fPIC

OBJECTS += modules/esma_reactor_poll.o
OBJECTS += modules/esma_reactor_epoll.o
OBJECTS += modules/esma_msg_queue_ring_buffer.o

all: $(OBJECTS)

clean:
	@rm -fv $(OBJECTS)
cleanall:
	@rm -fv $(OBJECTS)
