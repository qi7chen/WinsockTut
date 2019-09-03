# Copyright (C) 2012-present prototyped.cn All rights reserved.
# Distributed under the terms and conditions of the Apache License. 
# See accompanying files LICENSE.

project(echo)

file(GLOB PROJECT_HEADER_FILES *.h)
file(GLOB PROJECT_SOURCE_FILES *.cpp)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
add_executable(echo WIN32 ${PROJECT_HEADER_FILES} ${PROJECT_SOURCE_FILES})
target_link_libraries(echo libwinet ws2_32 mswsock)
