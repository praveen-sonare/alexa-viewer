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
find_package(Qt5Gui ${QT_MIN_VERSION} CONFIG REQUIRED Private)
find_package(PkgConfig REQUIRED)
find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)

qt5_add_resources(RESOURCES alexa-viewer.qrc)

PROJECT_TARGET_ADD(alexa-viewer)

add_custom_command(
	OUTPUT  agl-shell-desktop-client-protocol.h
	COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header
	< ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell-desktop.xml
	> ${CMAKE_SOURCE_DIR}/app/agl-shell-desktop-client-protocol.h
	DEPENDS ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell-desktop.xml
)


add_custom_command(
	OUTPUT  ${CMAKE_BINARY_DIR}/app/agl-shell-desktop-client-protocol.h
	COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header
	< ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell-desktop.xml
	> ${CMAKE_SOURCE_DIR}/app/agl-shell-desktop-client-protocol.h
	DEPENDS ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell-desktop.xml
)

add_custom_command(
	OUTPUT  agl-shell-desktop-protocol.c
	COMMAND ${WAYLAND_SCANNER_EXECUTABLE} code
	< ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell-desktop.xml
	> ${CMAKE_BINARY_DIR}/app/agl-shell-desktop-protocol.c
	DEPENDS ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell-desktop.xml
)

add_executable(${TARGET_NAME}
	main.cpp
	agl-shell-desktop-protocol.c
	agl-shell-desktop-client-protocol.h
	shell-desktop.cpp
	afbclient.cpp
	${RESOURCES}
)

pkg_check_modules(QLIBHOMESCREEN REQUIRED qlibhomescreen)
pkg_check_modules(QTAPPFW REQUIRED qtappfw-voice-metadata)
pkg_check_modules(LIBAFBWSC REQUIRED libafbwsc)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)

include_directories(
	include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
	${QTAPPFW_INCLUDE_DIRS}
	${LIBAFBWSC_INCLUDE_DIRS}
)

set_target_properties(${TARGET_NAME} PROPERTIES
	LABELS "EXECUTABLE"
	PREFIX ""
	COMPILE_FLAGS "${QLIBHOMESCREEN} ${QTAPPFW_FLAGS} ${EXTRAS_CFLAGS} -DFOR_AFB_BINDING"
	LINK_FLAGS "${BINDINGS_LINK_FLAG}"
	LINK_LIBRARIES "${EXTRAS_LIBRARIES}"
	OUTPUT_NAME "${TARGET_NAME}"
)

target_link_libraries(${TARGET_NAME}
	Qt5::QuickControls2
	Qt5::QuickWidgets
	${QLIBHOMESCREEN_LIBRARIES}
	${QTAPPFW_LIBRARIES}
	${LIBAFBWSC_LIBRARIES}
	${WAYLAND_CLIENT_LIBRARIES}
	-lpthread
)
