/**
 *	\file	rfid_iso14443A_picc.c
 *
 *
 *	\desp	模拟一张符合ISO14443A的卡.
 */

#include <stdlib.h>
#include "rfid_def.h"
#include "rfid_iso14443A_picc.h"
int main( int argc, char *argv[] )
{
	return picc_main( argc, argv );
}
/**
 *	命令行:
 *		picc_14443A n xxx1 xxx2
 *		n. 			仿真卡的数量(最多10张卡)
 *		xxx1/xxx2. 	卡号；
 */

// 命令行格式：ISO14443A_PICC uidLen UID
int picc_main(int argc, char *argv[] )
{
	ISO_14443A_PICC	stPICC;	
	
	if( argc == 2 )
	{
		// 从命令行接收UID
		stPICC.ucStatus = ISO_14443A_STATE_POWEROFF;
		stPICC.ucCmpBits = 0;
		switch( strlen( argv[1] ))
		{
			case 8:
				stPICC.ucCLn = 0;
				break;
			case 14:
				stPICC.ucCLn = 1;
				break;
			case 20:
				stPICC.ucCLn = 2;
				break;
			default:
				printf("\nuid invalid.\n");
				return -1;
		}
//		stPICC.ucCLn	= strlen( argv[2] );		// atoi(argv[1]);
		if( GetUIDFromString( &stPICC, argv[1] ) < 1 )
		{
			printf("\nInvalid command line.");
		}	
	}
	else
	{
		printf("input: iso14443A_picc uidLen(0/1/2) uid(HEX).\n" );
		stPICC.ucStatus = ISO_14443A_STATE_POWEROFF;
		stPICC.ucCmpBits = 0;
		stPICC.ucCLn	= 2; //atoi(argv[1]);
		stPICC.aucUID[0] = 0x12;
		stPICC.aucUID[1] = 0x34;
		stPICC.aucUID[2] = 0x56;
		stPICC.aucUID[3] = 0x78;
		
		stPICC.aucUID[4] = 0x88;
		stPICC.aucUID[5] = 0x90;
		stPICC.aucUID[6] = 0x91;
		stPICC.aucUID[7] = 0x92;

		stPICC.aucUID[8] = 0x88;
		stPICC.aucUID[9] = 0xA0;
		stPICC.aucUID[10] = 0xA1;
		stPICC.aucUID[11] = 0xA2;

		//return 0;
	}

	/**
	 *	开始进入循环
	 */
	stPICC.ucStatus = ISO_14443A_STATE_IDLE;

	/**
	 * 打印输出PICC信息.
	 */
	{
		int l1, l2, i;

		switch( stPICC.ucCLn )
		{
			case 0:
				l1 = 32;
				l2 = 4;
				break;
			case 1:
				l1 = 56;
				l2 = 8;
				break;
			case 2:
				l1 = 80;
				l2 = 12;
				break;
		}
		printf("\nPICC: uid length = %d, UID: ", l1/8 );
		for( i = 0; i < l2; i++ )
		{
			if( stPICC.aucUID[i] != 0x88 )
			{
				printf("0x%02X.", stPICC.aucUID[i]);
			}
		}	
	}
	PICC_ISO14443A_run( &stPICC );
	printf("\n\nEnd, press 'q' to end.\n");

	while(1)
	{
		char c = getchar();
		if( c == 'q' )
		{
			break;
		}
	}
	return 1;
}	

