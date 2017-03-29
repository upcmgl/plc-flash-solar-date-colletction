/*******************************************************************************
********************************************************************************
**
**  Filename:       file.c
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

////*******************NOTE:**********************/////
//this nandflash's character is :64K pages of (2048+64) bytes each
//this flash consists of 1K blocks of 64pages each;
//                       OOB
//[0..23]-->ECC   [24..25]-->this page has used.
//
//
//which is the present page,and which is the next page;
//we defined :the present page is a page is writing or is going to be written.
//the next page is the next page of the present page.
//
//
//

//********************************************************************************
//*******************************************************************************
///*******************NOTE:**********************/////
#include "file.h"

uint8_t flashWriteBuffer[2048];
uint8_t flashWriteBufferOOB[64];

uint8_t flashReadBuffer[2048];
uint8_t flashReadBufferOOB[64];

//define the RWTempbuffer just for copy block.the read write function index to the same buffer.
uint8_t flashRWTempbuffer[2048];
uint8_t flashRWTempbufferOOB[64];

struct flashDateStr flashRWdate, flashRWTempdate;
struct nandFlashInfoStr nandFlashInfo;
struct MonthInfoStr  monthInfo,monthInfoTemp;
/**
 * @describtion  initial the flash bufffer.include read &write,lengths /offset
 */
void initFlashBuffer()
{
    flashRWdate.flashReadDate[0].buffer = flashReadBuffer;
    flashRWdate.flashReadDate[0].nbytes = 2048;
    flashRWdate.flashReadDate[0].offset = 0;
    
    flashRWdate.flashReadDate[1].buffer = flashReadBufferOOB;
    flashRWdate.flashReadDate[1].nbytes = 64;
    flashRWdate.flashReadDate[1].offset =2048;
    
    flashRWdate.flashWriteDate[0].buffer = flashWriteBuffer;
    flashRWdate.flashWriteDate[0].nbytes = 2048;
    flashRWdate.flashWriteDate[0].offset = 0;
    
    flashRWdate.flashWriteDate[1].buffer = flashWriteBufferOOB;
    flashRWdate.flashWriteDate[1].nbytes = 64;
    flashRWdate.flashWriteDate[1].offset =2048;  
    
    //initial the flashRWTempdate date. they are indexed to the same array.
    flashRWTempdate.flashReadDate[0].buffer = flashRWTempbuffer;
    flashRWTempdate.flashReadDate[0].nbytes = 2048;
    flashRWTempdate.flashReadDate[0].offset = 0;
    
    flashRWTempdate.flashReadDate[1].buffer = flashRWTempbufferOOB;
    flashRWTempdate.flashReadDate[1].nbytes = 64;
    flashRWTempdate.flashReadDate[1].offset = 2048;
    
    flashRWTempdate.flashWriteDate[0].buffer = flashRWTempbuffer;
    flashRWTempdate.flashWriteDate[0].nbytes = 2048;
    flashRWTempdate.flashWriteDate[0].offset = 0;
    
    flashRWTempdate.flashWriteDate[1].buffer = flashRWTempbufferOOB;
    flashRWTempdate.flashWriteDate[1].nbytes = 64;
    flashRWTempdate.flashWriteDate[1].offset = 2048;
    
    
}

/**
 * initial the  nand flash information .
 * @param nandFlashInfo
 */
void initFlashInfo(struct nandFlashInfoStr *nandFlashInfo)
{
    uint16_t pageNum;
    nandFlashInfo ->structLens =sizeof(struct nandFlashInfoStr);
    nandFlashInfo ->badBlockNum=0;
    nandFlashInfo ->BlockNum  =1024;
    nandFlashInfo ->solarPanelCnt =20;
    for(pageNum = 0;pageNum <128; pageNum ++)
    {
        nandFlashInfo ->badBlockPos[pageNum] = 0;
    }
    nandFlashInfo ->blockUsedPresent = 3;
    nandFlashInfo ->blockUsedNext    = 4;
    nandFlashInfo ->pageUsedPresent  = 3*64;
    nandFlashInfo ->pageUsedNext     = 3*64+1;
    nandFlashInfo ->allBlockUsed     = 0;
    nandFlashInfo ->monthMappingPage =1;
}
/**
 * //initial  the monthinfo structure;
 * @param monthInfo
 */
void initMonthInfo(struct MonthInfoStr   *monthInfo)
{
    monthInfo ->structLens =sizeof(struct MonthInfoStr);
    monthInfo ->month      =0;
    memset(monthInfo,0,monthInfo ->structLens);     //all var in monthinfo is set as 0;
}
/**
 * 
 * @param flashReadDate
 * @param times   ----->>how many times of this loop .general is set 2.
 * @return return 0 --->this page  is bad,otherwise.
 */
