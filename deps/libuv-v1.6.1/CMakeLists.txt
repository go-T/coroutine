if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin") 
    set(UV_PLATFORM_SRC
        src/unix/fsevents.c
        src/unix/kqueue.c
        src/unix/darwin.c
        src/unix/darwin-proctitle.c
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(UV_PLATFORM_SRC
        src/unix/linux-core.c
        src/unix/linux-inotify.c
        src/unix/linux-syscalls.c
    )
else()
    message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} not support")
endif()

add_incs(${CMAKE_CURRENT_SOURCE_DIR}/include)
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