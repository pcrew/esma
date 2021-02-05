
.PHONY: clean cleanall

SOURCES = main.c master.c slave.c
OBJECTS = $(SOURCES:.c=.o)
PROGRAM = test
LDFLAGS = -ldl

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(CORE) $(UTILS) $(ENGINE) $(ENGINE_MODULES)
	$(CC) -rdynamic $(OBJECTS) $(OS) $(CORE) $(UTILS) $(ENGINE) $(ENGINE_MODULES) -o $@ $(LDFLAGS)

clean:
	@rm -fv $(OBJECTS)

cleanall:
	@rm -fv $(OBJECTS) $(PROGRAM)
