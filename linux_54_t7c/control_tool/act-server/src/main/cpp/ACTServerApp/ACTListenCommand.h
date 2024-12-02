//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef ACTLISTENCOMMAND_H
#define ACTLISTENCOMMAND_H

#include <ATL/ATLApplication.h>
#include <ATL/ATLTemplates.h>
#include <ATL/ATLComponent.h>

using namespace atl;

class CACTListenCommand : public virtual CATLApplicationCommand {
private:
    void OnServerFinish(void*);
    TSmartPtr<IATLServer> server;

protected:
    virtual ~CACTListenCommand() {}

public:
    CACTListenCommand();
    virtual CATLError Execute();
    virtual CATLError Stop();
    static void ShowUsage();

};

#endif // ACTLISTENCOMMAND_H

