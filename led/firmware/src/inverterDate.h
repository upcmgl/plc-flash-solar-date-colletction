//********************************************************************* */
//define some global date in here;
#ifndef _INVERTERDATE_H    /* Guard against multiple inclusion */
#define _INVERTERDATE_H
struct microInverterDateU
{
    uint8_t voltage[2];
    uint8_t current[3];
    uint8_t energy[4];
    uint8_t fault;
    //---7---------------6---------------5----------4--3--2--1--0--//
//-----overvoltage---undervoltage----overcurrent----//
};
#define  microInverterNum  20  //the number of micro inverter is initial to 20.
struct microInverterDateCtrl
{
    struct microInverterDateU  inverterDate[microInverterNum];
    uint8_t index;
};

void show(struct microInverterDateU date,uint8_t index);


#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
