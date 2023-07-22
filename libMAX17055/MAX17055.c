//
// Created by vgol on 13/07/2023.
//

#include "MAX17055.h"
#include "hal_i2c.h"

#define BIT(X)       (1 << (X))

#define MAX17055_DEVICE_ID          0x4010

/* Config reg (0x1d) flags */
#define CONF_AEN                    BIT(2)
#define CONF_IS                     BIT(11)
#define CONF_VS                     BIT(12)
#define CONF_TS                     BIT(13)
#define CONF_SS                     BIT(14)
#define CONF_TSEL                   BIT(15)
#define CONF_ALL_STICKY             (CONF_IS | CONF_VS | CONF_TS | CONF_SS)

/* Status reg (0x00) flags */
#define STATUS_POR                  BIT(1)
#define STATUS_IMN                  BIT(2)
#define STATUS_BST                  BIT(3)
#define STATUS_IMX                  BIT(6)
#define STATUS_VMN                  BIT(8)
#define STATUS_TMN                  BIT(9)
#define STATUS_SMN                  BIT(10)
#define STATUS_VMX                  BIT(12)
#define STATUS_TMX                  BIT(13)
#define STATUS_SMX                  BIT(14)
#define STATUS_ALL_ALRT                                                    \
	(STATUS_IMN | STATUS_IMX | STATUS_VMN | STATUS_VMX | STATUS_TMN |      \
	 STATUS_TMX | STATUS_SMN | STATUS_SMX)

#define BATTERY_MAX17055_RSENSE_MUL 500
#define BATTERY_MAX17055_RSENSE_DIV 1000

#define MAX17055_MAX_MIN_REG(mx, mn) ((((int16_t)(mx)) << 8) | ((mn)))
/* Converts voltages alert range for VALRTTH_REG */
#define MAX17055_VALRTTH_RESOLUTION 20
#define MAX17055_VALRTTH_REG(mx, mn)                                           \
	MAX17055_MAX_MIN_REG((uint8_t)(mx / MAX17055_VALRTTH_RESOLUTION),      \
			     (uint8_t)(mn / MAX17055_VALRTTH_RESOLUTION))
/* Converts temperature alert range for TALRTTH_REG */
#define MAX17055_TALRTTH_REG(mx, mn)                                           \
	MAX17055_MAX_MIN_REG((int8_t)(mx), (int8_t)(mn))
/* Converts state-of-charge alert range for SALRTTH_REG */
#define MAX17055_SALRTTH_REG(mx, mn)                                           \
	MAX17055_MAX_MIN_REG((uint8_t)(mx), (uint8_t)(mn))
/* Converts current alert range for IALRTTH_REG */
/* Current resolution: 0.4mV/RSENSE */
#define MAX17055_IALRTTH_MUL (10)
#define MAX17055_IALRTTH_DIV (4)
#define MAX17055_IALRTTH_REG(mx, mn)                                           \
	MAX17055_MAX_MIN_REG(                                                  \
		(int8_t)(mx * MAX17055_IALRTTH_MUL * BATTERY_MAX17055_RSENSE_MUL / (MAX17055_IALRTTH_DIV * BATTERY_MAX17055_RSENSE_DIV)),    \
		(int8_t)(mn * MAX17055_IALRTTH_MUL * BATTERY_MAX17055_RSENSE_MUL / (MAX17055_IALRTTH_DIV * BATTERY_MAX17055_RSENSE_DIV)))


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

    int ret = HalI2CWrite(buffer, sizeof(buffer));

    return ret;
}


u16 ReadRegister (u8 reg)  {

    u8 buffer[2] = { 0xFF, 0xFF };
    int ret = HalSensorReadReg(reg, buffer, sizeof(buffer));

    u16 val = buffer[0] | (buffer[1] << 8);

    return val;

}

void WriteAndVerifyRegister (u8 RegisterAddress, u16 RegisterValueToWrite){

    int attempt=0;
    u16 RegisterValueRead;

    do {

        WriteRegister (RegisterAddress, RegisterValueToWrite);
        Hal_WaitUs(1000); //1ms
        RegisterValueRead = ReadRegister (RegisterAddress) ;

    } while (RegisterValueToWrite != RegisterValueRead && attempt++<3);

}

