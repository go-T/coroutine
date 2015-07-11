CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(coroutine CXX C)

set(SRC_ROOT ${PROJECT_SOURCE_DIR}/src)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
file(GLOB CMAKE_PREFIX_PATH /opt/halo/* /opt/*)
include(halo)

add_definitions(-D_XOPEN_SOURCE=1)

add_pkg(libuv uv)
#add_subdirectory(${PROJECT_SOURCE_DIR}/deps/libuv-v1.6.1)

add_incs(src)
add_objs(couv_OBJS src/couv/*.cpp src/log/*.cpp)
set(LIB_OBJS ${LIB_OBJS} ${couv_OBJS})

build_exe(hello src/examples/hello.cpp)
build_exe(lock src/examples/lock.cpp)
build_exe(sem src/examples/sem.cpp)
build_exe(echo src/examples/echo.cpp)


enable_testing()
add_pkg(gtest gtest gtest_main)
add_objs(test_OBJS src/couv/test/*.cpp)
set(LIB_OBJS ${LIB_OBJS} ${test_OBJS})

build_exe(all_test)
add_test(all_test all_test)