#include "stdafx.h"

#include "rfid_iso14443A_cmdtx.h"
#include "rfid_iso14443A_cmdrx.h"
int cmd_main()
{
	BYTE abyCmd[100];
	BYTE abyBuf[200];
	BYTE sel;
	int k, n, r;

	// short frame.
	{
		//REQA
		k = ISO14443A_REQA();
		printf("\n   REQA: txlen = %3d. ",k);

		n = RFID_Card_Rx( 100, abyCmd );
		printf(" rxlen = %3d", n);
		r = ISO14443A_wait_REQA( abyCmd, n );
		printf(" Rx: %s", r > 0 ? "OK":"ER");

		// WUPA
		k = ISO14443A_WUPA();
		printf("\n   WUKA: txlen = %3d. ",k);

		n = RFID_Card_Rx( 100, abyCmd );
		printf(" rxlen = %3d", n);
		r = ISO14443A_wait_WUPA( abyCmd, n );
		printf(" Rx: %s", r > 0 ? "OK":"ER");

	}

	// std frame
	{
		// HALT.
		k = ISO14443A_HALT();
		printf("\n   HALT: txlen = %3d. ", k);
		
		n = RFID_Card_Rx( 100, abyCmd );
		printf(" rxlen = %3d", n);

		r = ISO14443A_wait_HALT( abyCmd, n );
		printf(" Rx: %s", r > 0 ? "OK":"ER");

		// PICC Send.
		// ATQA
		k = ISO14443A_ATQA( 1 );
		printf("\n   ATQA: txlen = %3d. ", k );
		
		n = RFID_Reader_Rx( 100, abyCmd );
		printf(" rxlen = %3d", n);
		r = ISO14443A_wait_ATQA( abyCmd, n, &abyBuf[0] );

		printf(" Rx: %s", abyBuf[0] == 1 ? "OK":"ER");

		// SAK
		k = ISO14443A_SAK( 2 );
		printf("\n    SAK: txlen = %3d. ", k);
		
		n = RFID_Reader_Rx( 100, abyCmd );
		printf(" rxlen = %3d", n);
		r = ISO14443A_wait_SAK( abyCmd, n, &abyBuf[0] );

		printf(" Rx: %s", abyBuf[0] == 2 ? "OK":"ER");
		//SELECT
		abyCmd[0] = 0x12;
		abyCmd[1] = 0x34;
		abyCmd[2] = 0x56;
		abyCmd[3] = 0x78;
		sel = 0x93;

		k = ISO14443A_SELECT(sel, abyCmd);
		printf("\n SELECT: txlen = %3d. sel = 0x%02X. uid = ", k, sel );
		for( k = 0; k < 4; k++ )
		{
			printf(" 0x%02x", abyCmd[k] );
		}

		n = RFID_Card_Rx( 100, abyCmd );
		printf("\n         rxlen = %3d.", n);
		r = ISO14443A_wait_SELECT( abyCmd, n, &sel, abyBuf );
		printf(" sel = 0x%02x. uid = ", sel );
		for( k = 0; k < 4; k++ )
		{
			printf(" 0x%02x", abyBuf[k] );
		}
		printf(" Rx: %s", r > 0 ? "OK":"ER");		
	}

	// anticollision frame
	{
		UINT16 u16BitLen;
		BYTE	tmp;

		abyCmd[0] = 0x12;
		abyCmd[1] = 0x34;
		abyCmd[2] = 0x56;
		abyCmd[3] = 0x78;

		k = ISO14443A_AnticollisionFrame_req( ISO_14443_CMD_SEL_CL1, abyCmd, 32 );
		printf("\n ANTIRQ: txlen = %3d.", k);
		for( k = 0; k < 4; k++ )
		{
			printf(" 0x%02x", abyCmd[k] );
		}

		n = RFID_Card_Rx( 100, abyCmd );
		printf("\n         rxlen = %3d.", n);
		r = ISO14443A_wait_anticollisionFrame_req( abyCmd, n, abyBuf, &u16BitLen ); 
		for( k = 2; k < 6; k++ )
		{
			printf(" 0x%02x", abyBuf[k] );
		}
		printf(" Rx: %s", r > 0 ? "OK":"ER");		

		// PICC->PCD.
		abyCmd[0] = 0x12;
		abyCmd[1] = 0x34;
		abyCmd[2] = 0x56;
		abyCmd[3] = 0x78;
		k = ISO14443A_AnticollisionFrame_rsp( 5, 0x15, abyCmd, 24, 0xA5 );
		printf("\n ANTIRP: txlen = %3d. firstbitlen = 5, firstbyte = 0x15. ", k);
		for( k = 0; k < 3; k++ )
		{
			printf(" 0x%02x", abyCmd[k] );
		}
		printf(" 0xA5" );

		n =  RFID_Reader_Rx( 100, abyCmd );
		printf("\n         rxlen = %3d. ", n);

		r = ISO14443A_wait_anticollisionFrame_rsp( 5, &tmp, abyCmd, n, abyBuf, &u16BitLen ); 
		printf("firstbitlen = 5, firstbyte = 0x%02x. ", tmp);

		for( k = 0; k < u16BitLen/8; k++ )
		{
			printf(" 0x%02X", abyBuf[k] );
		}
		printf("\n     Rx: %s", r > 0 ? "OK":"ER");	
	}
	return k;
}