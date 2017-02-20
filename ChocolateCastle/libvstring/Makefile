###########################################################################
# This makefile is a very generic one. It tries to identify both, the host
# and the target operating system for which YAM should be compiled.
# However, this auto-detection can be easily overridden by directly
# specifying an option on the commandline while calling 'make'.
#
# Example:
#
# # to explicitly compile for AmigaOS3
# > make OS=os3
#
# # to compile for AmigaOS4 but with debugging
# > make OS=os4 DEBUG=
#

#############################################
# find out the HOST operating system
# on which this makefile is run
HOST ?= $(shell uname)
ifeq ($(HOST), AmigaOS)
  ifeq ($(shell uname -m), powerpc)
    HOST = AmigaOS4
  endif
  ifeq ($(shell uname -m), ppc)
    HOST = AmigaOS4
  endif
endif

# if no host is identifed (no uname tool)
# we assume a AmigaOS build
ifeq ($(HOST),)
  HOST = AmigaOS
endif

#############################################
# now we find out the target OS for
# which we are going to compile YAM in case
# the caller didn't yet define OS himself
ifndef (OS)
  ifeq ($(HOST), AmigaOS4)
    OS = os4
  else
  ifeq ($(HOST), AmigaOS)
    OS = os3
  else
  ifeq ($(HOST), MorphOS)
    OS = mos
  else
  ifeq ($(HOST), AROS)
    # now we find out which CPU system aros will be used
    ifeq ($(shell uname -m), powerpc)
      OS = aros-ppc
    endif
    ifeq ($(shell uname -m), ppc)
      OS = aros-ppc
    endif
    ifeq ($(shell uname -m), i386)
      OS = aros-i386
    endif
    ifeq ($(shell uname -m), i686)
      OS = aros-i686
    endif
    ifeq ($(shell uname -m), x86_64)
      OS = aros-x86_64
    endif
  else
    OS = os4
  endif
  endif
  endif
  endif
endif

#############################################
# define common commands we use in this
# makefile. Please note that each of them
# might be overridden on the commandline.

# common commands
FLEX    = flex
FC      = flexcat
EXPR    = expr
DATE    = date
RM      = delete force
RMDIR   = delete force all
MKDIR   = makedir
CHMOD   = protect FLAGS=rwed
SED     = sed
CP      = copy
CC      = gcc
AR      = ar
RANLIB  = ranlib
STRIP   = strip
OBJDUMP = objdump

# override commands for native builds
ifeq ($(HOST), AmigaOS4)
  # AmigaOS4 host
  RM    = delete force
  RMDIR = delete force all
  MKDIR = makedir
  DATE  = gdate
else
ifeq ($(HOST), AmigaOS)
  # AmigaOS3 host
  RM    = delete force
  RMDIR = delete force all
  MKDIR = makedir
else
ifeq ($(HOST), MorphOS)
  # MorphOS host
  RM    = delete force
  RMDIR = delete force all
  MKDIR = makedir
endif
endif
endif

# path definitions
CDUP  = /
CDTHIS=

# override some variables for non-native builds (cross-compiler)
ifneq ($(HOST), AmigaOS)
ifneq ($(HOST), AmigaOS4)
ifneq ($(HOST), MorphOS)

  # when we end up here this is either a unix or Aros host
  # so lets use unix kind of commands
  RM    = rm -f
  RMDIR = rm -rf
  MKDIR = mkdir -p
  CHMOD = chmod 755
  CP    = cp -f

  CDUP  = ../
  CDTHIS= ./

endif
endif
endif

###########################################################################
# CPU and DEBUG can be defined outside, defaults to above
# using e.g. "make DEBUG= CPU=-mcpu=603e" produces optimized non-debug
# PPC-603e version
#
# OPTFLAGS are disabled by DEBUG normally!
#
# ignored warnings are:
# none - because we want to compile with -Wall all the time

# Common Directories
PREFIX   = $(CDTHIS)
OBJDIR   = .obj_$(OS)
BINDIR   = bin_$(OS)
VPATH    = $(OBJDIR)
DEPFILE  = Makefile.dep
DESTDIR  = MUI:Libs/MUI

# target definition
TARGET    = $(BINDIR)/libvstring.a

# Common compiler/linker flags
WARN     = -W -Wall -Wwrite-strings -Wpointer-arith -Wsign-compare
OPTFLAGS = -O3 -fomit-frame-pointer
DEBUG    = -DDEBUG -O0
DEBUGSYM = -g -gstabs
CFLAGS   = -I. -I../../include $(CPU) $(WARN) $(OPTFLAGS) \
           $(DEBUG) $(DEBUGSYM) $(USER_CFLAGS) -c
LDFLAGS  = $(CPU) $(DEBUGSYM)
LDLIBS   =

###########################################################################
# object files definition
#

OBJS = clonestrarray.o \
       fmtlen.o \
       fmtnew.o \
       fmtnput.o \
       fmtput.o \
       freestrarray.o \
       strarraylen.o \
       strcopy.o \
       strlen.o \
       strncopy.o \
       strnew.o \
       vfmtnew.o


# different options per target OS
ifeq ($(OS), os4)

  ##############################
  # AmigaOS4

  # Compiler/link/strip commands
  ifneq ($(HOST), AmigaOS4)
    CC      = ppc-amigaos-gcc
    AR      = ppc-amigaos-ar
    RANLIB  = ppc-amigaos-ranlib
    STRIP   = ppc-amigaos-strip
    OBJDUMP = ppc-amigaos-objdump
  endif

  # Compiler/Linker flags
  CRT      = newlib
  CPU      = -mcpu=powerpc
  WARN     += -Wdeclaration-after-statement -Wdisabled-optimization
  REDEFINE = -DCoerceMethod=ICoerceMethod -DDoMethod=IDoMethod \
             -DDoSuperMethod=IDoSuperMethod -DDoSuperMethodA=IDoSuperMethodA
  CFLAGS   += -mcrt=$(CRT) -D__USE_INLINE__ -D__NEW_TIMEVAL_DEFINITION_USED__ \
              $(REDEFINE) -Wa,-mregnames
  LDFLAGS  += -mcrt=$(CRT)

