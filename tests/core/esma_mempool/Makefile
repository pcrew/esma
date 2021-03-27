
.PHONY: clean cleanall

SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)
PROGRAM = test
LDFLAGS = -ldl -lm

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(CORE)
	$(CC) -rdynamic $(OBJECTS) $(CORE) -o $@ $(LDFLAGS)

clean:
	@rm -fv $(OBJECTS)

cleanall:
	@rm -fv $(OBJECTS) $(PROGRAM)
