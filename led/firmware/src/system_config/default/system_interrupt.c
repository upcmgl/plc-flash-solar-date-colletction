/*******************************************************************************
 System Interrupts File

  File Name:
    system_interrupt.c

  Summary:
    Raw ISR definitions.

  Description:
    This file contains a definitions of the raw ISRs required to support the
    interrupt sub-system.

  Summary:
    This file contains source code for the interrupt vector functions in the
    system.

  Description:
    This file contains source code for the interrupt vector functions in the
    system.  It implements the system and part specific vector "stub" functions
    from which the individual "Tasks" functions are called for any modules
    executing interrupt-driven in the MPLAB Harmony system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    interrupt-driven in the system.  These handles are passed into the individual
    module "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2011-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <xc.h>
#include <sys/attribs.h>
#include "app.h"
#include "system_definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector Functions
// *****************************************************************************
// *****************************************************************************
void __ISR(_EXTERNAL_0_VECTOR, IPL1AUTO) _IntHandlerExternalInterruptInstance0(void)
{
    uint8_t RtcState =0;
     uint8_t rtc[16];
    volatile uint8_t flag=0;
    IFS0bits.INT0IF = 0;
    drv_R8025T_read(0xE,&RtcState,1);
    flag = RtcState&0x18 ;
    if (flag== 0x10)                 //TF  fixed interrupt;
    {
        RtcState = 0;
        drv_R8025T_write(0xE,&RtcState,1);
        drv_R8025T_read(0,rtc,16);
        sysTime.timeDate.min   = rtc[1];
        sysTime.timeDate.hour  = rtc[2];
        sysTime.timeDate.day   = rtc[4];
        sysTime.timeDate.month = rtc[5];
        sysTime.timeDate.year  = rtc[6];
        if(sysTime.timeDate.hour == 0x21)
        {
            sysTime.sysTimeRunState = (uint8_t)DayEnd;     //the day's record is end;
        }
        
    }
    else if(flag == 0x08)                   //alarm interrupt;
    {
        RtcState = 0;
        drv_R8025T_write(0xE,&RtcState,1);
        minInterrupt(1);
        sysTime.sysTimeRunState =(uint8_t) DayStart;      //the 
    }
    else if (flag== 0x01)                 //TF  fixed interrupt;
    {
        RtcState = 0;
        drv_R8025T_write(0xE,&RtcState,1);
    }
    
 //   PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_EXTERNAL_0);
}

    
void __ISR(_TIMER_2_VECTOR, ipl1AUTO) IntHandlerDrvTmrInstance0(void)
{
    IFS0bits.T2IF=0;
    PORTBbits.RB5=~PORTBbits.RB5;
    if(msCount<0xffffffff) //the msCount is the index of the system ms clock;when it come 0xffffffff,it will be 0x0 again
    {
        msCount++;
    }
    else
        msCount=0;
}
 
 

 

 

 

 

 
 

void __ISR(_I2C_3_VECTOR, ipl1AUTO) _IntHandlerDrvI2CInstance0(void)
{
	DRV_I2C0_Tasks();
 
}

 

 

 

 

 

 
  
  
/*******************************************************************************
 End of File
*/

