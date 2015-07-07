/*
 * stack.h
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */

#ifndef SRC_COUV_STACK_H_
#define SRC_COUV_STACK_H_

#include <stddef.h>

namespace couv
{

class stack
{
public:
    int m_size;
    void * m_data;

public:
    stack();
    ~stack();
    void* data();
    size_t size();
};

} /* namespace coroutine */

#endif /* SRC_COUV_STACK_H_ */
