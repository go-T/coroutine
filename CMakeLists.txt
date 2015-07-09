CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(coroutine CXX C)

set(SRC_ROOT ${PROJECT_SOURCE_DIR}/src)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(halo)

add_definitions(-D_XOPEN_SOURCE=1)
add_definitions(-DGTEST_HAS_PTHREAD=1)

#add_pkg(libuv uv)
add_subdirectory(${PROJECT_SOURCE_DIR}/deps/libuv-v1.6.1)

add_incs(src)
add_objs(couv src/couv/*.cpp src/log/*.cpp)
set(LIB_OBJS ${LIB_OBJS} ${couv_OBJS})

build_exe(hello src/examples/hello.cpp)
build_exe(lock src/examples/lock.cpp)
build_exe(sem src/examples/sem.cpp)
build_exe(echo src/examples/echo.cpp)