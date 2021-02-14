
.PHONY: clean cleanall

SOURCES = main.c manager.c
OBJECTS = $(SOURCES:.c=.o)
PROGRAM = test
LDFLAGS = -ldl

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(SM) $(CORE) $(UTILS) $(ENGINE) $(ENGINE_MODULES)
	$(CC) -rdynamic $(OBJECTS) $(SM) $(CORE) $(UTILS) $(ENGINE) $(ENGINE_MODULES) -o $@ $(LDFLAGS)

clean:
	@rm -fv $(OBJECTS)

cleanall:
	@rm -fv $(OBJECTS) $(PROGRAM)
