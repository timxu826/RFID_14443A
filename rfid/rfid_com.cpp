/**
 *	\file rfid_com.c
 *
 *	\biref	仿真rfid的通信
 */

//#include "stdafx.h"
#include "rfid_com.h"

/**
 *	方案1: 采用文件交换
 *	定义交换文件
 *	文件格式：
 *
 *	1B:	方向:	0x50	Reader->Card
 *				0x51	Card->Reader
 *	1B:	reserved
 *	2B:	数据比特长度
 *	数据.
 */


#ifdef _RFID_COM_FILE_

	#include <io.h>

	#define RFID_COM_FILE_CARD_TO_READER				"d:\\rfid\\crwap"
	#define RFID_COM_FILE_READER_TO_CARD				"d:\\rfid\\rcwap"
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <errno.h>

	#define	RFID_CARD_UDP_PORT		10101
	#define RFID_READER_UDP_PORT	10201
	
	int		g_nSendSocketId = -1;
	int 	g_nRecvSocketId = -1;
	struct sockaddr_in g_stSendAddr;
	struct sockaddr_in g_stRecvAddr;
	
#endif

#define RFID_COM_CARD_LOG_NAME			"d:\\rfid\\rfid_card_log.txt"
#define RFID_COM_READER_LOG_NAME		"d:\\rfid\\rfid_reader_log.txt"

// Direction
#define RFID_COM_ID		0x50
#define RFID_COM_CARD_TX			0
#define RFID_COM_CARD_RX			1
#define RFID_COM_READER_TX			2
#define RFID_COM_READER_RX			3

int RFID_COM_LOG( UCHAR ucDirection, UINT16 u16BitLen, UCHAR *pBuf  );

#ifdef _RFID_COM_FILE_
char* RFID_COM_GetFileName( UCHAR ucDirection )
{
	switch( ucDirection )
	{
		case RFID_COM_CARD_TX:
		case RFID_COM_READER_RX:
			return RFID_COM_FILE_CARD_TO_READER;
		case RFID_COM_CARD_RX:
		case RFID_COM_READER_TX:
			return RFID_COM_FILE_READER_TO_CARD;
	}
	return NULL;
}
#endif


void RFID_COM_Init( int pcd_or_picc )
{
#ifdef _RFID_COM_FILE_
	if( pcd_or_picc == ISO_14443_PCD )
	{
		char 	*szFileName;
		szFileName = RFID_COM_GetFileName( RFID_COM_READER_TX );
		if( access( szFileName, 0) == 0 )
		{
			unlink( szFileName );
		}

		szFileName = RFID_COM_GetFileName( RFID_COM_READER_RX );
		if( access( szFileName, 0) == 0 )
		{
			unlink( szFileName );
		}
	}
#else
	char str[20];
	struct timeval timeout = {0,200000};  		// 200ms.

	g_nSendSocketId = socket( AF_INET, SOCK_DGRAM, 0 );
	g_nRecvSocketId = socket( AF_INET, SOCK_DGRAM, 0 );

	sprintf( str, "127.0.0.1");
	bzero( &g_stSendAddr, sizeof( g_stSendAddr ));
	g_stSendAddr.sin_family = AF_INET;
	g_stSendAddr.sin_addr.s_addr = inet_addr( str );
	
	bzero( &g_stRecvAddr, sizeof( g_stRecvAddr ));
	g_stRecvAddr.sin_family = AF_INET;
	g_stRecvAddr.sin_addr.s_addr = htonl( INADDR_ANY );

	if( pcd_or_picc == ISO_14443_PCD )
	{
		g_stSendAddr.sin_port = htons(RFID_CARD_UDP_PORT);
		g_stRecvAddr.sin_port = htons(RFID_READER_UDP_PORT);
	}
	else
	{
		g_stSendAddr.sin_port = htons(RFID_READER_UDP_PORT);
		g_stRecvAddr.sin_port = htons(RFID_CARD_UDP_PORT);
	}
	bind( g_nRecvSocketId, (struct sockaddr *)&g_stRecvAddr, sizeof( g_stRecvAddr ));
	setsockopt( g_nRecvSocketId, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));

