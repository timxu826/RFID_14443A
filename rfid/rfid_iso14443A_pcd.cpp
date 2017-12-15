/**
 *	\file	rfid_iso14443A_pcd.cpp
 *
 *	
 *			读卡器部分
 */

//#include "stdafx.h"
#include <stdlib.h>

#include "rfid_def.h"
#include "memory.h"
#include "rfid_iso14443A_cmdtx.h"
#include "rfid_iso14443A_cmdrx.h"

#include "rfid_iso14443A_pcd.h"

void PCD_ISO14443A_SetSELECT( ISO_14443A_PCD *pstPCD, BYTE cmd );
FILE *g_fpDebug = NULL;

int main( int argc, char* argv[] )
{
	return pcd_main( 10 );
}

int pcd_main( int numofpicc )
{
	ISO_14443A_PCD stPCD;
	int i, k;
	
	RFID_COM_Init( ISO_14443_PCD );
	g_fpDebug = fopen("d:\\rfid\\pcddebug.txt", "w");

	k = 0;
	while( k < numofpicc )
	{
		PCD_ISO14443A_run( &stPCD );
		if( stPCD.ucStatus == ISO_14443A_PCD_STATE_OK )
		{
			FILE *fp = fopen("d:\\rfid\\picc.txt", "a+");
			if( !fp )
			{
				fp = fopen( "d:\\rfid\\picc.txt", "w" );
			}
			if( k == 0 )
			{
				fprintf(fp, "\n--------------new test. " );
			}
			printf("\nUID[%d]: ", ++k );
			fprintf(fp, "\nUID[%d]: ", k );
			for( i = 0; i < stPCD.ucCmpBits/8; i++ )
			{
				if( stPCD.aucUID[i] != 0x88 )
				{
					printf("0x%02X.", stPCD.aucUID[i]);
					fprintf(fp, "0x%02X.", stPCD.aucUID[i]);
				}
			}
			fclose( fp );
		}
	}
	fclose( g_fpDebug ); 
	return 1;
}

// 一次只识别出一张卡
int PCD_ISO14443A_run( ISO_14443A_PCD *pstPCD )
{
	UCHAR 	aucRxBuf[256];
	UINT16 	u16BufLen = 256;
	int 	len, r, k;
	
	pstPCD->ucStatus 	= ISO_14443A_PCD_STATE_START;
	pstPCD->ucCmpBits	= 0;
	pstPCD->ucCLn		= 3;			// 没有收到CLn: 0,1,2.
	pstPCD->ucAntiBit	= 2;			// 防冲突过程中，先发送0
	pstPCD->u16WaitTime  = PCD_WAIT_TIME;			// 最大等待时间。

	memset( pstPCD->aucUID, 0x00, 12 );
	len = 0;
	while( ( pstPCD->ucStatus != ISO_14443A_PCD_STATE_ER ) 
		&& ( pstPCD->ucStatus != ISO_14443A_PCD_STATE_OK ) )
	{
		// 接收数据并解析
		
		// len可能为0.
		r = PCD_ISO14443A_process( pstPCD, aucRxBuf, len );
		if( r < 0 )
		{
			break;
		}
		
		if( pstPCD->ucStatus == ISO_14443A_PCD_STATE_OK )
		{
			// 结束了！
			break;
		}

		// 等待
		k = 10;
		while(k--)
		{
			len = RFID_Reader_Rx( u16BufLen, aucRxBuf );
			if( len > 0 )
			{
				break;
			}
		}
	}	
	
	return r;
}

// cmd: sel = 0, anti = 1;
void PCD_ISO14443A_SetSELECT( ISO_14443A_PCD *pstPCD, BYTE cmd )
{
	if( pstPCD->ucCmpBits < 32 )
	{
		pstPCD->ucSEL = ISO_14443_CMD_SEL_CL1;
	}
	else if( pstPCD->ucCmpBits == 32 )
	{
		pstPCD->ucSEL = ( cmd == 0 ) ? ISO_14443_CMD_SEL_CL1:ISO_14443_CMD_SEL_CL2;
	}
	else if( pstPCD->ucCmpBits < 64 )
	{
		pstPCD->ucSEL = ISO_14443_CMD_SEL_CL2;
	} 
	else if( pstPCD->ucCmpBits == 64 )
	{
		pstPCD->ucSEL = ( cmd == 0 ) ? ISO_14443_CMD_SEL_CL2:ISO_14443_CMD_SEL_CL3;
	}
	else
	{
		pstPCD->ucSEL = ISO_14443_CMD_SEL_CL3;
	}
}

