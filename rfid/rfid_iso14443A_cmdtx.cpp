/**
 *	\file	rfid_iso14443A_cmdtx.c
 *
 *	\brief	ISO14443A的命令.
 *
 */

/*-------------------------------------------------------------*\
 *	ISO14443A 
 *	1. 短帧：REQA(PCD) & WUPA(PCD)
 *	2. 标准帧：SELECT(PCD), HALT(PCD), ATQA(PICC), SAK(PICC)
 *	3. 防碰撞帧：ANTICOLLISION_req(PCD)/rsp(PICC)
 *-------------------------------------------------------------*/

#include "rfid_iso14443A_cmdtx.h"
#include "memory.h"

/*--------------------------------------------------------------*\
 * 短帧
\*---------------------------------------------------------------*/
/**
 * 	\fun	ISO14443A_REQA_req()
 *	\brief	PCD->PICC
 *
 *	\input	.
 *	\return	0成功，其他失败;
 */
int ISO14443A_REQA()
{
	/**
	 *	REQA命令： 
	 *	7bit，短帧，值：0x26.
	 */
	return ISO14443A_ShortFrame_req( ISO_14443_CMD_REQA );
}

/**
 * 	\fun	ISO14443A_WUPA_req()
 *	\brief	PCD->PICC
 *
 *	\input	.
 *	\return	0成功，其他失败;
 */
int ISO14443A_WUPA()
{
	/**
	 *	WUPA命令： 
	 *	7bit，短帧，值：0x52.
	 */
	return ISO14443A_ShortFrame_req( ISO_14443_CMD_WUPA );
}
 
// 短帧
// 做编码、成帧、发送。
// 在PCD/PICC端都相同
int ISO14443A_ShortFrame_req( BYTE byCmd )
{
	BYTE abyTxBuf[100], abyFrmBuf[100];
	UINT16 u16CodedLen,u16FrmLen;
 
	/**
	 *	1. 成帧
	 */	
	if( ISO14443A_ShortFraming( byCmd, abyFrmBuf, &u16FrmLen ) < 0 )
	{
		return -1;
	}
	
	/**
	 *	2. 编码
	 *	端帧一定有SOF和EOF
	 */
	if( ISO14443A_PCD_Code( abyFrmBuf, u16FrmLen, abyTxBuf, &u16CodedLen ) < 0 )
	{
		return -1;
	}
	
	return RFID_Reader_Tx( u16CodedLen, abyTxBuf );
}

/*--------------------------------------------------------------*\
 * 标准帧
 *---------------------------------------------------------------*/

/**
 * 	\fun	ISO14443A_SELECT()
 *	\brief	PCD->PICC
 *
 *	\input	.
 *	\return	0成功，其他失败;
 */
int ISO14443A_SELECT( BYTE sel, BYTE abyUID[] )
{
	/**
	 *	SELECT有CRC16.
	 */	
	BYTE abyCmd[7];
	int i;
	
	abyCmd[0] = sel;	// SELECT: 0x93,0x95,0x97
	abyCmd[1] = 0x70;	// SELECT
	abyCmd[6] = 0x00;
	for( i = 0; i < 4; i++ )
	{
		abyCmd[i+2] = abyUID[i];
		abyCmd[6] = abyCmd[6] ^ abyUID[i];
	}
	
	return ISO14443A_stdFrame_req( abyCmd, 56 );
}


/**
 * 	\fun	ISO14443A_HALT()
 *	\brief	PCD->PICC
 *
 *	\input	.
 *	\return	0成功，其他失败;
 */
int ISO14443A_HALT()
{
	BYTE abyCmd[]={0x50, 0x00};
	return ISO14443A_stdFrame_req( abyCmd, 16 );	
}

/**
 * 	\fun	ISO14443A_ATQA()
 *	\brief	PICC->PCD
 *			对REQA/WUKA的应答
 *			标准帧.
 *	\input	
 *	\return	0成功，其他失败;
 */
int ISO14443A_ATQA( BYTE byCLn )
{
	BYTE abyCmd[2];
	
	byCLn = byCLn & 0x03;		// 最低3位有效
	abyCmd[1] = (byCLn << 6) + 0x01;
	abyCmd[0] = 0x00;

	
	return ISO14443A_stdFrame_rsp( abyCmd, 16 );	
}

/**
 * 	\fun	ISO14443A_SAK()
 *	\brief	PICC->PCD
 *			对SELECT/ANTICOLLISION的应答
 *
 *	\input	cascade: 0/1.
 *	\return	0成功，其他失败;
 */
int ISO14443A_SAK( BYTE cascade )
{
	return ISO14443A_stdFrame_rsp( &cascade, 8 );
}


