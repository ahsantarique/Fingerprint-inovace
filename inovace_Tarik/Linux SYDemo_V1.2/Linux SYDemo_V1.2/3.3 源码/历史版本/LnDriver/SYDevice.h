#ifndef _SYDEVICE_H_
#define _SYDEVICE_H_
#include "vardef.h"
extern int       g_nComPort     ;     //ï¿½ï¿½ï¿½Úºï¿½
extern int       g_nBaudRate    ; //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½


extern void dDelay(int nTimes);
extern void ClearCom();
extern BOOL GetByte(unsigned char* tranChar);
extern BOOL  SendByte(unsigned char tranChar);
extern BOOL OpenUSB();
extern BOOL CloseUSB();
extern BOOL OpenCom(int comPort, int baudRate);
extern BOOL  CloseCom();
extern int USBDownData(unsigned char *image,int nLen);
extern int USBDownData1(unsigned char *image,int nLen);
extern int USBGetData(unsigned char* DataBuf,int nLen);
extern int USBGetImage(unsigned char* DataBuf,int nLen);
extern int USBDownImage(unsigned char *image,int nLen);

//ÎÞÇý
extern BOOL OpenUDisk();
extern BOOL CloseUDisk();
extern int UDiskGetData(unsigned char* DataBuf, int nLen);
extern int UDiskDownData(unsigned char* pBuf, int nLen);
extern int UDiskGetImage(unsigned char* Img, int nLen);
extern int UDiskDownImage(unsigned char* Img, int nLen);

int mydata_prt(const void *data, int length);

#endif
