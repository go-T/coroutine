CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(coroutine CXX)

set(SRC_ROOT ${PROJECT_SOURCE_DIR}/src)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(halo)

add_definitions(-D_XOPEN_SOURCE=1)
add_definitions(-DGTEST_HAS_PTHREAD=1)

add_incs(src)
add_objs(log src/coroutine/*.cpp src/log/*.cpp)

update_env()
add_executable(hello src/examples/hello.cpp ${log_OBJS})
add_executable(lock src/examples/lock.cpp ${log_OBJS})
add_executable(sem src/examples/sem.cpp ${log_OBJS})
