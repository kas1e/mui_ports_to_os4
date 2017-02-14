
CC= ppc-amigaos-gcc
CXX= ppc-amigaos-g++
AS= ppc-amigaos-as
AR= ppc-amigaos-ar rcs
LD= ppc-amigaos-ld

# do NOT include optimization level here
CFLAGS = -gstabs -D__USE_INLINE__ -D__MORPHOS__ -DHAVE_CONFIG_H -D_NO_PPCINLINE -DHAVE_CONFIG_H -Wno-write-strings -Wall -fstrength-reduce -mfused-madd -frerun-loop-opt -fstrict-aliasing -noixemul -ffast-math -mfused-madd -fomit-frame-pointer -mcpu=750
CPPFLAGS = -gstabs -DHAVE_CONFIG_H -D_NO_PPCINLINE -DHAVE_CONFIG_H -Wno-write-strings -Wall -fstrength-reduce -mfused-madd -frerun-loop-opt -fstrict-aliasing -noixemul -ffast-math -mfused-madd -fomit-frame-pointer -mcpu=750

#CFLAGS = -D__USE_INLINE__ -D__MORPHOS__ -DHAVE_CONFIG_H -D_NO_PPCINLINE -DHAVE_CONFIG_H -Wno-write-strings -Wall -fstrength-reduce -mfused-madd -frerun-loop-opt -fstrict-aliasing -noixemul -ffast-math -mfused-madd -fomit-frame-pointer -mcpu=750
#CPPFLAGS = -DHAVE_CONFIG_H -D_NO_PPCINLINE -DHAVE_CONFIG_H -Wno-write-strings -Wall -fstrength-reduce -mfused-madd -frerun-loop-opt -fstrict-aliasing -noixemul -ffast-math -mfused-madd -fomit-frame-pointer -mcpu=750



CFLAGS_LESS =  -Wall -fstrength-reduce -mfused-madd -frerun-loop-opt -fexpensive-optimizations -fschedule-insns2 -noixemul -ffast-math -mfused-madd -fomit-frame-pointer -mcpu=7507
GLOBAL_INCLUDES = -I.. -I. -I../poppler -Iincludes/ -I../../ -I../../../../libs/cairo/cairo-src/MorphOS/include/cairo -I../../../../libs/cairo/cairo-src/MorphOS/include \
-I../../../../libs/freetype/library/include -I../../../../libs/fontconfig/MorphOS/include -I../../../../libs/freetype/include
#LFLAGS=-gstabs
LFLAGS=





