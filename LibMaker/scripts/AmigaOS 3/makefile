# Makefile for ${LIBNAME}.
# Generated with ${GENERATOR}.

CC = m68k-amigaos-gcc
CFLAGS += -s -O2 -noixemul -nostdlib
CFLAGS += -W -Wall -Wpointer-arith
CFLAGS += -Ios-include/
LD = m68k-amigaos-gcc
LDFLAGS = -nostartfiles -nostdlib -noixemul
STRIP = m68k-amigaos-strip --strip-unneeded --remove-section .comment
OUTPUT = ${LIBNAME}
OBJS = dummy.o library.o
$${FUNCOBJECTS}
$${METHODOBJECTS}

.PHONY: all clean install

all: $(OUTPUT)
	@ls -l $<

clean:
	-rm -rf $(OBJS) $(FUNCOBJS) $(METHOBJS) *.bak *.s *.db $(OUTPUT)

install: all
	cp $(OUTPUT) ${INSTDIR}
	-flushlib $(OUTPUT)

$(FUNCOBJS): %.o: %.c library.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

$(METHOBJS): %.o: %.c library.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OUTPUT).db: $(OBJS) $(FUNCOBJS) $(METHOBJS)
	@echo "Linking $@..."
	@$(LD) $(LDFLAGS) $(OBJS) $(FUNCOBJS) $(METHOBJS) $(LIBS) -o $(OUTPUT).db

$(OUTPUT): $(OUTPUT).db
	@echo "Stripping $<..."
	@$(STRIP) -o $(OUTPUT) $(OUTPUT).db

library.o: library.c library.h lib_version.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

dummy.o: dummy.c lib_version.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

$${DEPENDENCIES}
