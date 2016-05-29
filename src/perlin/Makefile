ifdef WITH_ISPC
	USE_ISPC=1
else
	USE_ISPC=0
endif

# ONLY one of these!

MAC=1
#LUNIX=1

ifdef LINUX
ISPC=/work/01249/gda/maverick/VolViewer/ispc-v1.8.1-linux/ispc
OSPRAY_SRCDIR=/work/01249/gda/maverick/VolViewer/OSPRay/
OSPRAY_OBJDIR=/work/01249/gda/maverick/VolViewer/OSPRay/obj
CINEMA=/work/01249/gda/maverick/VolViewer/VolViewer/src
else
ISPC=/Users/gda/OSPRay/ispc-v1.8.1-osx/ispc
OSPRAY_SRCDIR=/Users/gda/OSPRay/
OSPRAY_OBJDIR=/Users/gda/OSPRay/obj
CINEMA=/Users/gda/VolViewer/src
endif

DEBUG=-g -DDEBUG
CXX=g++ ${DEBUG}

# Only if Cinema library was built with X11/OpenGL window for showing frames as they are rendered

GLX=-L/opt/X11/lib -lX11 -lGL

# Choose one of these depending on mac (.dylib) or linux (.so)

ifdef LINUX
	LIBNAME=libperlin.so
else
	LIBNAME=libperlin.dylib
endif

OSPRAY_INCDIRS = \
	-I${OSPRAY_SRCDIR}/ospray \
	-I${OSPRAY_SRCDIR}/ospray/ospray/include \
	-I${OSPRAY_SRCDIR}/ospray/ospray/embree \
	-I${OSPRAY_SRCDIR}/ospray/ospray/embree/common \
	-I${OSPRAY_SRCDIR}/ospray/ospray/embree 

CINEMA_INCDIRS = \
	-I${CINEMA}/common \
	-I${CINEMA}/cinema \
	-I${CINEMA}/cinema.obj \
	-I${CINEMA}

INCDIRS = ${OSPRAY_INCDIRS} ${CINEMA_INCDIRS}

OSPRAY_LIBDIRS = \
	-L${OSPRAY_OBJDIR}

CINEMA_LIBDIRS = \
	-L${CINEMA}/common.obj \
	-L${CINEMA}/cinema.obj \

LIBDIRS = -L. ${OSPRAY_LIBDIRS} ${CINEMA_LIBDIRS}

OSPRAY_LIBS = -lospray -lospray_embree
CINEMA_LIBS = -lcinema  -lcommon 

LIBS =  ${OSPRAY_LIBS} ${CINEMA_LIBS} ${GLX} -lpng -lpthread

all: sim dump tdump

%.o : %.cpp
	${CXX} ${INCDIRS} -DWITH_ISPC=$(USE_ISPC) -fPIC -DDEBUG -g -fPIC -c -o $*.o $*.cpp

sim: sim.o ${LIBNAME}
	${CXX} -o sim sim.o -lperlin ${LIBDIRS} ${LIBS} 

tdump: tdump.o ${LIBNAME}
	${CXX} -o tdump tdump.o -lperlin ${LIBDIRS} ${LIBS}

dump: dump.o ${LIBNAME}
	${CXX} -o dump dump.o -lperlin ${LIBDIRS} ${LIBS}

ifdef WITH_ISPC
${LIBNAME}:  perlin.ispc
	${ISPC} --arch=x86-64 --pic -O3 --woff --target=avx --addressing=32 \
					--cpu=corei7-avx --opt=fast-math \
					-h perlin_ispc.h -MMM perlin_dev.idep -o perlin.io perlin.ispc
ifdef LINUX
	${CXX} -fPIC -DWITH_ISPC=$(USE_ISPC) -DDEBUG -g -shared -Wl,-soname,${LIBNAME} -o ${LIBNAME} perlin.io tasksys.cpp
else
	${CXX} -DWITH_ISPC=$(ISPC) -dynamiclib -o ${LIBNAME} perlin.io tasksys.cpp ${LIBDIRS} ${LIBS}
endif

else

${LIBNAME}: perlin.cpp
ifdef LINUX
	${CXX} -fPIC -DDEBUG -DWITH_ISPC=$(USE_ISPC) -g -shared -Wl,-soname,${LIBNAME} -o ${LIBNAME} perlin.cpp tasksys.cpp
else
	${CXX} -dynamiclib -DWITH_ISPC=$(USE_ISPC) -o ${LIBNAME} perlin.io tasksys.cpp ${LIBDIRS} ${LIBS}
endif

endif

sim.o : ${LIBNAME}
dump.o : ${LIBNAME}
clean: ; rm -rf *.io *.o *.idep *_ispc.h ${LIBNAME} sim dump tdump
