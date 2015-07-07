/*
 * sem.cpp
 *
 *  Created on: 2015年7月7日
 *      Author: zhusheng
 */

#include "couv/sem.h"
#include "log/Logger.h"
#include "iostream"

#include "couv/coroutine.h"
#include "couv/scheduler.h"

using namespace couv;

int main()
{
    Logger::logger().setDevice(NULL);

    scheduler scheduler1;
    current_scheduler = &scheduler1;
    std::cout << "hello world 0\n";

    sem_t sem;
    scheduler1.add([&sem]{
            std::cout << "hello world 1.1\n";
            sem.wait();
            std::cout << "hello world 1.2\n";
    });
    scheduler1.add([&sem]{
            std::cout << "hello world 2.1\n";
            sem.wait();
            std::cout << "hello world 2.2\n";
    });
    scheduler1.add([&sem]{
            std::cout << "hello world 3.1\n";
            sem.wait();
            std::cout << "hello world 3.2\n";
    });
    scheduler1.add([&sem]{
            std::cout << "hello world 4.1\n";
            sem.signal();
            std::cout << "hello world 4.2\n";
            sem.signal();
            std::cout << "hello world 4.3\n";
            sem.signal();
            std::cout << "hello world 4.4\n";
    });
    scheduler1.run();
    std::cout << "hello world 5\n";
    return 0;
}