uint8_t allBytes0xFFCheck(struct nanddrv_transfer *flashReadDate,uint8_t times)
{
    volatile uint32_t lens;
    volatile uint8_t loopTimes;
    for(loopTimes = 0;loopTimes <times;loopTimes++)
    {
        for(lens = 0;lens <(flashReadDate+loopTimes)->nbytes;lens++)
        {
            if(*((flashReadDate+loopTimes)->buffer+lens) != 0xFF)
            {
                return 0;
            }
        }
    }
    return 1;
}

/**
 * 
 * @param blockNum
 * @param nandFlashInfo
 * @param flashRWdate
 * @return  0------>this block is bad ,otherwise.
 */
uint8_t perBadBlockCheck(uint32_t blockNum ,struct nandFlashInfoStr  *nandFlashInfo,struct flashDateStr *flashRWdate )
{
    volatile uint32_t pageNum=0;
    volatile uint8_t  badBlockResult;
    nanddrv_erase(blockNum);
    for(pageNum = 0;pageNum < NAND_FLASH_BLOCK_SIZE_PAGE;pageNum ++)
    {
        nanddrv_read_tr(blockNum*64+pageNum,flashRWdate->flashReadDate,2);
        badBlockResult=allBytes0xFFCheck(flashRWdate->flashReadDate,2);
        if (!badBlockResult)
        {
            nandFlashInfo ->badBlockNum++;
            *(nandFlashInfo ->badBlockPos+blockNum/8) = *(nandFlashInfo ->badBlockPos+blockNum/8)|(1 <<(blockNum%8));
            return 0;
        }
    }
    nandFlashInfo ->badBlockChecked = 1;    //if this var =1,we have checked the bad block;
    nandFlashInfo ->pageUsedPresent = 64*blockNum;  
    nandFlashInfo ->pageUsedNext    = 64*blockNum+1;
    return 1;
    
}
/**
 * 
 * @param nandFlashInfo
 * @return 
 */
void badBlockCheck (struct nandFlashInfoStr  *nandFlashInfo ,struct flashDateStr *flashRWdate )
{
    volatile uint32_t blockNum;
    for(blockNum = 0;blockNum < NAND_FLASH_BLOCK_COUNT; blockNum ++)
    {
        perBadBlockCheck(blockNum ,nandFlashInfo,flashRWdate);
    }

}


//******************************following ecc code is from linux 2.6******************************************//
//but the ecc is only designed for 256 bytes.
//however our nandflash is 2K bytes,so we need to designed another function for ecc.
//******************************following ecc code is from linux 2.6******************************************//
/*
 * Pre-calculated 256-way 1 byte column parity
 */
static const uint8_t nand_ecc_precalc_table[] = {
	0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00,
	0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
	0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
	0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
	0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
	0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
	0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
	0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
	0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
	0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
	0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
	0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
	0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
	0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
	0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
	0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00
};

/**
 * nand_trans_result - [GENERIC] create non-inverted ECC
 * @reg2:	line parity reg 2
 * @reg3:	line parity reg 3
 * @ecc_code:	ecc
 *
 * Creates non-inverted ECC code from line parity
 */
static void nand_trans_result(uint8_t reg2, uint8_t reg3,uint8_t *ecc_code)
{
	uint8_t a, b, i, tmp1, tmp2;

	/* Initialize variables */
	a = b = 0x80;
	tmp1 = tmp2 = 0;

	/* Calculate first ECC byte */
	for (i = 0; i < 4; i++) {
		if (reg3 & a)		/* LP15,13,11,9 --> ecc_code[0] */
			tmp1 |= b;
		b >>= 1;
		if (reg2 & a)		/* LP14,12,10,8 --> ecc_code[0] */
			tmp1 |= b;
		b >>= 1;
		a >>= 1;
	}

	/* Calculate second ECC byte */
	b = 0x80;
	for (i = 0; i < 4; i++) {
		if (reg3 & a)		/* LP7,5,3,1 --> ecc_code[1] */
			tmp2 |= b;
		b >>= 1;
		if (reg2 & a)		/* LP6,4,2,0 --> ecc_code[1] */
			tmp2 |= b;
		b >>= 1;
		a >>= 1;
	}

	/* Store two of the ECC bytes */
	ecc_code[0] = tmp1;
	ecc_code[1] = tmp2;
}

/**
 * nand_calculate_ecc - [NAND Interface] Calculate 3 byte ECC code for 256 byte block
 * @mtd:	MTD block structure
 * @dat:	raw data
 * @ecc_code:	buffer for ECC
 */
uint32_t nand_calculate_ecc(const uint8_t *dat, uint8_t *ecc_code)
{
	uint8_t idx, reg1, reg2, reg3;
	int j;

	/* Initialize variables */
	reg1 = reg2 = reg3 = 0;
	ecc_code[0] = ecc_code[1] = ecc_code[2] = 0;

	/* Build up column parity */
	for(j = 0; j < 256; j++) {

		/* Get CP0 - CP5 from table */
		idx = nand_ecc_precalc_table[dat[j]];
		reg1 ^= (idx & 0x3f);

		/* All bit XOR = 1 ? */
		if (idx & 0x40) {
			reg3 ^= (uint8_t) j;
			reg2 ^= ~((uint8_t) j);
		}
	}

	/* Create non-inverted ECC code from line parity */
	nand_trans_result(reg2, reg3, ecc_code);

	/* Calculate final ECC code */
	ecc_code[0] = ~ecc_code[0];
	ecc_code[1] = ~ecc_code[1];
	ecc_code[2] = ((~reg1) << 2) | 0x03;
	return 0;
}

