/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _DRV_I2C_H    /* Guard against multiple inclusion */
#define _DRV_I2C_H
#include "../include/stdbool.h"
#include "../include/stdint.h"
#include "../../../Harmony/framework/peripheral/i2c/plib_i2c.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */


/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif
#define SYS_FREQUENCY         (80000000UL)

// *****************************************************************************
// *****************************************************************************
// Section: Interface Headers for Instance 0 for the static driver
// *****************************************************************************
// *****************************************************************************
void DRV_I2C_Initialize(void);

void DRV_I2C_DeInitialize(void);

// *****************************************************************************
// Section: Instance 0 Byte transfer functions (Master/Slave)
// *****************************************************************************
bool DRV_I2C_SetUpByteRead(void);

bool DRV_I2C_MasterBusIdle(void);

bool DRV_I2C_WaitForReadByteAvailable(uint32_t time_out_us);

uint8_t DRV_I2C_ByteRead(void);

bool DRV_I2C_ByteWrite(const uint8_t byte);

bool DRV_I2C_WaitForByteWriteToComplete(uint32_t time_out_us);

bool DRV_I2C_WriteByteAcknowledged(void);

bool DRV_I2C_ReceiverBufferIsEmpty(void);


// *****************************************************************************
// Section: Instance 0 I2C Master functions
// *****************************************************************************
void DRV_I2C_BaudRateSet(I2C_BAUD_RATE baudRate);

bool DRV_I2C_MasterStart(void);

bool DRV_I2C_MasterRestart(void);

bool DRV_I2C_WaitForStartComplete(uint32_t time_out_us);

bool DRV_I2C_MasterStop(void);

bool DRV_I2C_WaitForStopComplete(uint32_t time_out_us);

void DRV_I2C_MasterACKSend(void);

void DRV_I2C_MasterNACKSend(void);

bool DRV_I2C_WaitForACKOrNACKComplete(uint32_t time_out_us);

void DRV_I2C_Errata32MZ_I2CModuleToggle(void);
    
    
    
    
    
    
    
    
    
    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
