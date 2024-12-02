//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#include <ATL/ATLNetwork.h>

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>

using namespace atl;

std::string network::getIPAddress(const std::string& defaultAddress) {
     if (defaultAddress == "0.0.0.0") {
         struct ifaddrs *ifAddrStruct = NULL;
         struct ifaddrs *ifa = NULL;
         void *tmpAddrPtr = NULL;
         std::string ext_ip_addr = "xxx.xxx.xxx.xxx";
         getifaddrs(&ifAddrStruct);
         for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
             if (!ifa->ifa_addr || (ifa->ifa_flags & IFF_LOOPBACK) || ifa->ifa_addr->sa_family != AF_INET) {
                 continue;
             }
             tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
             char addressBuffer[INET_ADDRSTRLEN];
             inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
             ext_ip_addr.assign(addressBuffer);
             break;
         }
         if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
         return ext_ip_addr;
     } else {
        return defaultAddress;
    }
}
