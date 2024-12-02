//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef ACCESSMANAGERCONFIG_H
#define ACCESSMANAGERCONFIG_H

#include <ATL/ATLObject.h>
#include <ATL/ATLTypes.h>

namespace act {

class CAccessManagerConfig : public virtual atl::CATLObject {
public:
    virtual const std::string GetObjectStaticName();
    atl::basebool initialized;
    atl::basebool hasDebugOffset;
    atl::baseaddr debugOffset;
    atl::basebool has1K;
    atl::baseaddr hwBufOffset;
    atl::UInt32 hwBufSize;
};

}

#endif // ACCESSMANAGERCONFIG_H
