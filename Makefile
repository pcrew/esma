
TOPDIR := $(shell /bin/pwd)
INCDIR := $(TOPDIR)

CC = $(CROSS_COMPILE)gcc
export CORE UTILS ENGINE ENGINE_MODULES

all:
	@$(MAKE) -C utils
	@$(MAKE) -C core
	@$(MAKE) -C engine
	@$(MAKE) -C tests
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
	@$(MAKE) cleanall -C programs
