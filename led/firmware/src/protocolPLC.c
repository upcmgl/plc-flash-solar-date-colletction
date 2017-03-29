/*******************************************************************************
********************************************************************************
**
**  Filename:       plc_protocol.c
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

#include "protocolPLC.h"
#include "uart.h"

uint8_t sendBufferLens=0;
uint8_t recvBufferLens2=0;
uint8_t plcState=0;
uint8_t picApplyState=0;

uint32_t picApplyLastTime=0;

struct uartBuffer uartBufferPLCRecv;
struct uartBuffer uartBufferPLCTran;
uint8_t RecvBuffer[recvBufferLens];
uint8_t TranBuffer[tranBufferLens];
//microInverter date collection address.
uint8_t moduleAddress[6] = {0x01,0x00,0x00,0x00,0x00,0x00};
uint8_t destinationAddr[microInverterNum][6]={{0x02,0x00,0x00,0x00,0x00,0x00},
{0x03,0x00,0x00,0x00,0x00,0x00},{0x04,0x00,0x00,0x00,0x00,0x00},
{0x05,0x00,0x00,0x00,0x00,0x00},{0x06,0x00,0x00,0x00,0x00,0x00},
{0x07,0x00,0x00,0x00,0x00,0x00},{0x08,0x00,0x00,0x00,0x00,0x00},
{0x09,0x00,0x00,0x00,0x00,0x00},{0x10,0x00,0x00,0x00,0x00,0x00},
{0x11,0x00,0x00,0x00,0x00,0x00},{0x12,0x00,0x00,0x00,0x00,0x00},
{0x13,0x00,0x00,0x00,0x00,0x00},{0x14,0x00,0x00,0x00,0x00,0x00},
{0x15,0x00,0x00,0x00,0x00,0x00},{0x16,0x00,0x00,0x00,0x00,0x00},
{0x17,0x00,0x00,0x00,0x00,0x00},{0x18,0x00,0x00,0x00,0x00,0x00},
{0x19,0x00,0x00,0x00,0x00,0x00},{0x20,0x00,0x00,0x00,0x00,0x00},
{0x21,0x00,0x00,0x00,0x00,0x00},{0x22,0x00,0x00,0x00,0x00,0x00}};
void initPLC()
{
    //initial  the plc channel
    plcCtl.picApplyLastTime = 0;
    plcCtl.picApplyPlcState=ShakeHandIng;
    plcCtl.picApplyTimeInterval=5;    //every 5 minutes;
    plcCtl.plcAckSuccess=0;
    plcCtl.plcApplyPlcSuccess=0;
    //-----------------init receive & send buffer-------------------//
   uartBufferPLCRecv.complete = 0;
   uartBufferPLCRecv.date     = RecvBuffer;
   uartBufferPLCRecv.lastTime = 0;
   uartBufferPLCRecv.lens     = 0;
   uartBufferPLCRecv.cnt      = 0;
   
   uartBufferPLCTran.complete = 0;
   uartBufferPLCTran.date     = TranBuffer;
   uartBufferPLCTran.lastTime = 0;
   uartBufferPLCTran.lens     = 0;
   uartBufferPLCTran.cnt      = 0;
}
/*
 *Function  Check the cs   
 */
uint8_t __attribute__((optimize ("O0"))) checkCs(uint8_t *data,uint8_t lens)
{
    uint8_t crc=0,i=0;
    for(i;i<lens;i++)
    {
        crc+=*(data+i);
    }
    return crc;
}
uint8_t checkAddress(uint8_t *addr,uint8_t *moduleAddress)
{
     /* ?????????????????????
     * b0: module address;
     * b1: broadcast address;
     * b2: broadcast address for electricity meter number
     * */
    uint8_t i,flag = 0x7;
    for(i =0; i < 6; i++)
    {
        if(*(addr+i) != moduleAddress[i])
            flag &= ~0x01;
    }
    for(i = 0; i < 6; i++)
    {
        if(*(addr+i) != 0x99)
            flag &= ~0x02;
    }
    for(i =0; i < 6; i++)
    {
        if(*(addr+i) !=0xAA)
            flag &= ~0x04;
    }
    return flag;
    
}


