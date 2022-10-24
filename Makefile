
TOPDIR := $(shell /bin/pwd)
INCDIR := $(TOPDIR)

PACKAGE = esma
PACKAGE_VERSION = 0.9

export PACKAGE PACKAGE_VERSION

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
CFLAGS += -Wall -I$(INCDIR) -O2
#CFLAGS += -Wall -Os -I$(INCDIR) -g

export CC STRIP CFLAGS TOPDIR INCDIR

CORE = $(TOPDIR)/core/*.o
UTILS = $(TOPDIR)/utils/*.o
ENGINE = $(TOPDIR)/engine/*.o
ENGINE_MODULES = $(TOPDIR)/engine/modules/*.o

export CORE UTILS ENGINE ENGINE_MODULES

all:
	@$(MAKE) -C utils
	@$(MAKE) -C core
	@$(MAKE) -C engine
	@$(MAKE) -C tests
	@$(MAKE) -C programs

documentation:
	@echo -n "Makiking documentation... "
	@rm -vfr doc > /dev/null || (echo "failed" && exit 1)
	@rm -vf Doxyfile > /dev/null || (echo "failed" && exit 1)
	@./doxygen_release $> /dev/null || (echo "failed" && exit 1)
	@echo "done"

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
	@$(MAKE) cleanall -C programs
