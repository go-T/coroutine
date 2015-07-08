/*
 * delegate.h
 *
 *  Created on: 2015年7月2日
 *      Author: zhusheng
 */

#ifndef SRC_COUV_DELEGATE_H_
#define SRC_COUV_DELEGATE_H_

#include <memory>

namespace couv
{

class coroutine_base;
typedef std::shared_ptr<coroutine_base> coroutine_ptr;
typedef std::weak_ptr<coroutine_base> coroutine_ref;

class delegate_t
{
public:
    virtual ~delegate_t() = default;
    virtual void on_start(coroutine_base*) = 0;
    virtual void on_stop(coroutine_base*) = 0;
};

} /* namespace coroutine */

#endif /* SRC_COUV_DELEGATE_H_ */
