#
# Makefile for LibMaker
#


CC     = ppc-morphos-gcc-4
STRIP  = strip
BUILD  = $(shell svnversion)
CFLAGS = -W -Wall -nostartfiles -noixemul -O2 -Wpointer-arith -Wno-parentheses
CFLAGS += -D__NOLIBBASE__ -DUSE_INLINE_STDARG -L./ -D__SVNVERSION__=\"$(BUILD)\"

#########################################################################

OBJS = start.o main.o application.o functionlist.o functioneditor.o argumentlist.o classeditor.o
OBJS += attributelist.o methodlist.o methodeditor.o
EXE = LibMaker
LIBS = -lvstring

#########################################################################

.PHONY: all clean deps allclean

all: startmsg libvstring.a program
	@echo "Build finished."
	@list $(EXE) lformat "%n %l %t"

startmsg:
	@echo "Starting build $(BUILD)."

dep:
	$(CC) -MM *.c

clean:
	rm -f *.o *.db $(EXE)

allclean: clean
	make -C libvstring clean
	rm libvstring.a

program: $(OBJS)
	@echo "Linking executable..."
	@$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(EXE).db
	@$(STRIP) --strip-unneeded --remove-section .comment $(EXE).db -o $(EXE)
	@protect $(EXE) +E

libvstring.a:
	make -C libvstring
	copy libvstring/libvstring.a libvstring.a

%.o:
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

application.o: application.c main.h application.h functionlist.h \
 functionargument.h functioneditor.h classeditor.h attributelist.h \
 libvstring/libvstring.h
argumentlist.o: argumentlist.c main.h argumentlist.h \
 libvstring/libvstring.h
attributelist.o: attributelist.c main.h attributelist.h \
 libvstring/libvstring.h
classeditor.o: classeditor.c main.h classeditor.h functionlist.h \
 functionargument.h application.h attributelist.h methodlist.h \
 methodeditor.h libvstring/libvstring.h
functioneditor.o: functioneditor.c main.h functioneditor.h \
 functionargument.h functionlist.h application.h argumentlist.h \
 libvstring/libvstring.h
functionlist.o: functionlist.c main.h functionlist.h functionargument.h \
 libvstring/libvstring.h
main.o: main.c main.h application.h functionlist.h functionargument.h \
 functioneditor.h argumentlist.h classeditor.h attributelist.h \
 methodlist.h methodeditor.h
methodeditor.o: methodeditor.c main.h methodeditor.h functionargument.h \
 classeditor.h libvstring/libvstring.h
methodlist.o: methodlist.c main.h methodlist.h libvstring/libvstring.h
start.o: start.c main.h
