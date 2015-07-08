/*
 * channel.h
 *
 *  Created on: 2015年7月8日
 *      Author: zhusheng
 */

#ifndef SRC_COUV_CHANNEL_H_
#define SRC_COUV_CHANNEL_H_

#include "couv/sem.h"
#include <deque>

namespace couv
{

template<typename T>
class channel
{
public:
    bool m_closed;
    sem_t m_sem;
    std::deque<T> m_queue;
public:
    channel():m_closed(false){}
    ~channel(){}

    bool is_closed() {
        return m_closed;
    }

    void close() {
        m_closed = true;
        m_sem.broadcast();
    }

    void send(const T& t) {
        if(m_closed)
            return;

        m_sem.signal();
        m_queue.push_back(t);
    }

    void receive(T& t) {
        if(m_closed)
            return;

        m_sem.wait();
        t = m_queue.front();
        m_queue.pop_front();
    }

    T receive() {
        if(m_closed)
            return T();

        m_sem.wait();
        T t = m_queue.front();
        m_queue.pop_front();
        return t;
    }
};

}

#endif /* SRC_COUV_CHANNEL_H_ */
