/*
 * lock.h
 *
 *  Created on: 2015年7月4日
 *      Author: zhusheng
 */

#ifndef SRC_COUV_LOCK_H_
#define SRC_COUV_LOCK_H_

#include <set>

#include "couv/coroutine_base.h"

namespace couv
{

class lock_t
{
public:
    lock_t();
    ~lock_t();

    void lock();
    void unlock();
    bool try_lock();
protected:
    lock_t(const lock_t& t) = delete;
    lock_t operator=(const lock_t& t) = delete;
protected:
    bool m_flag;
    coroutine_ptr m_owner;
    std::set<coroutine_ptr> m_queue;
};


template<typename T>class lock_guard_t;
template<> class lock_guard_t<lock_t>
{
    lock_t & m_lock;
public:
    lock_guard_t(lock_t& lock):m_lock(lock)
    {
        m_lock.lock();
    }
    ~lock_guard_t()
    {
        m_lock.unlock();
    }
};


} /* namespace coroutine */

#endif /* SRC_COUV_LOCK_H_ */
