/*
 * echo.cpp
 *
 *  Created on: 2015年7月8日
 *      Author: zhusheng
 */

#include "couv/scheduler.h"
#include "couv/uvpp.h"

#include <string>

using namespace couv;
using namespace couv::uvpp;

/**
 * go {
 *    declar yyy;
 *
 *    try{
 *
 *       main code;
 *
 *    } catch(...) {
 *
 *    }
 *
 *    save_end
 *
 * }
 *
 */


int main()
{
    Logger::logger().setDevice(NULL);

    uvscheduler_t scheduler;
    tcp_t server;

    go {
        server.listen(9527);
        server.accept([&](int status){
            go{
                std::shared_ptr<tcp_t> client = server.accept<tcp_t>();
                client->read([&](tcp_t* client, char* buf, ssize_t len){
                    int n = len-1;
                    while (n >= 0 && (buf[n] == ' ' || buf[n] == '\r' || buf[n] == '\n')) {
                        --n;
                    }
                    std::string cmd(buf, n+1);
                    
                    if (cmd == "end") {
                        client->write("bye", 3, [](tcp_t* client, int len){
                            client->close();
                        });
                    } else if(cmd == "close") {
                        client->write("byebye", 6, [&](tcp_t* client, int len){
                            client->close();
                            server.close();
                        });
                    } else if (cmd == "shutdown") {
                        client->write("bye all", 6, [&](tcp_t* client, int len){
                            scheduler.stop();
                        });
                    } else {
                        client->write(buf, len);
                    }
                });
            };
        });
    };

    scheduler.run();
    return 0;
}
