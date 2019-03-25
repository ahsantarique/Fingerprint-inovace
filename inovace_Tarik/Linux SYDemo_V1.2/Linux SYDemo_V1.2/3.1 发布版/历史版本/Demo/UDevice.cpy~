

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "UDevice.h"

#define ST_VENDOR  0x0fbc	//0x0453-USB
#define ST_PRODUCT 0x3090	//0x9005-USB

int			g_nComPort	= 1;	//端口号COM1~COM8
int			g_nBaudRate	= 57600;//波特率

const	int	WAITTIME_PER_BYTE = 100;

////////////////////////////////////////////////////////

int OpenUDisk()
{
	return 1;
}

int CloseUDisk()
{
	return 1;
}

//接收数据
int UDiskGetData(unsigned char* DataBuf, int nLen)
{//模式 DO_CBW + DI_DATA + DI_CSW
	int ret = -1;
	unsigned char do_CBW[32] = {0x55,0x53,0x42,0x43,
								0xb0,0xfa,0x69,0x86,
								0x00,0x00,0x00,0x00,
								0x80,0x00,0x0a,0x85,
								0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00};
	unsigned char di_CSW[16] = {0x00};

	do_CBW[8] = nLen & 0xff;
	do_CBW[9] = (nLen>>8) & 0xff;
	do_CBW[10]= (nLen>>16)& 0xff;
	do_CBW[11]= (nLen>>24)& 0xff;

	//DO_CBW

	//DI_DATA

	//DI_CSW

	return ret;
}

//发送数据
int UDiskDownData(unsigned char* pBuf, int nLen)
{//模式 DO_CBW + DO_CMD + DI_CSW
	int ret = -1;
	unsigned char do_CBW[32] = {0x55,0x53,0x42,0x43,
								0xb0,0xfa,0x69,0x86,
								0x00,0x00,0x00,0x00,
								0x00,0x00,0x0a,0x86,
								0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00};
	unsigned char di_CSW[16] = {0x00};

	do_CBW[8] = nLen & 0xff;
	do_CBW[9] = (nLen>>8) & 0xff;
	do_CBW[10]= (nLen>>16)& 0xff;
	do_CBW[11]= (nLen>>24)& 0xff;

	//DO_CBW

	//DO_CMD

	//DI_CSW

	return ret;
}

int UDiskDownData1(unsigned char* pBuf, int nLen)
{
	return 1;
}

int UDiskGetImage(unsigned char* Img, int nLen)
{
	return 1;
}

int UDiskDownImage(unsigned char* Img, int nLen)
{
	return 1;
}