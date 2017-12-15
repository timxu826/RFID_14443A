/**
 *	rfid_iso14443A_codec.c
 *
 *	ISO14443编解码
 *	1. 增加/去除 SOF/EOF
 *	2. 输出Buf: bit7 先送出.
 *	3. 编码顺序：bit7,6,...0.
 */

//#include "stdafx.h"
#include "rfid_iso14443A_codec.h"

int ISO14443A_PCD_Code(
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aoutBuf[], UINT16 *pu16OutBitLen 
		)
{
	return ISO14443A_Code(
			TRUE, 
			aInBuf, u16InBitLen, aoutBuf, pu16OutBitLen 
		);
}
			
int ISO14443A_PCD_Decode( 
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aOutBuf[], UINT16 *pu16OutBitLen 
		)
{
	return ISO14443A_Decode(
			true,
			aInBuf, u16InBitLen, aOutBuf, pu16OutBitLen 
		);	
}
			
int ISO14443A_PICC_Code( 
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aoutBuf[], UINT16 *pu16OutBitLen 
		)
{
	return ISO14443A_Code(
			FALSE, 
			aInBuf, u16InBitLen, aoutBuf, pu16OutBitLen 
		);
}

int ISO14443A_PICC_Decode(
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aOutBuf[], UINT16 *pu16OutBitLen 
		)
{
	return ISO14443A_Decode(
			FALSE, 
			aInBuf, u16InBitLen, aOutBuf, pu16OutBitLen 
		);
}



// 输入：
// 编码顺序：每个字节，从最左比特，即最高比特bit7开始
// 
// 		BYTE1 			BYTE2			...BYTEn
// bit: 7,6,5,4,3,2,1,0 7,...			   7,6,5.
//
// 输入已经包括了校验，CRC等。 
// 
// 返回编码后比长度.
int ISO14443A_Code( 
			BOOL bPCD,							// PCD编码端
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aOutBuf[], UINT16 *pu16OutBitLen 
		)
{
	// 增加SOF
	int bitLen, byLen, ibitLen;
	int i, j, k;
	int prebit,curbit;
	BYTE code;
	
	// 编码SOF
	bitLen = 0;
	byLen = 0;
	aOutBuf[byLen] = 0x00;

	// SOF	
	{
		if( bPCD )
		{
			aOutBuf[byLen] = ISO14443A_PCD_SOF << 4;
		}
		else
		{
			aOutBuf[byLen] = ISO14443A_PICC_SOF << 4;
		}
		
		bitLen++;
	}
	
	// 编码每个bit.
	prebit = 0;			// 第一个比特为0, 只对PCD有效。
	i = 0;
	ibitLen = 0;
	while( ibitLen < u16InBitLen )
	{
		// 每个BYTE，从最高位开始编码.
		if( (u16InBitLen - ibitLen) >= 8 )
		{
			k = 0;
		}
		else
		{
			k = 8 - u16InBitLen % 8;
		}
		
		// 先编最左bit
		for( j = 7; j >=k ; j-- )
		{
			curbit = ( aInBuf[i] >> j ) & 0x01;
			if( bPCD )
			{
				code = RFID_Code_Miller( prebit, curbit ) & 0x0F;
			}
			else
			{
				code = RFID_Code_Manchester( curbit );
			}
			
			if( bitLen % 2 == 0 )
			{
				aOutBuf[byLen] |= code << 4;
			}
			else
			{
				aOutBuf[byLen] |= code;
				byLen++;
				aOutBuf[byLen] = 0x00;
			}
			bitLen++;
			ibitLen++;
			prebit = curbit; 
		}
		i++;
	}
	
	// 编码EOF: EOF为逻辑0.
	// EOF
	{
		if( bPCD )
		{
			curbit = 0;
			code = RFID_Code_Miller( prebit, curbit ) & 0x0F;
		}
		else
		{
			code = ISO14443A_PICC_EOF;
		}
		
		if( bitLen % 2 == 0 )
		{
			aOutBuf[byLen] |= code << 4;
		}
		else
		{
			aOutBuf[byLen] |= code;
		}
		bitLen++;	
	}
	*pu16OutBitLen = bitLen * ISO14443A_BIT_CODEC_LEN;
	
	return *pu16OutBitLen;
}

