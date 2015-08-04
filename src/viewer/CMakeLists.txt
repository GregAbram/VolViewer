## ======================================================================== ##
## Copyright 2009-2014 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

cmake_minimum_required(VERSION 2.8)


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../ispc ${PROJECT_SOURCE_DIR}/../common)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../common.obj ${PROJECT_SOURCE_DIR}/../ispc)

# setup dependencies
SET(LIBS ${LIBS} ospray)

FIND_PACKAGE(Qt4 REQUIRED) 
SET(QT_USE_QTOPENGL TRUE)
INCLUDE(${QT_USE_FILE})
SET(LIBS ${LIBS} ${QT_LIBRARIES})

FIND_PACKAGE(VTK 6.2 REQUIRED NO_MODULE)
INCLUDE(${VTK_USE_FILE})

SET(LIBS ${LIBS} common)

FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS})
SET(LIBS ${LIBS} ${OPENGL_LIBRARIES})

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

SET(SRCS ${SRCS}
  main.cpp
  VColorMap.cpp
  LinearTransferFunctionWidget.cpp
  QOSPRayWindow.cpp
  TransferFunctionEditor.cpp
  VolumeViewer.cpp
	SlicesEditor.cpp
	IsosEditor.cpp
	RenderPropertiesEditor.cpp
	CameraEditor.cpp
	TimeEditor.cpp
  )

SET(MOC_HEADERS
  LinearTransferFunctionWidget.h
  TransferFunctionEditor.h
	SlicesEditor.h
	IsosEditor.h
	RenderPropertiesEditor.h
  VolumeViewer.h
	CameraEditor.h
	TimeEditor.h
  )

QT4_WRAP_CPP(MOC_OUTFILES ${MOC_HEADERS})

ADD_EXECUTABLE(viewer ${SRCS} ${MOC_OUTFILES})
TARGET_LINK_LIBRARIES(viewer ${LIBS} ${VTK_LIBRARIES})
# ------------------------------------------------------------
INSTALL(TARGETS viewer DESTINATION bin)
# ------------------------------------------------------------

