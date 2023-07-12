#include <windows.h>
#include <iostream>

#include <chrono>
#include <thread>

#include <ftd2xx.h>
#include <libmpsse_i2c.h>

#include "Haptics_2605.h"

using namespace std;

#define APP_CHECK_STATUS(X)     if ((X)) {printf("Error %u line %u \n", (X), __LINE__);}

// In MPSSE I2C ADBUS is used for synchronous serial communications (I2C/SPI/JTAG) and ACBUS is free to be used as GPIO

#define PIN_C(X)     (X)

static FT_HANDLE ftHandle;
static UCHAR m_i2c_address;

FT_STATUS i2c_read(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, PUCHAR value);
FT_STATUS i2c_read_multi(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, PUCHAR value, UCHAR length);
FT_STATUS i2c_write(FT_HANDLE ftHandle, UCHAR address, UCHAR value);
FT_STATUS i2c_write_multi(FT_HANDLE ftHandle, UCHAR address, PUCHAR value, UCHAR length);
FT_STATUS i2c_write_reg(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, UCHAR value);

/////////////////////////////////////////////////////////////////////////////////////

extern "C" void HalI2CInit(uint8 address) {
    m_i2c_address = address;
}

extern "C" void HalI2CWrite(uint16 length, const uint8 * const buffer) {
    FT_STATUS status = i2c_write_multi(ftHandle, m_i2c_address, (PUCHAR)&buffer[0], (UCHAR)length);
    APP_CHECK_STATUS(status);
}

extern "C" int HalSensorReadReg(uint8 addr, const uint8 * const buffer, uint16 length) {
    FT_STATUS status = i2c_read_multi(ftHandle, m_i2c_address, addr, (PUCHAR)buffer, (UCHAR)length);
    APP_CHECK_STATUS(status);
    return -status;
}

extern "C" void Haptics_WaitUs(uint16 microSecs) {
    std::this_thread::sleep_for(0.1s);
}

extern "C" void HalGPIOInit(pinID_t pin_id) {
    FT_STATUS ftStatus = FT_WriteGPIO(ftHandle, 1 << pin_id, 0);
    APP_CHECK_STATUS(ftStatus);
}

extern "C" void HalGPIOset(pinID_t pin_id, uint8 value) {
    FT_STATUS ftStatus = FT_WriteGPIO(ftHandle, 1 << pin_id, value << pin_id);
    APP_CHECK_STATUS(ftStatus);
}

/////////////////////////////////////////////////////////////////////////////////////

static void _drv2605_test(void) {

    for (UCHAR address = 1; address <= 127; address++) {
        FT_STATUS ftStatus;
        UCHAR value[5];
        ftStatus = i2c_write_multi(ftHandle, address, value, 5);

        if (!ftStatus) {
            cout << " Ack on: 0x" << std::hex << address << endl;
        }

        std::this_thread::sleep_for(0.5s);
    }

//    for (int i = 0; i < 100; i++) {
//
//        Haptics_Init(PIN_C(0)); // pin C0
//
//        std::this_thread::sleep_for(0.5s);
//    }
}

int main()
{
    FT_DEVICE_LIST_INFO_NODE devList;
    FT_STATUS ftStatus;
    DWORD channels = 0;

    Init_libMPSSE();

    {
        ftStatus = I2C_GetNumChannels(&channels);
        APP_CHECK_STATUS(ftStatus);
        cout << "Number of channels: " << channels << endl;
    }

    if (channels > 0)
    {
        for (DWORD i = 0; i < channels; i++)
        {
            ftStatus = I2C_GetChannelInfo(i, &devList);
            APP_CHECK_STATUS(ftStatus);
            printf("Information on channel number %d:\n", i);
            /*print the dev info*/
            cout << " Flags = 0x" << std::hex << devList.Flags << endl;
            cout << " Type = 0x" << std::hex << devList.Type << endl;
            cout << " ID = 0x" << std::hex << devList.ID << endl;
            cout << " LocId = 0x" << std::hex << devList.LocId << endl;
            cout << " SerialNumber = " << devList.SerialNumber << endl;
            cout << " Description = " << devList.Description << endl;
            cout << " ftHandle = " << devList.ftHandle << " (should be zero)" << endl;/*is 0 unless open*/
        }
    }

    // I2C init
    {
        ftStatus = I2C_OpenChannel(0, &ftHandle);
        APP_CHECK_STATUS(ftStatus);
    }

    {
        ChannelConfig channelConf;
        channelConf.ClockRate = I2C_CLOCK_STANDARD_MODE;
        channelConf.LatencyTimer = 2;
        channelConf.Options = 0b100;

        ftStatus = I2C_InitChannel(ftHandle, &channelConf);
        APP_CHECK_STATUS(ftStatus);
        cout << "Init Channel. Status: " << ftStatus << endl;
    }

    {
        _drv2605_test();
    }

    ftStatus = I2C_CloseChannel(ftHandle);
    APP_CHECK_STATUS(ftStatus);

    Cleanup_libMPSSE();

//	cout << "Press Enter to Continue";
//	cin.ignore();

    return 0;
}