/**
 * nand_correct_data - [NAND Interface] Detect and correct bit error(s)
 * @mtd:	MTD block structure
 * @dat:	raw data read from the chip
 * @read_ecc:	ECC from the chip
 * @calc_ecc:	the ECC calculated from raw data
 *
 * Detect and correct a 1 bit error for 256 byte block
 */

uint32_t nand_correct_data(uint8_t *dat, uint8_t *read_ecc, uint8_t *calc_ecc)
{
	uint8_t a, b, c, d1, d2, d3, add, bit, i;

	/* Do error detection */
	d1 = calc_ecc[0] ^ read_ecc[0];
	d2 = calc_ecc[1] ^ read_ecc[1];
	d3 = calc_ecc[2] ^ read_ecc[2];

	if ((d1 | d2 | d3) == 0) {
		/* No errors */
		return 0;
	}
	else {
		a = (d1 ^ (d1 >> 1)) & 0x55;
		b = (d2 ^ (d2 >> 1)) & 0x55;
		c = (d3 ^ (d3 >> 1)) & 0x54;

		/* Found and will correct single bit error in the data */
		if ((a == 0x55) && (b == 0x55) && (c == 0x54)) {
			c = 0x80;
			add = 0;
			a = 0x80;
			for (i=0; i<4; i++) {
				if (d1 & c)
					add |= a;
				c >>= 2;
				a >>= 1;
			}
			c = 0x80;
			for (i=0; i<4; i++) {
				if (d2 & c)
					add |= a;
				c >>= 2;
				a >>= 1;
			}
			bit = 0;
			b = 0x04;
			c = 0x80;
			for (i=0; i<3; i++) {
				if (d3 & c)
					bit |= b;
				c >>= 2;
				b >>= 1;
			}
			b = 0x01;
			a = dat[add];
			a ^= (b << bit);
			dat[add] = a;
			return 1;
		}
		else {
			i = 0;
			while (d1) {
				if (d1 & 0x01)
					++i;
				d1 >>= 1;
			}
			while (d2) {
				if (d2 & 0x01)
					++i;
				d2 >>= 1;
			}
			while (d3) {
				if (d3 & 0x01)
					++i;
				d3 >>= 1;
			}
			if (i == 1) {
				/* ECC Code Error Correction */
				read_ecc[0] = calc_ecc[0];
				read_ecc[1] = calc_ecc[1];
				read_ecc[2] = calc_ecc[2];
				return 2;
			}
			else {
				/* Uncorrectable Error */
				return -1;
			}
		}
	}

	/* Should never happen */
	return -1;
}

/**
 * 
 * @param date -->2K bytes date,3*8 =24bytes ecc code.
 * @param eccCode -->the length of eccCode is 3*8 =24.
 * @return 
 */
uint8_t nand2KEccCalculate(const uint8_t *date,uint8_t *eccCode)
{
    volatile uint8_t pos = 0;
    for(pos = 0;pos < 8;pos ++)
    {
        nand_calculate_ecc(date+pos * 256, eccCode+pos * 3);
    }
    return 0;
}
/**
 * 
 * @param date
 * @param readEcc
 * @param calcEcc
 * @return 
 */
uint8_t nand2KEccCorrect(uint8_t *date, uint8_t *readEcc, uint8_t *calcEcc )
{
    volatile uint8_t pos = 0;
    for(pos = 0;pos <8; pos ++)
    {
       if(!nand_correct_data(date+pos*256, readEcc+pos *3, calcEcc + pos * 3 ))
       {
           
       }
    }
    return 0;
}

/**
 * 
 * @param flashRWdate
 * @param page
 * @return 
 */
uint8_t fileWriteAllPageEcc(uint32_t page ,struct flashDateStr *flashRWdate)
{
    nand2KEccCalculate(flashRWdate ->flashWriteDate[0].buffer,flashRWdate ->flashWriteDate[1].buffer);
    nanddrv_write_tr(page,flashRWdate ->flashWriteDate ,2);
    return 0;
}

/**
 * 
 * @param page
 * @param flashRWdate
 * @param nandFlashInfo
 * @return 
 */
uint8_t fileReadAllPageEcc(uint32_t page,struct flashDateStr *flashRWdate,struct nandFlashInfoStr *nandFlashInfo)
{
    uint8_t calcEcc[24]; // define the calcecc array for calculate read date' ecc.
    
    nanddrv_read_tr(page,flashRWdate ->flashReadDate,2);
    nand2KEccCalculate(flashRWdate ->flashReadDate[0].buffer,calcEcc);
    
    nand2KEccCorrect(flashRWdate ->flashReadDate[0].buffer, flashRWdate ->flashReadDate[1].buffer,calcEcc);
    
    return 0;
}


