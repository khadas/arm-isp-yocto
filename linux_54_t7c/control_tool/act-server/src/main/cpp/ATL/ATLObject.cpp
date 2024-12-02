//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <ATL/ATLObject.h>
#include <ATL/ATLTemplates.h>
#include <ATL/ATLLogger.h>

#include <cassert>

namespace atl {

    CATLObject::~CATLObject() {
        assert (refCounter == 0);
    }

    refcounter CATLObject::AddRef() {
        return refCounter.FetchAdd(1);
    }

    refcounter CATLObject::Release() {
        refcounter result = refCounter.FetchSub(1);
        if (result == 1) {
            delete this;
        } else if ( (result == 0) || (result == (refcounter)0xFEFEFEFE) ) {
            throw "ATL object: invalid reference";
        }
        return result;
    }

} // atl namespace
