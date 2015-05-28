cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../ispc
${PROJECT_SOURCE_DIR}/../common)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../common
${PROJECT_SOURCE_DIR}/../ispc)

# setup dependencies
SET(LIBS ${LIBS} ospray vis)

INCLUDE_DIRECTORIES(../common ..)
LINK_DIRECTORIES(../common.obj ../ispc)
SET(LIBS ${LIBS} common)

SET(OSPRAY_SRCDIR "" CACHE FILEPATH "source directory of OSPRAY")
IF(OSPRAY_SRCDIR)
	MESSAGE("OSPRAY_SRCDIR OK")
	INCLUDE_DIRECTORIES(${OSPRAY_SRCDIR} 
											${OSPRAY_SRCDIR}/ospray/embree
											${OSPRAY_SRCDIR}/ospray/embree/common
											${OSPRAY_SRCDIR}/ospray/embree/include
											${OSPRAY_SRCDIR}/ospray/include
										 )
ELSE()
	MESSAGE("OSPRAY_SRCDIR NOT OK")
ENDIF()

SET(OSPRAY_OBJDIR "" CACHE FILEPATH "object directory of OSPRAY")
IF(OSPRAY_OBJDIR)
	MESSAGE("OSPRAY_OBJDIR OK")
	LINK_DIRECTORIES(${OSPRAY_OBJDIR})
ELSE()
	MESSAGE("OSPRAY_OBJDIR NOT OK")
ENDIF()

SET(SRCS ${SRCS}
  Window.cpp
  Renderer.cpp
  Cinema.cpp
	mypng.cpp
  )

ADD_LIBRARY(cinema SHARED ${SRCS})

ADD_EXECUTABLE(cinema_test main.cpp)
TARGET_LINK_LIBRARIES(cinema_test cinema ${LIBS} png)

# ------------------------------------------------------------
INSTALL(TARGETS cinema DESTINATION bin)
# ------------------------------------------------------------

