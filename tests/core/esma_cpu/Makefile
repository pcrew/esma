
.PHONY: clean cleanall

SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)
PROGRAM = test
LDFLAGS = -ldl -lm

CORE = $(TOPDIR)/core/esma_cpu.o

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(CORE)
	$(CC) -rdynamic $(OBJECTS) $(CORE) $(OS) -o $@ $(LDFLAGS)

clean:
	@rm -fv $(OBJECTS)

cleanall:
	@rm -fv $(OBJECTS) $(PROGRAM)
