/*
 * echo.cpp
 *
 *  Created on: 2015年7月8日
 *      Author: zhusheng
 */

#include "couv/scheduler.h"
#include "couv/uvpp.h"

using namespace couv;
using namespace couv::uvpp;

int main()
{
    //Logger::logger().setDevice(NULL);

    scheduler_t scheduler;

    loop_t loop;
    
    go{
        tcp_t server(&loop);
        server.listen(9527);
        server.accept([](tcp_t* client, int status){
            int nread = 1;
            while (nread) {
                nread = client->read([](tcp_t* client, char* buf, ssize_t len){
                    client->write(buf, len);
                });
            }
        });
    };
    
    go{
        loop.run();
    };
    
    scheduler.run();
    return 0;
}


