# Makefile to build class 'pan~' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name
lib.name = voicepool

# input source file (class name == source file basename)
class.sources = voicepool.c

# all extra files to be included in binary distribution of the library
datafiles =

cflags += -g

ldflags += -g

suppress-wunused += yes

define forWindows
PDINCLUDEDIR=C:\Users\Lucarda\Downloads\pd-0.50-0-32bit\src
PDBINDIR=C:\Users\Lucarda\Downloads\pd-0.50-0-32bit\bin
endef

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=./pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
