//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef __ACT_SERVER_APP__
#define __ACT_SERVER_APP__

#include <string>
#include <vector>

class CACTServerApp : public atl::CATLApplication {
private:
    virtual atl::CATLError CheckForImmediateAction();
public:
    CACTServerApp() {}
    ~CACTServerApp() {}

    virtual void ShowApplicationTitle();
    virtual void ShowUsage();
    virtual void ShowVersion();
    virtual atl::CATLError Initialize();
    virtual atl::CATLError InitCommands();
    virtual atl::CATLError CreateCommand(const std::string &name);

};

#endif // __ACT_SERVER_APP__
