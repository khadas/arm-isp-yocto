//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef ACTDIRECTCOMMAND_H
#define ACTDIRECTCOMMAND_H

#include <ATL/ATLApplication.h>
#include <ATL/ATLTemplates.h>
#include <ATL/ATLComponent.h>

#include <AccessManager/AccessManager.h>

using namespace atl;

class CACTDirectCommand : public virtual CATLApplicationCommand {
private:
    TSmartPtr<act::CAccessManager> am;

    basebool isAcessManagerConfigured() const;

protected:
    virtual ~CACTDirectCommand() {}

public:
    CACTDirectCommand();
    virtual CATLError CheckForImmediateAction();
    virtual CATLError Execute();
    virtual CATLError Stop();
    static void ShowUsage();

};

#endif // ACTDIRECTCOMMAND_H

