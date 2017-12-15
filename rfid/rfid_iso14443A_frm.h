/**
 *	rfid_iso14443A_frm.h
 *
 *
 *	对于标准帧／防碰撞帧，计算奇校验；
 *	对于标准帧，计算CRC;
 *	对于防碰撞帧，在PICC->PCD方向，考虑字节不全的情况的场景；
 *	调整比特序列：单个字节，先输出低比特，后输出高比特；
 */

#ifndef _RFID_ISO14443A_FRM_H_
#define _RFID_ISO14443A_FRM_H_

#include "rfid_def.h"
#include "rfid_chk.h"


int ISO14443A_stdFraming( BYTE aInBuf[], UINT16 u16InBitLen, BYTE aoutBuf[], UINT16 *pu16OutBitLen );
int ISO14443A_ShortFraming( BYTE byCmd, BYTE aoutBuf[], UINT16 *pu16OutBitLen );
int ISO14443A_AnticollisionFraming( 				
				BYTE		byBitLenofFirstByte,						// 第一个字节中的bit Len，在防碰撞帧里需要
				BYTE		byFirstByte,
				BYTE 		ainBuf[], UINT16 u16inBitLen, 
				BYTE 		aoutBuf[], UINT16 *pu16OutBitLen 
		);
		
int ISO14443A_Un_stdFraming(BYTE ainBuf[], UINT16 u16inBitLen, BYTE aoutBuf[], UINT16 *pu16OutbyLen );
int ISO14443A_Un_ShortFraming( BYTE ainBuf[], UINT16 u16InBitLen, BYTE *pbyCmd );
int ISO14443A_Un_AnticollisionFraming(
				BYTE		byBitLenofFirstByte,						// 第一个字节中的bit Len，在防碰撞帧里需要
				BYTE		*pbyFirstByte,
				BYTE 		ainBuf[], UINT16 u16inBitLen, 
				BYTE 		aoutBuf[], UINT16 *pu16OutBitLen 
		);	
		
/**
 *	ISO14443A Frame
 *	标准
 *	包括：
 *				调节一个字节内的比特顺序
 *				校验P奇校验.
 *	不包括SOF/EOF
 */
 
int ISO14443A_Framing(
				BYTE		byBitLenofFirstByte,						// 第一个字节中的bit Len，在防碰撞帧里需要
				BYTE		byFirstByte,								// 第一字节。若u16BitLenofFirstByte=0，可以不考虑 
				BYTE 		ainBuf[], UINT16 u16inBitLen, 				// 其他字节
				BYTE 		aoutBuf[], UINT16 *pu16OutBitLen
			 );

int ISO14443A_unFraming(
				BYTE		byBitLenofFirstByte,						// 第一个字节中的bit Len，在防碰撞帧里需要
				BYTE		*byFirstByte,								// 第一字节。若u16BitLenofFirstByte=0，可以不考虑 
				BYTE 		ainBuf[], UINT16 u16inBitLen, 				// 其他字节
				BYTE 		aoutBuf[], UINT16 *pu16OutBitLen			// 输出比特不包括*byBitLenofFirstByte
			);
#endif


 