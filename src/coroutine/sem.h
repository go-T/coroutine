/*
 * sem.h
 *
 *  Created on: 2015年7月4日
 *      Author: zhusheng
 */

#ifndef SRC_COROUTINE_SEM_H_
#define SRC_COROUTINE_SEM_H_

#include <deque>
#include "coroutine/coroutine_base.h"

namespace coroutine
{

class sem_t
{
public:
    sem_t(int n=0);
    ~sem_t();

    void wait();
    void signal();
    void broadcast();
    bool try_wait();
    int count();

protected:
    int m_counter;
    std::deque<coroutine_ptr> m_queue;
};

} /* namespace coroutine */

#endif /* SRC_COROUTINE_SEM_H_ */