int GetUIDFromString( ISO_14443A_PICC *pstPICC, char *szUID )
{
	int i, j, k, l;
	char c;
	
	memset( pstPICC->aucUID, 0x00, 12 );
	switch( pstPICC->ucCLn )
	{
		case 0:
			l = 4;			// 0,1,2,3;
			memset( pstPICC->aucUID + 4, 0x88, 8 );
			break;
		case 1:
			l = 7;
			pstPICC->aucUID[0] = 0x88;			// x 0,1,2, 3,4,5,6
			memset( pstPICC->aucUID + 8, 0x88, 4 );
			break;
		case 2:
			// x 0,1,2, x,3,4,5, 6,7,8,9
			l = 10;
			pstPICC->aucUID[0] = 0x88;
			pstPICC->aucUID[4] = 0x88;
			break;
		default:
			return -1;
	}
	
	k = strlen( szUID );
	if( k > l * 2 )
	{
		return -1;
	}
	
	// 靠左存放
	j = 0;
	i = 0;
	while( j < k )
	{
		c = szUID[j];
		if( c >= 'A' && c <= 'F' )
		{
			c = c-'A' + 10;
		}
		else if( c >= 'a' && c <= 'f' )
		{
			c = c - 'a' + 10;
		}
		else if( c >= '0' && c <= '9' )
		{
			c = c - '0';
		}
		
		if( pstPICC->aucUID[i] == 0x88 )
		{ 
			i++;
		}
	
		if( (j % 2) == 0 )
		{
			pstPICC->aucUID[i] |= c << 4;
		}
		else
		{
			pstPICC->aucUID[i] |= c;
			i++;
		}
		j++;
	}	
	return 1;
}

int PICC_ISO14443A_run(ISO_14443A_PICC	*pstPICC)
{
	UCHAR 	aucRxBuf[256];
	UINT16 	u16BufLen = 256;
	int 	len, rtn;
	
	RFID_COM_Init( ISO_14443_PICC );
	printf("\nIDLE:\n\tWaiting REQA......");
	while( 1 )
	{
		// 接收数据并解析
		len = RFID_Card_Rx( u16BufLen, aucRxBuf );
		if( len > 0 )
		{
			rtn = PICC_ISO14443A_process( pstPICC, aucRxBuf, len );
		}
		else
		{
//			sleep( 1000 );
		}

		if( rtn == -100 )
		{
			return 1;
		}
	}
	return 1;	
}
	
/**
 *	每个PICC的状态机.
 *
 *		ucPICCId: 从0开始；
 *
 */
