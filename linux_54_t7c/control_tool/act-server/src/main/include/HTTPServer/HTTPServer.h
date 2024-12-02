//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <ATL/ATLTypes.h>
#include <ATL/ATLError.h>
#include <ATL/ATLObject.h>
#include <ATL/ATLTemplates.h>
#include <ATL/ATLComponent.h>

using namespace atl;

namespace act {

     typedef enum __EHTTPServerStates {
        EHTTPServerNotInitialized = 0,
        EHTTPServerInitialized,
        EHTTPServerOpened,
        EHTTPServerRunning,
        EHTTPServerStopping,
        EHTTPServerStopped,
        EHTTPServerClosed
    } EHTTPServerStates;


    class CHTTPServer : public virtual CATLObject, public virtual IATLServer {
    private:
        TSmartPtr< CCommandManager > cm;  // command manager
        TSmartPtr< CEventEngine > engine; // http engine
        EHTTPServerStates state;

    public:
        CHTTPServer() : state(EHTTPServerNotInitialized) {}
        virtual ~CHTTPServer() {}

        // IATLObject
        inline const std::string GetObjectStaticName() {return "CHTTPServer";}

        // IATLComponent
        inline const std::string GetName() {return "CHTTPServer";}
        virtual CATLError Initialize(const flags32& options = 0);
        virtual CATLError Open();
        virtual CATLError Close();
        virtual CATLError Terminate();

        // IATLServer
        virtual void ServerFinish(void*);

    };
}

#endif // __HTTP_SERVER_H__
