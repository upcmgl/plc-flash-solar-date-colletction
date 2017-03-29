/*******************************************************************************
********************************************************************************
**
**  Filename:       drv_i2c.c
**  Copyright(c):   2017 Topscomm. All right reserved.
**  Author:         mgl
**  Date:           2017.3.25
**  Device:         MicroInverter Collector, MCU: PIC32MX664F128L
**  Modify by:
**  Modify date:
**  Version:        1.0.0
**  Describe:
**
**
********************************************************************************
*******************************************************************************/
#include "drv_i2c.h"
#include "system/clk/sys_clk.h"
#include "../include/stdint.h"
// *****************************************************************************
// *****************************************************************************
// Section: Instance 0 static driver functions
// *****************************************************************************
// *****************************************************************************
#define CPO_COUNT_US  (SYS_FREQUENCY/2/1000000)
static void Delay_us(uint32_t us)
{
    uint32_t start_tick;
    uint32_t timeout_tick;
    start_tick = _CP0_GET_COUNT();
    timeout_tick = us*(SYS_FREQUENCY/2)/1000000;
//    timeout_tick = us*99;
    while((_CP0_GET_COUNT()-start_tick)<timeout_tick);
}
void DRV_I2C_Initialize(void)
{
    /* Disable I2C0 */
    PLIB_I2C_Disable(I2C_ID_3);
    /* Initialize I2C0 */
    PLIB_I2C_BaudRateSet(I2C_ID_3, SYS_CLK_PeripheralFrequencyGet(CLK_BUS_PERIPHERAL_2), 100000);
    PLIB_I2C_StopInIdleDisable(I2C_ID_3);

    /* Low frequency is enabled (**NOTE** PLIB function revereted) */
    PLIB_I2C_HighFrequencyEnable(I2C_ID_3);

    /* Enable I2C0 */
    PLIB_I2C_Enable(I2C_ID_3);
    Delay_us(10);
}

void DRV_I2C_DeInitialize(void)
{
    /* Disable I2C0 */
    PLIB_I2C_Disable(I2C_ID_3);
}

// *****************************************************************************
// Section: Instance 0 Byte Transfer Functions (Master/Slave)
// *****************************************************************************
bool DRV_I2C_SetUpByteRead(void)
{
    /* Check for recieve overflow */
    if ( PLIB_I2C_ReceiverOverflowHasOccurred(I2C_ID_3))
    {
        PLIB_I2C_ReceiverOverflowClear(I2C_ID_3);
        return false;
    }

    /* Initiate clock to receive */
    PLIB_I2C_MasterReceiverClock1Byte(I2C_ID_3);
    return true;
}

bool DRV_I2C_WaitForReadByteAvailable(uint32_t time_out_us)
{
    uint32_t start_tick;
    start_tick = _CP0_GET_COUNT();
	/* Wait for Recieve Buffer Full */
    while(!PLIB_I2C_ReceivedByteIsAvailable(I2C_ID_3))
    {
        if((_CP0_GET_COUNT() - start_tick)>=(time_out_us*CPO_COUNT_US))
        {
            return false;
        }
    }
    return true;
}

uint8_t DRV_I2C_ByteRead(void)
{
    /* Return recieved value */
    return (PLIB_I2C_ReceivedByteGet(I2C_ID_3));
}

bool DRV_I2C_ByteWrite(const uint8_t byte)
{
    /* Check for recieve overflow */
    if ( PLIB_I2C_ReceiverOverflowHasOccurred(I2C_ID_3))
    {
        PLIB_I2C_ReceivedByteGet(I2C_ID_3);
        PLIB_I2C_ReceiverOverflowClear(I2C_ID_3);
        return false;
    }

    /* Check for transmit overflow */
    if (PLIB_I2C_TransmitterOverflowHasOccurred(I2C_ID_3))
    {
        PLIB_I2C_TransmitterOverflowClear(I2C_ID_3);
        return false;
    }

    /* Transmit byte */
    PLIB_I2C_TransmitterByteSend(I2C_ID_3, byte);

    return true;
}

bool DRV_I2C_WaitForByteWriteToComplete(uint32_t time_out_us)
{
    uint32_t start_tick;
    start_tick = _CP0_GET_COUNT();
    /* Check for transmit busy */
    while(PLIB_I2C_TransmitterIsBusy(I2C_ID_3))
    {
        if((_CP0_GET_COUNT() - start_tick)>=(time_out_us*CPO_COUNT_US))
        {
            return false;
        }
    }

    /* Check to see if transmit has completed */
    while(!PLIB_I2C_TransmitterByteHasCompleted(I2C_ID_3))
    {
        if((_CP0_GET_COUNT() - start_tick)>=(time_out_us*CPO_COUNT_US))
        {
            return false;
        }
    }

    return true;
}

bool DRV_I2C_WriteByteAcknowledged(void)
{
    /* Check to see if transmit ACKed = true or NACKed = false */
    if (PLIB_I2C_TransmitterByteWasAcknowledged(I2C_ID_3))
       return true;
    else
       return false;
}

// *****************************************************************************
// Section: Instance 0 I2C Master Mode Functions
// *****************************************************************************
void DRV_I2C_BaudRateSet(I2C_BAUD_RATE baudRate)
{
    /* Disable I2C0 */
    PLIB_I2C_Disable(I2C_ID_3);

    /* Change baud rate */
    PLIB_I2C_BaudRateSet(I2C_ID_3, SYS_CLK_PeripheralFrequencyGet(CLK_BUS_PERIPHERAL_2), baudRate);

    /* Low frequency is enabled (**NOTE** PLIB function revereted) */
    PLIB_I2C_HighFrequencyEnable(I2C_ID_3);

    /* Enable I2C0 */
    PLIB_I2C_Enable(I2C_ID_3);
}

