/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    protoclPLC.h

  @Summary
    parse the message from the plc.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _PROTOCOLPLC_H    /* Guard against multiple inclusion */
#define _PROTOCOLPLC_H

#include "app.h"
#include "uart.h"
#include "inverterDate.h"

/*
 *define  3 kinds address type ,normal ,broadcast, wildcard(for check the electronic meter serial number) 
 b*/
#define InvalidType         0x00
#define NormalType          0x01    //0x00000000000--->module address
#define BroadcastType       0x02    //0x99999999999
#define WildcardType        0x04    //0xAAAAAAAAAAA

#define MaxDataLens         200     //define the max length of data field

//define the state of pic apply ro
#define ShakeHandIng        0
#define ShakeHandSuccess    1
#define ApplyForVoltage     2
#define ApplyForCurrent     3
#define ApplyForEnergy      4
#define ApplyForFault       5


#define recvBufferLens     50
#define  tranBufferLens    50

//just for debug,so define a moduleAddress
extern uint8_t moduleAddress[6];
extern uint8_t sendBufferLens;
extern uint8_t picApplyState;

extern uint8_t RecvBuffer[];
extern uint8_t TranBuffer[];
extern struct uartBuffer uartBufferPLCRecv;
extern struct uartBuffer uartBufferPLCTran;
struct plcFrameStruct
{
    uint8_t frameWakeUp[4];
    uint8_t frameStart1;
    uint8_t addr[6];
    uint8_t frameStart2;
    uint8_t ctrCode;
    uint8_t length;
    uint8_t data[MaxDataLens];
    uint8_t cs;
    uint8_t frameEnd;  
    uint8_t isLocked;
    uint8_t isParsed;
};

struct uartControlU
{
    uint8_t sendCnt;
    uint8_t nextLoop;
    uint8_t recvComplete;
    uint8_t shakehandSuccess;
    uint8_t recvValid;            /**when the received buffer is blank,the recvInvalid is set 0.**/
};

struct plcControlU
{
    uint8_t picApplyPlcState;
    uint8_t plcApplyPlcSuccess;
    uint8_t plcAckSuccess;
    uint32_t picApplyLastTime;
    uint32_t picApplyTimeInterval;      //design pic apply for voltage and current every 5 mins;
    uint8_t  picApplyIndex;             //record the times of pic  apply microinverter.
    uint8_t picApplyCompleteLoop;
};
/*
 *describe :this structure is for storing the micro Inverter's date.
 */

/* the ControlByte is defined ,but it wasn't used;
typedef union
{
    uint8_t value;
    struct 
    {
        uint8_t CC  : 5;
        uint8_t SFV : 1;
        uint8_t PRM : 1;
        uint8_t DIR : 1;
    };
}ControlByte;
 */
struct plcControlU plcCtl;


struct microInverterDateCtrl invertDateCtrl; //initial 20 inverter date structure;
//define 20 inverter's destination.
extern uint8_t destinationAddr[microInverterNum][6];

uint8_t bufferToFrame(uint8_t *RecvBuffer,uint8_t lens,struct plcFrameStruct *plcRecvFrame,struct uartControlU uartCtl);
void frameParse(struct plcFrameStruct *plcRecvFrame,uint8_t* moduleAddress,uint8_t index);
void checkBufferValidOrInvalid(uint8_t *buffer,uint16_t lens,struct uartControlU uartCtl);
void picApplyFrame(uint8_t picApplyState,uint8_t *destinationAddressTest);
void initPLC();

uint8_t plcService();

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
