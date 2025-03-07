# This toplevel Makefile compiles the library in the lib subdirectory.
# If you want to see how to integrate the library in your own projects, check
# out the sub-directories examples-api-use/ and utils/
RGB_LIBDIR=./lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a

# Enable emulator support? (0=no, 1=yes)
ENABLE_EMULATOR?=0

# Some language bindings.
PYTHON_LIB_DIR=bindings/python
CSHARP_LIB_DIR=bindings/c\#

all : $(RGB_LIBRARY)
	$(MAKE) -C examples-api-use
	$(MAKE) -C examples
ifeq ($(ENABLE_EMULATOR), 1)
	$(MAKE) -C examples ENABLE_EMULATOR=1 emulator-demo
endif

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR) ENABLE_EMULATOR=$(ENABLE_EMULATOR)

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C utils clean
	$(MAKE) -C examples-api-use clean
	$(MAKE) -C examples clean
	$(MAKE) -C $(PYTHON_LIB_DIR) clean

build-csharp:
	$(MAKE) -C $(CSHARP_LIB_DIR) nuget
	$(MAKE) -C $(CSHARP_LIB_DIR) build

build-python: $(RGB_LIBRARY)
	$(MAKE) -C $(PYTHON_LIB_DIR) build

install-python: build-python
	$(MAKE) -C $(PYTHON_LIB_DIR) install

FORCE:
.PHONY: FORCE
