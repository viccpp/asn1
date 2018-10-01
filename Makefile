##############################################################
# GNU Make Makefile

.PHONY: all install test distclean clean

all: src/Makefile
	cd src && $(MAKE)

install: src/Makefile
	cd src && $(MAKE) install

test: test/Makefile
	cd test && $(MAKE)

distclean: clean
	-rm -f src/Makefile src/os_config.h test/Makefile

clean:
	-cd src && $(MAKE) clean
	-cd test && $(MAKE) clean