// 冲突模拟：
// 假设收到的比特数，少于实际收到的比特数
int PCD_ISO14443A_CollisionEmu( ISO_14443A_PCD *pstPCD,
			BYTE byBitLenofFirstByte, BYTE firstByte, 
			UCHAR aucCmdBuf[], UINT16 u16CmdBitLen )
{
	BYTE	bcc;
	int i, j, k, m, n;

	BYTE abyRxUID[4]={0x00,0x00,0x00,0x00};				// 4个够了！
	int  rxUIDbitLen;

	int ibyoffset, ibtoffset;
	int obyoffset, obtoffset;
	int curbit, bitlen;

	// 冲突模拟: 假设PCD只收到了随机个bits.
	// 实际收到的UID长度
	bitlen = byBitLenofFirstByte + u16CmdBitLen - 8;		// 
						
	// 可以是0个。0表示全部发生了碰撞.
	// 产生随机数.
	if( bitlen > 1 )
	{
		rxUIDbitLen = rand() % bitlen;
	}
	else
	{
		rxUIDbitLen = 1;
	}
	// 把接收到的bitlen个比特的数据，复制rxUIDbitLen个到abyRxUID[]中
	//

	// abyRxUID的字节偏移和bit便宜
	obyoffset = 0;
	obtoffset = 0;
	// 处理FirstByte.
	n = 0;			// 表示已经处理的bit数.
	if( byBitLenofFirstByte > 0 )
	{
		// 有部分bit.
		if( byBitLenofFirstByte <= rxUIDbitLen )
		{
			// 全部FirstByte.
			n = byBitLenofFirstByte;
		}
		else
		{
			// 部分FirstByte。最高部分要drop.
			n = rxUIDbitLen;
		}

		// 处理n个比特：firstByte的最低n个比特.
		i = n - 1;
		while( i >= 0 )
		{
			curbit = (firstByte >> i)&0x01;
			abyRxUID[obyoffset] |= curbit << ( 7 - obtoffset );
			i--;
			obtoffset++;
		}
	}

	// 继续: 处理后续字节.
	// 
	if( rxUIDbitLen > byBitLenofFirstByte )
	{
		// 把aucCmdBuf的最后一个字节往左移动8-k个比特就好.
		k = 8-((rxUIDbitLen - byBitLenofFirstByte)%8);	//

		// 最后一个字节.
		m = (rxUIDbitLen - byBitLenofFirstByte)/8;	// 最后一个字节：BCC.
		aucCmdBuf[m] = aucCmdBuf[m] << k;

		j = n;
		i = 0;
		while( j < rxUIDbitLen )
		{						
			ibyoffset = i / 8;
			ibtoffset = i % 8;
			curbit = ( aucCmdBuf[ibyoffset] >> ( 7-ibtoffset)) & 0x01;

			obyoffset = j / 8;
			obtoffset = j % 8;
			abyRxUID[obyoffset] |= curbit << ( 7 - obtoffset );
			i++;
			j++;
		}
		// 处理最后一个字节.
	}

	// 数据已经保存在abyRxUID[]内，共rxUIDbitLen个比特。
	
	// 下一步把数据拼接在UID内。
	bitlen = rxUIDbitLen;		// bitlen表示还需要处理的比特数.
	i = 0;						// 当前处理的
	if( bitlen > 0 )
	{
		// 先补上次的缺！
		if( pstPCD->ucCmpBits % 8)
		{
			// 有缺口: 
			k = byBitLenofFirstByte;		// 需要k个比特，凑成一个字节
			if( k > rxUIDbitLen )
			{
				k = rxUIDbitLen;			// 补不够，最多补k个.
			}
						 
			// 补k个！
			obyoffset = pstPCD->ucCmpBits/8;
			obtoffset = pstPCD->ucCmpBits%8;
			j = obtoffset + k - 1;
			ibyoffset = 0;
			ibtoffset = 0;

			while( j >= obtoffset )
			{
				curbit = ( abyRxUID[ibyoffset] >> ( 7-ibtoffset)) & 0x01;

				pstPCD->aucUID[obyoffset] |= curbit << j;
				ibtoffset++;
				j--;
			}
			bitlen -= k;
			i += k;
			pstPCD->ucCmpBits += k;
		}
	}

	// 开始，连续k个字节
	if( bitlen > 0 )
	{
		// 处理整数个bit.
		n = ( bitlen / 8 )*8;

		j = pstPCD->ucCmpBits;
		k = n;	// 处理k个.
		while( k-- )
		{
			ibyoffset = i / 8;
			ibtoffset = i % 8;
			curbit = ( abyRxUID[ibyoffset] >> ( 7-ibtoffset)) & 0x01;

			obyoffset = j / 8;
			obtoffset = j % 8;
			pstPCD->aucUID[obyoffset] |= curbit << ( 7 - obtoffset );
			i++;
			j++;
		}
		bitlen -= n;
		pstPCD->ucCmpBits += n;
	}

	// 如果还有几个剩余bit，则接到最低比特位置.
	if( bitlen > 0 )
	{
		// UID一定位于整字节界限上.
		obyoffset = pstPCD->ucCmpBits / 8;
		obtoffset = 0;

		pstPCD->ucCmpBits += bitlen;
		i = rxUIDbitLen-1;	// 从最后开始.
		while( bitlen-- )
		{
			ibyoffset = i / 8;
			ibtoffset = i % 8;
			curbit = ( abyRxUID[ibyoffset] >> ( 7-ibtoffset)) & 0x01;

			pstPCD->aucUID[obyoffset] |= curbit << obtoffset;
			obtoffset++;
			i--;
		}
	}


	// 如果正确接收到了32比特，则查看BCC是否正确
	if( (pstPCD->ucCmpBits != 0) && (pstPCD->ucCmpBits % 32 == 0) )
	{
		bcc = 0x00;
		k = pstPCD->ucCmpBits / 8 - 4; 
		for( i = k; i < 4+k; i++ )
		{
			bcc = bcc ^ pstPCD->aucUID[i];
		}
		if( bcc != aucCmdBuf[u16CmdBitLen/8-1] )
		{
			return -1;
		}
	}
	return 1;
}