FT_STATUS i2c_read(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, PUCHAR value)
{
    FT_STATUS status;
    DWORD xfer = 0;

    /* As per Bosch BME280 Datasheet Figure 9: I2C Multiple Byte Read. */
    status = I2C_DeviceWrite(ftHandle, address, 1, &reg, &xfer,
                             I2C_TRANSFER_OPTIONS_START_BIT |
                             I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);

    if (status == FT_OK)
    {
        /* Repeated Start condition generated. */
        status = I2C_DeviceRead(ftHandle, address, 1, value, &xfer,
                                I2C_TRANSFER_OPTIONS_START_BIT |
                                I2C_TRANSFER_OPTIONS_STOP_BIT |
                                I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE);
    }
    APP_CHECK_STATUS(status);

    return status;
}

FT_STATUS i2c_read_multi(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, PUCHAR value, UCHAR length)
{
    FT_STATUS status;
    DWORD xfer = 0;

    status = I2C_DeviceWrite(ftHandle, address, 1, &reg, &xfer,
                             I2C_TRANSFER_OPTIONS_START_BIT |
                             I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);

    if (status == FT_OK)
    {
        /* Repeated Start condition generated. */
        status = I2C_DeviceRead(ftHandle, address, length, value, &xfer,
                                I2C_TRANSFER_OPTIONS_START_BIT |
                                I2C_TRANSFER_OPTIONS_STOP_BIT |
                                I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE);
    }
    APP_CHECK_STATUS(status);

    return status;
}

FT_STATUS i2c_write(FT_HANDLE ftHandle, UCHAR address, UCHAR value)
{
    FT_STATUS status;
    DWORD xfer = 0;

    status = I2C_DeviceWrite(ftHandle, address, 1, &value, &xfer,
                             I2C_TRANSFER_OPTIONS_START_BIT |
                             I2C_TRANSFER_OPTIONS_STOP_BIT |
                             I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
    APP_CHECK_STATUS(status);

    return status;
}

FT_STATUS i2c_write_multi(FT_HANDLE ftHandle, UCHAR address, PUCHAR value, UCHAR length)
{
    FT_STATUS status;
    DWORD xfer = 0;

    status = I2C_DeviceWrite(ftHandle, address, length, value, &xfer,
                             I2C_TRANSFER_OPTIONS_START_BIT |
                             I2C_TRANSFER_OPTIONS_STOP_BIT |
                             I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
    APP_CHECK_STATUS(status);

    return status;
}

FT_STATUS i2c_write_reg(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, UCHAR value)
{
    FT_STATUS status;
    DWORD xfer = 0;

    status = I2C_DeviceWrite(ftHandle, address, 1, &reg, &xfer,
                             I2C_TRANSFER_OPTIONS_START_BIT |
                             I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
    APP_CHECK_STATUS(status);

    if (status == FT_OK)
    {
        /* Register address not sent on register write. */
        status = I2C_DeviceWrite(ftHandle, address, 1, &value, &xfer,
                                 I2C_TRANSFER_OPTIONS_NO_ADDRESS |
                                 I2C_TRANSFER_OPTIONS_STOP_BIT |
                                 I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
        APP_CHECK_STATUS(status);
    }

    return status;
}
