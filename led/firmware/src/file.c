#include "file.h"
///*******************NOTE:**********************/////
//this nandflash's character is :64K pages of (2048+64) bytes each
//this flash consists of 1K blocks of 64pages each;
//                       OOB
//[0..23]-->ECC   [24..25]-->this page has used.
///*******************NOTE:**********************/////
uint8_t flashWriteBuffer[2048];
uint8_t flashWriteBufferOOB[64];

uint8_t flashReadBuffer[2048];
uint8_t flashReadBufferOOB[64];


struct flashDateStr flashRWdate;
struct nandFlashInfoStr nandFlashInfo;

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
}

/**
 * initial the  nand flash information .
 * @param nandFlashInfo
 */
void initFlashInfo(struct nandFlashInfoStr *nandFlashInfo)
{
    uint16_t pageNum;
    nandFlashInfo ->badBlockNum=0;
    for(pageNum = 0;pageNum <1024; pageNum ++)
    {
        nandFlashInfo ->badBlockPos[pageNum] = 0;
    }
    nandFlashInfo ->blockUsedPresent = 0;
    nandFlashInfo ->blockUsedPresent = 0;
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
//        memset(flashWriteBuffer,0x11,2048);
//        memset(flashWriteBufferOOB,0x11,64);
//        nanddrv_write_tr(blockNum*64+pageNum, flashRWdate ->flashWriteDate,2);
        nanddrv_read_tr(blockNum*64+pageNum,flashRWdate->flashReadDate,2);
        badBlockResult=allBytes0xFFCheck(flashRWdate->flashReadDate,2);
        if (!badBlockResult)
        {
            nandFlashInfo ->badBlockNum++;
            *(nandFlashInfo ->badBlockPos+blockNum/8) = *(nandFlashInfo ->badBlockPos+blockNum/8)|(1 <<(blockNum%8));
            return 0;
        }
    }
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
    flashWriteBuffer[0] = 0x03;
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
 * fileTest function is test every function.
 */
void fileTest()
{
    invertDateCtrl.inverterDate[0].voltage[0] = 0x11;
    invertDateCtrl.inverterDate[0].voltage[1] = 0x00;
    invertDateCtrl.inverterDate[0].current[0] = 0x02;
    invertDateCtrl.inverterDate[0].current[1] = 0x01;
    invertDateCtrl.inverterDate[0].current[2] = 0x00;
    invertDateCtrl.inverterDate[0].energy[0]  = 0x11;
    invertDateCtrl.inverterDate[0].energy[1]  = 0x01;
    invertDateCtrl.inverterDate[0].energy[2]  = 0x03;
    invertDateCtrl.inverterDate[0].energy[3]  = 0x34;
    invertDateCtrl.inverterDate[0].fault      =0x11;
    
    invertDateCtrl.inverterDate[1].voltage[0] = 0x11;
    invertDateCtrl.inverterDate[1].voltage[1] = 0x00;
    invertDateCtrl.inverterDate[1].current[0] = 0x02;
    invertDateCtrl.inverterDate[1].current[1] = 0x01;
    invertDateCtrl.inverterDate[1].current[2] = 0x00;
    invertDateCtrl.inverterDate[1].energy[0]  = 0x11;
    invertDateCtrl.inverterDate[1].energy[1]  = 0x01;
    invertDateCtrl.inverterDate[1].energy[2]  = 0x03;
    invertDateCtrl.inverterDate[1].energy[3]  = 0x34;
    invertDateCtrl.inverterDate[1].fault      =0x11;
    nanddrv_erase(0);
    fileWritePartPageEcc(0,&flashRWdate,&nandFlashInfo,invertDateCtrl);
}




