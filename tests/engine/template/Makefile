
.PHONY: clean cleanall

SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)
PROGRAM = test
LDFLAGS = -ldl -lm

CORE    = $(TOPDIR)/core/*.o
ENGINE  = $(TOPDIR)/engine/esma_template.o

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(CORE) $(ENGINE)
	$(CC) -rdynamic $(OBJECTS) $(OS) $(CORE) $(ENGINE) -o $@ $(LDFLAGS)

clean:
	@rm -fv $(OBJECTS)

cleanall:
	@rm -fv $(OBJECTS) $(PROGRAM)
