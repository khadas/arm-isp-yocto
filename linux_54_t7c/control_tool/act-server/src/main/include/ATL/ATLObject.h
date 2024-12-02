//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#ifndef __ATL_BASE_OBJECT__
#define __ATL_BASE_OBJECT__

#include "ATLTypes.h"

#include <string>
#include <ARMCPP11/Atomic.h>

namespace atl {

    class IATLObject {
    public:
        virtual ~IATLObject() {}
        virtual refcounter AddRef() = 0;
        virtual refcounter Release() = 0;
        virtual const std::string GetObjectStaticName() = 0;
    };

    class CATLObject : public virtual IATLObject {
    protected:
        armcpp11::Atomic< refcounter > refCounter ;
    protected:
        inline CATLObject() : refCounter(0) {}
    public:
        virtual ~CATLObject();
        virtual refcounter AddRef();
        virtual refcounter Release();

    } ;

} // atl namespace

#endif // __ATL_BASE_OBJECT__
