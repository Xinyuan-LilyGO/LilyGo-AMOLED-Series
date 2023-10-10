/**
 *
 * @license MIT License
 *
 * Copyright (c) 2022 lewis he
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      PowersSY6970.tpp
 * @author    Lewis He (lewishe@outlook.com)
 * @date      2023-07-20
 *
 */
#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <math.h>
#endif /*ARDUINO*/
#include "XPowersCommon.tpp"
#include "REG/SY6970Constants.h"
// #include "XPowersLibInterface.hpp"

enum PowersSY6970BusStatus {
    POWERS_SY_NOINPUT,
    POWERS_SY_USB_SDP,
    POWERS_SY_USB_CDP,
    POWERS_SY_USB_DCP,
    POWERS_SY_HVDCP,
    POWERS_SY_UNKONW_ADAPTER,
    POWERS_SY_NO_STANDARD_ADAPTER,
    POWERS_SY_OTG
} ;

enum PowersSY6970ChargeStatus {
    POWERS_SY_NO_CHARGE,
    POWERS_SY_PRE_CHARGE,
    POWERS_SY_FAST_CHARGE,
    POWERS_SY_CHARGE_DONE,
    POWERS_SY_CHARGE_UNKOWN,
} ;

enum PowersSY6970NTCStatus {
    POWERS_SY_BUCK_NTC_NORMAL,
    POWERS_SY_BUCK_NTC_WARM,
    POWERS_SY_BUCK_NTC_COOL,
    POWERS_SY_BUCK_NTC_COLD,
    POWERS_SY_BUCK_NTC_HOT,
    POWERS_SY_BOOST_NTC_NORMAL,
    POWERS_SY_BOOST_NTC_COLD,
    POWERS_SY_BOOST_NTC_HOT,
};

enum SY6970_WDT_Timeout {
    SY6970_WDT_TIMEROUT_40SEC,      //40 Second
    SY6970_WDT_TIMEROUT_80SEC,      //80 Second
    SY6970_WDT_TIMEROUT_160SEC,     //160 Second
} ;

