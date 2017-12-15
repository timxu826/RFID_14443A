/**
 *	\file	rfid_frm.c
 *
 *	\brief	实现rfid中的组/解帧。
 *
 *			CRC在之前计算，因为在防碰撞帧中，要全部字节参与计算，但只有部分字节成帧。
 */

//#include "StdAfx.h"

#include "rfid_chk.h"
#include "rfid_iso14443A_frm.h"
#include "memory.h"

/** 
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
			 )
{
	int i, j, k, curbit;
	int byoffset, bitoffset;
	int bitLen = 0;

	// 非0，标示防碰撞帧
	byoffset = 0;
	aoutBuf[byoffset] = 0x00;
	bitoffset = 0;
	if( byBitLenofFirstByte )
	{
		for( i = 0; i < byBitLenofFirstByte; i++ )
		{
			curbit = ( byFirstByte >> i ) & 0x01; 

			byoffset = bitLen / 8;
			bitoffset = bitLen % 8;
			aoutBuf[byoffset] |= curbit << ( 7-bitoffset);
			bitLen++;
		}
		// OldParity.
		byoffset = bitLen / 8;
		bitoffset = bitLen % 8;
		aoutBuf[byoffset] |= 0 << ( 7-bitoffset);
		bitLen++;
	}
	
	// 开始编码所有的数据
	i = 0;
	while( u16inBitLen  )
	{
		// 根据 bitLen，计算字节.
		if( u16inBitLen > 8 )
		{
			k = 8;
		}
		else
		{
			k = u16inBitLen;
		}
		
		for( j = 0; j < k; j++ )
		{
			// 根据 bitLen，计算字节.
			curbit = ( ainBuf[i] >> j ) & 0x01;
			
			// 计算当前字节/比特.
			byoffset = bitLen / 8;
			bitoffset = bitLen % 8;
			if( bitoffset == 0 )
			{
				aoutBuf[byoffset] = 0x00;
			}
			aoutBuf[byoffset] |= curbit << ( 7-bitoffset);
			bitLen++;
		}
		
		if( k == 8 )
		{
			// 完整的字节后，才有计校验
			curbit = OldParity( ainBuf[i] );
			byoffset = bitLen / 8;
			bitoffset = bitLen % 8;
			if( bitoffset == 0 )
			{
				aoutBuf[byoffset] = 0x00;
			}			
			aoutBuf[byoffset] |= curbit << ( 7-bitoffset);
			bitLen++;
		}
		i++;
		u16inBitLen -= k;
	}
	*pu16OutBitLen = bitLen;
	
	return bitLen;
}

// 接开帧
// 要调整比特序
// 不管CRC，CRC在调用该程序前，已经去掉了.
// 返回：-1表示错误				
int ISO14443A_unFraming(
				BYTE		byBitLenofFirstByte,						// 第一个字节中的bit Len，在防碰撞帧里需要
				BYTE		*byFirstByte,								// 第一字节。若u16BitLenofFirstByte=0，可以不考虑 
				BYTE 		ainBuf[], UINT16 u16inBitLen, 				// 其他字节
				BYTE 		aoutBuf[], UINT16 *pu16OutBitLen			// 输出比特不包括*byBitLenofFirstByte
			)
{
	//首先
	int i;
	int obitLen, obyoffset, obtoffset; 	// in
	int ibitLen, ibyoffset, ibtoffset;	// out
	int curbit;
	
	
	// 解码时，用户知道自己是否有第一个字节
	// 在放碰撞时，PCD根据发送，知道自己要几个

	ibitLen = 0;
	obitLen = 0;

	// 其实：u16inBitLen % 9 == byBitLenofFirstByte
	if( byBitLenofFirstByte )
	{
		*byFirstByte = 0;
		for( i = 0; i < byBitLenofFirstByte; i++ )
		{
			curbit = ( ainBuf[0] >> (7-i)) & 0x01;  
			*byFirstByte |= curbit << i;
			ibitLen++;
		}
		// 解码校验位.
		// 但不核对校验位是否正确
		curbit = ainBuf[0] >> (7-byBitLenofFirstByte);
		ibitLen++;
	}
	
	// 开始所有比特
	obitLen = 0;
	obyoffset = 0;
	obtoffset = 0;

	i = 0;
	while( ibitLen < u16inBitLen )
	{
		// 取下一bit
		ibyoffset = ibitLen / 8;
		ibtoffset = ibitLen % 8;
		curbit = ( ainBuf[ibyoffset] >> (7-ibtoffset)) & 0x01;
		
		if( (i % 9) == 8 )
		{
			// 如果有校验，看看，看该bit是否就是校验位。
			// 计算
			if( curbit != OldParity( aoutBuf[obyoffset]))
			{
				return -1;
			}
		}
		else
		{			
			// 输出.
			obyoffset = obitLen / 8;
			obtoffset = obitLen % 8;	// 
			if( obtoffset == 0 )
			{
				aoutBuf[obyoffset] = 0x00;
			}
			aoutBuf[obyoffset] |= curbit << obtoffset;
			obitLen++;
		}
		i++;
		ibitLen++;
	} 
	
	*pu16OutBitLen = obitLen;
	
	return obitLen;
}

// 前面增加：0111,后面增加：1111

/**	
 *	ISO14443A_PCD_stFrame()
 *	PCD->PICC方向，
 *
 *	返回编码后比特长度。
 */