/**
 * 
 * @param pageBegin
 * @param pageEnd  two page interval will be copy,not included pageEnd
 * @return 
 */
uint8_t copyBlock(uint16_t srcBlock,uint16_t desBlock, uint8_t pageBegin, uint8_t pageEnd)
{
    uint8_t pos;
    nanddrv_erase(desBlock);
    for(pos = pageBegin ;pos<pageEnd;pos++)
    {
        fileReadAllPageEcc(pos+64*srcBlock,&flashRWTempdate,&nandFlashInfo);
        fileWriteAllPageEcc(pos+64*desBlock,&flashRWTempdate);
    }
    nanddrv_erase(srcBlock);
    
    return 0;
}
/**
 * //the operation is read 2K page,and record a microInverter date add this part in flashRWdate structure ,then write date to flash.
 * @param page
 * @param flashRWdate
 * @param microInverterDate
 * @return 
 */
uint8_t fileWritePartPageEcc(uint32_t page ,struct flashDateStr *flashRWdate ,struct nandFlashInfoStr *nandFlashInfo,struct microInverterDateCtrl *invertDateCtrl)
{
    uint16_t pos;
    fileReadAllPageEcc(page,flashRWdate,nandFlashInfo);
    memcpy(flashRWdate ->flashWriteDate[0].buffer,flashRWdate ->flashReadDate[0].buffer, (uint16_t)flashRWdate ->flashReadDate[1].buffer[24]);
    for(pos = 0;pos <2 ; pos ++)
    {
        flashRWdate ->flashWriteDate[0].buffer[(uint16_t)flashRWdate ->flashReadDate[1].buffer[24]+pos] = invertDateCtrl ->inverterDate[invertDateCtrl ->index].voltage[pos];
    }
    for(pos =0;pos <3;pos ++)
    {
        flashRWdate ->flashWriteDate[0].buffer[(uint16_t)flashRWdate ->flashReadDate[1].buffer[24]+pos+2] = invertDateCtrl ->inverterDate[invertDateCtrl ->index].current[pos];
    }
    for(pos =0;pos <4;pos ++)
    {
        flashRWdate ->flashWriteDate[0].buffer[(uint16_t)flashRWdate ->flashReadDate[1].buffer[24]+pos+2+3] = invertDateCtrl ->inverterDate[invertDateCtrl ->index].energy[pos];    
    }
        flashRWdate ->flashWriteDate[0].buffer[(uint16_t)flashRWdate ->flashReadDate[1].buffer[24]+pos+2+3+1] = invertDateCtrl ->inverterDate[invertDateCtrl ->index].fault;  
    fileWriteAllPageEcc(page ,flashRWdate); 
    return 0;
}
/**
 *  change main message to 2K date buffer ,for storing the main system date.
 * @param nandFlashInfo
 * @param buffer   we can use the flashWriteDate as date buffer.
 */
uint8_t storeMainSysMessage(uint32_t page,struct nandFlashInfoStr *nandFlashInfo)
{
    bufferClear(flashRWdate.flashWriteDate[0].buffer,2048);
    bufferClear(flashRWdate.flashWriteDate[1].buffer,64);  
    memcpy(flashRWdate.flashWriteDate[0].buffer,(uint8_t *)nandFlashInfo,sizeof(struct nandFlashInfoStr));
    fileWriteAllPageEcc(page,&flashRWdate);
    return 0;
} 
/**
 * ///load the main systme message to the ram
 * @param nandFlashInfo
 * @return 
 */
uint8_t loadMainSysMessage(uint32_t page ,struct nandFlashInfoStr *nandFlashInfo)
{
 
    fileReadAllPageEcc(page,&flashRWdate,nandFlashInfo);
    memcpy(nandFlashInfo,flashRWdate.flashReadDate[0].buffer,sizeof(struct nandFlashInfoStr));
     return 0;
}
/**
 * 
 * @param page
 * @param monthInfo
 * @return 
 */
uint8_t storeMonthInfo(uint32_t page,struct MonthInfoStr *monthInfo)
{
    bufferClear(flashRWdate.flashWriteDate[0].buffer,2048);
    bufferClear(flashRWdate.flashWriteDate[1].buffer,64); 
    memcpy(flashRWdate.flashWriteDate[0].buffer,(uint8_t *)monthInfo,sizeof(struct MonthInfoStr));
    fileWriteAllPageEcc(page,&flashRWdate);
    return 0;
}
/**
 * 
 * @param page
 * @param monthInfo
 * @return 
 */
uint8_t loadMonthInfo(uint32_t page,struct MonthInfoStr *monthInfo)
{
    
    fileReadAllPageEcc(page,&flashRWdate,&nandFlashInfo);
    memcpy(monthInfo,flashRWdate.flashReadDate[0].buffer,sizeof(struct MonthInfoStr));
    return 0;
}
/**
 * judge whether all block has been used.
 * @param nandFlashInfo
 * @return 
 */
