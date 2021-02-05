
.PHONY: clean cleanall

SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)
PROGRAM = test
LDFLAGS = -ldl -lm

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(CORE) $(ENGINE) $(UTILS)
	$(CC) -rdynamic $(OBJECTS) $(CORE) $(ENGINE) $(UTILS) -o $@ $(LDFLAGS)

clean:
	@rm -fv $(OBJECTS)

cleanall:
	@rm -fv $(OBJECTS) $(PROGRAM)
