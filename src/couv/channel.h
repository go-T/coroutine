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
    
    operator bool() const {
        return !m_closed;
    }

    void close() {
        m_closed = true;
        m_sem.broadcast();
    }

    void send(const T& t) {
        if(m_closed)
            return;

        m_queue.push_back(t);
        m_sem.signal();
    }

    channel<T>& receive(T& t) {
        if(!m_closed) {
            m_sem.wait();
            if(!m_closed) {
                t = m_queue.front();
                m_queue.pop_front();
            }
        }
        return *this;
    }

    T receive(const T& def = T()) {
        if(!m_closed) {
            m_sem.wait();
            if(!m_closed) {
                T t = m_queue.front();
                m_queue.pop_front();
                return t;
            }
        }
        return def;
    }
};

}

#endif /* SRC_COUV_CHANNEL_H_ */