uint8_t judgeAllBlockUsed(struct nandFlashInfoStr *nandFlashInfo)
{
    if(nandFlashInfo ->blockUsedPresent ==1024)
    {
        nandFlashInfo ->allBlockUsed =1;
        return 1;
    }
    return 0;
}
/**
 * every meet the end of every block,first you should check whether the next block  is good or not;
 * @param nandFlashInfo
 * @return 
 */
uint8_t judgeNextBlock(struct nandFlashInfoStr *nandFlashInfo)
{
    while((!perBadBlockCheck(nandFlashInfo ->blockUsedNext ,nandFlashInfo,&flashRWdate ))&&(nandFlashInfo ->badBlockChecked ==0))
    {
         nandFlashInfo ->blockUsedNext = nandFlashInfo ->blockUsedNext + 1;    //judge the next block go on.
    }
    return 0;
}

/**
 * When a new day coming ,we should calculate the needed all pages (85 pages) in time.
 * @param nandFlashInfo
 * @param monthInfo
 * @return 
 */
uint8_t calcAllPageNeededANewDay(struct nandFlashInfoStr *nandFlashInfo,struct MonthInfoStr *monthInfo)
{
    volatile uint8_t pageCnt =PageOneDay;   //pageCnt count the 85 pages one day;
    //firstly,we should make sure how many blocks needed for a new day;63-43=21;21+64=85;
    if(nandFlashInfo ->pageUsedPresent >(43+nandFlashInfo ->blockUsedPresent*64))
    {
        monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].BlockNum =3;
    }
    else
    {
        monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].BlockNum =2;
    }
    
    //if we need to distribute 3 blocks in time;
    if(monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].BlockNum == 3)
    {
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].whichBlockLeft     =nandFlashInfo ->blockUsedPresent;
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].leftBlockStartPage =nandFlashInfo ->pageUsedPresent;
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].leftBlockStartPage =nandFlashInfo ->blockUsedPresent*64+63;
       
       pageCnt =pageCnt -(64-nandFlashInfo ->blockUsedPresent);
       judgeNextBlock(nandFlashInfo);
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].whichBlockMiddle     =nandFlashInfo ->blockUsedNext;
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockStartPage =(nandFlashInfo ->blockUsedNext)*64;
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockEndPage =nandFlashInfo ->blockUsedNext*64+63;
       
       pageCnt =pageCnt -64;
       judgeNextBlock(nandFlashInfo);
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].whichBlockRight     =nandFlashInfo ->blockUsedNext;
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].rightBlockStartPage =(nandFlashInfo ->blockUsedNext)*64;
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].rightBlockEndPage =nandFlashInfo ->blockUsedNext*64+pageCnt-1; //don't include the right page.
       
    }
    //if we need to distribute 2 blocks in time;
    else if(monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].BlockNum ==2)
    {
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].whichBlockLeft     =nandFlashInfo ->blockUsedPresent;
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].leftBlockStartPage =nandFlashInfo ->pageUsedPresent;
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].leftBlockStartPage =nandFlashInfo ->blockUsedPresent*64+63;
       
       pageCnt =pageCnt -(64-nandFlashInfo ->blockUsedPresent);
       judgeNextBlock(nandFlashInfo);
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].whichBlockMiddle     =nandFlashInfo ->blockUsedNext;
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockStartPage =(nandFlashInfo ->blockUsedNext)*64;
       monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockEndPage =nandFlashInfo ->blockUsedNext*64+pageCnt-1;
    }
    return 0;
}
/**
 * //the core algorithm of flash main block store mapping to another  month page;
 * @param nandFlashInfo
 * @param monthInfo
 * @return 
 */
uint8_t timeMapping(struct nandFlashInfoStr *nandFlashInfo,struct MonthInfoStr *monthInfo,struct sysTimeStr *sysTime)
{
    
    if(nandFlashInfo ->blockUsedPresent !=(nandFlashInfo->pageUsedPresent+1) /64)
    {
        nandFlashInfo ->blockUsedPresent = nandFlashInfo ->blockUsedNext;
        nandFlashInfo ->badBlockChecked  = 0;
    }
    
    if(nandFlashInfo ->badBlockChecked ==0)     //judge the next block whether checked or not
    {
        nandFlashInfo ->blockUsedNext = nandFlashInfo ->blockUsedPresent + 1;
    }
//    //judge the bad block. If the present page is the end page of every block,then need to judge the next block.
//    if(nandFlashInfo ->pageUsedPresent %64 ==63)  
//    {
//        judgeNextBlock(nandFlashInfo);
//    }

    //the following code is recording the date mapping to data;
    //change another year;
    if(nandFlashInfo ->yearMonthCtl[nandFlashInfo ->yearCnt].year != sysTime ->timeDate.year)
    {
        nandFlashInfo ->yearCnt =nandFlashInfo ->yearCnt+1;
        nandFlashInfo ->monthCnt = 0;    //if a new year change,the nandFlashInfo ->monthCnt =0;
    }
    //change another month;
    if(nandFlashInfo ->yearMonthCtl[nandFlashInfo ->yearCnt].monthWithIndex[nandFlashInfo->monthCnt].month != sysTime ->timeDate.month)
    {
        nandFlashInfo ->monthCnt =nandFlashInfo ->monthCnt +1;
        nandFlashInfo ->monthMappingPage = nandFlashInfo ->monthMappingPage+1;
        nandFlashInfo ->dayCnt = 0;     //the index of day will be reset;
        monthInfo ->structLens =sizeof(struct MonthInfoStr);
        monthInfo ->month = sysTime ->timeDate.month;
        
        if(nandFlashInfo ->monthCnt == 12)
        {
            nandFlashInfo ->monthCnt = 0;     //if the month is great than 12,the monthCnt = 0;
        }
        
    }
    //change another day; when  a new day coming ,we need to calculate the day's date record consists of which 3 or 2 blocks;
    if(monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].day !=sysTime ->timeDate.day)
    {
        nandFlashInfo ->dayCnt = nandFlashInfo ->dayCnt +1;
        monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].day =sysTime ->timeDate.day;
        //if a new day is coming ,the next page will be used.