int PICC_ISO14443A_process( ISO_14443A_PICC	*pstPICC, BYTE abyRxBuf[], UINT16 u16BitLen )
{
	UCHAR aucRxCmd[32];			// 解码后的命令，或者发送命令
	UINT16	u16RxBitLen;
	int   r;
	
	switch( pstPICC->ucStatus )
	{
		case ISO_14443A_STATE_POWEROFF:
			// 几乎不存在！
			break;
		case ISO_14443A_STATE_IDLE:
			/**
			 *	处理REQA
			 */
			// printf("\nWaiting REQA...");
			r = ISO14443A_wait_REQA( abyRxBuf, u16BitLen ); 
			if( r > 0 )
			{
				printf("\n\tRx REQA. Tx ATQA. ");
				ISO14443A_ATQA( pstPICC->ucCLn );
				pstPICC->ucStatus = ISO_14443A_STATE_READY;
				printf("\n\nREADY:\n\tWaiting SELECT(anticollision)......");
			}
			// 所有非REQA的命令，都不处理.
			break;
		case ISO_14443A_STATE_READY:
			/**
			 *	处理ANTICOLLISION
			 */			
			//printf("\nWaiting anticollision...");
			r = ISO14443A_wait_anticollisionFrame_req( abyRxBuf, u16BitLen, aucRxCmd, &u16RxBitLen ); 
			if( r >= 16 )			// 至少16比特
			{
				BYTE sel, nvb;
				int m, n, l;

				sel = aucRxCmd[0];
				// sel是否合法？已经判过了，在此就不判了。
				if( ( sel != ISO_14443_CMD_SEL_CL1 ) && ( sel != ISO_14443_CMD_SEL_CL2 ) && ( sel != ISO_14443_CMD_SEL_CL3 ))
				{
					return -1;
				}	
				nvb = aucRxCmd[1];

				// 从nvb判断帧长
				m = ( nvb >> 4 ) & 0x0F;
				n = nvb &0x0F;
	
				l = m * 8;
				if( n != 0 )
				{
					l -= ( 8 - n );
				}
				if( aucRxCmd[1] == 0x70 )
				{
					// 可能是SELECT.
					if( l != u16RxBitLen - 16 )
					{
						return -1;
					}
				}
				else
				{
					if( l != u16RxBitLen )
					{
						return -1;
					}
				}
				// 帧没有问题.
				printf("\n\tRx %s", (l != u16RxBitLen) ? "SELECT":"anticollision.");
				printf("\n\t\tSEL: 0x%02X, NVB = 0x%02x.", sel, nvb );
				
				// 是anticollision帧。则继续
				if( aucRxCmd[1] == 0x70 )	// nvb.
				{
					// SELECT
					r = PICC_ISO14443A_PICC_select( pstPICC, aucRxCmd, u16RxBitLen );
					if( r < 0 )
					{
						printf(" \n\t\t------It's not my UID. do nothing."  );
					}
					else
					{
						printf("\n\t\tSELECT frame. Tx. SAK.[0x%02X]." , pstPICC->ucSAK );
						if( pstPICC->ucSAK == 0x00 )
						{
							printf("\nACTIVE: \n\tWaiting HALT...");
						}
						else
						{
							printf("\n\t\tWaiting anticollision...");
						}
					}
				}
				else
				{
					printf("\n\t\tAnticollision frame. Tx. anticollision." );
					r = PICC_ISO14443A_PICC_anticollision( pstPICC, aucRxCmd, u16RxBitLen );
					if( r < 0 )
					{
						printf(" \n\t:It's not my UID. do nothing."  );
					}
					else
					{
						printf("\n\t\tWaiting anticollision...");
					}
				}
			}
			break;
		case ISO_14443A_STATE_ACTIVE:
			/**
			 *	处理HALT命令
			 *
			 */
			// printf("\nWaiting halt...");
			r = ISO14443A_wait_HALT( abyRxBuf, u16BitLen ); 
			if( r > 0 )
			{
				// 不发送任何命令
				printf("\n\t\tRx HALT. Tx nothing. ");
				pstPICC->ucStatus = ISO_14443A_STATE_HALT;
				printf("\nHALT. \n\twaiting WUPA...");
			}
			else
			{
				printf("\n\t\tNot HALT cmd, continue waiting.");
			}
			break;			
		case ISO_14443A_STATE_HALT:
			/**
			 *	处理WUPA命令
			 *
			 */
			// printf("\nWaiting WUPA...");
			r = ISO14443A_wait_WUPA( abyRxBuf, u16BitLen ); 
			if( r > 0 )
			{
				printf("\n\tRx WUPA, Tx ATQA.");
				ISO14443A_ATQA( pstPICC->ucCLn );
				printf("\nREADY. Waiting anticollision");
				pstPICC->ucStatus = ISO_14443A_STATE_READY;
			}
			else
			{
				printf("\n\t\tNot WUPA cmd, continue waiting...");
				return -100;
			}
			break;			
	}


	return r;
}

