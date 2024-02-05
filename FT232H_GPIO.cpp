#include <windows.h>
#include <iostream>

#include <chrono>
#include <thread>

#include <ftd2xx.h>
#include <libmpsse_i2c.h>

using namespace std;

#define APP_CHECK_STATUS(X)     if ((X)) {printf("Error %u line %u \n", (X), __LINE__);}

// In MPSSE I2C ADBUS is used for synchronous serial communications (I2C/SPI/JTAG) and ACBUS is free to be used as GPIO

#define PIN_C(X)     (X)

FT_STATUS i2c_read(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, PUCHAR value);
FT_STATUS i2c_read_multi(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, PUCHAR value, UCHAR length);
FT_STATUS i2c_write(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, UCHAR value);

int main()
{
	FT_DEVICE_LIST_INFO_NODE devList;
	FT_HANDLE ftHandle;
    FT_STATUS ftStatus;
    DWORD channels = 0;

    std::cout.setf(std::ios::unitbuf);

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
        channelConf.LatencyTimer = 10;
        channelConf.Options = 0;

        ftStatus = I2C_InitChannel(ftHandle, &channelConf);
        APP_CHECK_STATUS(ftStatus);
        cout << "Init Channel. Status: " << ftStatus << endl;
    }

#ifndef USE_I2C
    // GPIO toggle
    uint8_t output_mask = 1 << PIN_C(1);
    uint8_t output_value = 1 << PIN_C(1);

    for (int i = 0; i < 10; i++)
    {
        ftStatus = FT_WriteGPIO(ftHandle, output_mask, output_value);
        APP_CHECK_STATUS(ftStatus);
        cout << "> ON" << endl;
        std::this_thread::sleep_for(0.5s);

        ftStatus = FT_WriteGPIO(ftHandle, output_mask, 0x00);
        APP_CHECK_STATUS(ftStatus);
        cout << "> OFF" << endl;
        std::this_thread::sleep_for(0.5s);
    }
#else
    #define BME280_REGISTER_DEVICE_ID 0xd0

    UCHAR address = 0x76;
	UCHAR rev;
	UCHAR reg;
	UCHAR regbuf[32];
	FT_STATUS status;

    for (int i = 0; i < 100; i++) {
        status = i2c_read(ftHandle, address, BME280_REGISTER_DEVICE_ID, &rev);
        APP_CHECK_STATUS(status);

        std::this_thread::sleep_for(0.5s);
    }
#endif

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

    /* As per Bosch BME280 Datasheet Figure 9: I2C Multiple Byte Read. */
    status = I2C_DeviceWrite(ftHandle, address, 1, &reg, &xfer,
                             I2C_TRANSFER_OPTIONS_START_BIT |
                             I2C_TRANSFER_OPTIONS_STOP_BIT |
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

FT_STATUS i2c_write(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, UCHAR value)
{
    FT_STATUS status;
    DWORD xfer = 0;

    /* As per Bosch BME280 Datasheet Figure 9: I2C Multiple Byte Write (not autoincremented) */
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
