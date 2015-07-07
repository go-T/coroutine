/*
 * sem.cpp
 *
 *  Created on: 2015年7月4日
 *      Author: zhusheng
 */

#include "../couv/sem.h"

#include "../couv/scheduler.h"

namespace couv
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
        current_coroutine->set_blocked(true);
        m_queue.push_back(current_coroutine);
        yield();
    }
}

void sem_t::signal()
{
    if(m_counter >= 0) {
        m_counter++;
    }
    else {
        coroutine_ptr current = m_queue.front(); m_queue.pop_front();
        current->set_blocked(false);
        yield(current);
    }
}

void sem_t::broadcast()
{
    if(!m_queue.empty()) {
        m_counter = 0;
        for(auto r: m_queue) {
            r->set_blocked(false);
            yield(r);
        }
        m_queue.clear();
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
