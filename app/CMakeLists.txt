###########################################################################
# Copyright 2018-2020 Konsulko Group
#
# Author: Scott Murray <scott.murray@konsulko.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###########################################################################

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OE_QMAKE_PATH_EXTERNAL_HOST_BINS $ENV{OE_QMAKE_PATH_HOST_BINS})

find_package(Qt5 COMPONENTS Core Gui QuickControls2 QuickWidgets REQUIRED)
find_package(PkgConfig REQUIRED)

qt5_add_resources(RESOURCES alexa-viewer.qrc)

PROJECT_TARGET_ADD(alexa-viewer)

add_executable(${TARGET_NAME}
	main.cpp
	afbclient.cpp
	${RESOURCES}
)

pkg_check_modules(QLIBWINDOWMGR REQUIRED qlibwindowmanager)
pkg_check_modules(QLIBHOMESCREEN REQUIRED qlibhomescreen)
pkg_check_modules(QTAPPFW REQUIRED qtappfw)
pkg_check_modules(LIBAFBWSC REQUIRED libafbwsc)

include_directories(
	${QTAPPFW_INCLUDE_DIRS}
	${LIBAFBWSC_INCLUDE_DIRS}
)

set_target_properties(${TARGET_NAME} PROPERTIES
	LABELS "EXECUTABLE"
	PREFIX ""
	COMPILE_FLAGS "${QLIBWINDOWMGR_FLAGS} ${QLIBHOMESCREEN} ${QTAPPFW_FLAGS} ${EXTRAS_CFLAGS} -DFOR_AFB_BINDING"
	LINK_FLAGS "${BINDINGS_LINK_FLAG}"
	LINK_LIBRARIES "${EXTRAS_LIBRARIES}"
	OUTPUT_NAME "${TARGET_NAME}"
)

target_link_libraries(${TARGET_NAME}
	Qt5::QuickControls2
	Qt5::QuickWidgets
	${QLIBWINDOWMGR_LIBRARIES}
	${QLIBHOMESCREEN_LIBRARIES}
	${QTAPPFW_LIBRARIES}
	${LIBAFBWSC_LIBRARIES}
	-lpthread
)
