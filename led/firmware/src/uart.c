#include "uart.h"
#include <xc.h>
#include <sys/attribs.h>
#include "system_definitions.h"

uint8_t  shakeHandSuccess=0;   //for plc asks for MCU number
uint8_t shakeHandBuffer[14],shakeHandBufferCnt=0;  //for storing the plc asks for message;

uint16_t RecvCnt=0;
uint16_t TranCnt=0;
volatile uint8_t recvComplete=0;
uint8_t tranComplete=0;
uint8_t uartReceiveState=uartReceiveStateBegin;
void uart2Init(){
    U2MODEbits.ON=0;
    U2MODEbits.UEN=0x0;   //uart enable bits;
    U2MODEbits.PDSEL=1;   //8bits data even parity
    U2MODEbits.STSEL=0;   //1 stop bit
    U2MODEbits.BRGH=1;
    U2STAbits.UTXISEL=0x2;
    U2STAbits.URXISEL=0x0;
    
    U2BRG=0x822;    //uart boud rate 9600;
    IFS1bits.U2RXIF=0;
    IEC1bits.U2RXIE=1;
    IEC1bits.U2TXIE=0;
    IFS1bits.U2TXIF=0;
    U2STAbits.UTXEN=1;
    U2STAbits.URXEN=1;
    U2MODEbits.ON=1;
    
    
}
/**************************************************************
 *describe :start uart 2 receive;
 *note: if 2 bytes interval is than 10ms,then this message is finished;
 ***************************************************************/
void startUart2Recv()
{
    IEC1bits.U2RXIE=1;
    if(msCount-uartBufferPLCRecv.lastTime>3&&uartReceiveState==uartReceiveStateGoOn)
    {
        IEC1bits.U2RXIE=0;
        uartBufferPLCRecv.complete = 1;
        uartBufferPLCRecv.cnt      = 0;
    //    uartBufferPLCRecv.lens     = 0;
        uartReceiveState=uartReceiveStateBegin;
    }
}


void __ISR(_UART_2_VECTOR, ipl1AUTO) _IntHandlerDrvUsartInstance0(void)
{
    //handle the TX ISR
     if(IFS1bits.U2TXIF==1&&IEC1bits.U2TXIE==1)
     {
        IFS1bits.U2TXIF=0; 
       // U2TXREG=TranBuffer[TranCnt++];
        U2TXREG=uartBufferPLCTran.date[uartBufferPLCTran.cnt++];
     }
     if(uartBufferPLCTran.cnt >= uartBufferPLCTran.lens&&IEC1bits.U2TXIE==1){
         uartBufferPLCTran.complete=1;
         uartBufferPLCTran.cnt = 0;
         uartCtl.sendCnt=uartCtl.sendCnt+1;
         if(uartCtl.sendCnt>0)
         {
            IEC1bits.U2TXIE=0;
            uartCtl.nextLoop=1;
         }
     }
     //handle the RX ISR  
     if(IFS1bits.U2RXIF == 1&&IEC1bits.U2RXIE == 1)
     {
         IFS1bits.U2RXIF=0;
         uartBufferPLCRecv.lastTime=msCount;
     //    RecvBuffer[RecvCnt]=U2RXREG;
         *(uartBufferPLCRecv.date+uartBufferPLCRecv.cnt)=U2RXREG;
         uartBufferPLCRecv.lens=uartBufferPLCRecv.cnt;
         uartBufferPLCRecv.cnt =uartBufferPLCRecv.cnt + 1;
         uartReceiveState=uartReceiveStateGoOn;
//         if(RecvBuffer[RecvCnt]==0x68&&uartReceiveState==uartReceiveStateBegin)
//         { 
//             uartReceiveState=uartReceiveStateGoOn;
//             RecvCnt=RecvCnt+1;
//         }
//         if(RecvCnt<50&&uartReceiveState==uartReceiveStateGoOn)
//         {
//             RecvBuffer[RecvCnt++]=U2RXREG;
//             if(RecvBuffer[RecvCnt]==0x16)     //for some short message ,end the date flow in time.
//             {
//                recvComplete=1;
//                IEC1bits.U2RXIE=0;
//                RecvCnt=0;
//                uartReceiveState=uartReceiveStateBegin;
//             }
//         }
//         else if(RecvCnt>=50&&uartReceiveState==uartReceiveStateGoOn)
//         {
         if(uartBufferPLCRecv.cnt >= 50&&IEC1bits.U2RXIE == 1)
         {
           uartBufferPLCRecv.complete=1;
           uartBufferPLCTran.complete=0;
            IEC1bits.U2RXIE=0;
            uartBufferPLCRecv.cnt = 0;
    
            uartReceiveState=uartReceiveStateBegin;
         }   
     }
    // }
     if(U2STAbits.URXDA!=1){
         IFS1bits.U2RXIF=0;
     //    IFS1bits.U2TXIF=1;
     }
}
 