/**
 *	\file	rfid_chk.h
 *
 *	\brief	实现rfid中的校验。
 */


#ifndef _RFID_CHECK_H_
#define _RFID_CHECK_H_

#include "rfid_def.h"
/**
 * 	\function 奇校验
 *
 *	\input	输入：BYTE bDat: 0/1
 *	\ouput
 *	\return	返回校验的值：0或者1.
 */
int OldParity(BYTE Dat );

/**
 * 	CRC校验
 *
 *	\input	BYTE 	aBuf[],缓冲区
 *			UINT16	u16Len 缓冲区长度
 *	\ouput	BYTE 	*pCRCMSB	CRC高8bit
 *			BYTE	*pCRCLSB	CRC低8bit
 *	\return	1成功，<0失败.
 */

int ISO14443A_CRC( BYTE aBuf[], UINT16 u16Len,BYTE *pCRCMSB, BYTE *pCRCLSB );
int ISO14443B_CRC( BYTE aBuf[], UINT16 u16Len,BYTE *pCRCMSB, BYTE *pCRCLSB );
int ISO15693_CRC( BYTE aBuf[], UINT16 u16Len, BYTE *pCRCMSB, BYTE *pCRCLSB );

/**
 *	ISO14443B
 *
 *
 *	每个字节，前加1bit start, 后面加1bit的 stop和1比特的egt.
 *	考虑到传输：先传输LSB，所以要对调比特序:
 */
// 同时要对调比特序！
int ISO14443B_ByteExp( BYTE byDat );
// 返回原始字节
BYTE IOS14443B_UnByteExp( UINT16 u16Dat );

#endif