int PCD_ISO14443A_SendSELECT( ISO_14443A_PCD *pstPCD )
{
	int r;
	int i, byoffset, btoffset, k;

	PCD_ISO14443A_SetSELECT( pstPCD, 0 );
	// 发送SELECT.
	r = (pstPCD->ucCmpBits - 32 )/8;
	r = ISO14443A_SELECT( pstPCD->ucSEL, &(pstPCD->aucUID[r]) );
	pstPCD->ucStatus = ISO_14443A_PCD_STATE_SELECT;
	printf("\n\nTx SELECT, waiting SAK");
	fprintf(g_fpDebug, "\n\nTx SELECT, waiting SAK");

	printf("\nUID[0x%02x]:", pstPCD->ucSEL);
	fprintf(g_fpDebug, "\nUID[0x%02x]:", pstPCD->ucSEL);
	k = (pstPCD->ucSEL-0x93)/2;
	for( i = k*32; i < pstPCD->ucCmpBits; i++ )
	{
		byoffset = i/8;
		btoffset = i%8;
		printf("%d", ( pstPCD->aucUID[byoffset] >> btoffset)&0x01);
		fprintf(g_fpDebug, "%d", (pstPCD->aucUID[byoffset] >> btoffset)&0x01);
		if( i%4 == 3 )
		{
			printf(".");
			fprintf(g_fpDebug,".");
		}
	}
	printf("\nwait[0x%02x]", pstPCD->ucSEL);
	fprintf(g_fpDebug, "\nwait[0x%02x]", pstPCD->ucSEL);
	return r;
}

