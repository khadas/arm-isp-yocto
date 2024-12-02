//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <ATL/ATLNetwork.h>

#include <winsock.h>

using namespace atl;

std::string network::getIPAddress(const std::string& defaultAddress) {
    if (defaultAddress == "0.0.0.0") {
        std::string ext_ip_addr = "xxx.xxx.xxx.xxx";
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        char ac[80];
        if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
            WSACleanup();
            return ext_ip_addr;
        }
        struct hostent *phe = gethostbyname(ac);
        if (phe == 0) {
            WSACleanup();
            return ext_ip_addr;
        }
        for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
            struct in_addr addr;
            memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
            ext_ip_addr.assign(inet_ntoa(addr));
            break;
        }
        WSACleanup();
        return ext_ip_addr;
    } else {
        return defaultAddress;
    }
}
