#include <windows.h>
#include <iostream>

#include <chrono>
#include <thread>

#include <ftd2xx.h>
#include <libmpsse_i2c.h>

#include "MAX17055.h"

using namespace std;

#define APP_CHECK_STATUS(X)     if ((X)) {printf("Error %u line %u \n", (X), __LINE__);}

// In MPSSE I2C ADBUS is used for synchronous serial communications (I2C/SPI/JTAG) and ACBUS is free to be used as GPIO


//Like the FT232H guide mentions, when using I2C you'll need to setup your circuit with external pull-up resistors connected to the I2C clock and data lines.  This is necessary because the FT232H is a general purpose chip which doesn't include built-in pull-up resistors.
//
//As a quick summary of the I2C wiring, make the following connections:
//
//Connect FT232H D1 and D2 together with a jumper wire.  This combined connection is the I2C SDA data line.
//Add a 4.7 kilo-ohm resistor from the I2C SDA data line (pins D1 and D2 above) up to FT232H 5V.
//Add a 4.7 kilo-ohm resistor from FT232H D0 up to FT232H 5V.  This pin D0 is the I2C SCL clock line.
//Next wire your I2C device to the FT232H SDA and SCL lines just as you would for a Raspberry Pi or BeagleBone Black.  For other digital pins on the device, such as chip select, reset, data, etc., you should connect those pins to any free GPIO pins on the FT232H, such as C0 to C7 (GPIO numbers 8 to 15).
//
//For device power you can use the 5V pin on the FT232H board to supply up to ~500mA of 5 volt power.  Also remember the FT232H board digital outputs operate at 3.3 volts and the digital inputs can safely accept either 3.3 volts or 5 volts.
//
//Note with the setup above the I2C pins SDA and SCL will be pulled up to 5 volts when idle!  Make sure your I2C device can handle this voltage (Adafruit breakout boards, unless noted otherwise, are made to handle 5 volts). If you are using a 3.3V device, simply connect the pullups to 3.3V instead of 5V


//To use one of these devices you'll first want to first read the device's tutorial to get an overview of its connections and library usage.  Then connect the device to the FT232H breakout just like you're connecting the device to a Raspberry Pi or BeagleBone Black, but with the following SPI connections:
//
//Device SCLK or clock to FT232H D0 / serial clock.
//Device MOSI or data in to FT232H D1 / serial output.
//Device MISO or data out to FT232H D2 / serial input.
//For other digital pins on the device, such as chip select, reset, data, etc., you should connect those pins to any free GPIO pins on the FT232H, such as C0 to C7 (GPIO numbers 8 to 15).


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

extern "C" int HalI2CWrite(uint8 * buffer, uint16 length) {
    FT_STATUS status = i2c_write_multi(ftHandle, m_i2c_address, (PUCHAR)buffer, (UCHAR)length);
    APP_CHECK_STATUS(status);
    std::this_thread::sleep_for(0.1s);
    return -status;
}

extern "C" int HalSensorReadReg(uint8 addr, uint8 * buffer, uint16 length) {
    FT_STATUS status = i2c_read_multi(ftHandle, m_i2c_address, addr, (PUCHAR)buffer, (UCHAR)length);
    APP_CHECK_STATUS(status);
    std::this_thread::sleep_for(0.1s);
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
        channelConf.Options = 0;

        ftStatus = I2C_InitChannel(ftHandle, &channelConf);
        APP_CHECK_STATUS(ftStatus);
        cout << "Init Channel. Status: " << ftStatus << endl;

        std::this_thread::sleep_for(1s);
    }

    {
        MAX17055_init();
    }

    ftStatus = I2C_CloseChannel(ftHandle);
    APP_CHECK_STATUS(ftStatus);

    Cleanup_libMPSSE();

//	cout << "Press Enter to Continue";
//	cin.ignore();

    return 0;
}

//FT_STATUS i2c_read(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, PUCHAR value)
//{
//    FT_STATUS status;
//    DWORD xfer = 0;
//
//    /* As per Bosch BME280 Datasheet Figure 9: I2C Multiple Byte Read. */
//    status = I2C_DeviceWrite(ftHandle, address, 1, &reg, &xfer,
//                             I2C_TRANSFER_OPTIONS_START_BIT |
//                             I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
//
//    if (status == FT_OK)
//    {
//        /* Repeated Start condition generated. */
//        status = I2C_DeviceRead(ftHandle, address, 1, value, &xfer,
//                                I2C_TRANSFER_OPTIONS_START_BIT |
//                                I2C_TRANSFER_OPTIONS_STOP_BIT |
//                                I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE);
//    }
//    APP_CHECK_STATUS(status);
//
//    return status;
//}

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

FT_STATUS i2c_write_multi(FT_HANDLE ftHandle, UCHAR address, PUCHAR value, UCHAR length)
{
    FT_STATUS status;
    DWORD xfer = 0;

    status = I2C_DeviceWrite(ftHandle, address, 1, &value[0], &xfer,
                             I2C_TRANSFER_OPTIONS_START_BIT |
                             I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
    APP_CHECK_STATUS(status);

    if (status == FT_OK)
    {
        /* Register address not sent on register write. */
        status = I2C_DeviceWrite(ftHandle, address, length-1, &value[1], &xfer,
                                 I2C_TRANSFER_OPTIONS_NO_ADDRESS |
                                 I2C_TRANSFER_OPTIONS_STOP_BIT |
                                 I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
        APP_CHECK_STATUS(status);
    }

    return status;
}