//      monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].index = nandFlashInfo ->pageUsedNext;
        //calculate the blocks we need when a new day coming.
        calcAllPageNeededANewDay(nandFlashInfo,monthInfo);
    }
    
    switch (nandFlashInfo ->yearCnt)
    {   
        case 0:
            nandFlashInfo ->yearMonthCtl[0].year =sysTime ->timeDate.year;
            nandFlashInfo ->yearMonthCtl[0].monthWithIndex[nandFlashInfo ->monthCnt].month = sysTime ->timeDate.month;
            nandFlashInfo ->yearMonthCtl[0].monthWithIndex[nandFlashInfo ->monthCnt].index = nandFlashInfo ->monthMappingPage;
            break;
        case 1:
            nandFlashInfo ->yearMonthCtl[1].year =sysTime ->timeDate.year;
            nandFlashInfo ->yearMonthCtl[1].monthWithIndex[nandFlashInfo ->monthCnt].month = sysTime ->timeDate.month;
            nandFlashInfo ->yearMonthCtl[1].monthWithIndex[nandFlashInfo ->monthCnt].index = nandFlashInfo ->monthMappingPage;
            break;
        case 2:
            nandFlashInfo ->yearMonthCtl[2].year =sysTime ->timeDate.year;
            nandFlashInfo ->yearMonthCtl[2].monthWithIndex[nandFlashInfo ->monthCnt].month = sysTime ->timeDate.month;
            nandFlashInfo ->yearMonthCtl[2].monthWithIndex[nandFlashInfo ->monthCnt].index = nandFlashInfo ->monthMappingPage;
            break;
        case 3:
            nandFlashInfo ->yearMonthCtl[3].year =sysTime ->timeDate.year;
            nandFlashInfo ->yearMonthCtl[3].monthWithIndex[nandFlashInfo ->monthCnt].month = sysTime ->timeDate.month;
            nandFlashInfo ->yearMonthCtl[3].monthWithIndex[nandFlashInfo ->monthCnt].index = nandFlashInfo ->monthMappingPage;
            break;
        default:
            break;
    }
    return 0;
    
}
/**
 * 
 * @param nandFlashInfo
 * @param monthInfo
 * @return 
 */
uint8_t SeekNextPage(struct nandFlashInfoStr *nandFlashInfo ,struct MonthInfoStr   *monthInfo)
{
    //if the new day needs 3 blocks.
    if(monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].BlockNum ==3)
    {
        if((nandFlashInfo ->blockUsedPresent>=monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].leftBlockStartPage)&&(nandFlashInfo ->blockUsedPresent<=monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].leftBlockEndPage))
        {
            nandFlashInfo ->blockUsedPresent = nandFlashInfo ->pageUsedNext ;
            nandFlashInfo ->pageUsedNext++;
            if(nandFlashInfo ->pageUsedNext > monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].leftBlockEndPage)
            {
                nandFlashInfo ->pageUsedNext =monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockStartPage;
            }
        }
        else if((nandFlashInfo ->blockUsedPresent>=monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockStartPage)&&(nandFlashInfo ->blockUsedPresent<=monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockEndPage))
        {
            nandFlashInfo ->blockUsedPresent = nandFlashInfo ->pageUsedNext ;
            nandFlashInfo ->pageUsedNext++;
            if(nandFlashInfo ->pageUsedNext > monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockEndPage)
            {
                nandFlashInfo ->pageUsedNext =monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].rightBlockStartPage;
            }
        }
        else if((nandFlashInfo ->blockUsedPresent >= monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].rightBlockStartPage)&&(nandFlashInfo ->blockUsedPresent <= monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].rightBlockEndPage))
        {
            nandFlashInfo ->blockUsedPresent = nandFlashInfo ->pageUsedNext ;
            nandFlashInfo ->pageUsedNext++;
        }
    }
    //if the new day needs 2 blocks
    else if(monthInfo ->dayWithIndex[nandFlashInfo ->dayCnt].BlockNum ==2)
    {
         if((nandFlashInfo ->blockUsedPresent>=monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].leftBlockStartPage)&&(nandFlashInfo ->blockUsedPresent<=monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].leftBlockEndPage))
        {
            nandFlashInfo ->blockUsedPresent = nandFlashInfo ->pageUsedNext ;
            nandFlashInfo ->pageUsedNext++;
            if(nandFlashInfo ->pageUsedNext > monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].leftBlockEndPage)
            {
                nandFlashInfo ->pageUsedNext =monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockStartPage;
            }
        }
        else if((nandFlashInfo ->blockUsedPresent>=monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockStartPage)&&(nandFlashInfo ->blockUsedPresent<=monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockEndPage))
        {
            nandFlashInfo ->blockUsedPresent = nandFlashInfo ->pageUsedNext ;
            nandFlashInfo ->pageUsedNext++;
            if(nandFlashInfo ->pageUsedNext > monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].middleBlockEndPage)
            {
                nandFlashInfo ->pageUsedNext =monthInfo->dayWithIndex[nandFlashInfo ->dayCnt].rightBlockStartPage;
            }
        }
    }
}
/**
 * //store the microinverter date according to pages. one page can store 20 panels
 * @param nandFlashInfo
 * @param invertDateCtrl
 * @return 
 */
