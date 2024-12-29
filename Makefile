# This makefile can call out to other sub-makefiles depending on which
# targets are specified by the invoking command. It also contains some
# information that is common to all targets.

# make sure the pwd is actually set
PWD ?= $(shell pwd)
export PWD

# Generic info about the generated target that may be overridden
# either via environment variables or the top level makefile
BINARY	    ?= $(shell basename $(PWD))
APP_TITLE   ?= $(shell basename $(PWD))
APP_AUTHOR  ?= $(shell git log -1 --pretty=format:'%an')
GIT_HASH    ?= $(shell git rev-parse --short HEAD)

APP_VERSION ?= 0.0.0
APP_NAME    ?= $(APP_TITLE)

ICON_JPG    ?= $(PWD)/resin/res/icon.jpg
ICON_PNG    ?= $(PWD)/assets/icon.png

CHESTO_DIR  := $(PWD)/libs/chesto
HELPERS     := $(CHESTO_DIR)/helpers

# if debug has been specified, tack on the git hash to the version
ifeq ($(DEBUG_BUILD),1)
APP_VERSION := $(APP_VERSION)-$(GIT_HASH)
CFLAGS    += -DDEBUG_BUILD
endif

# warn those who came in here uninitiated
ifeq (,$(MAKECMDGOALS))
all:
	@echo "This is a Chesto app! For more information see: https://github.com/fortheusers/chesto"
	@echo "No targets were specified, try:\n\tmake <target>"
	@echo "Where <target> is one of: pc, wiiu, switch, 3ds, wii"
endif

# common variables that all the makefiles will need, and can be appended to by the toplevel
# makefile, or even by the makefile fragments
SOURCES   += libs/chesto/src
INCLUDES  += libs/chesto/src

# for resin platform (non-PC), include some needed vars
ifeq (,$(findstring pc,$(MAKECMDGOALS)))
SOURCES   += libs/chesto/libs/resinfs/source
INCLUDES  += libs/chesto/libs/resinfs/include
endif

# for sdl2 platforms, use sdl font cache
SOURCES += $(CHESTO_DIR)/libs/SDL_FontCache
VPATH   += $(CHESTO_DIR)/libs/SDL_FontCache

CFLAGS	  += $(INCLUDE) -DAPP_VERSION=\"$(APP_VERSION)\" -frandom-seed=84248
CXXFLAGS  += $(CFLAGS) -fexceptions -std=gnu++20
ASFLAGS   += -g $(ARCH)

export TOPDIR := $(CURDIR)

# flatten list of source files
export CFILES   +=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export CPPFILES +=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
export SFILES   +=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

# our common dkP toolchain locations, not used by the PC targets
DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM
DEVKITPPC ?= $(DEVKITPRO)/devkitPPC
DEVKITA64 ?= $(DEVKITPRO)/devkitA64

# export our variables out to our other scripts
export APP_AUTHOR APP_TITLE ICON_JPG ICON_PNG APP_VERSION BINARY APP_NAME
export CFLAGS CXXFLAGS ASFLAGS RAMFS_DIR INCLUDES SOURCES HELPERS
export CHESTO_DIR DEVKITPRO DEVKITARM DEVKITPPC DEVKITA64 OFILES

# some makefile globals
export VPATH	+=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR	+=	$(CURDIR)/$(BUILD)

# some more files and paths that we can gather up
export LIBPATHS     +=  $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export OFILES       +=  $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export INCLUDE      +=  $(foreach dir,$(INCLUDES),-I$(dir)) \
						$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
						-I$(CURDIR)/$(BUILD)
export SRCFILES     +=  $(CFILES) $(CPPFILES) $(SFILES)

# rules for each of the targets, which includes the respective makefile fragment

ifeq (pc,$(MAKECMDGOALS))
include $(HELPERS)/Makefile.pc
endif

ifeq (wiiu,$(MAKECMDGOALS))
include $(HELPERS)/Makefile.wiiu
endif

ifeq (switch,$(MAKECMDGOALS))
include $(HELPERS)/Makefile.switch
endif

ifeq (3ds,$(MAKECMDGOALS))
include $(HELPERS)/Makefile.3ds
endif

ifeq (wii,$(MAKECMDGOALS))
include $(HELPERS)/Makefile.wii
endif

.PHONY: clean
clean:
	$(shell rm -rf build_3ds build_wii build_wiiu build_switch)
	$(shell find . -name "*.o" -exec rm {} \;)
	@echo "It is done."