// 进入该函数，一定没有收到所有的比特：
// newbit = 0/1. 第一次取0.
int PCD_ISO14443A_SendANTICOLLISION( ISO_14443A_PCD *pstPCD )
{
	int r;
	int k;
	int i, byoffset, btoffset;

	// 增加1 bit. 因为第一次取0，因此取1时，一定是取0没有返回。
	// 这样：取１时，一定是可以返回的，再没有返回，就结束了。

	if( pstPCD->ucAntiBit == 0 )
	{
		pstPCD->ucCmpBits ++;		// 第一次加1个比特;
	}

	// 把ucCmpBits设置为newbit.
	byoffset = (pstPCD->ucCmpBits-1) / 8;
	btoffset = (pstPCD->ucCmpBits-1) % 8;

	pstPCD->aucUID[byoffset] &= ~(1 << btoffset);		// 设置为0.
	pstPCD->aucUID[byoffset] |= pstPCD->ucAntiBit << btoffset;

	// 继续发送UID等.
	if( pstPCD->ucCmpBits == 32 || pstPCD->ucCmpBits == 64 || pstPCD->ucCmpBits == 96 )
	{
		// 增加一个比特后，成了32bit，此时需要发送SELECT.
		r = PCD_ISO14443A_SendSELECT( pstPCD );
	}
	else
	{
		// 否则发送anticollision.
		PCD_ISO14443A_SetSELECT( pstPCD, 1 );
		k = ( pstPCD->ucCmpBits / 32 ) * 4;

		r = ISO14443A_AnticollisionFrame_req( pstPCD->ucSEL, &(pstPCD->aucUID[k]), pstPCD->ucCmpBits % 32 );
		printf("\n\nTx anticollision(new bit=%d), waiting anticollision.", pstPCD->ucAntiBit);
		fprintf( g_fpDebug, "\n\nTx anticollision(new bit=%d), waiting anticollision.", pstPCD->ucAntiBit );

		printf("\nUID[0x%02x]:", pstPCD->ucSEL);
		fprintf(g_fpDebug, "\nUID[0x%02x]: ", pstPCD->ucSEL);
		k = (pstPCD->ucSEL-0x93)/2;
		for( i = k * 32; i < pstPCD->ucCmpBits; i++ )
		{
			byoffset = i/8;
			btoffset = i%8;
			printf("%d", ( pstPCD->aucUID[byoffset] >> btoffset)&0x01);
			fprintf(g_fpDebug, "%d", (pstPCD->aucUID[byoffset] >> btoffset)&0x01);
			if( i % 4 == 3 )
			{
				printf(".");
				fprintf(g_fpDebug, ".");
			}
		}
		printf("\nwait[0x%02x]", pstPCD->ucSEL );
		fprintf(g_fpDebug, "\nwait[0x%02x]", pstPCD->ucSEL);
	}
	return r;
}

