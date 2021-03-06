/*
 * lock.cpp
 *
 *  Created on: 2015年7月4日
 *      Author: zhusheng
 */

#include "couv/lock.h"
#include "couv/scheduler.h"

namespace couv
{

lock_t::lock_t():m_flag(false)
{
}

lock_t::~lock_t()
{
}

void lock_t::lock()
{
    coroutine_ptr current = coroutine_base::self();
    if(m_flag) { // wait
        if(m_owner != current && m_queue.find(current) == m_queue.end()) {
            current->set_blocked(true);
            m_queue.insert(current);
            yield();
        }
    } else {
        m_flag = true;
        m_owner = current;
    }
}

void lock_t::unlock()
{
    m_flag = false;
    if(!m_queue.empty()) {
        m_owner = *m_queue.begin();
        m_queue.erase(m_queue.begin());

        m_owner->set_blocked(false);
        yield(m_owner);
    }
}

bool lock_t::try_lock()
{
    if(!m_flag) {
        m_flag = true;
        m_owner = coroutine_base::self();
        return true;
    }
    return false;
}

} /* namespace coroutine */
