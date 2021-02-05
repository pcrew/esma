
.PHONY: clean cleanall

SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)
PROGRAM = test
LDFLAGS = -ldl -lm

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(CORE) $(UTILS) $(REACTOR)
	$(CC) -rdynamic $(OBJECTS) $(CORE) $(UTILS) $(REACTOR) -o $@ $(LDFLAGS)

clean:
	@rm -fv $(OBJECTS)

cleanall:
	@rm -fv $(OBJECTS) $(PROGRAM)
