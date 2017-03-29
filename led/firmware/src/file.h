
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
#include "baseFun.h"
//#include "osTimer.h"
/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#define    NewDayStart    4
#define    NewDayEnd      21
#define    PageOneDay  (NewDayEnd-NewDayStart)*microInverterNum/20     //alomost 85 pages one day;



struct timeDateStr
{
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
};
struct sysTimeStr
{
   struct  timeDateStr timeDate; 
   uint8_t sysTimeRunState;
   uint8_t fixedMinInterrupt;  //if fixed time interrupt coming,(for example 6 mins), 
                               //fixedMinInterrupt =1;
};

extern  struct sysTimeStr sysTime;

/*---------------this area is maintain for flash test------------------------*/

struct microInverterDateCtrl invertDateCtrl;    
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
//***************two kinds of date ,flashRWdate is for storing  inverter's date.*******//
//***************and the flashRWTempdate is for exchanging the date,mainly for system date (the 0,1 blocks),******************//
extern struct flashDateStr flashRWdate, flashRWTempdate; 

struct monthWithIndexStr  //set month mapping to index   month ------>index.
{
    uint8_t month;
    uint16_t index;
};

/**
 * in dayWithIndexStr structure ,some day is mapping to 3 blocks extremely,sometimes one day often mapping to 2 blocks.every block will be arranged start page and the end page when a new day coming.
 */
struct dayWithIndexStr  
{
    uint8_t   day;
    uint16_t  index;     //set day mapping to index   day ------>index. 
    uint8_t   BlockNum;  //record the block's number a new day' need;
    
    uint16_t  whichBlockLeft;
    uint16_t  leftBlockStartPage;
    uint16_t  leftBlockEndPage;
    
    uint16_t  whichBlockMiddle;
    uint16_t  middleBlockStartPage;
    uint16_t  middleBlockEndPage;
    
    uint16_t  whichBlockRight;
    uint16_t  rightBlockStartPage;
    uint16_t  rightBlockEndPage;
};

struct MonthInfoStr
{
    uint16_t structLens;
    uint8_t month;
    struct dayWithIndexStr  dayWithIndex[31]; 
};

extern struct MonthInfoStr   monthInfo,monthInfoTemp;

struct yearMonthCtlStr
{
    uint8_t  year;
    struct monthWithIndexStr monthWithIndex[12];
    
};

//define the date struct of the date store system;&it could be very gaint.
struct nandFlashInfoStr
{
     uint16_t structLens; 
     uint16_t BlockNum;            //flash has 1K blocks;
     uint16_t badBlockNum;
     uint8_t  badBlockChecked;     //if this var =1,we have checked the bad block;
     uint8_t  badBlockPos[128];    //define the badBlock position ,1K blocks = 128bytes = 1K bits.
     uint8_t  dayMappingPage[85];  //every a new day coming, the system will establish 85 new page. 17*5 = 85 pages.
     
     uint16_t blockUsedPresent;   //the inverter date is store form 3rd block;
     uint16_t blockUsedNext;
     uint8_t  pageUsedPresent;
     uint8_t  pageUsedNext;
     uint8_t  allBlockUsed;        //all block has been used ,the new loop begin;
     
     uint8_t  passName[3];
     uint8_t  passWord[3];
     
     uint8_t  yearCnt;
     uint8_t  monthCnt;
     uint8_t  dayCnt;             //index day in this month;
     
     uint8_t  monthMappingPage;    //3~63;  month mapping to page;
     uint8_t  solarPanelCnt;       //define the number of solar panel,we predefined the number is 100.
     uint8_t  sysRestarted;        //if system restarted ,this var is set 1;
     struct yearMonthCtlStr  yearMonthCtl[4];  //temporary  defined 4 year. 
};

extern struct nandFlashInfoStr nandFlashInfo;


void initFlashBuffer();
void initFlashInfo(struct nandFlashInfoStr *nandFlashInfo);
//badBlockCheck function ,it will record the bad block info into the nandFlashInfo struct.
void badBlockCheck (struct nandFlashInfoStr  *nandFlashInfo ,struct flashDateStr *flashRWdate );
uint8_t fileWriteAllPageEcc(uint32_t page,struct flashDateStr *flashRWdate);
uint8_t fileReadAllPageEcc(uint32_t page,struct flashDateStr *flashRWdate,struct nandFlashInfoStr *nandFlashInfo);

uint8_t storeMainSysMessage(uint32_t page,struct nandFlashInfoStr *nandFlashInfo);
uint8_t loadMainSysMessage(uint32_t page,struct nandFlashInfoStr *nandFlashInfo);

uint8_t storeMicroInverterDatePage(struct nandFlashInfoStr *nandFlashInfo ,struct microInverterDateCtrl *invertDateCtrl,struct sysTimeStr *sysTime,struct microInverterDatePageStr *microInverterDatePage,struct MonthInfoStr  *monthInfo);
uint8_t loadMicroInverterDatePage(struct nandFlashInfoStr *nandFlashInfo ,struct microInverterDateCtrl *invertDateCtrl);

uint8_t storeMonthInfo(uint32_t page,struct MonthInfoStr *monthInfo);
uint8_t loadMonthInfo(uint32_t page,struct MonthInfoStr *monthInfo);
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
