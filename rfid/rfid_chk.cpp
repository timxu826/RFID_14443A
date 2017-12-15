/**
 *	\file	rfid_chk.c
 *
 *	\brief	实现rfid中的校验。
 */
//#include "StdAfx.h"

#include "rfid_chk.h"

/**
 * 	CRC校验
 *
 *	\input	BYTE 	aBuf[],缓冲区
 *			UINT16	u16Len 缓冲区长度
 *			UINT16	u16InitPrm	CRC16初值.
 *	\ouput	BYTE 	*pCRCMSB	CRC高8bit
 *			BYTE	*pCRCLSB	CRC低8bit
 *	\return	1成功，<0失败.
 */
int CRC16( UINT16 u16InitPrm, BYTE aBuf[], UINT16 u16Len,BYTE *pCRCMSB, BYTE *pCRCLSB );

int OldParity(BYTE Dat )
{
	int k=0;
	int i;
	
	for( i = 0; i < 8; i++ )
	{
		if(( Dat >> i ) & 0x01 )
		{
			k++;
		}
	}
	return k % 2 ? 0:1;
}

int ISO14443A_CRC( BYTE aBuf[], UINT16 u16Len,BYTE *pCRCMSB, BYTE *pCRCLSB )
{
	return CRC16( 0x6363, aBuf, u16Len, pCRCMSB, pCRCLSB ); 
}


UINT16 UpdateCrc(BYTE ch, UINT16 *lpwCrc)
{
	ch = (ch^(unsigned char)((*lpwCrc) & 0x00FF));
	ch = (ch^(ch<<4));

	*lpwCrc = (*lpwCrc >> 8)^((UINT16)ch << 8)^((UINT16)ch<<3)^((UINT16)ch>>4);
	return(*lpwCrc);
}

/**
 * 	CRC校验
 *
 *	\input	BYTE 	aBuf[],缓冲区
 *			UINT16	u16Len 缓冲区长度
 *			UINT16	u16InitPrm	CRC16初值.
 *	\ouput	BYTE 	*pCRCMSB	CRC高8bit
 *			BYTE	*pCRCLSB	CRC低8bit
 *	\return	1成功，<0失败.
 */
int CRC16( UINT16 u16InitValue, BYTE aBuf[], UINT16 u16Len,BYTE *pCRCMSB, BYTE *pCRCLSB )
{
	UINT16 wCrc;
	unsigned char chBlock;
	int i;
	
	wCrc = u16InitValue;
	for( i = 0; i < u16Len; i++ )
	{
		chBlock = aBuf[i];
		UpdateCrc(chBlock, &wCrc);
	}
	if( u16InitValue == 0xFFFF )
	{
		wCrc = ~wCrc; // ISO 3309
	}
	*pCRCLSB = (BYTE) (wCrc & 0xFF);
	*pCRCMSB = (BYTE) ((wCrc >> 8) & 0xFF);

	return 1;
}

#if 0
/*------------------------------------------------------------*\
 *	如下是标准中，CRC的计算demo.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CRC_A 1
#define CRC_B 2
#define BYTE unsigned char

unsigned short UpdateCrc(unsigned char ch, unsigned short *lpwCrc)
{
ch = (ch^(unsigned char)((*lpwCrc) & 0x00FF));
ch = (ch^(ch<<4));

*lpwCrc = (*lpwCrc >> 8)^((unsigned short)ch << 8)^((unsigned
short)ch<<3)^((unsigned short)ch>>4);
return(*lpwCrc);
}

void ComputeCrc(int CRCType, char *Data, int Length,
BYTE *TransmitFirst, BYTE *TransmitSecond)
{
unsigned char chBlock;
unsigned short wCrc;

switch(CRCType) {
case CRC_A:
wCrc = 0x6363; // ITU-V.41
break;
case CRC_B:
wCrc = 0xFFFF; // ISO 3309
break;
default:
return;
}

do {
chBlock = *Data++;
UpdateCrc(chBlock, &wCrc);
} while (--Length);
if (CRCType == CRC_B)
wCrc = ~wCrc; // ISO 3309
*TransmitFirst = (BYTE) (wCrc & 0xFF);
*TransmitSecond = (BYTE) ((wCrc >> 8) & 0xFF);

return;
}

BYTE BuffCRC_A[10] = {0x12, 0x34};
BYTE BuffCRC_B[10] = {0x0A, 0x12, 0x34, 0x56};
unsigned short Crc;
BYTE First, Second;
FILE *OutFd;
int i;

int main(void)
{
printf("CRC-16 reference results 3-Jun-1999\n");

printf("by Mickey Cohen - mickey@softchip.com\n\n");
printf("Crc-16 G(x) = x^16 + x^12 + x^5 + 1\n\n");

printf("CRC_A of [ ");
for(i=0; i<2; i++) printf("%02X ",BuffCRC_A[i]);
ComputeCrc(CRC_A, BuffCRC_A, 2, &First, &Second);
printf("] Transmitted: %02X then %02X.\n", First, Second);

printf("CRC_B of [ ");
for(i=0; i<4; i++) printf("%02X ",BuffCRC_B[i]);
ComputeCrc(CRC_B, BuffCRC_B, 4, &First, &Second);
printf("] Transmitted: %02X then %02X.\n", First, Second);

return(0);
}
/*------------------------------------------------------------*\
 *	如下是标准中，CRC的计算demo.
 *
 */

#endif