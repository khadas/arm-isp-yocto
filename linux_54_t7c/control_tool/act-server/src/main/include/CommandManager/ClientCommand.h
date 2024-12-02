//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#ifndef CLIENTCOMMAND_H
#define CLIENTCOMMAND_H

#include <ATL/ATLObject.h>
#include <ATL/ATLTypes.h>

namespace act {

class CRegPollCommand : public virtual atl::CATLObject {
public:
    virtual const std::string GetObjectStaticName();
    std::string name;
    std::string type;
    atl::UInt32 offset;
    atl::UInt32 size;
};

class CApiPollCommand : public virtual atl::CATLObject {
public:
    virtual const std::string GetObjectStaticName();
    std::string name;
    atl::UInt8 section;
    atl::UInt8 command;
};

}

#endif // CLIENTCOMMAND_H
