/*
 * coroutine.h
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */

#ifndef SRC_COROUTINE_COROUTINE_H_
#define SRC_COROUTINE_COROUTINE_H_

#include <ucontext.h>
#include <functional>

#include "coroutine/stack.h"
#include "coroutine/coroutine_base.h"

namespace coroutine
{

class coroutine_base;

// customer
class coroutine: public coroutine_base
{
public:
    coroutine(func_type f);

protected:
    static void on_run(coroutine* thiz);

    virtual void run();

protected:
    func_type m_fun;
    stack m_stack;

};

} /* namespace coroutine */

#endif /* SRC_COROUTINE_COROUTINE_H_ */