// 状态机
int PCD_ISO14443A_process( ISO_14443A_PCD *pstPCD, UCHAR aucRxBuf[], UINT16 u16RxBufBitLen )
{
	int r = 1;
	
	switch( pstPCD->ucStatus )
	{
		case ISO_14443A_PCD_STATE_START:
			// 发送REQA，收到ATRQ后，转为
			if( pstPCD->ucCLn == 3 )
			{
				// 发送REQA().
				ISO14443A_REQA();
				printf("\n\nTx REQA. waiting ATQA.");
				fprintf(g_fpDebug, "\n\nTx REQA. waiting ATQA.");
				pstPCD->u16WaitTime = PCD_WAIT_TIME;
				pstPCD->ucCLn = 0;
			}
			else
			{
				// 接收
				BYTE CL;
				r = ISO14443A_wait_ATQA( aucRxBuf, u16RxBufBitLen, &CL );
				if( r > 0 )
				{
					pstPCD->ucStatus = ISO_14443A_PCD_STATE_ANTI;
					pstPCD->ucCLn 	 = CL;		// 0.1.2
					pstPCD->ucSEL	= ISO_14443_CMD_SEL_CL1;
					pstPCD->ucCmpBits= 0;
					pstPCD->ucAntiBit = 2;			// 刚开始。
					pstPCD->u16WaitTime = PCD_WAIT_TIME;
					
					r = ISO14443A_AnticollisionFrame_req( ISO_14443_CMD_SEL_CL1, NULL, 0 );

					printf("\nRx ATQA.");
					fprintf(g_fpDebug, "\nRx ATQA.");
					printf("\n\nTx SELECT(anticollision), waiting anticollision.\nwait");
					fprintf( g_fpDebug,"\n\nTx SELECT(anticollision), waiting anticollision.\nwait"); 
				}
			}
			break;
		case ISO_14443A_PCD_STATE_ANTI:
			// 执行防冲突循环. 
			if( u16RxBufBitLen == 0 )
			{
				if( pstPCD->ucAntiBit == 2 || pstPCD->ucAntiBit == 1 )
				{
					if( pstPCD->u16WaitTime )
					{
						pstPCD->u16WaitTime--;
#ifdef _RFID_DEBUG_
						printf("\nwaitTime = %d", pstPCD->u16WaitTime );
#else
						printf(".");
#endif
						return 0;
					}
					else
					{
#ifdef _RFID_DEBUG_
						printf("\nwaitTime = %d. not wait.", pstPCD->u16WaitTime );
#endif
						printf("\nTimeout. not wait." );
						return -1;		// 不等了。
					}
				}
				else
				{
					// 说明发送过比特为０了.
					// 等待。
					if( pstPCD->u16WaitTime == 1 )
					{
						// 不等了，换newbit = 1;
						pstPCD->ucAntiBit = 1;
						pstPCD->u16WaitTime = PCD_WAIT_TIME;
 						return PCD_ISO14443A_SendANTICOLLISION( pstPCD );
					}
					else
					{
						// 等待bit = 0的那个
						pstPCD->u16WaitTime--;
#ifdef _RFID_DEBUG_
						printf("\nwaitTime = %d", pstPCD->u16WaitTime );
#else
						printf(".");
#endif
						return 0;
					}
				}
			}			
			else
			{
				BYTE firstByte;
				BYTE byBitLenofFirstByte;
				BYTE aucCmdBuf[100];
				UINT16 u16CmdBitLen;
				
				if( pstPCD->ucCmpBits%8 )
				{
					byBitLenofFirstByte = 8 - pstPCD->ucCmpBits%8;
				}
				else
				{
					byBitLenofFirstByte = 0;
				}
				
				r = ISO14443A_wait_anticollisionFrame_rsp( byBitLenofFirstByte, &firstByte,  
						aucRxBuf, u16RxBufBitLen,
						aucCmdBuf, &u16CmdBitLen
						);
				if( r < 0 )
				{
					pstPCD->ucStatus = ISO_14443A_PCD_STATE_ER;
					return r;
				}
				// 打印收到的UID

				printf("\nRx UID: ");
				fprintf(g_fpDebug, "\nRx UID:");
				if( byBitLenofFirstByte )
				{
					int i, k;
					for( i = 0; i < byBitLenofFirstByte; i++ )
					{
						k = ( firstByte >> i ) & 0x01 ;
						printf("%d", k);
						fprintf(g_fpDebug, "%d", k);
						if( i == byBitLenofFirstByte - 5 )
						{
							printf(".");
							fprintf(g_fpDebug, ".");
						}
					}
					printf(".");
					fprintf(g_fpDebug, ".");
				}
				
				if( u16CmdBitLen-8 > 0 )
				{
					int i,k;
					int btoffset, byoffset;

					for( i = 0; i < u16CmdBitLen-8; i++ )
					{
						byoffset = i/8;
						btoffset = i%8;
						k = ( aucCmdBuf[byoffset] >> btoffset ) & 0x01;
						printf("%d", k);
						fprintf(g_fpDebug, "%d", k);
						if( i % 4 == 3 )
						{
							printf(".");
							fprintf(g_fpDebug, ".");
						}
					}
				}
				
				// 冲突模仿.
				PCD_ISO14443A_CollisionEmu( pstPCD, byBitLenofFirstByte, firstByte, aucCmdBuf, u16CmdBitLen);

				// 以下是不考虑冲突的情况.
				// 
				//int k = pstPCD->ucCmpBits / 8;
				//BYTE mask[] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};

				//if( byBitLenofFirstByte != 0 )
				//{
				//	// 前比特。
				//	pstPCD->aucUID[k] &= mask[byBitLenofFirstByte-1];
				//	firstByte		  &= ~mask[byBitLenofFirstByte-1];
				//	pstPCD->aucUID[k] |= firstByte;
				//	k++;				// 后一个字节.
				//	pstPCD->ucCmpBits += byBitLenofFirstByte;
				//}

				//j = 0;
				//for( i = k; i < 4+k; i++ )
				//{
				//	pstPCD->aucUID[i] = aucCmdBuf[j++];
				//	pstPCD->ucCmpBits += 8;
				//}

				printf("\nRx anticollision.");
				fprintf( g_fpDebug, "\nRx anticollision.");

				if( pstPCD->ucCmpBits == 32 || pstPCD->ucCmpBits == 64 || pstPCD->ucCmpBits == 96 )
				{
					// 清晰收到了所有的UID.
					pstPCD->ucAntiBit = 2;			// 正常.
					r = PCD_ISO14443A_SendSELECT(pstPCD);
				}
				else
				{
					pstPCD->ucAntiBit = 0;			// 正常.
					r = PCD_ISO14443A_SendANTICOLLISION(pstPCD );
				}
				pstPCD->u16WaitTime = PCD_WAIT_TIME;
			}
			// 发送SELECT后，转为ACTIVE
			break;
		case ISO_14443A_PCD_STATE_SELECT:
			/**
			 * 有两种情况：
			 *	1. 防冲突循环中，正常接收到了32bits的UID，此时发送SELECT后，再改状态等待。
			 *  2. 防冲突循环中，接收到的比特小于31bits，PCD新增加比特设置为 0，然后发送此时：
			 *		2.1可能没有回应，
			 *			2.1.1 若此时等待 ucAntiBit 次后，把新增加比特设置为1，发送，然后一定可以等到；
			 *			2.1.2 若继续没有回应，则认为结束。
			 *		2.2有回应，则处理。
			 */
			if( u16RxBufBitLen == 0 )
			{
				// 没有回应。
				if( pstPCD->ucAntiBit == 2 || pstPCD->ucAntiBit == 1 )
				{
					// 正常的SELECT　frame
					if( pstPCD->u16WaitTime )
					{
						pstPCD->u16WaitTime--;
						return 0;
					}
					else
					{
						// 等够了
						return -1;
					}
				}
				else
				{
					// 第一次尝试，新增加比特为0.pstPCD->ucAntiBit == 0
					// 正常的SELECT　frame
					if( pstPCD->u16WaitTime == 1)
					{
						// 不等了，换newbit = 1;
						pstPCD->ucAntiBit = 1;
						pstPCD->u16WaitTime = PCD_WAIT_TIME;
 						return PCD_ISO14443A_SendANTICOLLISION( pstPCD );
					}
					else if( pstPCD->u16WaitTime )
					{
						pstPCD->u16WaitTime--;
#ifdef _RFID_DEBUG_
						printf("\nwaitTime = %d.", pstPCD->u16WaitTime );
#endif
						return 0;
					}
				}
			}

			// 收到SAK.
			// 等待接收SAK
			r = ISO14443A_wait_SAK( aucRxBuf, u16RxBufBitLen, &pstPCD->ucSAK );
			if( r < 0 )
			{
				return r;
			}
			printf("\nRx SAK[0x%02x].", pstPCD->ucSAK);
			fprintf( g_fpDebug, "\nRx SAK[0x%02x].", pstPCD->ucSAK);
				
				
			// 是否还有UID.
			if( ( pstPCD->ucSAK >> 2 ) & 0x01 )
			{
				// 还有UID.
				pstPCD->ucStatus = ISO_14443A_PCD_STATE_ANTI;
				
				// 继续发送ANTI.
				pstPCD->ucSEL = ( pstPCD->ucCmpBits == 32 )?ISO_14443_CMD_SEL_CL2:ISO_14443_CMD_SEL_CL3;
				
				r = ISO14443A_AnticollisionFrame_req( pstPCD->ucSEL, NULL, 0 );
				if( r < 0 )
				{
					return r;
				}
				printf("\n\nTx SELECT(anticollision), waiting anticollision.\nwait");
				fprintf( g_fpDebug, "\n\nTx SELECT(anticollision), waiting anticollision.\nwait");
			}
			else
			{
				// bit3=0;
				// 若结束，则转为END
				// 发送HALT.
				ISO14443A_HALT();
				pstPCD->ucStatus = ISO_14443A_PCD_STATE_OK;
				printf("\nEnd.");
				fprintf( g_fpDebug, "\nEnd." );
			}
			break;
	}
	return r;
}