bool DRV_I2C_MasterBusIdle(void)
{
    if (PLIB_I2C_BusIsIdle(I2C_ID_3))
       return true;
    else
       return false;
}

bool DRV_I2C_MasterStart(void)
{
    /* Check for recieve overflow */
    if ( PLIB_I2C_ReceiverOverflowHasOccurred(I2C_ID_3))
    {
        PLIB_I2C_ReceiverOverflowClear(I2C_ID_3);
        return false;
    }

    /* Check for transmit overflow */
    if (PLIB_I2C_TransmitterOverflowHasOccurred(I2C_ID_3))
    {
        PLIB_I2C_TransmitterOverflowClear(I2C_ID_3);
        return false;
    }

    /* Check for bus collision errors */
    if (PLIB_I2C_ArbitrationLossHasOccurred(I2C_ID_3))
    {
        return false;
    }

    /* Issue start */
    PLIB_I2C_MasterStart(I2C_ID_3);

    return true;
}

bool DRV_I2C_WaitForStartComplete(uint32_t time_out_us)
{
    uint32_t start_tick;
    start_tick = _CP0_GET_COUNT();
    /* Wait for start/restart sequence to finish  (hardware clear) */
    while(!PLIB_I2C_StartWasDetected(I2C_ID_3))
    {
        if((_CP0_GET_COUNT() - start_tick)>=(time_out_us*CPO_COUNT_US))
        {
            return false;
        }
    }
    while(!PLIB_I2C_BusIsIdle(I2C_ID_3))
    {
        if((_CP0_GET_COUNT() - start_tick)>=(time_out_us*CPO_COUNT_US))
        {
            return false;
        }
    }

    return true;
}

bool DRV_I2C_MasterRestart(void)
{
    /* Check for recieve overflow */
    if ( PLIB_I2C_ReceiverOverflowHasOccurred(I2C_ID_3))
    {
        PLIB_I2C_ReceiverOverflowClear(I2C_ID_3);
        return false;
    }

    /* Check for transmit overflow */
    if (PLIB_I2C_TransmitterOverflowHasOccurred(I2C_ID_3))
    {
        PLIB_I2C_TransmitterOverflowClear(I2C_ID_3);
        return false;
    }

    /* Issue restart */
    PLIB_I2C_MasterStartRepeat(I2C_ID_3);

    return true;
}

bool DRV_I2C_MasterStop(void)
{
    /* Check for transmit overflow */
    if (PLIB_I2C_TransmitterOverflowHasOccurred(I2C_ID_3))
    {
        PLIB_I2C_TransmitterOverflowClear(I2C_ID_3);
        return false;
    }

    /* Issue stop */
    PLIB_I2C_MasterStop(I2C_ID_3);
    PLIB_I2C_MasterStop(I2C_ID_3);
    return true;
}

bool DRV_I2C_WaitForStopComplete(uint32_t time_out_us)
{
    uint32_t start_tick;
    start_tick = _CP0_GET_COUNT();
    while (!PLIB_I2C_StopWasDetected(I2C_ID_3))
    {
        if((_CP0_GET_COUNT() - start_tick)>=(time_out_us*CPO_COUNT_US))
        {
            return false;
        }
    }
    while(!PLIB_I2C_BusIsIdle(I2C_ID_3))
    {
        if((_CP0_GET_COUNT() - start_tick)>=(time_out_us*CPO_COUNT_US))
        {
            return false;
        }
    }

    return true;
}

void DRV_I2C_MasterACKSend(void)
{
    /* Check if receive is ready to ack */
    if ( PLIB_I2C_MasterReceiverReadyToAcknowledge(I2C_ID_3) )
    {
        PLIB_I2C_ReceivedByteAcknowledge (I2C_ID_3, true);
    }

}

void DRV_I2C_MasterNACKSend(void)
{
    /* Check if receive is ready to nack */
    if ( PLIB_I2C_MasterReceiverReadyToAcknowledge(I2C_ID_3) )
    {
       PLIB_I2C_ReceivedByteAcknowledge (I2C_ID_3, false);
    }
}

bool DRV_I2C_WaitForACKOrNACKComplete(uint32_t time_out_us)
{
    uint32_t start_tick;
    start_tick = _CP0_GET_COUNT();
    /* Check for ACK/NACK to complete */
    while(!PLIB_I2C_BusIsIdle(I2C_ID_3))
    {
        if((_CP0_GET_COUNT() - start_tick)>=(time_out_us*CPO_COUNT_US))
        {
            return false;
        }
    }
//    while(PLIB_I2C_ReceiverByteAcknowledgeHasCompleted(I2C_ID_3))
//    {
//        if((_CP0_GET_COUNT() - start_tick)>=(time_out_us*CPO_COUNT_US))
//        {
//            return false;
//        }
//    }

    return true;
}

void DRV_I2C_Errata32MZ_I2CModuleToggle(void)
{
    PLIB_I2C_Disable(I2C_ID_3);
    PLIB_I2C_Enable(I2C_ID_3);
    Delay_us(10);
}
/*******************************************************************************
 End of File
*/