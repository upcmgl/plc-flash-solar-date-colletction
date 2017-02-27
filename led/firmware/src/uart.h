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

#ifndef _UART_H    /* Guard against multiple inclusion */
#define _UART_H

#include "app.h"
#include "protocolPLC.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */
void uart2Init();


extern volatile uint8_t recvComplete;
extern uint8_t tranComplete;
extern uint16_t TranCnt;
#define uartReceiveStateBegin     0
#define uartReceiveStateGoOn      1 
extern uint8_t uartReceiveState;

extern uint8_t recvBufferLens2;    //define the length of receiver buffer

void startUart2Recv();
struct uartBuffer    //define the template of uart date 
{
    uint8_t  *date;
    uint32_t lastTime;
    uint8_t  complete;
    uint8_t  lens;
    uint8_t  cnt;     //be used for count the date numbers;
};

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


    


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