uint8_t storeMicroInverterDatePage(struct nandFlashInfoStr *nandFlashInfo ,struct microInverterDateCtrl *invertDateCtrl,struct sysTimeStr *sysTime,struct microInverterDatePageStr *microInverterDatePage,struct MonthInfoStr   *monthInfo)
{
    
    if((invertDateCtrl ->index ==(20-1))&&(invertDateCtrl ->index <nandFlashInfo ->solarPanelCnt))
    {
        microInverterDatePage ->structLens =sizeof(struct microInverterDatePageStr);
        microInverterDatePage ->day        =sysTime ->timeDate.day;
        microInverterDatePage ->hour       =sysTime ->timeDate.hour;
        microInverterDatePage ->inverterDate[0] = (struct microInverterDateU )invertDateCtrl ->inverterDate[(invertDateCtrl ->index)-19];
        memcpy(flashRWdate.flashWriteDate[0].buffer,(uint8_t *)microInverterDatePage,sizeof(struct microInverterDatePageStr));
        fileWriteAllPageEcc(nandFlashInfo->pageUsedPresent,&flashRWdate);      // the typical problem is get the exact page mapping.

        SeekNextPage(nandFlashInfo,monthInfo);
    }
    if((invertDateCtrl ->index ==(20*2-1))&&(invertDateCtrl ->index <nandFlashInfo ->solarPanelCnt))
    {
        microInverterDatePage ->structLens =sizeof(struct microInverterDatePageStr);
        microInverterDatePage ->day        =sysTime ->timeDate.day;
        microInverterDatePage ->hour       =sysTime ->timeDate.hour;
        microInverterDatePage ->inverterDate[0] = (struct microInverterDateU )invertDateCtrl ->inverterDate[(invertDateCtrl ->index)-19];
        memcpy(flashRWdate.flashWriteDate[0].buffer,(uint8_t *)microInverterDatePage,sizeof(struct microInverterDatePageStr));
        fileWriteAllPageEcc(nandFlashInfo->pageUsedPresent,&flashRWdate); 
        
        SeekNextPage(nandFlashInfo,monthInfo);
    }
    
    if((invertDateCtrl ->index ==(20*3-1))&&(invertDateCtrl ->index <nandFlashInfo ->solarPanelCnt))
    {
        microInverterDatePage ->structLens =sizeof(struct microInverterDatePageStr);
        microInverterDatePage ->day        =sysTime ->timeDate.day;
        microInverterDatePage ->hour       =sysTime ->timeDate.hour;
        microInverterDatePage ->inverterDate[0] = (struct microInverterDateU )invertDateCtrl ->inverterDate[(invertDateCtrl ->index)-19];
        memcpy(flashRWdate.flashWriteDate[0].buffer,(uint8_t *)microInverterDatePage,sizeof(struct microInverterDatePageStr));
        fileWriteAllPageEcc(nandFlashInfo->pageUsedPresent,&flashRWdate);    
        
        SeekNextPage(nandFlashInfo,monthInfo);
    }
    if((invertDateCtrl ->index ==(20*4-1))&&(invertDateCtrl ->index <nandFlashInfo ->solarPanelCnt))
    {
        microInverterDatePage ->structLens =sizeof(struct microInverterDatePageStr);
        microInverterDatePage ->day        =sysTime ->timeDate.day;
        microInverterDatePage ->hour       =sysTime ->timeDate.hour;
        microInverterDatePage ->inverterDate[0] = (struct microInverterDateU )invertDateCtrl ->inverterDate[(invertDateCtrl ->index)-19];
        memcpy(flashRWdate.flashWriteDate[0].buffer,(uint8_t *)microInverterDatePage,sizeof(struct microInverterDatePageStr));
        fileWriteAllPageEcc(nandFlashInfo->pageUsedPresent,&flashRWdate);   
        
        SeekNextPage(nandFlashInfo,monthInfo);
    }
    if((invertDateCtrl ->index ==(20*5-1))&&(invertDateCtrl ->index <nandFlashInfo ->solarPanelCnt))
    {
        microInverterDatePage ->structLens =sizeof(struct microInverterDatePageStr);
        microInverterDatePage ->day        =sysTime ->timeDate.day;
        microInverterDatePage ->hour       =sysTime ->timeDate.hour;
        microInverterDatePage ->inverterDate[0] = (struct microInverterDateU )invertDateCtrl ->inverterDate[(invertDateCtrl ->index)-19];
        memcpy(flashRWdate.flashWriteDate[0].buffer,(uint8_t *)microInverterDatePage,sizeof(struct microInverterDatePageStr));
        fileWriteAllPageEcc(nandFlashInfo->pageUsedPresent,&flashRWdate);   
        
        SeekNextPage(nandFlashInfo,monthInfo);
    }
    return 0;
}
/**
 * //load microinverter date, load one page ,you can get 20 panel's date;
 * @param nandFlashInfo
 * @param invertDateCtrl
 * @return 
 */
