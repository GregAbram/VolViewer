ISPC=/Users/gda/OSPRay/ispc-v1.8.1-osx/ispc
OSPRAY_SRCDIR=/Users/gda/OSPRay/ospray
OSPRAY_OBJDIR=/Users/gda/OSPRay/obj
CINEMA=/Users/gda/VolViewer/src
DEBUG=-g -DDEBUG
CXX=g++ ${DEBUG}

# Only if Cinema library was built with X11/OpenGL window for showing frames as they are rendered

GLX=-L/opt/X11/lib -lX11 -lgl

# Choose one of these depending on mac (.dylib) or linux (.so)

# LIBNAME=libperlin.so
LIBNAME=libperlin.dylib


OSPRAY_INCDIRS = \
	-I${OSPRAY_SRCDIR} \
	-I${OSPRAY_SRCDIR}/ospray/include \
	-I${OSPRAY_SRCDIR}/ospray/embree \
	-I${OSPRAY_SRCDIR}/ospray/embree/common \
	-I${OSPRAY_SRCDIR}/ospray/embree 

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

all: sim dump

sim: sim.o ${LIBNAME}
	${CXX} -o sim sim.o -lperlin ${LIBDIRS} ${LIBS} 

dump: dump.o ${LIBNAME}
	${CXX} -o dump dump.o -lperlin ${LIBDIRS} ${LIBS}

perlin.io perlin_ispc.h: perlin.ispc 
	${ISPC} --arch=x86-64 --pic -O3 --woff --target=avx --addressing=32 \
					--cpu=corei7-avx --opt=fast-math \
					-h perlin_ispc.h -MMM perlin_dev.idep -o perlin.io perlin.ispc

%.o : %.cpp
	${CXX} ${INCDIRS} -fPIC -DDEBUG -g -fPIC -c -o $*.o $*.cpp

${LIBNAME}: perlin.io 
	# ${CXX} -fPIC -DDEBUG -g -shared -Wl,-soname,${LIBNAME} -o ${LIBNAME} perlin.io tasksys.cpp
	${CXX} -dynamiclib -o ${LIBNAME} perlin.io tasksys.cpp ${LIBDIRS} ${LIBS}

sim.o : perlin_ispc.h
dump.o : perlin_ispc.h
clean: ; rm -rf *.io *.o *.idep *_ispc.h ${LIBNAME} sim dump
