OSPRAY=/work/01249/gda/maverick/git/ospray
CINEMA=/work/01249/gda/maverick/VolViewer.test/VolViewer/src

OSPRAY_INCDIRS = \
	-I${OSPRAY} \
	-I${OSPRAY}/ospray/include \
	-I${OSPRAY}/ospray/embree \
	-I${OSPRAY}/ospray/embree/common \
	-I${OSPRAY}/ospray/embree 

CINEMA_INCDIRS = \
	-I${CINEMA}/common \
	-I${CINEMA}/cinema \
	-I${CINEMA}/ispc \
	-I${CINEMA}

INCDIRS = ${OSPRAY_INCDIRS} ${CINEMA_INCDIRS}

OSPRAY_LIBDIRS = \
	-L${OSPRAY}/release.64

CINEMA_LIBDIRS = \
	-L${CINEMA}/common.obj \
	-L${CINEMA}/cinema.obj \
	-L${CINEMA}/ispc \

LIBDIRS = -L. ${OSPRAY_LIBDIRS} ${CINEMA_LIBDIRS}

OSPRAY_LIBS = -lospray -lospray_embree
CINEMA_LIBS = -lcinema  -lcommon 

LIBS = -lperlin ${OSPRAY_LIBS} ${CINEMA_LIBS} -lpng -lpthread

ISPC=/work/01249/gda/maverick/git/ospray/../ispc-v1.8.1-linux/ispc
CXX=g++ -g

all: sim dump

sim: sim.o libperlin.so 
	${CXX} -o sim sim.o ${LIBDIRS} ${LIBS}

dump: dump.o libperlin.so 
	${CXX} -o dump dump.o ${LIBDIRS} ${LIBS}

perlin.io perlin_ispc.h: perlin.ispc 
	${ISPC} --arch=x86-64 --pic -O3 --woff --target=avx --addressing=64 \
					--cpu=corei7-avx --opt=fast-math \
					-h perlin_ispc.h -MMM perlin_dev.idep -o perlin.io perlin.ispc

%.o : %.cpp
	${CXX} ${INCDIRS} -fPIC -DDEBUG -g -fPIC -c -o $*.o $*.cpp

libperlin.so: perlin.io 
	${CXX} -fPIC -fPIC -DDEBUG -g -shared -Wl,-soname,libperlin.so -o libperlin.so perlin.io tasksys.cpp

sim.o : perlin_ispc.h
dump.o : perlin_ispc.h

clean: ; rm -rf *.io *.o *.idep *_ispc.h libperlin.so sim dump
