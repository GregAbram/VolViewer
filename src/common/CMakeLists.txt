cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../ispc)


SET(OSPRAY_SRCDIR "" CACHE FILEPATH "source directory of OSPRAY")
IF(OSPRAY_SRCDIR)
  INCLUDE_DIRECTORIES(${OSPRAY_SRCDIR} 
											${OSPRAY_SRCDIR}/ospray/embree
                      ${OSPRAY_SRCDIR}/ospray/embree/common
                      ${OSPRAY_SRCDIR}/ospray/include
                     )
ELSE()
  MESSAGE("OSPRAY_SRCDIR NOT OK")
ENDIF()

ADD_LIBRARY(common SHARED TransferFunction.cpp Importer.cpp Camera.cpp Lights.cpp)

