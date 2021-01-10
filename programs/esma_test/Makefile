
.PHONY: clean cleanall

SOURCES = main.c master.c slave.c
OBJECTS = $(SOURCES:.c=.o)
PROGRAM = test
LDFLAGS = -ldl

CORE = $(TOPDIR)/core/*.o
UTILS = $(TOPDIR)/utils/*.o
ENGINE  = $(TOPDIR)/engine/*.o
ENGINE += $(TOPDIR)/engine/modules/*.o

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(CORE) $(UTILS) $(ENGINE)
	$(CC) -rdynamic $(OBJECTS) $(OS) $(CORE) $(UTILS) $(ENGINE) -o $@ $(LDFLAGS)

clean:
	@rm -fv $(OBJECTS)

cleanall:
	@rm -fv $(OBJECTS) $(PROGRAM)