int ISO14443A_stdFraming( BYTE aInBuf[], UINT16 u16InBitLen, BYTE aoutBuf[], UINT16 *pu16OutBitLen )
{
	// 标准帧一定带CRC校验！
	BYTE abyBuf[100];
	int k;

	k = u16InBitLen/8;
	
	memcpy( abyBuf, aInBuf, k );
	// 计算CRC校验
	ISO14443A_CRC( aInBuf, k, &abyBuf[k+1], &abyBuf[k]);
	
	k += 2;
	
	return ISO14443A_Framing( 0, 0, 	// 标准帧都是整字节开始
						abyBuf, k*8, 		
						aoutBuf, pu16OutBitLen 
					);									
}

// 短帧固定比特任长度：7，没有奇偶校验和CRC校验
int ISO14443A_ShortFraming( BYTE byCmd, BYTE aoutBuf[], UINT16 *pu16OutBitLen )
{
	int i;
	int curbit;
	byCmd = byCmd & 0x7F;
	
	aoutBuf[0] = 0x00;
	for( i = 0; i < 7; i++ )
	{
		curbit = ( byCmd >> i ) & 0x01;
		aoutBuf[0] |= curbit << (7-i);
	}
	*pu16OutBitLen = 7;
	
	return 7;
}

int ISO14443A_AnticollisionFraming( 				
				BYTE		byBitLenofFirstByte,						// 第一个字节中的bit Len，在防碰撞帧里需要
				BYTE		byFirstByte,
				BYTE 		ainBuf[], UINT16 u16InBitLen, 
				BYTE 		aoutBuf[], UINT16 *pu16OutBitLen 
		)
{
	// 防碰撞帧没有CRC.
	return ISO14443A_Framing( 
					byBitLenofFirstByte, byFirstByte, 	// 标准帧都是整正字节开始
					ainBuf, u16InBitLen, 		
					aoutBuf, pu16OutBitLen
			);	
}

// 标准帧
// 输入：比特长度，输出：字节长度！
int ISO14443A_Un_stdFraming(BYTE ainBuf[], UINT16 u16inBitLen, BYTE aoutBuf[], UINT16 *pu16OutBufLen )
{
	BYTE abyCRC[2];
	BYTE abyBuf[1000];
	int k;
	
	if( ISO14443A_unFraming( 0,0,ainBuf, u16inBitLen, abyBuf, pu16OutBufLen ) < 0 )
	{
		return -1;
	}
	
	k = (*pu16OutBufLen)/8;
	k -= 2;
	*pu16OutBufLen -= 16;

	// 计算CRC.
	ISO14443A_CRC( abyBuf, k, &abyCRC[1], &abyCRC[0] );
	if( abyBuf[k] != abyCRC[0] || abyBuf[k+1] != abyCRC[1] )
	{
		return -1;
	}
	memcpy( aoutBuf, abyBuf, k );

	return k;		// 返回字节长度
}

// 只有7比特.
int ISO14443A_Un_ShortFraming( BYTE aInBuf[], UINT16 u16InBitLen, BYTE *pbyCmd )
{
	int i;
	int curbit;
	
	*pbyCmd = 0x00;
	for( i = 0; i < 7; i++ )
	{
		curbit = ( aInBuf[0] >> i ) & 0x01;
		*pbyCmd |= curbit << (7-i);
	}
	
	return 7;
}

int ISO14443A_Un_AnticollisionFraming(
				BYTE		byBitLenofFirstByte,						// 第一个字节中的bit Len，在防碰撞帧里需要
				BYTE		*pbyFirstByte,
				BYTE 		aInBuf[], UINT16 u16inBitLen, 
				BYTE 		aoutBuf[], UINT16 *pu16OutBitLen 
		)		
{
	return ISO14443A_unFraming( byBitLenofFirstByte, pbyFirstByte, 
		aInBuf, u16inBitLen, aoutBuf, pu16OutBitLen );
}



