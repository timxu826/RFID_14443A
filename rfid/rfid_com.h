/**
 *	\file	RFID_com.h
 *
 *	\brief	负责RFID系统中，卡/读卡器方向的收发.
 *
 */
#ifndef _RFID_COM_H_
#define _RFID_COM_H_

#include "rfid_def.h"

void RFID_COM_Init( int pcd_or_picc );

/**
 *	发送数据，立即返回
 */
int RFID_Card_Tx( UINT16 u16BitLen, UCHAR *pBuf );

/**
 *	非阻塞操作.
 *
 *	返回值定义为：
 *		>0，实际收到数据的比特长度;
 *		=0, 没有收到数据;
 */
int RFID_Card_Rx( UINT16 u16ByteLen, UCHAR *pBuf );

/**
 *	发送数据，立即返回
 */
int RFID_Reader_Tx( UINT16 u16BitLen, UCHAR *pBuf );

/**
 *	非阻塞操作.
 *
 *	返回值定义为：
 *		>0，实际收到数据的比特长度;
 *		=0, 没有收到数据;
 */
int RFID_Reader_Rx( UINT16 u16ByteLen, UCHAR *pBuf );

#endif

 