#endif
}

int RFID_COM_Tx( UCHAR ucDirection, UINT16 u16BitLen, UCHAR *pBuf )
{
#ifdef _RFID_COM_FILE_
	FILE *fp;
	UCHAR	aucBuf[4]={0x00};
	char 	*szFileName;

	szFileName = RFID_COM_GetFileName( ucDirection );
	// 2016/11/23
	// 如果文件已经存在，说明对方没有读取，则等待.
	while(1)
	{
		if( access( szFileName, 0) == -1 )
		{
			break;
		}
		// access: 0表示存在，-1表示不存在.
		{
			int i, k=0;
			for( i = 0; i < 200000; i++ )
			{
				k++;
			}
		}
	}

	fp = fopen( szFileName, "wb" );
	if( !fp )
	{
		return -1;
	}
	aucBuf[0] = RFID_COM_ID;
	*(UINT16 *)&aucBuf[2] = u16BitLen;
	
	fwrite( aucBuf, 1, 4, fp );
	if( u16BitLen % 8 )
	{
		fwrite( pBuf, 1, u16BitLen/8+1, fp );
	}
	else
	{
		fwrite( pBuf, 1, u16BitLen/8, fp );
	}
	fclose( fp );
	
	RFID_COM_LOG( ucDirection, u16BitLen, pBuf );

	return u16BitLen;
#else
	UINT16 u16BufLen;
	int txport;
	UCHAR aucBuf[200];

	u16BufLen = u16BitLen/8;
	if( u16BitLen % 8 )
	{
		u16BufLen++;
	}

	if( g_nSendSocketId != -1 )
	{
		int i;
#ifdef _RFID_DEBUG_
		printf("\nTxLen = %d, bitLen = %d ", u16BufLen, u16BitLen );
		for( i = 0; i < u16BufLen; i++ )
		{
			printf("0x%02x ", pBuf[i]);
		}
#endif
		aucBuf[0] = RFID_COM_ID;
		*(UINT16 *)&aucBuf[2] = u16BitLen;
		memcpy( aucBuf+4, pBuf, u16BufLen );

		return sendto( g_nSendSocketId, aucBuf, u16BufLen+4, 
					0, (struct sockaddr *)&g_stSendAddr, sizeof( g_stSendAddr )); 
	}
	else
	{
		return 0;	
	}	
#endif
}

int RFID_COM_Rx( UCHAR ucDirection, UINT16 u16ByteLen, UCHAR *pBuf, int block )
{
#ifdef _RFID_COM_FILE_
	FILE *fp;
	UCHAR	aucBuf[4]={0x00};
	UINT16	u16BitLen;
	char 	*szFileName;

	szFileName = RFID_COM_GetFileName( ucDirection );
	
	// 2016/11/23
	// 如果文件已经存在，说明对方没有读取，则等待.
	while(1)
	{
		// access: 0表示存在，-1表示不存在.
		if( access( szFileName, 0) == 0 )
		{
			break;
		}
		if( block == 0)
		{
			// 不等.
			return 0;
		}
		
		// 等待，然后继续。
		{
			int i, k=0;
			for( i = 0; i < 200000; i++ )
			{
				k++;
			}
		}
	}

	// 存在，但可能文件创建，数据还没有写入，所以等待一会，确保数据写入。
		{
			int i, k=0;
			for( i = 0; i < 150000; i++ )
			{
				k++;
			}
		}


	fp = fopen( szFileName, "rb" );
	if( !fp )
	{
		return 0;
	}
	fread( aucBuf, 1, 4, fp );
	if( aucBuf[0] != RFID_COM_ID )
	{
		fclose( fp );
		return 0;
	}
	u16BitLen = *(UINT16 *)&aucBuf[2];
	
	if( u16BitLen % 8 )
	{
		fread( pBuf, 1, u16BitLen/8+1, fp );
	}
	else
	{
		fread( pBuf, 1, u16BitLen/8, fp );
	}
	// 清空文件：文件没有以独占方式打开
	fclose( fp );

	remove( szFileName );

	RFID_COM_LOG( ucDirection, u16BitLen, pBuf );
	return u16BitLen;
#else
	UCHAR aucBuf[200];
	int sin_len;
	int n, k, i;

	if( g_nRecvSocketId != -1 )
	{
		n = recv( g_nRecvSocketId, aucBuf, 200, 0 );		
		if( n > 0 )
		{
			k = *(UINT16 *)&aucBuf[2];
#ifdef _RFID_DEBUG_
			printf("\nrx dat, len = %d, bitlen = %d.", n, k);
			for( i = 0; i < n; i++ )
			{
				printf("0x%02x ", aucBuf[i]);
			}
#endif
			memcpy( pBuf, aucBuf+4, n-4 );
			return k;
		}	
		else
		{
#ifdef _RFID_DEBUG_
			printf("\nrx: %d, error = %d", n, errno);
#endif
			return 0;
		}	
	}	
	else
	{
		return 0;
	}
#endif	
}

