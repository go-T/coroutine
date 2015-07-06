/*
 * sem.cpp
 *
 *  Created on: 2015年7月4日
 *      Author: zhusheng
 */

#include <coroutine/sem.h>
#include <coroutine/scheduler.h>

namespace coroutine
{

sem_t::sem_t(int n)
:m_counter(n)
{
}

sem_t::~sem_t()
{
}

void sem_t::wait()
{
    if(--m_counter < 0) {
        m_queue.push_back(current_coroutine);
    }
}

void sem_t::signal()
{
    if(m_counter != 0) {
        m_counter++;
    }
    else {
        coroutine_ptr current = m_queue.front(); m_queue.pop_front();
        yield(current);
    }
}

void sem_t::broadcast()
{
    if(!m_queue.empty()) {
        m_counter = 0;
        for(auto r: m_queue) {
            yield(r);
        }
    }
}

bool sem_t::try_wait()
{
    if(m_counter > 0) {
        m_counter--;
        return true;
    }
    return false;
}

int sem_t::count()
{
    return m_counter;
}

} /* namespace coroutine */
