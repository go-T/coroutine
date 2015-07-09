
add_definitions(-DPACKAGE_NAME=\"libuv\" )
add_definitions(-DPACKAGE_TARNAME=\"libuv\" )
add_definitions(-DPACKAGE_VERSION=\"1.6.1\" )
add_definitions(-DPACKAGE_STRING=\"libuv\ 1.6.1\" )
add_definitions(-DPACKAGE_BUGREPORT=\"https://github.com/libuv/libuv/issues\" )
add_definitions(-DPACKAGE=\"libuv\" )
add_definitions(-DVERSION=\"1.6.1\" )
add_definitions(-DSTDC_HEADERS=1 )
add_definitions(-DHAVE_SYS_TYPES_H=1 )
add_definitions(-DHAVE_SYS_STAT_H=1 )
add_definitions(-DHAVE_STDLIB_H=1 )
add_definitions(-DHAVE_STRING_H=1 )
add_definitions(-DHAVE_MEMORY_H=1 )
add_definitions(-DHAVE_STRINGS_H=1 )
add_definitions(-DHAVE_INTTYPES_H=1 )
add_definitions(-DHAVE_STDINT_H=1 )
add_definitions(-DHAVE_UNISTD_H=1 )
add_definitions(-DHAVE_DLFCN_H=1 )
add_definitions(-DHAVE_LIBDL=1 )
add_definitions(-DHAVE_LIBPTHREAD=1 )

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin") 
	add_definitions(-D_DARWIN_USE_64_BIT_INODE=1 )
	add_definitions(-D_DARWIN_UNLIMITED_SELECT=1)
	add_definitions(-D_DARWIN_C_SOURCE=1)
    set(UV_PLATFORM_SRC
        src/unix/fsevents.c
        src/unix/kqueue.c
        src/unix/darwin.c
        src/unix/darwin-proctitle.c
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_definitions(-DHAVE_LIBNSL=1 )
	add_definitions(-DHAVE_LIBRT=1 )
	add_definitions(-D_GNU_SOURCE)
    set(UV_PLATFORM_SRC
        src/unix/linux-core.c
        src/unix/linux-inotify.c
        src/unix/linux-syscalls.c
    )
else()
    message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} not support")
endif()




add_incs(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_objs(uv
  src/fs-poll.c
  src/inet.c
  src/threadpool.c
  src/uv-common.c
  src/version.c
  src/unix/async.c
  src/unix/core.c
  src/unix/dl.c
  src/unix/fs.c
  src/unix/getaddrinfo.c
  src/unix/getnameinfo.c
  src/unix/loop-watcher.c
  src/unix/loop.c
  src/unix/pipe.c
  src/unix/poll.c
  src/unix/process.c
  src/unix/signal.c
  src/unix/stream.c
  src/unix/tcp.c
  src/unix/thread.c
  src/unix/timer.c
  src/unix/tty.c
  src/unix/udp.c
  src/unix/proctitle.c
  ${UV_PLATFORM_SRC}
)

set(LIB_INCS ${LIB_INCS} ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
set(LIB_OBJS ${LIB_OBJS} ${uv_OBJS} PARENT_SCOPE)