void MAX17055_EnableIAlert(void) {

    // https://chromium.googlesource.com/chromiumos/platform/ec/+/master/driver/battery/max17055.c

    // Initial Value: 0x2210 for Config, 0x3658 for Config2

    u16 cfg1 = ReadRegister(MAX17055_CONFIG_REG);
    u16 cfg2 = ReadRegister(MAX17055_CONFIG2_REG);
    u16 st = ReadRegister(MAX17055_STATUS_REG);

    // Aen: CFG1 D2 (Enable ALRT Pin Output)
    // When Aen = 1, violation if any of the alert threshold register values by
    // temperature, voltage, current, or SOC triggers an alert. This bit affects the ALRT pin operation only

    u16 i_alert_mxmn = MAX17055_IALRTTH_REG(8, 2); // 0.4mV/RSENSE resolution => 40mA with 0.01 ohms => 4mA with 0.1 ohms
    printf("Setting IAlert to 0x%04X \n", i_alert_mxmn);

    WriteRegister(MAX17055_IALRTTH_REG, i_alert_mxmn);
	/* Disable all sticky bits; enable alert AEN */
    WriteRegister(MAX17055_CONFIG_REG, (cfg1 & ~CONF_ALL_STICKY) | CONF_AEN);
	/* Clear alerts */
    WriteRegister(MAX17055_STATUS_REG, st & ~STATUS_ALL_ALRT);
}

// COMMSH: CFG1 D6 (Communication Shutdown):

void MAX17055_init(void) {

    // Configure ModelCfg.ModelID = 6 to enable LiFePO4 mode

    u16 designCap = 0x190; // LSB = 0.5mah with 0.01 ohms
    u16 ichgterm = 0x333; // LSB = 1.5625μV/RSENSE
    u16 modelcfg = 0x8000 | (6u << 4u);
    u16 VEmpty = 0x8C02; // LSB = 0.078125mV

    u16 chg_V_high = 51200; // scaling factor high voltage charger
    u16 chg_V_low = 44138;

    float ChargeVoltage = 3.6f;

    u16 dQAcc = (designCap >> 5);

    HalI2CInit(0x36);

    u16 version = ReadRegister(MAX17055_VERSION_REG);
    LOG("MAX17055 ID: %04X vs %04X \n", version, MAX17055_DEVICE_ID);

    if (version != MAX17055_DEVICE_ID) {
        return;
    }

    u16 StatusPOR = ReadRegister(MAX17055_STATUS_REG);
    LOG("MAX17055 POR: %04X \n", StatusPOR);

    u16 HibCFG = ReadRegister(MAX17055_HIBCFG_REG); //Store original HibCFG value
    LOG("MAX17055 HIBCFG: %04X \n", HibCFG);

    if ((StatusPOR & 0x0002)==0){
        goto Step_4_3;
    } //then go to Step 4.3. else { //then do Steps 2–3.}

Step_2:

    {
        u16 fstat;
        while ((fstat = ReadRegister(MAX17055_FSTAT_REG)) & 0x1) {
            LOG("MAX17055 FSTAT: %04X \n", fstat);
            Hal_WaitUs(100000);
        } //do not continue until FSTAT.DNR == 0}
    }

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
            WriteRegister(MAX17055_DPACC_REG, dQAcc * chg_V_high / designCap); // Write dPAcc
            WriteRegister(MAX17055_MODELCFG_REG, modelcfg | 0x400); // Write ModelCFG
        } else {
            WriteRegister(MAX17055_DPACC_REG, dQAcc * chg_V_low / designCap); //Write dPAcc
            WriteRegister(MAX17055_MODELCFG_REG, modelcfg); // Write ModelCFG
        }
        //Poll ModelCFG.Refresh(highest bit), proceed to Step 4 when ModelCFG.Refresh = 0.
        while (ReadRegister(MAX17055_MODELCFG_REG) & 0x8000) {
            Hal_WaitUs(10000); //10ms wait loop. Do not continue until ModelCFG.Refresh == 0.
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
