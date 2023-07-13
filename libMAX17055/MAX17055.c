//
// Created by vgol on 13/07/2023.
//

#include "MAX17055.h"
#include "hal_i2c.h"

enum max17055_register{
    MAX17055_STATUS_REG                 = 0x00,
    MAX17055_VALRTTH_REG                = 0x01,
    MAX17055_TALRTTH_REG                = 0x02,
    MAX17055_SALRTTH_REG                = 0x03,
    MAX17055_REPCAP_REG                 = 0x05,
    MAX17055_REPSOC_REG                 = 0x06,
    MAX17055_TEMP_REG                   = 0x08,
    MAX17055_VCELL_REG                  = 0x09,
    MAX17055_CURRENT_REG                = 0x0A,
    MAX17055_AVGCURRENT_REG             = 0x0B,
    MAX17055_MIXCAP_REG                 = 0x0F,

    MAX17055_FULLCAPREP_REG             = 0x10,
    MAX17055_TTE_REG                    = 0X11,
    MAX17055_QRTABLE00_REG              = 0x12,
    MAX17055_FULLSOCTHR_REG             = 0x13,
    MAX17055_CYCLES_REG                 = 0x17,
    MAX17055_DESIGNCAP_REG              = 0x18,
    MAX17055_AVGVCELL_REG               = 0x19,
    MAX17055_MAXMINVOLT_REG             = 0x1B,
    MAX17055_CONFIG_REG                 = 0x1D,
    MAX17055_ICHGTERM_REG               = 0x1E,

    MAX17055_VERSION_REG                = 0x21,
    MAX17055_QRTABLE10_REG              = 0x22,
    MAX17055_FULLCAPNOM_REG             = 0x23,
    MAX17055_LEARNCFG_REG               = 0x28,
    MAX17055_RELAXCFG_REG               = 0x2A,
    MAX17055_TGAIN_REG                  = 0x2C,
    MAX17055_TOFF_REG                   = 0x2D,

    MAX17055_QRTABLE20_REG              = 0x32,
    MAX17055_RCOMP0_REG                 = 0x38,
    MAX17055_TEMPCO_REG                 = 0x39,
    MAX17055_VEMPTY_REG                 = 0x3A,
    MAX17055_FSTAT_REG                  = 0x3D,

    MAX17055_QRTABLE30_REG              = 0x42,
    MAX17055_DQACC_REG                  = 0x45,
    MAX17055_DPACC_REG                  = 0x46,
    MAX17055_VFSOC0_REG                 = 0x48,
    MAX17055_QH0_REG                    = 0x4C,
    MAX17055_QH_REG                     = 0x4D,

    MAX17055_VFSOC0_QH0_LOCK_REG        = 0x60,
    MAX17055_LOCK1_REG                  = 0x62,
    MAX17055_LOCK2_REG                  = 0x63,

    MAX17055_MODELDATA_START_REG        = 0x80,

    MAX17055_IALRTTH_REG                = 0xB4,
    MAX17055_CURVE_REG                  = 0xB9,
    MAX17055_HIBCFG_REG                 = 0xBA,
    MAX17055_CONFIG2_REG                = 0xBB,

    MAX17055_MODELCFG_REG               = 0xDB,

    MAX17055_OCV_REG                    = 0xFB,
    MAX17055_VFSOC_REG                  = 0xFF,
};

int WriteRegister (u8 reg, u16 value) {

    u8 buffer[3];
    buffer[0] = reg;

    buffer[1] = value & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;

    int ret = HalI2CWrite(buffer, 3);

    return ret;
}


u16 ReadRegister (u8 reg)  {

    u8 buffer[2];
    int ret = HalSensorReadReg(reg, buffer, 2);

    u16 val = buffer[0] | (buffer[1] << 8);

    return val;

}

void WriteAndVerifyRegister (u8 RegisterAddress, u16 RegisterValueToWrite){

    int attempt=0;
    u16 RegisterValueRead;

    do {

        WriteRegister (RegisterAddress, RegisterValueToWrite);
        Haptics_WaitUs(1000); //1ms
        RegisterValueRead = ReadRegister (RegisterAddress) ;

    } while (RegisterValueToWrite != RegisterValueRead && attempt++<3);

}


