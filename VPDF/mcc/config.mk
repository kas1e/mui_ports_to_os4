
ifeq "$(shell test -f /gg/bin/ppc-amigaos-gcc-3.4.6 && echo YES)" "YES"
	CC3 = ppc-amigaos-gcc-3.4.6 -mabi=altivec -maltivec
else
	CC3=ppc-amigaos-gcc
endif

CC= ppc-amigaos-gcc
AS= ppc-amigaos-as
AR= ppc-amigaos-ar rcs
LD= ppc-amigaos-ld

CFLAGS = -gstabs -O3 -Wall -D__USE_INLINE__ -D__MORPHOS__ -fstrength-reduce -mfused-madd -frerun-loop-opt -fexpensive-optimizations -fstrict-aliasing -fschedule-insns2 -noixemul -ffast-math -mfused-madd -fomit-frame-pointer -mcpu=750
CFLAGS_LESS = -gstabs -O2 -Wall -D__USE_INLINE__ -D__MORPHOS__  -fstrength-reduce -mfused-madd -frerun-loop-opt -fexpensive-optimizations -fschedule-insns2 -noixemul -ffast-math -mfused-madd -fomit-frame-pointer -mcpu=750
INCLUDES =  -I.. -I../exif
LFLAGS=-gstabs

#CFLAGS = -O3 -Wall -D__USE_INLINE__ -D__MORPHOS__ -fstrength-reduce -mfused-madd -frerun-loop-opt -fexpensive-optimizations -fstrict-aliasing -fschedule-insns2 -noixemul -ffast-math -mfused-madd -fomit-frame-pointer -mcpu=750
#CFLAGS_LESS = -O2 -Wall -D__USE_INLINE__ -D__MORPHOS__  -fstrength-reduce -mfused-madd -frerun-loop-opt -fexpensive-optimizations -fschedule-insns2 -noixemul -ffast-math -mfused-madd -fomit-frame-pointer -mcpu=750
#INCLUDES =  -I.. -I../exif
#LFLAGS=

# For building with the public SDK
INCLUDES += -I../include

