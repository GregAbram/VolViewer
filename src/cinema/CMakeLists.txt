cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/../common)

# setup dependencies
SET(LIBS ${LIBS} ospray)

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../common.obj)
SET(LIBS ${LIBS} common)

SET(OSPRAY_SRCDIR "" CACHE FILEPATH "source directory of OSPRAY")
IF(OSPRAY_SRCDIR)
	INCLUDE_DIRECTORIES(${OSPRAY_SRCDIR} 
											${OSPRAY_SRCDIR}/ospray
											${OSPRAY_SRCDIR}/ospray/embree
											${OSPRAY_SRCDIR}/ospray/embree/common
											${OSPRAY_SRCDIR}/ospray/embree/include
											${OSPRAY_SRCDIR}/ospray/include
										 )
ELSE()
	MESSAGE("OSPRAY_SRCDIR NOT OK")
ENDIF()

SET(WITH_DISPLAY OFF CACHE BOOL "set to TRUE to show images as they are generated")
IF (${WITH_DISPLAY})
	SET(WITH_DISPLAY_WINDOW 1)
	FIND_PACKAGE(OpenGL)
	IF (NOT OPENGL_FOUND) 
		MESSAGE("Can't find OpenGL to use with DISPLAY... You will need to turn
WITH_DISPLAY or add the necessary stuff to CMAKE_CXX_FLAGS and CMAKE_EXE_LINKER_FLAGS")
	ELSE()
		INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS})
	ENDIF()
ELSE()
	SET(WITH_DISPLAY_WINDOW 0)
ENDIF()

SET(OSPRAY_OBJDIR "" CACHE FILEPATH "object directory of OSPRAY")
IF(OSPRAY_OBJDIR)
	LINK_DIRECTORIES(${OSPRAY_OBJDIR})
ELSE()
	MESSAGE("OSPRAY_OBJDIR NOT OK")
ENDIF()

SET(SRCS ${SRCS}
  CinemaWindow.cpp
  Renderer.cpp
  Cinema.cpp
	Variable.cpp
	util.cpp
  )

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/cinema_cfg.h.in ${PROJECT_BINARY_DIR}/cinema_cfg.h)

ADD_LIBRARY(cinema SHARED ${SRCS})
TARGET_LINK_LIBRARIES(cinema ${LIBS} png)

ADD_EXECUTABLE(cinema_test main.cpp)
TARGET_LINK_LIBRARIES(cinema_test cinema ${LIBS} ${OPENGL_LIBRARIES})

# ------------------------------------------------------------
INSTALL(TARGETS cinema DESTINATION bin)
# ------------------------------------------------------------

