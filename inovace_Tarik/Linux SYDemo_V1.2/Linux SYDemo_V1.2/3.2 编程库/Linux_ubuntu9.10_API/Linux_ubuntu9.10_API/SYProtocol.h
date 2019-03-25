#ifndef  _SYPROTOCOL_H_
#define  _SYPROTOCOL_H_

//#include "SYDevice.h"


///////////////////return code////////////////////
#define PS_OK                0x00
#define PS_COMM_ERR          0x01
#define PS_NO_FINGER         0x02
#define PS_GET_IMG_ERR       0x03
#define PS_FP_TOO_DRY        0x04
#define PS_FP_TOO_WET        0x05
#define PS_FP_DISORDER       0x06
#define PS_LITTLE_FEATURE    0x07
#define PS_NOT_MATCH         0x08
#define PS_NOT_SEARCHED      0x09
#define PS_MERGE_ERR         0x0a
#define PS_ADDRESS_OVER      0x0b
#define PS_READ_ERR          0x0c
#define PS_UP_TEMP_ERR       0x0d
#define PS_RECV_ERR          0x0e
#define PS_UP_IMG_ERR        0x0f
#define PS_DEL_TEMP_ERR      0x10
#define PS_CLEAR_TEMP_ERR    0x11
#define PS_SLEEP_ERR         0x12
#define PS_INVALID_PASSWORD  0x13
#define PS_RESET_ERR         0x14
#define PS_INVALID_IMAGE     0x15
#define PS_HANGOVER_UNREMOVE 0X17


///////////////buffer zone//////////////////////////////
#define CHAR_BUFFER_A          0x01
#define CHAR_BUFFER_B          0x02
#define MODEL_BUFFER           0x03

//////////////////serial com////////////////////////
#define COM1                   0x01
#define COM2                   0x02
#define COM3                   0x03

//////////////////baud rate setting////////////////////////
#define BAUD_RATE_9600         0x00
#define BAUD_RATE_19200        0x01
#define BAUD_RATE_38400        0x02
#define BAUD_RATE_57600        0x03   //default
#define BAUD_RATE_115200       0x04

#define WINAPI

typedef unsigned char BYTE;

/////////////////////////////////////////
//for open:0-false 1- true
int WINAPI PSOpenDevice(int nDeviceType,int nPortNum,int nPortPara,int nPackageSize=2);
int WINAPI PSCloseDevice();
void WINAPI Delay(int nTimes);
///////////////////////////////////////////////
//////         function                //////
///////////////////////////////////////////////
 
int WINAPI      PSGetImage(int nAddr);

 
int  WINAPI    PSGenChar(int nAddr,int iBufferID);

 
int WINAPI     PSMatch(int nAddr,int* iScore);

 
int  WINAPI    PSSearch(int nAddr,int iBufferID, int iStartPage, int iPageNum, int *iMbAddress);

 
int  WINAPI    PSRegModule(int nAddr);

 
int  WINAPI    PSStoreChar(int nAddr,int iBufferID, int iPageID);

 
int  WINAPI     PSLoadChar(int nAddr,int iBufferID,int iPageID);

 
int WINAPI     PSUpChar(int nAddr,int iBufferID, unsigned char* pTemplet, int* iTempletLength);

 
int WINAPI     PSDownChar(int nAddr,int iBufferID, unsigned char* pTemplet, int iTempletLength);

 
int WINAPI     PSUpImage(int nAddr,unsigned char* pImageData, int* iImageLength);

 
int WINAPI     PSDownImage(int nAddr,unsigned char *pImageData, int iLength);

 
int  WINAPI     PSImgData2BMP(unsigned char* pImgData,const char* pImageFile);

 

int  WINAPI     PSGetImgDataFromBMP(const char *pImageFile,unsigned char *pImageData,int *pnImageLen);

 
int WINAPI     PSDelChar(int nAddr,int iStartPageID,int nDelPageNum);

 
int WINAPI     PSEmpty(int nAddr);

 
int WINAPI     PSReadParTable(int nAddr,unsigned char* pParTable);

 
int WINAPI     PSPowerDown(int nAddr);

 
int WINAPI     PSSetPwd(int nAddr,unsigned char* pPassword);

 
int WINAPI     PSVfyPwd(int nAddr,unsigned char* pPassword);

 
int WINAPI      PSReset(int nAddr);

 
int	WINAPI	    PSReadInfo(int nAddr,int nPage,unsigned char* UserContent);

 
int	WINAPI	    PSWriteInfo(int nAddr,int nPage,unsigned char* UserContent);

 
int  WINAPI     PSSetBaud(int nAddr,int nBaudNum);
 
int WINAPI     PSSetSecurLevel(int nAddr,int nLevel);
 
int   WINAPI   PSSetPacketSize(int nAddr,int nSize);

int   WINAPI    PSUpChar2File(int nAddr,int iBufferID, const char* pFileName);

int  WINAPI     PSDownCharFromFile(int nAddr,int iBufferID, const char* pFileName);

int WINAPI PSGetRandomData(int nAddr,unsigned char* pRandom);

int WINAPI PSSetChipAddr(int nAddr,unsigned char* pChipAddr);

 
char* WINAPI   PSErr2Str(int nErrCode);

#define DEVICE_USB		0
#define DEVICE_COM		1
#define DEVICE_UDisk	2


#define IMAGE_X 256
#define IMAGE_Y 288


#endif