/**
 * 
 * @param buffer
 * @param lens
 * @param uartCtl
 * @describe   check the buffer is valid or invalid
 */
void checkBufferValidOrInvalid(uint8_t *buffer,uint16_t lens,struct uartControlU uartCtl)
{
    volatile uint16_t pos,notZeroCnt=0;
    for(pos=0;pos<lens;pos++)
    {
        if(*(buffer+pos)!=0)
            notZeroCnt++;
    }
    if(notZeroCnt!=0)
        uartCtl.recvValid=1;
    else
        uartCtl.shakehandSuccess=1;
}


uint8_t bufferToFrame(uint8_t *RecvBuffer,uint8_t lens,struct plcFrameStruct *plcRecvFrame,struct uartControlU uartCtl)
{
    uint8_t pos;
    volatile uint8_t dataCnt=0;
    uint8_t crc=0;
        for(pos=0;pos<30;pos++){
            if(*(RecvBuffer+pos)==0x68&&*(RecvBuffer+pos+7)==0x68)
            {
                plcRecvFrame->frameStart1=0x68;
                plcRecvFrame->addr[0]=*(RecvBuffer+pos+1);
                plcRecvFrame->addr[1]=*(RecvBuffer+pos+2);
                plcRecvFrame->addr[2]=*(RecvBuffer+pos+3);
                plcRecvFrame->addr[3]=*(RecvBuffer+pos+4);
                plcRecvFrame->addr[4]=*(RecvBuffer+pos+5);
                plcRecvFrame->addr[5]=*(RecvBuffer+pos+6);
                plcRecvFrame->frameStart2=0x68;
                plcRecvFrame->ctrCode=*(RecvBuffer+pos+8);
                plcRecvFrame->length=*(RecvBuffer+pos+9); 
                for(dataCnt=0;dataCnt<plcRecvFrame->length;dataCnt++)
                {
                    *(plcRecvFrame->data+dataCnt)=*(RecvBuffer+pos+9+dataCnt+1)-0x33;
                }
                plcRecvFrame->cs=*(RecvBuffer+pos+9+plcRecvFrame->length+1);
                crc=checkCs(RecvBuffer+pos,10+plcRecvFrame->length);            //check the cs of the frame;
                if(crc!=plcRecvFrame->cs)
                    continue;
                if(*(RecvBuffer+pos+9+plcRecvFrame->length+1+1)==0x16)
                {
                    plcRecvFrame->frameEnd=0x16;
                    return 1;
                }
            }
        }
    return 0;
}
uint8_t packedFrame(struct plcFrameStruct *plcSendFrame,uint8_t *moduleAddress, uint8_t ctrCode,uint8_t lens,uint8_t *data)
{
    volatile uint8_t i=0,crc=0;
    
    plcSendFrame->frameWakeUp[0]=0xFE;
    plcSendFrame->frameWakeUp[1]=0xFE;
    plcSendFrame->frameWakeUp[2]=0xFE;
    plcSendFrame->frameWakeUp[3]=0xFE;
    
    plcSendFrame->frameStart1 = 0x68;
    crc += plcSendFrame->frameStart1;
    for(i = 0; i < 6; i ++)
    {
        *(plcSendFrame->addr+i)=*(moduleAddress+i);
        crc += *(moduleAddress+i);
    }
    plcSendFrame->frameStart2 = 0x68;
    crc += plcSendFrame->frameStart2;
    plcSendFrame->ctrCode = ctrCode;
    crc += plcSendFrame->ctrCode;
    plcSendFrame->length = lens;
    crc += plcSendFrame->length;
    for(i=0;i<plcSendFrame->length;i++)
    {
        *(plcSendFrame->data+i)=*(data+i)+0x33;
        crc += *(data+i)+0x33;
    }
    plcSendFrame ->cs = crc;
    plcSendFrame ->frameEnd = 0x16;

    return (plcSendFrame->length + 12+4);             //return the length used for control the uart.
    
}
void frameToBuffer(struct plcFrameStruct *plcSendFrame,uint8_t *sendBuffer)
{
    volatile uint8_t cnt,sendBufferLens;
    sendBufferLens=plcSendFrame->length+12+4;  //include 4 byte wake up frame.
    *sendBuffer=plcSendFrame->frameWakeUp[0];
    *(sendBuffer + 1)=plcSendFrame->frameWakeUp[1];
    *(sendBuffer + 2)=plcSendFrame->frameWakeUp[2];
    *(sendBuffer + 3)=plcSendFrame->frameWakeUp[3];
    
    *(sendBuffer + 4)=plcSendFrame->frameStart1;
    for(cnt = 0;cnt < 6;cnt++)
    {
        *(sendBuffer + cnt + 1 + 4) = *(plcSendFrame->addr + cnt);
    }
    *(sendBuffer + 7 + 4) = plcSendFrame->frameStart2;
    *(sendBuffer + 8 + 4) = plcSendFrame->ctrCode;
    *(sendBuffer + 9 + 4) = plcSendFrame->length;
    for(cnt = 0;cnt < plcSendFrame->length;cnt++)
    {
        *(sendBuffer + cnt + 10 + 4) = *(plcSendFrame->data + cnt);
    }
    *(sendBuffer + cnt + 10 + 4) = plcSendFrame->cs;
    *(sendBuffer + cnt + 11 + 4) = plcSendFrame->frameEnd;
    
}
uint8_t picApplyPLCType(struct plcFrameStruct *plcRecvFrame)
{
    volatile uint8_t applyType;
    if(*(plcRecvFrame->data) == 0x00&&*(plcRecvFrame->data+1) == 0x01
            &&*(plcRecvFrame->data+2)==0x01&&*(plcRecvFrame->data+3)==0x2)
    {
        applyType = ApplyForVoltage;
    }
    else if (*(plcRecvFrame->data) == 0x00&&*(plcRecvFrame->data+1) == 0x01
            &&*(plcRecvFrame->data+2)==0x02&&*(plcRecvFrame->data+3)==0x2)
    {
         applyType = ApplyForCurrent;
    }
    else if (*(plcRecvFrame->data) == 0x00&&*(plcRecvFrame->data+1) == 0x00
            &&*(plcRecvFrame->data+2)==0x01&&*(plcRecvFrame->data+3)==0x00)
    {
         applyType = ApplyForEnergy;
    }
    /*
     *TODO add the apply fault code here;
     */
    
    return applyType;
}
/*
 *
 */
