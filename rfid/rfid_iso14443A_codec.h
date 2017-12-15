/**
 *	rfid_iso14443A_codec.h
 *
 */

#ifndef _RFID_ISO14443A_CODEC_H_
#define _RFID_ISO14443A_CODEC_H_

#include "rfid_def.h"

#define ISO14443A_PCD_X			0xD		// 1101, '1'
#define ISO14443A_PCD_Y 		0xF		// 1111, '0'	EOF
#define ISO14443A_PCD_Z 		0x7		// 0111, '0', 	SOF

#define ISO14443A_PCD_SOF		ISO14443A_PCD_Z
// EOF为0, #define ISO14443A_PCD_EOF
#define ISO14443A_PCD_1			ISO14443A_PCD_X
#define ISO14443A_PCD_0P1		ISO14443A_PCD_Y		// 编码0，前一个比特为1
#define ISO14443A_PCD_0P0		ISO14443A_PCD_Z		// 编码0，前一个比特为0
#define ISO14443A_BIT_CODEC_LEN	4					// 每个比特编码长度为4


#define ISO14443A_PICC_D 		0xC		// 1100，'1', SOF
#define ISO14443A_PICC_E 		0x3		// 0011, '0'
#define ISO14443A_PICC_F		0x0		// 0000,      EOF

#define ISO14443A_PICC_SOF		ISO14443A_PICC_D
#define ISO14443A_PICC_EOF		ISO14443A_PICC_F
#define ISO14443A_PICC_1		ISO14443A_PICC_D
#define ISO14443A_PICC_0		ISO14443A_PICC_E

// ISO/IEC 14443A, PCD->PICC使用
#define RFID_CODE_MILLER_1			13	// 1101
#define RFID_CODE_MILLER_0_p1		15	// 1111
#define RFID_CODE_MILLER_0_p0		7	// 0111

// ISO/IEC 14443A, PICC->PCD使用
#define RFID_CODE_MANCHESTER_1		12	// 1100
#define RFID_CODE_MANCHESTER_0		3	// 0011


/**
 * 	\function
 *
 *	\input	BYTE 	aInBuf[],		输入缓冲区
 *			UINT16	u16InBitLen		缓冲区比特长度
 *	\ouput	BYTE 	aoutBuf			解码后输出的数据
 *			BYTE	*pu16OutBitLen	解码后数据长度
 *
 *	\return	-1，失败， 其他返回解码后比特长度
 */

int ISO14443A_PCD_Code( 
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aOutBuf[], UINT16 *pu16OutBitLen 
		);
			
int ISO14443A_PCD_Decode( 
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aOutBuf[], UINT16 *pu16OutBitLen 
		);
			
int ISO14443A_PICC_Code( 
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aoutBuf[], UINT16 *pu16OutBitLen 
		);

int ISO14443A_PICC_Decode(
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aOutBuf[], UINT16 *pu16OutBitLen 
		);
		
/**
 *	1. bPCD:　		从PCD/PICC发出的帧，其编码方式/SOF和EOF都不同；
 *	下述函数，考虑了该情况;
 */
int ISO14443A_Code( 
			BOOL bPCD,							// PCD编码端
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aoutBuf[], UINT16 *pu16OutBitLen 
		);
			
int ISO14443A_Decode(
			BOOL bPCD, 							// PCD编码端
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aOutBuf[], UINT16 *pu16OutBitLen 
		);
			
/*-------------------------------------------------------------*\
 *	Miller
 *	ISO14443A PCD->PICC 用
 *-------------------------------------------------------------*/
/**
 * 	改进的Miller编码,返回4比特编码
 *
 *	\input	输入：BYTE prebit: 0/1，当前比特的前一个比特
 *			输入：BYTE bitDat: 0/1，当前待编码比特
 *	\ouput
 *	\return	bitDat=1, 返回1101
 *			prebit=1,bitDat=0，返回：1111
 *			prebit=0,bitDat=0，返回：0111
 */
int RFID_Code_Miller( BYTE prebit, BYTE bitDat );	
/**
 * 	改进的Miller解码：返回0或者1.其他失败
 *
 *	\input	输入：BYTE codedDat: 1101 for 1, 1111/0111 for 0
 *	\ouput
 *	\return	0/1
 *			-1 ,非法输入
 */
int RFID_Decode_Miller( BYTE codedDat );

/*-------------------------------------------------------------*\
 *	Manchester
 *	ISO14443A PICC->PCD
 *-------------------------------------------------------------*/

/**
 * 	 Manchester编码，返回4比特编码
 *
 *	\input	输入：BYTE bitDat: 0/1，当前待编码比特
 *	\ouput
 *	\return	bitDat=1, 返回：1100
 *			bitDat=0，返回：0011
 */
int RFID_Code_Manchester( BYTE bitDat );	// bitDat=1 or 0
/**
 * 	Manchester解码：返回0或者1.其他失败
 *
 *	\input	输入：BYTE codedDat: 1100 for 1, 0011 for 0
 *	\ouput
 *	\return	0/1
 *			-1 ,非法输入
 */
int RFID_Decode_Manchester( BYTE codedDat );

#endif
