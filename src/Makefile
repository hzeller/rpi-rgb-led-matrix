# TODO: This contains a lot of {c|p}ython build boilerplate, this needs cleanup.
PYTHON     ?= python
SETUP      := setup.py
BUILD_ARGS := build --build-lib .
INST_ARGS  := install
ifdef DESTDIR
INST_ARGS  += --root=$(DESTDIR)
endif
CLEAN_ARGS := clean --all

MANPAGES := $(patsubst %.txt,%,$(wildcard *.txt))
TXTTOMAN := a2x -f manpage

# Where our library resides. It is split between includes and the binary
# library in lib
RGB_LIBDIR=../../lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a

ifneq "$(wildcard debian/changelog)" ""
PKGNAME := $(shell dpkg-parsechangelog | sed -n 's/^Source: //p')
VERSION := $(shell dpkg-parsechangelog | sed -n 's/^Version: \([^-]*\).*/\1/p')
UPSDIST := $(PKGNAME)-$(VERSION).tar.gz
DEBDIST := $(PKGNAME)_$(VERSION).orig.tar.gz
endif

all: build
build: build-python
install: install-python
clean: clean-python
	find ./rgbmatrix -type f -name \*.so -delete
	find . -type f -name \*.pyc -delete
	$(RM) build-* install-* test-*

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)

test: test-python
test-python:
ifneq "$(wildcard tests/*.py)" ""
	nosetests -v -w tests
else
	$(info Test suite is not implemented...)
endif

ifneq "$(wildcard debian/control)" ""
PYVERS := $(shell pyversions -r -v debian/control)
PYEXEC := $(shell pyversions -d)
BUILD_ARGS += --executable=/usr/bin/$(PYEXEC)
INST_ARGS  += --no-compile -O0

build-python: $(PYVERS:%=build-python-%)
build-python-%: $(RGB_LIBRARY)
	$(info * Doing build for $(PYTHON)$* ...)
	$(PYTHON)$* $(SETUP) $(BUILD_ARGS)

install-python: $(PYVERS:%=install-python-%)
install-python-%:
	$(info * Doing install for $(PYTHON)$* ...)
	$(PYTHON)$* $(SETUP) $(INST_ARGS)

clean-python: $(PYVERS:%=clean-python-%)
clean-python-%:
	$(PYTHON)$* $(SETUP) $(CLEAN_ARGS)
else
build-python: $(RGB_LIBRARY)
	$(PYTHON) $(SETUP) $(BUILD_ARGS)

install-python:
	$(PYTHON) $(SETUP) $(INST_ARGS)

clean-python:
	$(PYTHON) $(SETUP) $(CLEAN_ARGS)
endif

distclean: clean
dist: distclean
	$(info * Creating ../$(UPSDIST) and ../$(DEBDIST))
	@tar --exclude='.svn' \
	    --exclude='*.swp' \
	    --exclude='debian' \
	    -czvf ../$(UPSDIST) \
	    -C ../ $(notdir $(CURDIR))
	@cp ../$(UPSDIST) ../$(DEBDIST)
	@if test -d ../tarballs; then \
		mv -v ../$(DEBDIST) ../tarballs; \
	fi

FORCE:
.PHONY: FORCE
.PHONY: build install test clean dist distclean
.PHONY: build-python install-python clean-python
