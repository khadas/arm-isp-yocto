//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#ifndef __ATL_BASE_COMPONENTS__
#define __ATL_BASE_COMPONENTS__

#include "ATLError.h"
#include "ATLObject.h"
#include "ATLTemplates.h"
#include "ATLConfig.h"

#include <string>

namespace atl {
    class IATLComponent : public virtual IATLObject {
    public:

        /**
         * @brief GetName
         * @return
         */
        virtual const std::string GetName() = 0;

        /**
         *   Initialize component
         *
         *   This routine initializes ATL component.
         *
         *   @param options - initialization flags
         *   @return EATLErrorOk - initialization succeed
         *           EATLErrorFatal - initialization has failed.
         */
        virtual CATLError Initialize(const flags32& options = 0) = 0;

        /**
        *   Open initialized ATL component
        *
        *   This routine brings ATL component to active state
        *
        *   @return EATLErrorOk - initialization succeed
        *           EATLErrorFatal - initialization has failed.
        */
        virtual CATLError Open() = 0;

        /**
         *   Close active ATL component
         *
         *   This routine closes active ATL component.
         *
         *   @return EATLErrorOk - initialization succeed
         *           EATLErrorFatal - initialization has failed.
         */
        virtual CATLError Close() = 0;

        /**
         *   Terminate ATL component
         *
         *   This routine terminates ATL component
         *
         *   @return EATLErrorOk - initialization succeed
         *           EATLErrorFatal - initialization has failed.
         */
        virtual CATLError Terminate() = 0;
    };
    class IATLServer : public virtual IATLComponent {
    public:
    public:
        virtual void ServerFinish(void*) = 0;
        CEventListener<> OnServerFinish;
    };
} // atl namespace

#endif // __ATL_BASE_COMPONENTS__
