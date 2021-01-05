project(c-lws-websocket-server C)
cmake_minimum_required(VERSION 2.8.9)
include(CheckCSourceCompiles)

set(SAMP c-lws-websocket-server)
set(SRCS main.c server/server.h server/serverp.h server/serverp.c main.h)
add_executable(${SAMP} ${SRCS})

# Search for programs only in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers only in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


find_package(libwebsockets CONFIG REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${LWS_CMAKE_DIR})

if (websockets_shared)
    target_link_libraries(${SAMP} websockets_shared ${LIBWEBSOCKETS_DEP_LIBS})
    add_dependencies(${SAMP} websockets_shared)
else()
    target_link_libraries(${SAMP} websockets ${LIBWEBSOCKETS_DEP_LIBS})
endif()


find_package(json-c REQUIRED)
target_link_libraries(${SAMP} websockets json-c)