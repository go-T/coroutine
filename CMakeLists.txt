CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(coroutine CXX)

set(SRC_ROOT    ${PROJECT_SOURCE_DIR}/src)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} cmake)
#include(Halo)

### add_objs base src/base/*.cpp
macro(add_objs name expr)
file(GLOB ${name}_SRCS ${expr} ${ARGN})
add_library(${name} OBJECT ${${name}_SRCS})
set(${name}_OBJS $<TARGET_OBJECTS:${name}>) 
endmacro()

### add_cxx_flags [DEBUG|MINSIZE|RELEASE|RELWITH|gnu|clang] ...
macro(add_cxx_flags)
endmacro()

set(A a;b;c;d)
list(LENGTH "${A}" list_len)
message("list_len ${list_len}")

add_objs(log src/coroutine/*.cpp src/log/*.cpp)
add_executable(coroutine src/main.cpp ${log_OBJS})