void frameParse(struct plcFrameStruct *plcRecvFrame,uint8_t* moduleAddress,uint8_t index)
{
    volatile uint8_t addrFlag=0x0;
    volatile uint8_t applyType=0;   //index the pic apply plc type /*voltage,current,energy,fault*/
    struct plcFrameStruct plcSendFrame; //for shake hand translate frame;
   // uint8_t moduleAddressTest[9]={0x32,0xC0,0x01,0x00,0x00,0x00,0x00,0x00,0x03};
    uint8_t moduleAddressMaster[6]={0x01,0x00,0x00,0x00,0x00,0x00};
    addrFlag=checkAddress(plcRecvFrame->addr,moduleAddress);
    if(addrFlag == NormalType)
    {
        applyType = picApplyPLCType(plcRecvFrame);  //return apply type;
        switch (applyType)
        {
                case ApplyForVoltage :
                    invertDateCtrl.inverterDate[index].voltage[0]=plcRecvFrame->data[4];
                    invertDateCtrl.inverterDate[index].voltage[1]=plcRecvFrame->data[5];
                    break;
                case ApplyForCurrent:
                    invertDateCtrl.inverterDate[index].current[0] = plcRecvFrame->data[4];
                    invertDateCtrl.inverterDate[index].current[1] = plcRecvFrame->data[5];
                    invertDateCtrl.inverterDate[index].current[2] = plcRecvFrame->data[6];
                    break;
                case ApplyForEnergy:
                    invertDateCtrl.inverterDate[index].energy[0] = plcRecvFrame->data[4];
                    invertDateCtrl.inverterDate[index].energy[1] = plcRecvFrame->data[5];
                    invertDateCtrl.inverterDate[index].energy[2] = plcRecvFrame->data[6];
                    invertDateCtrl.inverterDate[index].energy[3] = plcRecvFrame->data[7];
                     break;
                case ApplyForFault:
                    invertDateCtrl.inverterDate[index].fault     = plcRecvFrame->data[4];
                    break;
                default:
                    break;
        }
        
    }
    else if(addrFlag == BroadcastType)
    {
        
    }
    else if(addrFlag == WildcardType)
    {
        uartBufferPLCTran.lens = packedFrame(&plcSendFrame,moduleAddressMaster,0x93,6,moduleAddressMaster);
        frameToBuffer(&plcSendFrame,TranBuffer);
        delayN10ms(1);
        uartCtl.sendCnt=0;
        TranCnt=0;
        IEC1bits.U2TXIE=1;
        IFS1bits.U2TXIF=1;
    }
    else
    {
        
    }
}

