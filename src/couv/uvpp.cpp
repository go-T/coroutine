/*
 * uvpp.cpp
 *
 *  Created on: 2015年7月7日
 *      Author: zhusheng
 */

#include "uvpp.h"

namespace couv
{
namespace uvpp
{

const char* get_handle_name(uv_handle_t* h)
{
    if(h == nullptr)
    {
        return "null";
    }

    switch(h->type)
    {
#define XX(uc, lc) case UV_##uc: return "@" #lc ;
        UV_HANDLE_TYPE_MAP(XX)
#undef XX
        case UV_FILE: return "@file";
        default: return "@unknown";
    }
}

}
} /* namespace couv */
