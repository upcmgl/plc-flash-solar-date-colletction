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

#ifndef _FILE_H    /* Guard against multiple inclusion */
#define _FILE_H

#include "../include/stdint.h"
#include "../include/stdbool.h"
#include "../include/stddef.h"
#include "../include/stdlib.h"
#include "system_config/default/system_config.h"
#include "system_config/default/system_definitions.h"

#include "nandFlashDrv.h"
#include "inverterDate.h"
/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

    
/*---------------this area is maintain for flash test------------------------*/
    
uint32_t nandID;

extern uint8_t flashWriteBuffer[2048];
extern uint8_t flashWriteBufferOOB[64];

extern uint8_t flashReadBuffer[2048];
extern uint8_t flashReadBufferOOB[64];

//flash date struct for write&read;
struct flashDateStr
{
    struct nanddrv_transfer flashWriteDate[2];
    struct nanddrv_transfer flashReadDate[2];   
};

extern struct flashDateStr flashRWdate;

//define the date struct of the date store system;&it could be very gaint.
struct nandFlashInfoStr
{
    volatile uint16_t badBlockNum;
    volatile uint8_t  badBlockPos[128];  //define the badBlock position ,1K blocks = 128bytes = 1K bits.
    volatile uint16_t blockUsedPresent;
    volatile uint16_t blockUsedNext;
    volatile uint8_t  pageUsedPresent;
    volatile uint8_t  pageUsedNext;

};

extern struct nandFlashInfoStr nandFlashInfo;

void initFlashBuffer();
void initFlashInfo(struct nandFlashInfoStr *nandFlashInfo);
//badBlockCheck function ,it will record the bad block info into the nandFlashInfo struct.
void badBlockCheck (struct nandFlashInfoStr  *nandFlashInfo ,struct flashDateStr *flashRWdate );
uint8_t fileWriteAllPageEcc(uint32_t page,struct flashDateStr *flashRWdate);
uint8_t fileReadAllPageEcc(uint32_t page,struct flashDateStr *flashRWdate,struct nandFlashInfoStr *nandFlashInfo);

//test function --->support the test function.
void fileTest();
/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