void MAX17055_init(void) {

    // Configure ModelCfg.ModelID = 6 to enable LiFePO4 mode

    u16 designCap = 0x190; // LSB = 0.5mah with 0.01 ohms
    u16 ichgterm = 0x333; // LSB = 1.5625μV/RSENSE
    u16 modelcfg = 0x8000 | (6u << 4u);
    u16 VEmpty = 0x8C02; // LSB = 0.078125mV

    float ChargeVoltage = 3.6f;

    u16 dQAcc = (designCap / 32);

    u16 StatusPOR = ReadRegister(MAX17055_STATUS_REG) & 0x0002;

    if (StatusPOR==0){
        goto Step_4_3;
    } //then go to Step 4.3. else { //then do Steps 2–3.}

Step_2:

    while(ReadRegister(MAX17055_FSTAT_REG) & 0x1) {
        Haptics_WaitUs(10000);
    } //do not continue until FSTAT.DNR == 0

Step_3:

    {
        u16 HibCFG = ReadRegister(MAX17055_HIBCFG_REG); //Store original HibCFG value
        WriteRegister(MAX17055_VFSOC0_QH0_LOCK_REG, 0x90); // Exit Hibernate Mode step 1
        WriteRegister(MAX17055_HIBCFG_REG, 0x0); // Exit Hibernate Mode step 2
        WriteRegister(MAX17055_VFSOC0_QH0_LOCK_REG, 0x0); // Exit Hibernate Mode step 3

        // 3.1 OPTION 1 EZ Config (no INI file is needed):

        WriteRegister(MAX17055_DESIGNCAP_REG, designCap); // Write DesignCap
        WriteRegister(MAX17055_DQACC_REG, dQAcc); //Write dQAcc
        WriteRegister(MAX17055_ICHGTERM_REG, ichgterm); // Write IchgTerm
        WriteRegister(MAX17055_VEMPTY_REG, VEmpty); // Write VEmpty
        //Only use integer portion of dQAcc=int(DesignCap/32) in the calculation of dPAcc to avoid quantization of FullCapNom
        if (ChargeVoltage > 4.275f) {
            WriteRegister(MAX17055_DPACC_REG, dQAcc * 51200 / designCap); // Write dPAcc
            WriteRegister(MAX17055_MODELCFG_REG, modelcfg | 0x400); // Write ModelCFG
        } else {
            WriteRegister(MAX17055_DPACC_REG, dQAcc * 44138 / designCap); //Write dPAcc
            WriteRegister(MAX17055_MODELCFG_REG, modelcfg); // Write ModelCFG
        }
        //Poll ModelCFG.Refresh(highest bit), proceed to Step 4 when ModelCFG.Refresh = 0.
        while (ReadRegister(MAX17055_MODELCFG_REG) & 0x8000) {
            Haptics_WaitUs(10000); //10ms wait loop. Do not continue until ModelCFG.Refresh == 0.
        }

        WriteRegister(MAX17055_HIBCFG_REG, HibCFG); // Restore Original HibCFG value
    }
    // Proceed to Step 4.

Step_4_0:

    {
        u16 Status = ReadRegister(MAX17055_STATUS_REG); //Read Status
        WriteAndVerifyRegister(MAX17055_STATUS_REG, Status & 0xFFFD); //Write and Verify Status with POR bit cleared

        Status = ReadRegister(MAX17055_STATUS_REG); //Read Status
        WriteAndVerifyRegister(MAX17055_STATUS_REG, Status & 0xFFFD); //Write and Verify Status with POR bit cleared
    }

Step_4_3:

    {
        u16 RepCap = ReadRegister(MAX17055_REPCAP_REG); //Read RepCap
        u16 RepSOC = ReadRegister(MAX17055_REPSOC_REG); //Read RepSOC
        u16 TTE = ReadRegister(MAX17055_TTE_REG) ; //Read TTE
     }

}