// 解码在PICC端调用.
// 去除SOF/EOF等.
// 成功，返回解码比特长度，-1失败　
int ISO14443A_Decode( 
			BOOL bPCD,							// PCD编码端
			BYTE aInBuf[], UINT16 u16InBitLen, 
			BYTE aOutBuf[], UINT16 *pu16OutBitLen )
{
	int i, k;
	int bitLen, ibitLen;
	BYTE coded;
	int byoffset, btoffset;
	
	if( u16InBitLen % 4 != 0 )
	{
		return -1;
	}
	
	// 开始解码
	i = 0;
	
	// SOF;
	ibitLen = 0;
	// SOF
	if( u16InBitLen > 4 )
	{	
		BYTE bySOF;
		bySOF = bPCD ? ISO14443A_PICC_SOF : ISO14443A_PCD_SOF;
		
		coded = ( aInBuf[i] >> 4 ) & 0x0F; 
		if( coded != bySOF )
		{
			return -1;
		}
		ibitLen++;
		u16InBitLen -= ISO14443A_BIT_CODEC_LEN;
	}
	
	bitLen = 0;
	// 最后长度-2;
	while( u16InBitLen > ISO14443A_BIT_CODEC_LEN)
	{
		if( ibitLen % 2 == 1 )
		{
			coded = aInBuf[i] & 0x0F;
			i++;
		}
		else
		{
			coded = ( aInBuf[i] >> 4 ) & 0x0F;
		}
		
		// PCD侧，解码的是Manchester.
		if( !bPCD )
		{
			k = RFID_Decode_Miller( coded );
		}
		else
		{
			k = RFID_Decode_Manchester( coded );
		}
		
		if( k == -1 )
		{
			return -1;
		}
		byoffset = bitLen / 8;
		btoffset = bitLen % 8;

		if( btoffset == 0 )
		{
			aOutBuf[byoffset] = 0x00;
		}
		aOutBuf[byoffset] |= k << ( 7 - btoffset);
		
		bitLen++;
		ibitLen++;
		u16InBitLen -= ISO14443A_BIT_CODEC_LEN;
	}

	// EOF	
	if( u16InBitLen >= 4 )
	{
		BOOL bEOFResult;
		if( ibitLen % 2 == 1)
		{
			coded = aInBuf[i] & 0x0F;
			i++;
		}
		else
		{
			coded = ( aInBuf[i] >> 4 ) & 0x0F;
		}
		
		if( bPCD )
		{
			// PCD解码方向
			bEOFResult = ( coded == ISO14443A_PICC_EOF)?TRUE:FALSE;			
		}
		else
		{
			bEOFResult = ( (coded == ISO14443A_PCD_0P0) || ( coded == ISO14443A_PCD_0P1) ) ? TRUE:FALSE;
		}		
		if( !bEOFResult )
		{
			return -1;
		} 	
		u16InBitLen -= ISO14443A_BIT_CODEC_LEN;
	}	
	*pu16OutBitLen = bitLen;
	return bitLen;
}

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
 
int RFID_Code_Miller( BYTE prebit, BYTE bitDat )
{
	bitDat &= 0x01;
	prebit &= 0x01;
	if( bitDat == 0 )
	{
		if( prebit == 0 )
		{
			return RFID_CODE_MILLER_0_p0;			// 0111
		}
		else
		{
			return RFID_CODE_MILLER_0_p1;			// 1111
		}
	}
	else
	{
		return RFID_CODE_MILLER_1;				// 1101
	}
}

/**
 * 	改进的Miller解码：返回0或者1.其他失败
 *
 *	\input	输入：BYTE codedDat: 1101 for 1, 1111/0111 for 0
 *	\ouput
 *	\return	0/1
 *			-1 ,非法输入
 */
int RFID_Decode_Miller( BYTE codedDat )
{
	codedDat &= 0x0F;
	if( codedDat == RFID_CODE_MILLER_1 )
	{
		return 1;
	}
	else if( codedDat == RFID_CODE_MILLER_0_p1 || codedDat == RFID_CODE_MILLER_0_p0 )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

/*-------------------------------------------------------------*\
 *	Manchester
 *	ISO14443A PICC->PCD & ISO15693 VICC->VCD
 *-------------------------------------------------------------*/

/**
 * 	 Manchester编码，返回4比特编码
 *
 *	\input	输入：BYTE bitDat: 0/1，当前待编码比特
 *	\ouput
 *	\return	bitDat=1, 返回：1100
 *			bitDat=0，返回：0011
 */
int RFID_Code_Manchester( BYTE bitDat )
{
	return bitDat ? RFID_CODE_MANCHESTER_1:RFID_CODE_MANCHESTER_0;
}

/**
 * 	Manchester解码：返回0或者1.其他失败
 *
 *	\input	输入：BYTE codedDat: 1100 for 1, 0011 for 0
 *	\ouput
 *	\return	0/1
 *			-1 ,非法输入
 */
int RFID_Decode_Manchester( BYTE codedDat )
{
	if( codedDat == RFID_CODE_MANCHESTER_1 )
	{
		return 1;
	}
	else if( codedDat == RFID_CODE_MANCHESTER_0 )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
