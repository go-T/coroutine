CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(coroutine CXX)

set(SRC_ROOT ${PROJECT_SOURCE_DIR}/src)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(halo)

add_definitions(-D_XOPEN_SOURCE=1)
add_definitions(-DGTEST_HAS_PTHREAD=1)

add_pkg(libuv uv)

add_incs(src)
add_objs(couv src/couv/*.cpp src/log/*.cpp)

build_exe(hello src/examples/hello.cpp ${couv_OBJS})
build_exe(lock src/examples/lock.cpp ${couv_OBJS})
build_exe(sem src/examples/sem.cpp ${couv_OBJS})
build_exe(echo src/examples/echo.cpp ${couv_OBJS})