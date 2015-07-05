PYTHON     ?= python
CYTHON     ?= cython
SETUP      := setup.py
BUILD_ARGS := build
INST_ARGS  := install
ifdef DESTDIR
INST_ARGS  += --root=$(DESTDIR)
endif
CLEAN_ARGS := clean --all

MANPAGES := $(patsubst %.txt,%,$(wildcard *.txt))
TXTTOMAN := a2x -f manpage

ifneq "$(wildcard debian/changelog)" ""
PKGNAME := $(shell dpkg-parsechangelog | sed -n 's/^Source: //p')
VERSION := $(shell dpkg-parsechangelog | sed -n 's/^Version: \([^-]*\).*/\1/p')
UPSDIST := $(PKGNAME)-$(VERSION).tar.gz
DEBDIST := $(PKGNAME)_$(VERSION).orig.tar.gz
endif

all: build
build: build-python build-man
install: install-python install-man
clean: clean-python clean-man
	find . -type f -name \*.pyc -delete
	$(RM) build-* install-* test-*

test: test-python
test-python:
ifneq "$(wildcard tests/*.py)" ""
	nosetests -v -w tests
else
	$(info Test suite is not implemented...)
endif
	touch $@

ifneq "$(wildcard debian/control)" ""
PYVERS := $(shell pyversions -r -v debian/control)
PYEXEC := $(shell pyversions -d)
BUILD_ARGS += --executable=/usr/bin/$(PYEXEC)
INST_ARGS  += --no-compile -O0

build-python: $(PYVERS:%=build-python-%)
build-python-%: $(RGB_LIBRARY)
	$(info * Doing build for $(PYTHON)$* ...)
	$(PYTHON)$* $(SETUP) $(BUILD_ARGS)
	touch $@

install-python: $(PYVERS:%=install-python-%)
install-python-%:
	$(info * Doing install for $(PYTHON)$* ...)
	$(PYTHON)$* $(SETUP) $(INST_ARGS)
	touch $@

clean-python: $(PYVERS:%=clean-python-%)
clean-python-%:
	$(PYTHON)$* $(SETUP) $(CLEAN_ARGS)
else
build-python:
	$(PYTHON) $(SETUP) $(BUILD_ARGS)
	touch $@

install-python:
	$(PYTHON) $(SETUP) $(INST_ARGS)
	touch $@

clean-python:
	$(PYTHON) $(SETUP) $(CLEAN_ARGS)
endif

# For updating the generated code (not a direct dependency of build-python
# as the generated file is included in the distribution)
rgbmatrix.cpp : rgbmatrix.pyx
%.cpp : %.pyx
	$(CYTHON) --cplus -o $@ $^

build-man: $(MANPAGES:%=build-man-%)
build-man-%: %.txt
	$(info * Creating $* ...)
	$(TXTTOMAN) $<
	touch $@

install-man: $(MANPAGES:%=install-man-%)
install-man-%:
	$(info Installation of $* not implemented...)
	touch $@

clean-man: $(MANPAGES:%=clean-man-%)
clean-man-%:
	$(RM) $* $*.xml

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

.PHONY: build install test clean dist distclean
.PHONY: build-python install-python clean-python
.PHONY: build-man install-man clean-man