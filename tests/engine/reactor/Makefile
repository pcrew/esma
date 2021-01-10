
.PHONY: clean cleanall

SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)
PROGRAM = test
LDFLAGS = -ldl -lm

CORE     = $(TOPDIR)/core/*.o
UTILS    = $(TOPDIR)/utils/*.o
REACTOR  = $(TOPDIR)/engine/modules/*.o

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(CORE) $(UTILS) $(REACTOR)
	$(CC) -rdynamic $(OBJECTS) $(OS) $(CORE) $(UTILS) $(REACTOR) -o $@ $(LDFLAGS)

clean:
	@rm -fv $(OBJECTS)

cleanall:
	@rm -fv $(OBJECTS) $(PROGRAM)
