//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#ifndef __BUS_FACTORY_H__
#define __BUS_FACTORY_H__

#include <ATL/ATLTemplates.h>

#include "BaseDriverInterface.h"

#include <string>
#include <vector>

using namespace atl ;

namespace act {
    class CDriverFactory {
    public:
        static TSmartPtr<IDriver> CreateDriver(const std::string &provider);
        const static std::vector<std::string> GetSupportedDrivers();
    };
}

#endif // __BUS_FACTORY_H__
