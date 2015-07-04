/*
 * stack.h
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */

#ifndef SRC_COROUTINE_STACK_H_
#define SRC_COROUTINE_STACK_H_

#include <stddef.h>

namespace coroutine
{

class stack
{
public:
    int m_size;
    int m_page_size;
    void * m_data;

public:
    stack();
    ~stack();
    void* data();
    size_t size();
};

} /* namespace coroutine */

#endif /* SRC_COROUTINE_STACK_H_ */
