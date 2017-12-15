/**
 *	\file	rfid_def.h
 *
 *	\brief	头文件定义
 */
#ifndef _RFID_DEF_H_
#define _RFID_DEF_H_

//#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <memory.h>

//#define _RFID_DEBUG_

//#define _OS_WINDOWS_

#ifdef _OS_WINDOWS_
	#define _RFID_COM_FILE_
	#define PCD_WAIT_TIME	25
#else
	#define PCD_WAIT_TIME	1
#endif

typedef	unsigned char BYTE;
typedef unsigned char UCHAR;
typedef unsigned short UINT16;
typedef unsigned long ULONG;
typedef unsigned long UINT32;
typedef unsigned int  BOOL;
#define TRUE	1
#define FALSE	0

/**
 *	系统最多仿10个 PICC.
 */

#define MAX_PICC_NUM				10

/*-----------------------------------------------------------------*\
 *	定义ISO14443A命令和状态
 *
 *
 *-----------------------------------------------------------------*/
// 实际上不存在
#define	ISO_14443A_STATE_POWEROFF	0xA0			
// 系统启动，就处于IDLE状态，相当于已经加电.		
#define	ISO_14443A_STATE_IDLE		0xA1
#define	ISO_14443A_STATE_READY		0xA2
#define	ISO_14443A_STATE_ACTIVE		0xA3
#define	ISO_14443A_STATE_HALT		0xA4

// 定义PCD的状态机
#define ISO_14443A_PCD_STATE_OK			0xA8		// PCD识别出一张卡
#define ISO_14443A_PCD_STATE_ER			0xA9		// PCD识别卡过程中，错误

#define ISO_14443A_PCD_STATE_START		0xAA		// 开始。如果没有收到，继续发送REQA
#define ISO_14443A_PCD_STATE_ANTI		0xAB		// 收到ATQA，转入READY.开始防冲突循环?
#define ISO_14443A_PCD_STATE_SELECT		0xAC		// 收到SAK.


#define ISO_14443_CMD_REQA			0x26
#define ISO_14443_CMD_WUPA			0x52
#define ISO_14443_CMD_SEL_CL1		0x93
#define ISO_14443_CMD_SEL_CL2		0x95
#define ISO_14443_CMD_SEL_CL3		0x97
#define ISO_14443_CMD_HALT			0x50

#define ISO_14443_PCD				0xF0
#define ISO_14443_PICC				0xF1

int codec_main();
int framing_main();
int cmd_main();
int picc_main( int argc, char *argv[] );
int pcd_main( int numofpicc );

#endif