// picc收到SELECT.
int PICC_ISO14443A_PICC_anticollision( ISO_14443A_PICC *pstPICC, BYTE abyRxCmd[], UINT16 u16CmdBitLen )
{
	BYTE sel, CL, bcc;
	int  uidBitLen;
	int  m, n;
	int	 i;
	int	 byoffset, btoffset;
	
	// 第一个字节中的bit Len，在防碰撞帧里需要	
	BYTE		byBitLenofFirstByte;
	BYTE		byFirstByte;
	BYTE 		abyUID[4]; 
	UINT16 		u16UIDBitLen; 
	BYTE		mask[]={0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};

	// 简化方法：u16CmdBitLen % 8 是否和 n 相等。
	sel = abyRxCmd[0];

	// 开始组织返回的帧.
	
	// PCD发送出来的UID的比特长度.
	uidBitLen = u16CmdBitLen - 16;		// sel+nvb.

	if( sel == ISO_14443_CMD_SEL_CL1 )
	{
		CL = 0;
	}
	else if( sel == ISO_14443_CMD_SEL_CL2 )
	{
		CL = 4;
	}
	else	// ISO_14443_CMD_SEL_CL3.
	{
		CL = 8;
	}
	
	// 比较是否相同，同时组返回包
	// 先比较最低bit，是完全正确的！
	{	
		printf("\n\t\tMY UID:");
		for( i = 0; i < 32; i++ )
		{
			byoffset = i / 8;
			btoffset = i % 8; 
			printf("%d", (pstPICC->aucUID[CL+byoffset] >> btoffset) & 0x01 );
			if( i % 4 == 3 )
			{
				printf(".");
			}
		}
		printf("\n\t\tRX UID:");
		for( i = 0; i < uidBitLen; i++ )
		{
			byoffset = i / 8;
			btoffset = i % 8;
			printf("%d", (abyRxCmd[2+byoffset] >> btoffset) & 0x01);
			if( i % 4 == 3 )
			{
				printf(".");
			}

			if( ((abyRxCmd[2+byoffset] >> btoffset) & 0x01) 
						!= (( pstPICC->aucUID[CL+byoffset] >> btoffset) & 0x01) )
			{
				return -1;		// 不理会
			}
		}
		// 前面相同！
		// 补充完整
		
		// uidBitLen用于计算发送方向比特数量.
		uidBitLen = 32 - uidBitLen;
		byBitLenofFirstByte = uidBitLen % 8;
		
		// 新发包。
		m = uidBitLen / 8;	// 字节数
		n = CL+3;
		u16UIDBitLen = 0;
		for( i = m-1; i >= 0; i-- )
		{
			abyUID[i] = pstPICC->aucUID[n--];
			u16UIDBitLen += 8;
		}
		if( byBitLenofFirstByte != 0 )
		{
			byFirstByte = ( pstPICC->aucUID[n] >> (8-byBitLenofFirstByte)) & mask[byBitLenofFirstByte-1];			// 其实已经可以了.
		}
		else
		{
			byFirstByte = 0x00;
		}
		
		// bcc
		bcc = 0x00;
		for( i = 0; i < 4; i++ )
		{
			bcc = bcc ^ pstPICC->aucUID[i+CL];
		}
	}

	return ISO14443A_AnticollisionFrame_rsp(byBitLenofFirstByte, byFirstByte, abyUID, u16UIDBitLen, bcc);
}

int PICC_ISO14443A_PICC_select( ISO_14443A_PICC *pstPICC, BYTE abyRxCmd[], UINT16 u16CmdBitLen )
{
	BYTE sel, bcc, sak;
	BYTE abyCRC[2];
	int i, CL;
	
	sel = abyRxCmd[0];
	//nvb = abyRxCmd[1];		// 一定为0x70
	
	// SELECT frame 长度：SEL + NVB + UID[4] + BCC + CRC[2]
	if( u16CmdBitLen != 72 )		
	{
		return -1;
	}


	// 看BCC是否正确
	bcc = 0x00;
	for( i = 2; i < 6; i++ )
	{
		bcc = bcc ^ abyRxCmd[i];
	}	
	if( bcc != abyRxCmd[6] )
	{
		return -1;
	}	
	// CRC是否正确
	ISO14443A_CRC( abyRxCmd, 7, &abyCRC[1], &abyCRC[0] );
	if( abyRxCmd[7] != abyCRC[0] || abyRxCmd[8] != abyCRC[1] )
	{
		return -1;
	}	
	
	// 都正确！

	sak = 0;
	switch( sel )
	{
		case ISO_14443_CMD_SEL_CL1:
			if( pstPICC->ucCLn == 0 )
			{
				sak = 0x00;
			}
			else
			{
				sak = 1 << 2;
			}
			CL = 0;
			break;
		case ISO_14443_CMD_SEL_CL2:
			if( pstPICC->ucCLn == 1 )
			{
				sak = 0x00;
			}
			else
			{
				sak = 1 << 2;
			}
			CL = 4;
			break;
		case ISO_14443_CMD_SEL_CL3:
			sak = 0x00;
			CL = 8;
			break;
	}

	// UID是否是我的UID.
	printf("\n\t\tMY UID[RX UID]: ");
	for( i = 0; i < 4; i++ )
	{
		printf("%02x[%02x]", pstPICC->aucUID[CL+i], abyRxCmd[2+i] );
		if( abyRxCmd[2+i] != pstPICC->aucUID[CL+i] )
		{
			return -1;
		}
	}

	// 发送SAK
	if( sak == 0x00 )
	{
		pstPICC->ucStatus	= ISO_14443A_STATE_ACTIVE;	
	}
	pstPICC->ucSAK		= sak;
	return ISO14443A_SAK(sak);
}
