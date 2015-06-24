cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/../common)

# setup dependencies
SET(LIBS ${LIBS} ospray)

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../common.obj)
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

SET(OPENGL_INCLUDE_DIR "" CACHE STRING "include directory X11/OpenGL/glx (optional)")
SET(OPENGL_LINK "" CACHE STRING "link arguments for X11/OpenGL/glx (optional)")

IF ((${OPENGL_LINK} STREQUAL "") OR (${OPENGL_INCLUDE_DIR} STREQUAL ""))
	SET(WITH_OPENGL FALSE)
ELSE()
	SET(WITH_OPENGL TRUE)
	SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${OPENGL_INCLUDE_DIR})
ENDIF()

SET(OSPRAY_OBJDIR "" CACHE FILEPATH "object directory of OSPRAY")
IF(OSPRAY_OBJDIR)
	MESSAGE("OSPRAY_OBJDIR OK")
	LINK_DIRECTORIES(${OSPRAY_OBJDIR})
ELSE()
	MESSAGE("OSPRAY_OBJDIR NOT OK")
ENDIF()

SET(SRCS ${SRCS}
  CinemaWindow.cpp
  Renderer.cpp
  Cinema.cpp
	mypng.cpp
  )

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/cinema_cfg.h.in ${PROJECT_BINARY_DIR}/cinema_cfg.h)

ADD_LIBRARY(cinema SHARED ${SRCS})
TARGET_LINK_LIBRARIES(cinema ${LIBS} png ${OPENGL_LINK})

ADD_EXECUTABLE(cinema_test main.cpp)
TARGET_LINK_LIBRARIES(cinema_test cinema ${LIBS} ${OPENGL_LINK})

# ------------------------------------------------------------
INSTALL(TARGETS cinema DESTINATION bin)
# ------------------------------------------------------------

