

const int UBX_CHAR_SYNC0 = 0xB5;    /* '? */
const int UBX_CHAR_SYNC1 = 0x62;    /* 'b' */
const int UBX_HDR_SIZE = 6;
const int UBX_FTR_SIZE = 2;
const int UBX_FRM_SIZE = UBX_HDR_SIZE + UBX_FTR_SIZE;

//B5 62 01 02
void ProcessNavSvInfo(u8* pBuffer, int iSize) {
	
}