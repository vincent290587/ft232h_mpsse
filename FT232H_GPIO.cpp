#include <windows.h>
#include <iostream>

#include <chrono>
#include <thread>

#include <libMPSSE_i2c.h>

using namespace std;

// Note that pin numbers 0 to 15 map to pins D0 to D7 then C0 to C9 on the board.

#define PIN_C(X)     (8+(X))
#define PIN_D(X)     (X)

int main()
{
	FT_DEVICE_LIST_INFO_NODE devList;
	FT_STATUS status;
	FT_HANDLE ftHandle;
	uint32 channels = 0;
	Init_libMPSSE();
	FT_STATUS ftStatus = I2C_GetNumChannels(&channels);
	cout << "Number of channels: %d" << channels <<endl;

	if (channels > 0)
	{
		for (int i = 0; i < channels; i++)
		{
			status = I2C_GetChannelInfo(i, &devList);
			printf("Information on channel number %d:\n", i);
			/*print the dev info*/
			cout << " Flags = 0x" << std::hex << devList.Flags << endl;
			cout << " Type = 0x" << std::hex << devList.Type << endl;
			cout << " ID = 0x" << std::hex << devList.ID << endl;
			cout << " LocId = 0x" << std::hex << devList.LocId << endl;
			cout << " SerialNumber = " << devList.SerialNumber << endl;
			cout << " Description = " << devList.Description << endl;
			cout << " ftHandle = " << devList.ftHandle << endl;/*is 0 unless open*/
		}
	}

	// I2C init
	status = I2C_OpenChannel(0, &ftHandle);

	ChannelConfig channelConf;
	channelConf.ClockRate = I2C_CLOCK_FAST_MODE;/*i.e. 400000 KHz*/
	channelConf.LatencyTimer = 255;
	status = I2C_InitChannel(ftHandle, &channelConf);
	cout << "Init Channel. Status: " << status << endl;

    // GPIO toggle
	int pin = PIN_C(0);
	for (int i = 0; i < 10; i++)
	{
		FT_WriteGPIO(ftHandle, 1 << pin, 1 << pin);
		cout << "> ON" << endl;
		std::this_thread::sleep_for(1s);

		FT_WriteGPIO(ftHandle, 1 << pin, 0);
		cout << "> OFF" << endl;
		std::this_thread::sleep_for(1s);
	}

	status = I2C_CloseChannel(ftHandle);
	Cleanup_libMPSSE();

	cout << "Press Enter to Continue";
	cin.ignore();
	return 0;
}
