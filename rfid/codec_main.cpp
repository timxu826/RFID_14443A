#include "stdafx.h"
#include "rfid_iso14443A_codec.h"

int codec_main()
{
	BYTE aInBuf[] = {0x11,0x12,0x13};
	BYTE aOutBuf[1000];
	UINT16 u16InBitLen = 24;
	UINT16 u16OutBitLen;
    int i;
	int k;
  
	printf("\ncodec test. initial dat: ");

	for( i = 0; i < 3; i++ )
	{
         printf("0x%02x ", aInBuf[i]);
    }

	printf("\n----PCD code: ");
    k = ISO14443A_PCD_Code( aInBuf, 24, aOutBuf, &u16OutBitLen );
	printf("\n    coded len = %d: \n    ", k );
	for( i = 0; i < u16OutBitLen / 8; i++ )
	{
         printf("0x%02x ", aOutBuf[i]);
    }
    
    k = ISO14443A_PICC_Decode( aOutBuf, u16OutBitLen, aInBuf, &u16InBitLen );
	printf("\n    decode len = %d: \n    ", k );
	for( i = 0; i < u16InBitLen / 8; i++ )
	{
         printf("0x%02x ", aInBuf[i]);
    } 
    
	printf("\n----PICC code: ");
    k = ISO14443A_PICC_Code( aInBuf, 24, aOutBuf, &u16OutBitLen );
	printf("\n    coded len = %d: \n    ", k );
	for( i = 0; i < u16OutBitLen / 8; i++ )
	{
         printf("0x%02x ", aOutBuf[i]);
    }
    
    k = ISO14443A_PCD_Decode( aOutBuf, u16OutBitLen, aInBuf, &u16InBitLen );
	printf("\n    decode len = %d: \n    ", k );
    
	for( i = 0; i < u16InBitLen / 8; i++ )
	{
         printf("0x%02x ", aInBuf[i]);
    } 

    printf("\nend");   

	return 1;
}
