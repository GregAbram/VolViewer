OSPRAY=/work/01249/gda/maverick/git/ospray
ISPC=/work/01249/gda/maverick/git/ospray/../ispc-v1.8.1-linux/ispc
CXX=icpc

ISPC_LIST = VisRenderer.ispc 
CPP_LIST = VisRenderer.cpp SymbolRegistry.cpp 

%.io : %.ispc 
	${ISPC} -I ${OSPRAY} -I ${OSPRAY}/ospray  -I ${OSPRAY}/ospray/embree -I ${OSPRAY}/ospray/embree/include \
            -I ${OSPRAY}/ospray/embree/common -I ${OSPRAY}/ospray/embree/kernels -I ${OSPRAY}/ospray/embree/common/kernels/xeon \
            --arch=x86-64 --pic -O3 --woff --target=avx --addressing=32 --cpu=corei7-avx --opt=fast-math \
            -h $*_ispc.h -MMM $*_dev.idep -o $*.io $*.ispc

%.o : %.cpp
	${CXX}	-I ${OSPRAY} -I ${OSPRAY}/ospray  -I ${OSPRAY}/ospray/embree -I ${OSPRAY}/ospray/embree/include \
					-I ${OSPRAY}/ospray/embree/common -I ${OSPRAY}/ospray/embree/kernels -I ${OSPRAY}/ospray/embree/common/kernels/xeon \
          -DEMBREE_DISABLE_HAIR=1 -DOSPRAY_SPMD_WIDTH=8 -DOSPRAY_TARGET_AVX=1 -D__BUFFER_STRIDE__ -D__EXPORT_ALL_SYMBOLS__  \
          -D__FIX_RAYS__ -D__INTERSECTION_FILTER__ -D__TARGET_AVX__ -D__TARGET_SSE41__ -D__TARGET_SSE42__ -Dospray_EXPORTS  \
          -fPIC -DDEBUG -g -fPIC  \
					-c -o $*.o $*.cpp

libvis.so: ${ISPC_LIST:.ispc=.io} ${CPP_LIST:.cpp=.o}
	${CXX} -fPIC -fPIC -DDEBUG -g -shared -Wl,-soname,libvis.so -o libvis.so ${ISPC_LIST:.ispc=.io} ${CPP_LIST:.cpp=.o}


clean: ; rm -rf *.io *.o *.idep *_ispc.h libvis.so
