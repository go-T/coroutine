/*
 * sem.h
 *
 *  Created on: 2015年7月4日
 *      Author: zhusheng
 */

#ifndef SRC_COUV_SEM_H_
#define SRC_COUV_SEM_H_

#include <deque>

#include "couv/coroutine_base.h"

namespace couv
{

class sem_t
{
public:
    sem_t(int n=0, int limit=-1);
    ~sem_t();

    void wait();
    void signal();
    void broadcast();
    bool try_wait();
    int count();

protected:
    int m_counter;
    int m_limit;
    std::deque<coroutine_ptr> m_queue;
};

} /* namespace coroutine */

#endif /* SRC_COUV_SEM_H_ */
