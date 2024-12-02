//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#ifndef __DRV_FTDI_H__
#define __DRV_FTDI_H__

#include <stdexcept>
#include <vector>
#include <ATL/ATLLogger.h>

#ifdef _WIN32
#include <windows.h>
#else

#endif

typedef std::vector<std::string> DevList;

#define FTDI_DIS_DIV_5      0x8A
#define FTDI_DIS_ADAPT_CLK  0x97
#define FTDI_EN_3_PHASE     0x8C
#define FTDI_DIS_LOOPBACK   0x85
#define FTDI_TCK_DIVIDER    0x86
#define FTDI_SET_BITS_LOW   0x80
#define FTDI_SET_BITS_HIGH  0x82
#define FTDI_SEND_ANSWER    0x87

#define FTDI_SEND_BYTE      0x11
#define FTDI_SEND_BIT       0x13
#define FTDI_REC_BYTE       0x20
#define FTDI_REC_BIT        0x22

// General FTDI interface
namespace act {

struct aml_device_list
{
    struct aml_device_list *mNextDevNode;
    struct aml_usb_device  *mDevNode;
};

enum aml_chip_type
{
    TYPE_ZERO = 0,
    TYPE_ONE,
    TYPE_TWO,
    TYPE_THREE,
    TYPE_FOUR,
    TYPE_FIVE,
    TYPE_SIX,
    TYPE_SEVEN
};

enum aml_detach_mode
{
    MODE_ZERO = 0,
    MODE_ONE
};

typedef char aml_char;
typedef unsigned char aml_uchar;
typedef int aml_int;
typedef unsigned int aml_uint;

struct aml_context
{
    struct aml_usb_context    *mDeviceContext;
    struct aml_usb_dev_handle *mDevice;
    aml_int                   mReadTimeout;
    aml_int                   mWriteRimeout;

    enum aml_chip_type        mChipType;
    aml_int                   mBaudRate;
    aml_uchar                 mIsBitBangEnabled;
    aml_uchar                 *mReadBuffer;
    aml_uint                  mReadBufferOffset;
    aml_uint                  mReadBufferPendingCount;
    aml_uint                  mReadBufferPacketSize;
    aml_uint                  mWriteBufferPacketSize;
    aml_uint                  mMaxPacketSize;

    aml_int                   mInterface;
    aml_int                   mIndex;
    aml_int                   mInEndPoint;
    aml_int                   mOutEndPoint;
    aml_uchar                 mBitbangMode;
    struct aml_eeprom         *mEEPROMStruct;
    aml_char                  *mErrorString;
    enum aml_detach_mode      mDetachMode;
};



class FTDI_BASE {
public:
    class Error : std::runtime_error {
    public:
        Error(const char* what) : runtime_error(what) { atl::SysLog()->LogError("FTDI", "exception %s", what); }
    };

