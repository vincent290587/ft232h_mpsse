
#include "drv_spi.h"

#include <chrono>
#include <cstdio>
#include <ftd2xx.h>
#include <ftdi_common.h>
#include <iostream>
#include <libmpsse_spi.h>
#include <thread>


#define APP_CHECK_STATUS(X)     if ((X)) {printf("Error %u line %u \n", (X), __LINE__);}

static FT_HANDLE ftHandle;

int drv_spi__start() {

    FT_DEVICE_LIST_INFO_NODE devList;
    FT_STATUS ftStatus;
    DWORD channels = 0;
    DWORD channel_spi = 0;

    {
        ftStatus = SPI_GetNumChannels(&channels);
        APP_CHECK_STATUS(ftStatus);
        std::cout << "Number of channels: " << channels << std::endl;
    }

    {
        if (channels > 0)
        {
            for (DWORD i = 0; i < channels; i++)
            {
                ftStatus = SPI_GetChannelInfo(i, &devList);
                APP_CHECK_STATUS(ftStatus);
                printf("Information on channel number %d:\n", i);
                /*print the dev info*/
                std::cout << " Flags = 0x" << std::hex << devList.Flags << std::endl;
                std::cout << " Type = 0x" << std::hex << devList.Type << std::endl;
                std::cout << " ID = 0x" << std::hex << devList.ID << std::endl;
                std::cout << " LocId = 0x" << std::hex << devList.LocId << std::endl;
                std::cout << " SerialNumber = " << devList.SerialNumber << std::endl;
                std::cout << " Description = " << devList.Description << std::endl;
                std::cout << " ftHandle = " << devList.ftHandle << " (should be zero if free)" << std::endl;/*is 0 unless open*/

                if (devList.ftHandle == 0) {
                    channel_spi = i;
                }
            }
        }
    }

    {
        ftStatus = SPI_OpenChannel(channel_spi, &ftHandle);
        APP_CHECK_STATUS(ftStatus);
    }

    {
        ChannelConfig channelConf;
        channelConf.ClockRate = 1000000;
        channelConf.LatencyTimer = 2;
        channelConf.configOptions = SPI_CONFIG_OPTION_MODE0;

        ftStatus = SPI_InitChannel(ftHandle, &channelConf);
        APP_CHECK_STATUS(ftStatus);
        std::cout << "SPI Init Channel. Status: " << ftStatus << std::endl;

    }


    return 0;
}
