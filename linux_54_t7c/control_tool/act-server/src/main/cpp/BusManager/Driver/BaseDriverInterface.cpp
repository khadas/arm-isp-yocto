//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#define LOG_CONTEXT "IDriver"

#include <BusManager/BaseDriverInterface.h>

using namespace std;
using namespace atl;

namespace act {

    CATLError CBaseDriver::AddListerner (EACTDriverListenerTypes event_name, IATLObserver* proc) {
        if (event_name == EACTForwardReply) {
            OnReplyReceived += proc;
            return EATLErrorOk;
        }
        return atl::EATLErrorInvalidParameters;
    }

    CATLError CBaseDriver::RemoveListerner (EACTDriverListenerTypes event_name, IATLObserver* proc) {
        if (event_name == EACTForwardReply) {
            OnReplyReceived -= proc;
            return EATLErrorOk;
        }
        return EATLErrorInvalidParameters;
    }

    CATLError CBaseDriver::NotifyListerners (TSmartPtr< CTransferPacket > packet) {
        EPacketNotify ntf = packet->notify;
        EPacketCommand cmd = packet->command;
        EPacketState stt = packet->state;
        if ((ntf == EPacketNotifyAlways) ||
            (ntf == EPacketNotifyOnError && stt != EPacketSuccess) ||
            (ntf == EPacketNotifyOnRead  && cmd == EPacketRead) ||
            (ntf == EPacketNotifyOnWrite && cmd == EPacketWrite)) {
            OnReplyReceived.Invoke( packet );
        }
        return EATLErrorOk;
    }
}
