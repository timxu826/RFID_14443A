#include "stdafx.h"
#include "rfid_iso14443A_frm.h"

int framing_main()
{
	BYTE aInBuf[] = {0x11,0x12,0x13};
	BYTE FirstByte = 0x0d;
	int  firstBitLen = 5;		// 01101
	BYTE aOutBuf[1000];
	UINT16 u16InBitLen = 24;
	UINT16 u16OutBitLen;
    int i;
	int k;
    
	// framing

	printf("\ninitial: ");
	for( i = 0; i < 3; i++ )
	{
		printf("0x%02X ", aInBuf[i] );
	}

	printf("\n--------std framing: ");
    k = ISO14443A_stdFraming( aInBuf, 24, aOutBuf, &u16OutBitLen );
    printf("\n        frame bitlen = %d: ", k );
	k = k/8;
	k += (k%8)?1:0;
	for( i = 0; i < k; i++ )
	{
         printf("0x%02x ", aOutBuf[i]);
    }
	// unframing
    k = ISO14443A_Un_stdFraming( aOutBuf, u16OutBitLen, aInBuf, &u16InBitLen );
    printf("\n        unframing: " );
    printf("\n        frame bytelen = %d: ", k );
    
	for( i = 0; i < u16InBitLen / 8; i++ )
	{
         printf("0x%02x ", aInBuf[i]);
    } 
 
	printf("\n--------anticollision framing: ");
    k = ISO14443A_AnticollisionFraming( firstBitLen, FirstByte, aInBuf, 24, aOutBuf, &u16OutBitLen );
    printf("\n        frame bitlen = %d: ", k );
	for( i = 0; i < u16OutBitLen / 8; i++ )
	{
         printf("0x%02x ", aOutBuf[i]);
    }
	
    k = ISO14443A_Un_AnticollisionFraming( firstBitLen, &FirstByte, aOutBuf, u16OutBitLen, aInBuf, &u16InBitLen );
    printf("\n        unframing: " );
    printf("\n        frame bytelen = %d: ", k );
   
	for( i = 0; i < u16InBitLen / 8; i++ )
	{
         printf("0x%02x ", aInBuf[i]);
    } 

    printf("framing test");   

	return 1;
}
