cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../ispc)

SET(LIBS ${LIBRS} ospray)

FIND_PACKAGE(VTK 6.2 REQUIRED NO_MODULE)
INCLUDE(${VTK_USE_FILE})

SET(OSPRAY_SRCDIR "" CACHE FILEPATH "source directory of OSPRAY")
IF(OSPRAY_SRCDIR)
  INCLUDE_DIRECTORIES(${OSPRAY_SRCDIR}
											${OSPRAY_SRCDIR}/ospray
											${OSPRAY_SRCDIR}/ospray/embree
                      ${OSPRAY_SRCDIR}/ospray/embree/common
                      ${OSPRAY_SRCDIR}/ospray/include
                     )
ELSE()
  MESSAGE("OSPRAY_SRCDIR NOT OK")
ENDIF()

SET(OSPRAY_OBJDIR "" CACHE FILEPATH "object directory of OSPRAY")
IF(OSPRAY_OBJDIR)
  LINK_DIRECTORIES(${OSPRAY_OBJDIR})
ELSE()
  MESSAGE("OSPRAY_OBJDIR NOT OK")
ENDIF()

ADD_LIBRARY(common 
						SHARED 
						TransferFunction.cpp 
						Camera.cpp 
						Lights.cpp 
						RenderProperties.cpp 
						Volume.cpp 
						ColorMap.cpp
						VTIReader.cpp
						mypng.cpp)

TARGET_LINK_LIBRARIES(common ${LIBS} png ${VTK_LIBRARIES})