void picApplyFrame(uint8_t picApplyState,uint8_t *destinationAddressTest)
{
    struct plcFrameStruct plcSendFrame;
    uint8_t commandApplyForVoltage[4]={0x00,0x01,0x01,0x02};
    uint8_t commandApplyForCurrent[4]={0x00,0x01,0x02,0x02};
    uint8_t commandApplyForEnergy[4] = {0x00,0x00,0x01,0x00};
    uint8_t commandApplyForFault[4]  = {0x00,0x00,0x01,0x01}; //define the fault apply command;
  //  uint8_t destinationAddressTest[6]={0x02,0x00,0x00,0x00,0x00,0x00};
    switch (picApplyState)
    {
        case ApplyForVoltage :
        {
            uartBufferPLCTran.lens=packedFrame(&plcSendFrame,destinationAddressTest,0x11,0x4,commandApplyForVoltage);
            frameToBuffer(&plcSendFrame,TranBuffer);
            delayN10ms(5);
            uartCtl.sendCnt=0;
            IEC1bits.U2TXIE=1;
            IFS1bits.U2TXIF=1;
            break;
        }
        case ApplyForCurrent :
        {
            uartBufferPLCTran.lens=packedFrame(&plcSendFrame,destinationAddressTest,0x11,0x4,commandApplyForCurrent);
            frameToBuffer(&plcSendFrame,TranBuffer);
            delayN10ms(5);
            uartCtl.sendCnt=0;
            IEC1bits.U2TXIE=1;
            IFS1bits.U2TXIF=1;
            break;
        }
        case ApplyForEnergy :
        {
            uartBufferPLCTran.lens=packedFrame(&plcSendFrame,destinationAddressTest,0x11,0x4,commandApplyForEnergy);
            frameToBuffer(&plcSendFrame,TranBuffer);
            delayN10ms(5);
            uartCtl.sendCnt=0;
            IEC1bits.U2TXIE=1;
            IFS1bits.U2TXIF=1;
            break;
        }
        case ApplyForFault :
        {
            uartBufferPLCTran.lens=packedFrame(&plcSendFrame,destinationAddressTest,0x11,0x4,commandApplyForFault);
            frameToBuffer(&plcSendFrame,TranBuffer);
            delayN10ms(5);
            uartCtl.sendCnt=0;
            IEC1bits.U2TXIE=1;
            IFS1bits.U2TXIF=1;
            break;
        }
        default :
        {
            break;
        }
    
    }
}
uint8_t __attribute__((optimize ("O0")))  plcService(struct nandFlashInfoStr *nandFlashInfo)
{ 
 switch (plcCtl.picApplyPlcState)
    {
    case ShakeHandIng:
           // plcCtl.picApplyPlcState=ShakeHandSuccess;
        
             if(!uartBufferPLCRecv.complete)
            {  
                startUart2Recv();
             }
            if( uartBufferPLCRecv.complete == 1){
               if(bufferToFrame(uartBufferPLCRecv.date,uartBufferPLCRecv.lens,&plcRecvFrame,uartCtl))
               {
                   if(checkAddress(plcRecvFrame.addr,moduleAddress)==WildcardType)
                   {
                       frameParse(&plcRecvFrame,moduleAddress,0);
                       uartBufferPLCRecv.complete = 0;
                       uartCtl.nextLoop = 0;
                       uartCtl.shakehandSuccess = 1;
                       plcCtl.picApplyPlcState=ShakeHandSuccess;
                       plcCtl.plcApplyPlcSuccess=0;
                       invertDateCtrl.index=0;          //define the solar panel index..
                       delayN10ms(10);
                       plcCtl.picApplyIndex=0;              //control the apply count;
                   }
               } 
            else
            {
                uartCtl.nextLoop = 1;
               
            }
            }
           break;
    case ShakeHandSuccess:
            plcCtl.picApplyPlcState=ApplyForVoltage;
            break;
    case ApplyForVoltage:
            if(!plcCtl.plcApplyPlcSuccess)
            {
                 delayN10ms(1);
                 bufferClear(RecvBuffer,recvBufferLens);
                 picApplyFrame(ApplyForVoltage,destinationAddr[invertDateCtrl.index]);
                 plcCtl.plcApplyPlcSuccess=1;
                 picApplyLastTime=msCount;     
            }
            if(!uartBufferPLCRecv.complete)
            {  
                startUart2Recv();  
            }
            
            if( uartBufferPLCRecv.complete==1){
               if(bufferToFrame(RecvBuffer,recvBufferLens,&plcRecvFrame,uartCtl))
               {
                //   moduleAddress[0] = 0x02;      //lead to the apply address;
                   frameParse(&plcRecvFrame,destinationAddr[invertDateCtrl.index],invertDateCtrl.index);
                   recvComplete=0;
                   uartCtl.nextLoop=0;
                   plcCtl.picApplyPlcState=ApplyForCurrent;
                   plcCtl.plcApplyPlcSuccess=0;
                   delayN10ms(10);
               } 
               
                else
                {
                    uartCtl.nextLoop=1;
                }
            }
            if(msCount-picApplyLastTime>150)
            {
                uartCtl.nextLoop=1;
                plcCtl.picApplyIndex++;
                plcCtl.plcApplyPlcSuccess=0;
                if(plcCtl.picApplyIndex>0)
                {
                    plcCtl.picApplyPlcState=ApplyForCurrent; //if send apply number >2,then give up this apply,start next apply;
                    plcCtl.picApplyIndex=0;
                }
            }
           
            break;
    case ApplyForCurrent:
           if(!plcCtl.plcApplyPlcSuccess)
            {
                 delayN10ms(1);
                 bufferClear(RecvBuffer,recvBufferLens);
                 picApplyFrame(ApplyForCurrent,destinationAddr[invertDateCtrl.index]);
                 plcCtl.plcApplyPlcSuccess=1;
                 picApplyLastTime=msCount;     
            }
            if(!uartBufferPLCRecv.complete)
            {  
                startUart2Recv();    
            }
            
            if( uartBufferPLCRecv.complete==1){
               if(bufferToFrame(RecvBuffer,recvBufferLens,&plcRecvFrame,uartCtl))
               {
                //   moduleAddress[0] = 0x02;      //lead to the apply address;
                   frameParse(&plcRecvFrame,destinationAddr[invertDateCtrl.index],invertDateCtrl.index);
                   recvComplete=0;
                   uartCtl.nextLoop=0;
                   plcCtl.picApplyPlcState=ApplyForEnergy;
                   plcCtl.plcApplyPlcSuccess=0;
                   delayN10ms(10);
               } 
               
                else
                {
                    uartCtl.nextLoop=1;
                }
            }
           
           if(msCount-picApplyLastTime>150)
            {
                uartCtl.nextLoop=1;
                plcCtl.picApplyIndex++;
                plcCtl.plcApplyPlcSuccess=0;
                if(plcCtl.picApplyIndex>0)
                {
                    plcCtl.picApplyPlcState=ApplyForEnergy; //if send apply number >2,then give up this apply,start next apply;
                    plcCtl.picApplyIndex=0;
                }
            }
            break;
    case ApplyForEnergy:
           if(!plcCtl.plcApplyPlcSuccess)
            {
                 delayN10ms(1);
                 bufferClear(RecvBuffer,recvBufferLens);
                 picApplyFrame(ApplyForEnergy,destinationAddr[invertDateCtrl.index]);
                 plcCtl.plcApplyPlcSuccess=1;
                 picApplyLastTime=msCount;     
            }
            if(!uartBufferPLCRecv.complete)
            {  
                startUart2Recv();
             }
            
            if( uartBufferPLCRecv.complete==1){
               if(bufferToFrame(RecvBuffer,recvBufferLens,&plcRecvFrame,uartCtl))
               {
                //   moduleAddress[0] = 0x02;      //lead to the apply address;
                   frameParse(&plcRecvFrame,destinationAddr[invertDateCtrl.index],invertDateCtrl.index);
                   recvComplete=0;
                   uartCtl.nextLoop=0;
                   plcCtl.picApplyPlcState=ApplyForFault;
                   plcCtl.plcApplyPlcSuccess=0;
                   delayN10ms(10);
               } 
               
                else
                {
                    uartCtl.nextLoop=1;
                }
            }
           if(msCount-picApplyLastTime>150)
            {
                uartCtl.nextLoop=1;
                plcCtl.picApplyIndex++;
                plcCtl.plcApplyPlcSuccess=0;
                if(plcCtl.picApplyIndex>0)
                {
                    plcCtl.picApplyPlcState=ApplyForFault; //if send apply number >2,then give up this apply,start next apply;
                    plcCtl.picApplyIndex=0;
                }
            }
            break;
    case ApplyForFault:
      if(!plcCtl.plcApplyPlcSuccess)
            {
                 delayN10ms(1);
                 bufferClear(RecvBuffer,recvBufferLens);
                 picApplyFrame(ApplyForFault,destinationAddr[invertDateCtrl.index]);
                 plcCtl.plcApplyPlcSuccess=1;
                 picApplyLastTime=msCount;     
            }
            if(!uartBufferPLCRecv.complete)
            {  
                startUart2Recv();
            }
            
            if( uartBufferPLCRecv.complete==1){
               if(bufferToFrame(RecvBuffer,recvBufferLens,&plcRecvFrame,uartCtl))
               {
                //   moduleAddress[0] = 0x02;      //lead to the apply address;
                   frameParse(&plcRecvFrame,destinationAddr[invertDateCtrl.index],invertDateCtrl.index);
                   recvComplete=0;
                   uartCtl.nextLoop=0;
                   plcCtl.picApplyPlcState=ApplyForVoltage;
                   plcCtl.plcApplyPlcSuccess=0;
                   delayN10ms(10);
                   
                   //index++ means this solar panel apply is done,the next solar panel is will begin.
                   invertDateCtrl.index++;
                   if(invertDateCtrl.index>nandFlashInfo->solarPanelCnt-1)
                    {
                        plcCtl.picApplyCompleteLoop=1;
                        plcCtl.plcApplyPlcSuccess=1;
                    }
                 
               } 
               
                else
                {
                    uartCtl.nextLoop=1;
                }
            }    
           if(msCount-picApplyLastTime>150)
            {
                uartCtl.nextLoop=1;
                plcCtl.picApplyIndex++;
                plcCtl.plcApplyPlcSuccess=0;
                if(plcCtl.picApplyIndex>0)
                {
                    plcCtl.picApplyPlcState=ApplyForVoltage; //if send apply number >2,then give up this apply,start next apply;
                    plcCtl.picApplyIndex=0;
                    invertDateCtrl.index++;
                    if(invertDateCtrl.index > nandFlashInfo->solarPanelCnt-1)
                    {
                        plcCtl.picApplyCompleteLoop=1;
                        plcCtl.plcApplyPlcSuccess=1;
                    }
                }
            }
            break;
    default:
         break;
            
     
    } 
     if(uartCtl.nextLoop == 1)
        {
            IEC1bits.U2RXIE=1;
            uartCtl.nextLoop=0;
            uartBufferPLCRecv.complete=0;
            uartBufferPLCRecv.cnt=0;
        }
}

/* *****************************************************************************
 End of File
 */