else
ifeq ($(OS), os3)

  ##############################
  # AmigaOS3

  # Compiler/link/strip commands
  ifneq ($(HOST), AmigaOS)
    CC      = m68k-amigaos-gcc
    AR      = m68k-amigaos-ar
    RANLIB  = m68k-amigaos-ranlib
    STRIP   = m68k-amigaos-strip
    OBJDUMP = m68k-amigaos-objdump
  endif

  # Compiler/Linker flags
  CPU	    = -m68020-60 -msoft-float
  CFLAGS  += -mcrt=clib2 -DNO_INLINE_STDARG -D__amigaos3__
  LDFLAGS += -mcrt=clib2

else
ifeq ($(OS), mos)

  ##############################
  # MorphOS

  # Compiler/link/strip commands
  ifneq ($(HOST), MorphOS)
    CC      = ppc-morphos-gcc
    AR      = ppc-morphos-ar
    RANLIB  = ppc-morphos-ranlib
    STRIP   = ppc-morphos-strip
    OBJDUMP = ppc-morphos-objdump
  endif

  # Compiler/Linker flags
  CPU     = -mcpu=powerpc
  CFLAGS  += -noixemul -DNO_PPCINLINE_STDARG
  LDFLAGS += -noixemul

else
ifeq ($(OS), aros-i386)

  ##############################
  # AROS (i386)

  ifneq ($(HOST), AROS)
    CC      = i386-aros-gcc
    STRIP   = i386-aros-strip
    OBJDUMP = i386-aros-objdump
  endif

  # Compiler/Linker flags
  OPTFLAGS = -O2 -fomit-frame-pointer
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG

else
ifeq ($(OS), aros-ppc)

  ##############################
  # AROS (PPC)

  ifneq ($(HOST), AROS)
    CC      = ppc-aros-gcc
    STRIP   = ppc-aros-strip
    OBJDUMP = ppc-aros-objdump
  endif

  # Compiler/Linker flags
  OPTFLAGS = -O2 -fomit-frame-pointer
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG

else
ifeq ($(OS), aros-x86_64)

  ##############################
  # AROS (x86_64)

  ifneq ($(HOST), AROS)
    CC      = x86_64-aros-gcc
    STRIP   = x86_64-aros-strip
    OBJDUMP = x86_64-aros-objdump
  endif

  # Compiler/Linker flags
  OPTFLAGS = -O2 -fomit-frame-pointer
  CFLAGS += -Wno-pointer-sign -DNO_INLINE_STDARG

endif
endif
endif
endif
endif
endif

# main target
.PHONY: all
ifeq ($(wildcard $(DEPFILE)),$(DEPFILE))
# great, we have a dependecies file, let's make our target
all: $(BINDIR) $(OBJDIR) $(TARGET)
else
# no dependecies, create it and then call make again
all: depend
	@make --no-print-directory all
endif

# make the object directories
$(OBJDIR):
	@echo "  MK $@"
	@$(MKDIR) $(OBJDIR)

# make the binary directories
$(BINDIR):
	@echo "  MK $@"
	@$(MKDIR) $(BINDIR)

# for compiling single .c files
$(OBJDIR)/%.o: %.c
	@echo "  CC $<"
	@$(CC) $(CFLAGS) $< -o $@

# for linking the target
$(TARGET): $(addprefix $(OBJDIR)/,$(OBJS))
	@echo "  AR $@"
	@$(AR) rcs $@ $(addprefix $(OBJDIR)/,$(OBJS))
	@$(RANLIB) $@

.PHONY: clean
clean:
	-$(RM) $(TARGET)
	-$(RM) $(addprefix $(OBJDIR)/,$(OBJS))

.PHONY: distclean
distclean: clean
	-$(RMDIR) $(OBJDIR)
	-$(RMDIR) $(BINDIR)

.PHONY: help
help:
	@echo "Cleaning targets:"
	@echo "  clean     - Cleanup working directory for clean compile"
	@echo "  distclean - Also cleanup autogenerated files"
	@echo ""
	@echo "Other generic targets:"
	@echo "  all      - Build $(TARGET)"
	@echo "  catalogs - Build all available catalogs"
	@echo ""
	@echo "Install targets:"
	@echo "  install  - Install $(TARGET) into $(DESTDIR)"
	@echo ""
	@echo "Parameters:"
	@echo "  make OS=os3|os4|mos|aros-i386|aros-i686|aros-ppc|aros-x86_64"
	@echo "  make DEBUG= : build $(TARGET) without debugging information"
	@echo ""

## DEPENDENCY GENERATION ##############

.PHONY: depend
depend:
	@echo "  MK $(DEPFILE)"
	@echo "# AUTOGENERATED! DO NOT EDIT!!!" >$(DEPFILE)
	@$(CC) -MM -MG $(CFLAGS) $(wildcard *.c) >>$(DEPFILE)
	@echo "# AUTOGENERATED! DO NOT EDIT!!!" >>$(DEPFILE)
	@$(SED) -i 's,^\(.*\)\.o:,$$\(OBJDIR\)/\1.o:,g' $(DEPFILE)

# include dependencies file
-include $(DEPFILE)