uint8_t loadMicroInverterDatePage(struct nandFlashInfoStr *nandFlashInfo ,struct microInverterDateCtrl *invertDateCtrl)
{

}
/**
 * fileTest function is test every function.
 */

void fileTest()
{
//      nanddrv_erase(0);
//      initFlashInfo(&nandFlashInfo);
//      storeMainSysMessage(0,&nandFlashInfo);
////      monthInfo.structLens = sizeof(struct MonthInfoStr);
////      monthInfo.month =3;
////      monthInfo.dayWithIndex[0].day = 12;
////      storeMonthInfor(1,&monthInfo);
////      loadMonthInfor(1,&monthInfoTemp);
//      loadMainSysMessage(0 ,&nandFlashInfo);
      
//    invertDateCtrl.inverterDate[0].voltage[0] = 0x11;
//    invertDateCtrl.inverterDate[0].voltage[1] = 0x00;
//    invertDateCtrl.inverterDate[0].current[0] = 0x02;
//    invertDateCtrl.inverterDate[0].current[1] = 0x01;
//    invertDateCtrl.inverterDate[0].current[2] = 0x00;
//    invertDateCtrl.inverterDate[0].energy[0]  = 0x11;
//    invertDateCtrl.inverterDate[0].energy[1]  = 0x01;
//    invertDateCtrl.inverterDate[0].energy[2]  = 0x03;
//    invertDateCtrl.inverterDate[0].energy[3]  = 0x34;
//    invertDateCtrl.inverterDate[0].fault      =0x11;
//    
//    invertDateCtrl.inverterDate[1].voltage[0] = 0x11;
//    invertDateCtrl.inverterDate[1].voltage[1] = 0x00;
//    invertDateCtrl.inverterDate[1].current[0] = 0x02;
//    invertDateCtrl.inverterDate[1].current[1] = 0x01;
//    invertDateCtrl.inverterDate[1].current[2] = 0x00;
//    invertDateCtrl.inverterDate[1].energy[0]  = 0x11;
//    invertDateCtrl.inverterDate[1].energy[1]  = 0x01;
//    invertDateCtrl.inverterDate[1].energy[2]  = 0x03;
//    invertDateCtrl.inverterDate[1].energy[3]  = 0x34;
//    invertDateCtrl.inverterDate[1].fault      =0x11;
//
    
//      nanddrv_erase(0);
//      initFlashBuffer();
//      initFlashInfo(&nandFlashInfo);
//      initFlashInfo(&nandFlashInfo2);
//      nandFlashInfo2.blockUsedNext =0;
//      nandFlashInfo2.blockUsedPresent=0;
//      nandFlashInfo2.pageUsedNext=0;
//      nandFlashInfo2.pageUsedPresent=0;
  //    changeMainMessageTo2K(&nandFlashInfo,&nandFlashInfo2);
      
      
//      nanddrv_erase(0);
//      memset(flashWriteBuffer,0x36,2048);
//      fileWriteAllPageEcc(0,&flashRWdate);
//      copyBlock(0,1,0, 1);
//      fileReadAllPageEcc(64,&flashRWdate,&nandFlashInfo);
//      fileReadAllPageEcc(0,&flashRWdate,&nandFlashInfo);
//      copyBlock(1,0, 0, 1);
//      fileReadAllPageEcc(64,&flashRWdate,&nandFlashInfo);
//      fileReadAllPageEcc(0,&flashRWdate,&nandFlashInfo);
      
    
//    fileWritePartPageEcc(0,&flashRWdate,&nandFlashInfo,invertDateCtrl);
}




