//********************************************************************* */
//define some global date in here;
#ifndef _INVERTERDATE_H    /* Guard against multiple inclusion */
#define _INVERTERDATE_H
//#include "file.h"

#ifdef __cplusplus
extern "C" {
#endif
struct microInverterDateU
{
    uint8_t voltage[2];
    uint8_t current[3];
    uint8_t energy[4];
    uint8_t fault;
    //---7---------------6---------------5----------4--3--2--1--0--//
//-----overvoltage---undervoltage----overcurrent----//
};

struct microInverterDatePageStr
{
    uint16_t structLens;
    uint8_t day;
    uint8_t hour;
    struct microInverterDateU  inverterDate[20];
};

struct microInverterDatePageStr microInverterDatePage;
//every solar panel date need 10 bytes.10*12*17 =2040 bytes.

#define microInverterNum  100   //the number of micro inverter is initial to 20.

struct microInverterDateCtrl
{
    uint8_t index;
    struct microInverterDateU  inverterDate[microInverterNum];
    
};

extern struct microInverterDateCtrl invertDateCtrl; //initial 20 inverter date structure;
void show(struct microInverterDateU date,uint8_t index);

#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
