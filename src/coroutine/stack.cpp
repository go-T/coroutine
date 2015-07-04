/*
 * stack.cpp
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */

#include "stack.h"
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

void release_memory(void* data, size_t size, size_t page_size) {
    munmap(data, size + page_size);
}

}

namespace coroutine
{
stack::stack() :m_size(SIGSTKSZ), m_page_size(getpagesize()), m_data(mmap(
        nullptr,
        SIGSTKSZ + getpagesize(),
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
    )) {
        // Guard the bottom of the stack
        mprotect(m_data, getpagesize(), PROT_NONE);
    }


stack::~stack() { munmap(m_data, SIGSTKSZ + getpagesize()); }

/*
stack::stack(): m_size(SIGSTKSZ),
        m_page_size(getpagesize()),
        m_data(make_memory(m_size, m_page_size)) {
}

stack::~stack(){
    release_memory(m_data, m_size, m_page_size);
}
*/
void* stack::data(){
    return m_data;
}

size_t stack::size() {
    return m_size;
}

} /* namespace coroutine */
