/*
 * stack.cpp
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */

#include "../couv/stack.h"
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

namespace {

static void* make_memory(size_t size, size_t page_size) {
    void* memory = mmap(
        nullptr,
        size + page_size,
        PROT_READ | PROT_WRITE,
#ifdef MAP_ANONYMOUS
        MAP_ANONYMOUS
#else
        MAP_ANON
#endif
        | MAP_SHARED
#ifdef MAP_GROWSDOWN
        | MAP_GROWSDOWN
#endif
        , -1, 0
    );

    mprotect(memory, page_size, PROT_NONE);

    return memory;
}

static void release_memory(void* data, size_t size, size_t page_size) {
    munmap(data, size + page_size);
}

} /* namespace */

namespace couv
{

stack::stack(): m_size(SIGSTKSZ),
        m_data(make_memory(m_size, 0)) {
}

stack::~stack(){
    release_memory(m_data, m_size, 0);
}

void* stack::data(){
    return m_data;
}

size_t stack::size() {
    return m_size;
}

} /* namespace coroutine */