    const std::string& get_chip_name() {return chip_name;}
protected:
    std::string chip_name;
};

#ifdef _WIN32

// Definitions for supporting FT Devices
typedef void * AML_HANDLE;
typedef ULONG AML_STATUS;

enum {
    AML_OK
};

#define FTDI_BITMODE_RESET      0x0
#define FTDI_BITMODE_MPSSE      0x02
#define FTDI_BITMODE_BITBANG    0x04


// FTDI driver for D2XX
typedef AML_STATUS (_STDCALL *f_FT_Open)(int deviceNumber,AML_HANDLE *pHandle);
typedef AML_STATUS (_STDCALL *f_FT_SetBaudRate)(AML_HANDLE ftHandle,ULONG BaudRate);
typedef AML_STATUS (_STDCALL *f_FT_SetBitMode)(AML_HANDLE ftHandle,UCHAR ucMask,UCHAR ucEnable);
typedef AML_STATUS (_STDCALL *f_FT_Read)(AML_HANDLE ftHandle,LPVOID lpBuffer,DWORD dwBytesToRead,LPDWORD lpBytesReturned);
typedef AML_STATUS (_STDCALL *f_FT_Write)(AML_HANDLE ftHandle,LPCVOID lpBuffer,DWORD dwBytesToWrite,LPDWORD lpBytesWritten);
typedef AML_STATUS (_STDCALL *f_FT_ResetDevice)(AML_HANDLE ftHandle);
typedef AML_STATUS (_STDCALL *f_FT_Close)(AML_HANDLE ftHandle);
typedef AML_STATUS (_STDCALL *f_FT_CreateDeviceInfoList)(LPDWORD lpdwNumDevs);
typedef AML_STATUS (_STDCALL *f_FT_GetDeviceInfoDetail)(DWORD dwIndex,LPDWORD lpdwFlags,LPDWORD lpdwType,LPDWORD lpdwID,LPDWORD lpdwLocId,LPVOID lpSerialNumber,LPVOID lpDescription,AML_HANDLE *pftHandle);
typedef AML_STATUS (_STDCALL *f_FT_SetLatencyTimer)(AML_HANDLE ftHandle,UCHAR ucLatency);
typedef AML_STATUS (_STDCALL *f_FT_SetUSBParameters)(AML_HANDLE ftHandle, ULONG ulInTransferSize, ULONG ulOutTransferSize);
typedef AML_STATUS (_STDCALL *f_FT_SetTimeouts)(AML_HANDLE ftHandle, ULONG ReadTimeout, ULONG WriteTimeout);

class FTDI : public FTDI_BASE {
public:
    static const bool read_before_write = false;
    FTDI();
    ~FTDI();
    bool open(const std::string& name);
    void close();
    void reset();
    void set_bitmode(UInt8 mask, UInt8 mode);
    void set_baudrate(UInt32 baudrate);
    void write(const std::vector<UInt8>& data);
    void read(std::vector<UInt8>& data);
    const DevList get_devices();
private:
    void* hLib;
    AML_HANDLE hFTDI;
    f_FT_Open FT_Open;
    f_FT_SetBaudRate FT_SetBaudRate;
    f_FT_SetBitMode FT_SetBitMode;
    f_FT_Read FT_Read;
    f_FT_Write FT_Write;
    f_FT_ResetDevice FT_ResetDevice;
    f_FT_Close FT_Close;
    f_FT_CreateDeviceInfoList FT_CreateDeviceInfoList;
    f_FT_GetDeviceInfoDetail FT_GetDeviceInfoDetail;
    f_FT_SetLatencyTimer FT_SetLatencyTimer;
    f_FT_SetUSBParameters FT_SetUSBParameters;
    f_FT_SetTimeouts FT_SetTimeouts;
};

#else

#define FTDI_BITMODE_RESET      0x00
#define FTDI_BITMODE_MPSSE      0x02
#define FTDI_BITMODE_BITBANG    0x04

typedef struct aml_context* (*f_context_void)(void);
typedef void (*f_void_context)(struct aml_context *);
typedef int (*f_int_context)(struct aml_context *);
typedef int (*f_int_context_int)(struct aml_context *, int);
typedef int (*f_int_context_int_int)(struct aml_context *, int, int);
typedef int (*f_int_context_char_char)(struct aml_context *, unsigned char, unsigned char);
typedef int (*f_int_context_pchar_int)(struct aml_context *, unsigned char*, int);
typedef int (*f_int_context_cpchar_int)(struct aml_context *, const unsigned char*, int);
typedef int (*f_get_strings)(struct aml_context *, struct aml_usb_device *,char *, int,char *, int,char *, int);
typedef int (*f_find_all) (struct aml_context *, struct aml_device_list **, int, int);
typedef void (*f_pplist) (struct aml_device_list **);
typedef int (*f_int_context_pdev)(struct aml_context *, struct aml_usb_device *);
typedef int (*f_int_context_char)(struct aml_context *, unsigned char);
typedef int (*f_int_context_uint)(struct aml_context *, unsigned int);

class FTDI : public FTDI_BASE {
public:
    static const bool read_before_write = true;
    FTDI();
    ~FTDI();
    bool open(const std::string& name);
    void close();
    void reset();
    void set_bitmode(UInt8 mask, UInt8 mode);
    void set_baudrate(UInt32 baudrate);
    void write(const std::vector<UInt8>& data);
    void read(std::vector<UInt8>& data);
    const DevList get_devices();
private:
    void detach();
    struct aml_context* hFTDI;
    void* hLib;
    f_context_void ftdi_new;
    f_void_context ftdi_free;
    f_int_context_int_int ftdi_usb_open;
    f_int_context ftdi_usb_close;
    f_int_context ftdi_usb_reset;
    f_int_context_int ftdi_set_baudrate;
    f_int_context_char_char ftdi_set_bitmode;
    f_int_context_pchar_int ftdi_read_data;
    f_int_context_cpchar_int ftdi_write_data;
    f_int_context ftdi_usb_purge_buffers;
    f_find_all ftdi_usb_find_all ;
    f_pplist ftdi_list_free;
    f_int_context_pdev ftdi_usb_open_dev;
    f_int_context_char ftdi_set_latency_timer;
    f_get_strings ftdi_usb_get_strings;
    f_int_context_uint ftdi_write_data_set_chunksize;
    f_int_context_uint ftdi_read_data_set_chunksize;
};
#endif

}
#endif /* __DRV_FTDI_H__ */
