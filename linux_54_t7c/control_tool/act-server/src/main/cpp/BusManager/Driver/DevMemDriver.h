//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#ifndef __DEVMEM_DRIVER_H__
#define __DEVMEM_DRIVER_H__

#include <ATL/ATLObject.h>
#include <ATL/ATLComponent.h>
#include <BusManager/BaseDriverInterface.h>

namespace act {

    class CDevMemDriver : public virtual CBaseDriver, public virtual CATLObject {
    public:
        CDevMemDriver();
        ~CDevMemDriver();

        // IATLObject interface
        inline virtual const std::string GetObjectStaticName() {return "CDevMemDriver";}

        // IATLComponent
        inline virtual const std::string GetName() {return "devmem";}
        virtual CATLError Initialize(const flags32& options = 0);
        virtual CATLError Open();
        virtual CATLError Close();
        virtual CATLError Terminate();

        virtual CATLError ProcessPacket(TSmartPtr< CTransferPacket > packet, const flags32& options = 0);
        virtual const std::vector<EACTDriverMode> GetSupportedModes();
        virtual const EACTDriverMode GetCurrentMode();
        virtual const std::vector<std::string> GetDeviceList();
        virtual const std::vector<EPacketCommand> GetCommandList();

    private:
        int devmemfd;
        UInt8* base_addr;
        std::string dev_name;
        baseaddr mem_offset;
        baseaddr max_addr;

        void close();
        CATLError read(TSmartPtr<CTransferPacket>& packet);
        CATLError write(TSmartPtr<CTransferPacket>& packet);

        CATLError mapmemoryblock(const off64_t& offset);
    };
}
#endif /* __DEVMEM_DRIVER_H__ */
