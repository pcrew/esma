
TOPDIR := $(shell /bin/pwd)
INCDIR := $(TOPDIR)

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
CFLAGS += -Wall -I$(INCDIR) -O2 -D_FILE_OFFSET_BITS=64
#CFLAGS += -Wall -Os -I$(INCDIR) -g -D_FILE_OFFSET_BITS=64

export CC STRIP CFLAGS TOPDIR INCDIR

SM = $(TOPDIR)/state_machines/*.o
CORE = $(TOPDIR)/core/*.o
UTILS = $(TOPDIR)/utils/*.o
ENGINE = $(TOPDIR)/engine/*.o
ENGINE_MODULES = $(TOPDIR)/engine/modules/*.o

export SM CORE UTILS ENGINE ENGINE_MODULES

all:
	@$(MAKE) -C utils
	@$(MAKE) -C core
	@$(MAKE) -C engine
	@$(MAKE) -C tests
	@$(MAKE) -C state_machines
	@$(MAKE) -C programs

clean:
	@rm -vf `find . -name *.o`
	@rm -vf `find . -name *.sw?`

cleanall:
	@rm -vf `find . -name *.o`
	@rm -vf `find . -name *.so`
	@rm -vf `find . -name *.sw?`

	@$(MAKE) cleanall -C utils
	@$(MAKE) cleanall -C core
	@$(MAKE) cleanall -C engine
	@$(MAKE) cleanall -C tests
	@$(MAKE) cleanall -C state_machines
	@$(MAKE) cleanall -C programs