class PowersSY6970 :
    public XPowersCommon<PowersSY6970> //, public XPowersLibInterface
{
    friend class XPowersCommon<PowersSY6970>;

public:

#if defined(ARDUINO)
    PowersSY6970(TwoWire &w, int sda = SDA, int scl = SCL, uint8_t addr = SY6970_SLAVE_ADDRESS)
    {
        __wire = &w;
        __sda = sda;
        __scl = scl;
        __addr = addr;
    }
#endif

    PowersSY6970()
    {
#if defined(ARDUINO)
        __wire = &Wire;
        __sda = SDA;
        __scl = SCL;
#endif
        __addr = SY6970_SLAVE_ADDRESS;
    }

    ~PowersSY6970()
    {
        log_i("~PowersSY6970");
        deinit();
    }

#if defined(ARDUINO)
    bool init(TwoWire &w, int sda = SDA, int scl = SCL, uint8_t addr = SY6970_SLAVE_ADDRESS)
    {
        __wire = &w;
        __sda = sda;
        __scl = scl;
        __addr = addr;
        return begin();
    }
#endif

    uint8_t getChipID()
    {
        int res = readRegister(POWERS_SY6970_REG_14H);
        return (res & 0x03);
    }

    bool init()
    {
        return begin();
    }

    void deinit()
    {
        end();
    }

    ///REG0B
    bool isVbusIn()
    {
        return getBusStatus() != POWERS_SY_NOINPUT;
    }

    bool isOTG()
    {
        return getBusStatus() == POWERS_SY_OTG;
    }

    bool isCharging(void)
    {
        return chargeStatus() != POWERS_SY_NO_CHARGE;
    }

    bool isChargeDone()
    {
        return chargeStatus() != POWERS_SY_CHARGE_DONE;
    }

    bool isBatteryConnect(void)
    {
        //TODO:
        return false;
    }

    bool isPowerGood()
    {
        return getRegisterBit(POWERS_SY6970_REG_0BH, 2);
    }

    bool isEnableCharge()
    {
        return getRegisterBit(POWERS_SY6970_REG_03H, 4);
    }

    void disableCharge()
    {
        clrRegisterBit(POWERS_SY6970_REG_03H, 4);
    }

    void enableCharge()
    {
        setRegisterBit(POWERS_SY6970_REG_03H, 4);
    }

    bool isEnableOTG()
    {
        return getRegisterBit(POWERS_SY6970_REG_03H, 5);
    }

    void disableOTG()
    {
        clrRegisterBit(POWERS_SY6970_REG_03H, 5);
    }

    void enableOTG()
    {
        setRegisterBit(POWERS_SY6970_REG_03H, 5);
    }


    void feedWatchdog()
    {
        setRegisterBit(POWERS_SY6970_REG_03H, 6);
    }

    void disableWatchdog()
    {
        int regVal = readRegister(POWERS_SY6970_REG_07H);
        regVal  &= 0xCF;
        writeRegister(POWERS_SY6970_REG_07H, regVal);
    }

    void enableWatchdog(enum SY6970_WDT_Timeout val = SY6970_WDT_TIMEROUT_40SEC )
    {
        int regVal = readRegister(POWERS_SY6970_REG_07H);
        regVal  &= 0xCF;
        switch (val) {
        case SY6970_WDT_TIMEROUT_40SEC:
            writeRegister(POWERS_SY6970_REG_07H, regVal | 0x10);
            break;
        case SY6970_WDT_TIMEROUT_80SEC:
            writeRegister(POWERS_SY6970_REG_07H, regVal | 0x20);
            break;
        case SY6970_WDT_TIMEROUT_160SEC:
            writeRegister(POWERS_SY6970_REG_07H, regVal | 0x30);
            break;
        default:
            break;
        }
    }


    bool isEnableBattery()
    {
        return getRegisterBit(POWERS_SY6970_REG_03H, 7);
    }

    void disableBattery()
    {
        clrRegisterBit(POWERS_SY6970_REG_03H, 7);
    }

    void enableBattery()
    {
        setRegisterBit(POWERS_SY6970_REG_03H, 7);
    }

    bool isEnableStatLed()
    {
        return getRegisterBit(POWERS_SY6970_REG_07H, 6) == false;
    }

    void disableStatLed()
    {
        setRegisterBit(POWERS_SY6970_REG_07H, 6);
    }

    void enableStatLed()
    {
        clrRegisterBit(POWERS_SY6970_REG_07H, 6);
    }


    PowersSY6970BusStatus getBusStatus()
    {
        int val =  readRegister(POWERS_SY6970_REG_0BH);
        return (PowersSY6970BusStatus)((val >> 5) & 0x07);
    }

    const char *getBusStatusString()
    {
        PowersSY6970BusStatus status = getBusStatus();
        switch (status) {
        case POWERS_SY_NOINPUT:
            return "No input";
        case POWERS_SY_USB_SDP:
            return "USB Host SDP";
        case POWERS_SY_USB_CDP:
            return "USB CDP";
        case POWERS_SY_USB_DCP:
            return "USB DCP";
        case POWERS_SY_HVDCP:
            return "HVDCP";
        case POWERS_SY_UNKONW_ADAPTER:
            return "Unknown Adapter";
        case POWERS_SY_NO_STANDARD_ADAPTER:
            return "Non-Standard Adapter";
        case POWERS_SY_OTG:
            return "OTG";
        default:
            return "Unknown";
        }
    }

    PowersSY6970ChargeStatus chargeStatus()
    {
        int val =  readRegister(POWERS_SY6970_REG_0BH);
        if (val == -1)return POWERS_SY_CHARGE_UNKOWN;
        return (PowersSY6970ChargeStatus)((val >> 3) & 0x03);
    }

    const char *getChargeStatusString()
    {
        PowersSY6970ChargeStatus status = chargeStatus();
        switch (status) {
        case POWERS_SY_NO_CHARGE:
            return "Not Charging";
        case POWERS_SY_PRE_CHARGE:
            return "Pre-charge";
        case POWERS_SY_FAST_CHARGE:
            return "Fast Charging";
        case POWERS_SY_CHARGE_DONE:
            return "Charge Termination Done";
        default:
            return "Unknown";
        }
    }

    PowersSY6970NTCStatus getNTCStatus()
    {
        int val =  readRegister(POWERS_SY6970_REG_0CH);
        return (PowersSY6970NTCStatus)(val & 0x07);
    }

    const char *getNTCStatusString()
    {
        PowersSY6970NTCStatus status = getNTCStatus();
        switch (status) {
        case POWERS_SY_BUCK_NTC_NORMAL:
            return "Buck mode NTC normal";
        case POWERS_SY_BUCK_NTC_WARM:
            return "Buck mode NTC warm";
        case POWERS_SY_BUCK_NTC_COOL:
            return "Buck mode NTC cool";
        case POWERS_SY_BUCK_NTC_COLD:
            return "Buck mode NTC cold";
        case POWERS_SY_BUCK_NTC_HOT:
            return "Buck mode NTC hot";
        case POWERS_SY_BOOST_NTC_NORMAL:
            return "Boost mode NTC hot";
        case POWERS_SY_BOOST_NTC_COLD:
            return "Boost mode NTC cold";
        case POWERS_SY_BOOST_NTC_HOT:
            return "Boost mode NTC hot";
        default:
            return "Unknown";
        }
    }

    bool isWatchdogNormal()
    {
        return getRegisterBit(POWERS_SY6970_REG_0CH, 7) == false;
    }

    bool isBoostNormal()
    {
        return getRegisterBit(POWERS_SY6970_REG_0CH, 6) == false;
    }

    bool isChargeNormal()
    {
        int val = readRegister(POWERS_SY6970_REG_0CH);
        return ((val & 0x30) >> 3) == 0;
    }

    bool isBatteryNormal()
    {
        return getRegisterBit(POWERS_SY6970_REG_0CH, 3) == false;
    }

    bool isNtcNormal()
    {
        int val = readRegister(POWERS_SY6970_REG_0CH);
        return (val & 0x7) == 0;
    }

    bool enableADCMeasure()
    {
        int val = readRegister(POWERS_SY6970_REG_02H);
        val |= _BV(7);
        val |= _BV(6);
        return writeRegister(POWERS_SY6970_REG_02H, val);
    }

    bool disableADCMeasure()
    {
        int val = readRegister(POWERS_SY6970_REG_02H);
        val &= (~_BV(7));
        val &= (~_BV(6));
        return writeRegister(POWERS_SY6970_REG_02H, val);
    }



    uint16_t getVbusVoltage()
    {
        if (!isVbusIn()) {
            return 0;
        }
        int val = readRegister(POWERS_SY6970_REG_11H);
        return (POWERS_SY6970_VBUS_MASK_VAL(val) * POWERS_SY6970_VBUS_VOL_STEP) + POWERS_SY6970_VBUS_BASE_VAL;
    }

    uint16_t getBattVoltage()
    {
        int val = readRegister(POWERS_SY6970_REG_0EH);
        val = POWERS_SY6970_VBAT_MASK_VAL(val);
        if (val == 0)return 0;
        return (val * POWERS_SY6970_VBAT_VOL_STEP) + POWERS_SY6970_VBAT_BASE_VAL;
    }

    uint16_t getSystemVoltage()
    {
        int val = readRegister(POWERS_SY6970_REG_0FH);
        return (POWERS_SY6970_VSYS_MASK_VAL(val) * POWERS_SY6970_VSYS_VOL_STEP) + POWERS_SY6970_VSYS_BASE_VAL;
    }

    // Range: 64mA ~ 1024mA ,step:64mA
    bool setPrechargeCurr(uint16_t milliampere)
    {
        if (milliampere % POWERS_SY6970_PRE_CHG_CUR_STEP) {
            log_e("Mistake ! The steps is must %u mA", POWERS_SY6970_PRE_CHG_CUR_STEP);
            return false;
        }
        XPOWERS_CHECK_RANGE(milliampere, POWERS_PRE_CHG_CURRENT_MIN, POWERS_PRE_CHG_CURRENT_MAX, false);
        int val = readRegister(POWERS_SY6970_REG_05H);
        val &= 0x0F;
        milliampere = ((milliampere - POWERS_SY6970_PRE_CHG_CUR_BASE) / POWERS_SY6970_PRE_CHG_CUR_STEP);
        val |=  milliampere << 4;
        return writeRegister(POWERS_SY6970_REG_05H, val) != -1;
    }

    uint16_t getPrechargeCurr(void)
    {
        int val = readRegister(POWERS_SY6970_REG_05H);
        val &= 0xF0;
        val >>= 4;
        return POWERS_SY6970_PRE_CHG_CUR_STEP + (val * POWERS_SY6970_PRE_CHG_CUR_STEP);
    }

    uint16_t getChargerConstantCurr()
    {
        int val = readRegister(POWERS_SY6970_REG_04H);
        val &= 0x7F;
        return val * POWERS_SY6970_FAST_CHG_CUR_STEP;
    }

    // Range:0~5056mA ,step:64mA
    bool setChargerConstantCurr(uint16_t milliampere)
    {
        if (milliampere % POWERS_SY6970_FAST_CHG_CUR_STEP) {
            log_e("Mistake ! The steps is must %u mA", POWERS_SY6970_FAST_CHG_CUR_STEP);
            return false;
        }
        XPOWERS_CHECK_RANGE(milliampere, POWERS_FAST_CHG_CURRENT_MIN, POWERS_FAST_CHG_CURRENT_MAX, false);
        int val = readRegister(POWERS_SY6970_REG_04H);
        val &= 0x80;
        val |= (milliampere / POWERS_SY6970_FAST_CHG_CUR_STEP);
        return  writeRegister(POWERS_SY6970_REG_04H, val) != -1;
    }

    uint16_t getChargeTargetVoltage()
    {
        int val = readRegister(POWERS_SY6970_REG_06H);
        val = (val & 0xFC) >> 2;
        if (val > 0x30) {
            return POWERS_FAST_CHG_VOL_MAX;
        }
        return val * POWERS_SY6970_CHG_VOL_STEP + POWERS_SY6970_CHG_VOL_BASE;
    }

    // Range:3840 ~ 4608mV ,step:16 mV
    bool setChargeTargetVoltage(uint16_t millivolt)
    {
        if (millivolt % POWERS_SY6970_CHG_VOL_STEP) {
            log_e("Mistake ! The steps is must %u mV", POWERS_SY6970_CHG_VOL_STEP);
            return false;
        }
        XPOWERS_CHECK_RANGE(millivolt, POWERS_FAST_CHG_VOL_MIN, POWERS_FAST_CHG_VOL_MAX, false);
        int val = readRegister(POWERS_SY6970_REG_06H);
        val &= 0x03;
        val |= (((millivolt - POWERS_SY6970_CHG_VOL_BASE) / POWERS_SY6970_CHG_VOL_STEP) << 2);
        return  writeRegister(POWERS_SY6970_REG_06H, val) != -1;
    }

    bool initImpl()
    {

        if (getChipID() != 0x00) {
            return false;
        }

        //Default disbale Watchdog
        disableWatchdog();

        return true;
    }

private:

};



