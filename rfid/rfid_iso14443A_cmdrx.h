/**
 *	\file	rfid_iso14443A_cmdrx.h
 *
 *	\brief	ISO14443A的命令.
 *
 *			分别考虑PCD/PICC接收方向获得的数据是哪个命令
 *			
 *			不同状态下，要等待的命令不同，所以考虑等待哪个命令，如果不是这个命令，则丢弃
 */
 
#ifndef _RFID_ISO14443A_CMDRX_H_
#define _RFID_ISO14443A_CMDRX_H_

#include "rfid_def.h"
#include "rfid_iso14443A_codec.h"
#include "rfid_iso14443A_frm.h"
#include "rfid_com.h"

//PCD
int ISO14443A_wait_ATQA( BYTE abyRxBuf[], UINT16 u16BufBitLen, BYTE *pByCLn );
int ISO14443A_wait_SAK( BYTE abyRxBuf[], UINT16 u16BufBitLen, BYTE *pbyCmd );
int ISO14443A_wait_anticollisionFrame_rsp( 
		BYTE byFirstByteLen, BYTE *pFirstByte, 
		BYTE abyRxBuf[], UINT16 u16BufBitLen, 
		BYTE abyCmdBuf[], UINT16 *pu16CmdBitLen );
		
//PICC
int ISO14443A_wait_REQA( BYTE abyRxBuf[], UINT16 u16BufBitLen );
int ISO14443A_wait_WUPA( BYTE abyRxBuf[], UINT16 u16BufBitLen );
int ISO14443A_wait_SELECT( BYTE abyRxBuf[], UINT16 u16BufBitLen, BYTE *psel, BYTE abyUID[] );
int ISO14443A_wait_HALT( BYTE abyRxBuf[], UINT16 u16BufBitLen );
int ISO14443A_wait_anticollisionFrame_req( 
		BYTE abyRxBuf[], UINT16 u16BufBitLen, 
		BYTE abyCmdBuf[], UINT16 *pu16CmdBitLen );

// 基本命令
int ISO14443A_wait_ShortFrame( BYTE abyRxBuf[], UINT16 u16BufBitLen, BYTE *pbyCmd );
int ISO14443A_wait_stdFrame( BOOL bPCD, UINT16 u16HopeCmdLen, 
		BYTE abyRxBuf[], UINT16 u16BufBitLen, 
		BYTE abyCmdBuf[], UINT16 *pu16CmdBitLen );
int ISO14443A_wait_anticollisionFrame( 
		BOOL bPCD,
		BYTE byFirstByteLen, BYTE *pFirstByte, 
		BYTE abyRxBuf[], UINT16 u16BufBitLen, 
		BYTE abyCmdBuf[], UINT16 *pu16CmdBitLen );
		
#endif