int RFID_COM_LOG( UCHAR ucDirection, UINT16 u16BitLen, UCHAR *pBuf  )
{
#ifdef _RFID_COM_FILE_
	FILE *fp;
	int i;
	char *szBuf[] = {"CardTx", "CardRx", "ReaderTx", "ReaderRx"};
	char *szFileName = NULL;

	if( ucDirection == RFID_COM_CARD_TX || ucDirection == RFID_COM_CARD_RX )
	{
		szFileName = RFID_COM_CARD_LOG_NAME;
	}
	else
	{
		szFileName = RFID_COM_READER_LOG_NAME;
	}
	fp = fopen(	szFileName, "a+" );
	if( !fp )
	{
		fp = fopen(	szFileName, "w" );
	}
	fprintf(fp, "\n\n--%10s bitlen=%3d: \n--          ", szBuf[ucDirection], u16BitLen );
#ifdef _RFID_DEBUG_
	printf("\n--%10s bitlen=%3d:\n--           ", szBuf[ucDirection], u16BitLen );
#endif
	for( i = 0; i < u16BitLen; i+=8 )
	{
		fprintf( fp, "%02x ", *pBuf );
#ifdef _RFID_DEBUG_
		printf("%02x ", *pBuf );
#endif
		pBuf++;
	}
	fclose( fp );
#endif
	return 1;
}

/**
 *	发送数据，立即返回
 */
int RFID_Card_Tx( UINT16 u16BitLen, UCHAR *pBuf )
{
	return RFID_COM_Tx( RFID_COM_CARD_TX, u16BitLen, pBuf );
}

/**
 *	非阻塞操作.
 *
 *	返回值定义为：
 *		>0，实际收到数据的比特长度;
 *		=0, 没有收到数据;
 */
int RFID_Card_Rx( UINT16 u16ByteLen, UCHAR *pBuf )
{
	return RFID_COM_Rx( RFID_COM_CARD_RX, u16ByteLen, pBuf, 1 );		// wait 
}

/**
 *	发送数据，立即返回
 */
int RFID_Reader_Tx( UINT16 u16BitLen, UCHAR *pBuf )
{
	return RFID_COM_Tx( RFID_COM_READER_TX, u16BitLen, pBuf );
}

/**
 *	非阻塞操作.
 *
 *	返回值定义为：
 *		>0，实际收到数据的比特长度;
 *		=0, 没有收到数据;
 */
int RFID_Reader_Rx( UINT16 u16ByteLen, UCHAR *pBuf )
{
	return RFID_COM_Rx( RFID_COM_READER_RX, u16ByteLen, pBuf, 0 );		// 不等
}





 
