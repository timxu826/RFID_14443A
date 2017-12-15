/**
 *	\file	rfid_iso14443A_pcd.h	
 *
 *
 *
 */

#ifndef _RFID_ISO14443A_PCD_H_
#define _RFID_ISO14443A_PCD_H_

#include "rfid_def.h"
#include "rfid_iso14443A_cmdtx.h"
#include "rfid_iso14443A_cmdrx.h"

typedef struct ISO_14443A_PCD_tag
{	
	UCHAR	ucStatus;
	UCHAR	ucCmpBits;					// 当前已经送出去多少个bit了。
	UCHAR	ucCLn;						// 0/1/2分别表示1/2/3个UID及长.
	UCHAR	ucSEL;						// 当前SEL的值.
	UCHAR	ucSAK;						// SAK的值
	UCHAR	ucAntiBit;					// 防碰撞过程中，新增加的bit，默认2，表示不增加/没有处在防碰撞过程中。
	UINT16	u16WaitTime;					// 命令发出去后，等待次数。 
	UCHAR	aucUID[12];					// 最多10个字节
}ISO_14443A_PCD;

int PCD_ISO14443A_run( ISO_14443A_PCD *pstPCD );
int PCD_ISO14443A_process( ISO_14443A_PCD *pstPCD, UCHAR aucRxBuf[], UINT16 u16RxBufBitLen );
// 冲突模拟：
// 假设收到的比特数，少于实际收到的比特数
int PCD_ISO14443A_CollisionEmu( ISO_14443A_PCD *pstPCD,
			BYTE byBitLenofFirstByte, BYTE firstByte, 
			UCHAR aucCmdBuf[], UINT16 u16CmdBitLen );
// 发送SELECT帧
int PCD_ISO14443A_SendSELECT( ISO_14443A_PCD *pstPCD );
// 发送防碰撞帧，
// 其中新增加1比特。
int PCD_ISO14443A_SendANTICOLLISION( ISO_14443A_PCD *pstPCD );
#endif




