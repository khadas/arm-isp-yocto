//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef __ATL_NETWORK_H__
#define __ATL_NETWORK_H__

#include <string>

namespace atl {
    class network {
    public:
        static std::string getIPAddress(const std::string& defaultAddress);
    };
}

#endif // __ATL_NETWORK_H__