// 标准帧，不考虑是否有CRC，但考虑校验
// PCD使用
int ISO14443A_stdFrame_req( BYTE abyBuf[], UINT16 u16BitLen )
{
	BYTE abyCodeBuf[1024], abyFrmBuf[1024];
	UINT16 u16CodedLen,u16FrmLen;
	
	/**
	 *	1. 成帧
	 */	
	if( ISO14443A_stdFraming( abyBuf, u16BitLen, abyFrmBuf, &u16FrmLen ) < 0 )
	{
		return -1;
	}

	/**
	 *	2. 编码
	 *	标准帧一定有SOF和EOF
	 */		
	if( ISO14443A_PCD_Code( abyFrmBuf, u16FrmLen, abyCodeBuf, &u16CodedLen ) < 0 )
	{
		return -1;
	}
	
	/**
	 *	3. 发送
	 */
	return RFID_Reader_Tx( u16CodedLen, abyCodeBuf );
}

// 标准帧，不考虑是否有CRC，但考虑校验
// PICC端使用
int ISO14443A_stdFrame_rsp( BYTE abyBuf[], UINT16 u16BitLen )
{
	BYTE abyCodeBuf[1024], abyFrmBuf[1024];
	UINT16 u16CodedLen,u16FrmLen;
	
	/**
	 *	1. 成帧
	 */	
	if( ISO14443A_stdFraming( abyBuf, u16BitLen, abyFrmBuf, &u16FrmLen ) < 0 )
	{
		return -1;
	}

	/**
	 *	2. 编码
	 *	标准帧一定有SOF和EOF
	 */		
	if( ISO14443A_PICC_Code( abyFrmBuf, u16FrmLen, abyCodeBuf, &u16CodedLen ) < 0 )
	{
		return -1;
	}
	
	/**
	 *	3. 发送
	 */
	return RFID_Card_Tx( u16CodedLen, abyCodeBuf );
}
/*--------------------------------------------------------------*\
 * 防碰撞帧
 *---------------------------------------------------------------*/
 
/**
 *	防碰撞帧，在PCD和PICC端不同
 */
int ISO14443A_AnticollisionFrame_req( 
	BYTE bySEL, BYTE abyUID[], UINT16 u16BitLen )
{
	BYTE abyCmd[7];
	BYTE abyCodeBuf[1024], abyFrmBuf[1024];
	UINT16 u16CodedLen,u16FrmLen, u16CmdBitLen;
	int m,n,k,i;

	abyCmd[0] = bySEL;
	
	n = u16BitLen % 8;
	k = u16BitLen/8 + ((n != 0 )? 1:0);
	m = 2 + k;
	abyCmd[1] = ((m & 0x0F) << 4 )+(n & 0x0F);
	
	for( i = 0; i < k; i++ )
	{
		abyCmd[i+2] = abyUID[i];
	}
	u16CmdBitLen = 16 + u16BitLen;
	
	/**
	 *	1. 成帧
	 */	
	if( ISO14443A_AnticollisionFraming( 0, 0, abyCmd, u16CmdBitLen, abyFrmBuf, &u16FrmLen ) < 0 )
	{
		return -1;
	}

	/**
	 *	2. 编码
	 *	防碰撞帧一定有SOF，但没有EOF
	 */		
	if( ISO14443A_PCD_Code( abyFrmBuf, u16FrmLen, abyCodeBuf, &u16CodedLen ) < 0 )
	{
		return -1;
	}
	
	/**
	 *	3. 发送
	 */
	return RFID_Reader_Tx( u16CodedLen, abyCodeBuf );
	
	return 0;	 
}

// PICC发出，一定是RSP.
// BCC要求上层计算出来。
int ISO14443A_AnticollisionFrame_rsp( 
				BYTE		byBitLenofFirstByte,		// 第一个字节中的bit Len，在防碰撞帧里需要
				BYTE		byFirstByte,
				BYTE 		abyUID[], UINT16 u16UIDBitLen, 
				BYTE		byBCC						
		)
{
	BYTE abyCmd[7];
	BYTE abyCodeBuf[1024], abyFrmBuf[1024];
	UINT16 u16CodedLen,u16FrmLen;
	int k;
	
	/**
	 *	1. 成帧
	 */	
	k = u16UIDBitLen / 8;
	memcpy( abyCmd, abyUID, k );
	abyCmd[k] = byBCC;
	
	if( ISO14443A_AnticollisionFraming( byBitLenofFirstByte, byFirstByte, 
					abyCmd, (k+1)*8, abyFrmBuf, &u16FrmLen ) < 0 )
	{
		return -1;
	}

	/**
	 *	2. 编码
	 *	标准帧一定有SOF和EOF
	 */		
	if( ISO14443A_PICC_Code( abyFrmBuf, u16FrmLen, abyCodeBuf, &u16CodedLen ) < 0 )
	{
		return -1;
	}
	
	/**
	 *	3. 发送
	 */
	return RFID_Card_Tx( u16CodedLen, abyCodeBuf );
}
 

