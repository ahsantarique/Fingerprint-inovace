#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/times.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>


#include "SYDevice.h"
#include "SYdevice.cpp"
#include "SYProtocol.h"



/*---------------------- command define----------------------*/
#define    	GET_IMAGE			0x01//��ȡͼ��
#define    	GEN_CHAR			0x02//����ԭʼͼ������ָ����������CharBuffer1��CharBuffer2
#define    	MATCH				0x03//��ȷ�ȶ�CharBuffer1��CharBuffer2�е������ļ�
#define    	SEARCH			    0x04//��CharBuffer1��CharBuffer2�е������ļ����������򲿷�ָ�ƿ�
#define    	REG_MODULE			0x05//��CharBuffer1��CharBuffer2�е������ļ��ϲ�����ģ������CharBuffer2
#define    	STORE_CHAR			0x06//�����������е��ļ����浽flashָ�ƿ���
#define    	LOAD_CHAR			0x07//��flashָ�ƿ��ж�ȡһ��ģ�嵽��������
#define    	UP_CHAR				0x08//�����������е��ļ��ϴ�����λ��
#define    	DOWN_CHAR			0x09//����λ������һ�������ļ�����������
#define    	UP_IMAGE		    0x0a//�ϴ�ԭʼͼ��
#define    	DOWN_IMAGE			0x0b//����ԭʼͼ��
#define    	DEL_CHAR		    0x0c//ɾ��flashָ�ƿ��е�һ�������ļ�
#define    	EMPTY 		        0x0d//����flashָ�ƿ�
#define		WRITE_REG			0x0e//дģ���Ĵ���ָ��
#define     READ_PAR_TABLE      0x0f//��ϵͳ������
#define     ENROLL              0x10//�͹���ָ��
#define     IDENTIFY            0x11//ϵͳ��ʼ��ָ��Դ�����ƫ����Ƚ��м�����¼
#define    	SET_PWD				0x12//�����豸���ֿ���
#define    	VFY_PWD				0x13//��֤�豸���ֿ���

#define     GET_RANDOM          0x14//��ȡ������_
#define     SET_CHIPADDR        0x15//����оƬ��ַ
#define     READ_INFPAGE        0x16//��ɣΣƣ�ҳ
#define    	PORT_CONTROL		0x17//ϵͳ��λ�������ϵ���ʼ״̬

#define    	WRITE_NOTEPAD   	0x18//д���±�
#define    	READ_NOTEPAD 		0x19//����±�         
#define     BURN_CODE           0x1a//��д����
#define     HIGH_SPEED_SEARCH     0x1b//��������
#define     GEN_BINIMAGE        0x1c//����ϸ��ͼ
#define     TEMPLATE_NUM         0x1d

#define     REG_BAUD            4//�����ʼĴ���
#define    	REG_SECURE_LEVEL	5//��ȫ�ȼ��Ĵ��� 
#define		REG_PACKETSIZE		6//���ݰ���С�Ĵ���


//����ʽ
const unsigned char     CMD      =  0x01;  //������
const unsigned char     DATA     =  0x02;  //���ݰ�
const unsigned char     ENDDATA  =  0x08;  //����һ�����ݰ�
const unsigned char     RESPONSE =  0x07;  //Ӧ����

const DWORD PACKET_HEAD = 0xEF01;//��ͷ

const int   MAX_PACKAGE_SIZE  =  350;   //���ݰ����󳤶�
const int   HEAD_LENGTH       =  3;     //��ͷ����

const int   TIMEOUT           =  100;   //��ʱʱ��100ms
#define   APP_TIME_OUT      (8*sysconf (_SC_CLK_TCK))  //Ӧ�ò㳬ʱʱ��



static char      g_sErrorString[128];    //������Ϣ

static int       g_nPackageSize = 128;   //���ݰ���С

#define DEVICE_USB		0
#define DEVICE_COM		1
#define DEVICE_UDisk	2
int gnDeviceType;
 
#define IMAGE_X 256
#define IMAGE_Y 288
 
int GetTickCount()
{
  return times(NULL);
}
 
/**********************************************
  FUNCTION:   EnCode
  PURPOSE:    �����ݰ�Э��Ҫ���ĸ�ʽ���б���,����EF01��У���롣
  ARGS:       
       pSource:          ָ��Ҫ���������ݵ�ַ
	   iSourceLength:    Դ���ݳ���
	   pDestination:           ָ�������������ݵ�ַ
	   iDestinationLength:     �����������ݳ���
  RETURN:
       FALSE:            ʧ��
	   TRUE:             �ɹ�
***********************************************/
BOOL WINAPI EnCode(int nAddr,unsigned char* pSource, int iSourceLength, unsigned char* pDestination, int* iDestinationLength)
{
	unsigned char* pSrc;
	unsigned char* pDest;
 
	int i, count;

	*iDestinationLength = 0;
	if ( !pSource || !pDestination )
		return FALSE;
	if ( iSourceLength > MAX_PACKAGE_SIZE-4 )  //��ȥ��ͷ��У����
		return FALSE;

	pSrc = pSource;
	pDest = pDestination;

	*pDest++ = 0xEF;      
	*pDest++=0x01;
	*pDest++=(nAddr>>24)&0xff;
	*pDest++=(nAddr>>16)&0xff;
	*pDest++=(nAddr>>8)&0xff;
	*pDest++=nAddr&0xff;
 
 
	i = 0;
	count = 1;
	int ChkSum=0;

	for (i=0; i<iSourceLength-2; i++)
	{
		 ChkSum+=*(pSrc+i);
		 *pDest++=*(pSrc+i);
	}
    int ValH,ValL;
    ValL=ChkSum&0xff;
	ValH=(ChkSum>>8)&0xff;
    *pDest++=ValH;
	*pDest++=ValL;

	*iDestinationLength = iSourceLength+6;

	return TRUE;
}

/**********************************************
  FUNCTION:   DeCode
  PURPOSE:    �԰�Э��Ҫ���ĸ�ʽ���б��������ݽ���
  ARGS:       
       pSource:          ָ��Ҫ���������ݵ�ַ
	   iSourceLength:    Դ���ݳ���
	   pDestination   :         ָ�������������ݵ�ַ
	   iDestinationLength:      �����������ݳ���
  RETURN:
       FALSE:            ʧ��
	   TRUE:             �ɹ�
***********************************************/
BOOL WINAPI DeCode(unsigned char* pSource, int iSourceLength, unsigned char* pDestination, int* iDestinationLength)
{
	unsigned char*   pSrc;
	unsigned char*   pDest;
     
	*iDestinationLength = 0;
	if ( !pSource || !pDestination)
		return FALSE;
    if ( iSourceLength<0 || iSourceLength>MAX_PACKAGE_SIZE )
		return FALSE;

	
	pSrc  = pSource;
	pDest = pDestination;

	if(*pSrc!=0xEF || *(pSrc+1)!=0x01) return FALSE;
    
	pSrc=pSource+6;

	int ChkSum=0;

	for(int i=0;i<(iSourceLength-8);i++)
	{
		*(pDest+i)=*(pSrc+i);
		ChkSum+=(unsigned char)(*(pSrc+i));
	}

	int ValH,ValL,Val;
	ValH=*(pSource+iSourceLength-2);
	ValL=*(pSource+iSourceLength-1);
    Val=(ValH<<8)+ValL;

	if(ChkSum!=Val) 
		return FALSE; 

	*iDestinationLength = iSourceLength-8;

	return TRUE;
}
//---------------------------------------------------------------------------------------------------------------
BOOL WINAPI DeCodeUSB(unsigned char* pSource, int iSourceLength, unsigned char* pDestination, int* iDestinationLength)
{
	unsigned char*   pSrc;
	unsigned char*   pDest;
     
	*iDestinationLength = 0;
	if ( !pSource || !pDestination)
		return FALSE;
   

	
	pSrc  = pSource;
	pDest = pDestination;

	if(*pSrc!=0xEF || *(pSrc+1)!=0x01) return FALSE;
    
	pSrc=pSource+6;
	int nLen=(*(pSource+7)<<8)+*(pSource+8)+1;

	for(int i=0;i<nLen;i++)

       *(pDestination+i)=*(pSrc+i);

	 

	*iDestinationLength = nLen;

	return TRUE;
}

/**********************************************
  FUNCTION:   GetPackage
  PURPOSE:    �հ�							SELF 
  ARGS:
       pData:  �������ݵ�ַ
  RETURN:
       0:    ʧ��
	   1:    �ɹ�
***********************************************/
BOOL WINAPI    GetPackageCom(unsigned char *pData,int nTimeOut=APP_TIME_OUT)
{   
	int i;
    unsigned char cChar;
	unsigned char headerBuf[10];//֡ͷ���⻺����0xEF 01 + Addr+ type + PacketSizeH + PacketSizeL
	unsigned char recvBuf[MAX_PACKAGE_SIZE];
	unsigned char* decodedBuf;
	int           nCount;
	int           nDecodedLen;
	BOOL          bSuccess = FALSE;
    long          checkSum = 0;
	int           nRepeatTime = 0;
	DWORD         dwStart, dwStop;
//	COMSTAT  comstat;
	DWORD    dwError;
    
	if (!pData)
		return FALSE;

	decodedBuf = pData;
    
	//���յ�һ���ֽڣ�������0xA5����������һ���ֽ�ֱ���յ�0xA5����ʱ
	for(i=0;i<10;i++)
		headerBuf[i]=0;					//֡ͷ���⻺��������
	int RecNum=0;

	dwStart = GetTickCount();
 

	while (1)
	{
		bSuccess = GetByte(&cChar);
		if(bSuccess)
		{
			for(i=0;i<8;i++)
				headerBuf[i]=headerBuf[i+1];		//֡ͷ���⻺������������
			headerBuf[8]=cChar;
			
			if((headerBuf[0]==0xef)&&(headerBuf[1]==0x01))
				break;
		}
		dwStop = GetTickCount();
		if ( dwStop-dwStart > nTimeOut )
		{
		//	ClearCommError(g_hCom, &dwError, &comstat);
			return FALSE;
		}
	}
    
	
	int PacketLen=0;
	for(nCount=0;nCount<9;nCount++)
		recvBuf[nCount]=headerBuf[nCount];
	PacketLen=(recvBuf[7]<<8)+recvBuf[8];
	//���պ�������
	dwStart = GetTickCount();
	while (1)
	{
		//����һ���ֽ�
		if ( !GetByte(&cChar) )
			continue;
		
		recvBuf[nCount++] = cChar;

		if((PacketLen>0) && (nCount>=PacketLen+9))
			break;
		dwStop = GetTickCount();
		if ( dwStop-dwStart > nTimeOut )
		{
		//	ClearCommError(g_hCom, &dwError, &comstat);
			return FALSE;
		}
		
	 
	}
	//����
	if ( !DeCode(recvBuf, nCount, decodedBuf, &nDecodedLen) )
		return FALSE;
	
	return TRUE;
}
BOOL WINAPI    GetPackageUSB(unsigned char *pData,int nLen)
{   
 
   	unsigned char recvBuf[1024];
	unsigned char* decodedBuf; 
	int nDecodedLen;
    
	if (!pData)
		return FALSE;
	
	decodedBuf = pData;
    if(USBGetData(recvBuf,nLen))
		return FALSE;
	
 
	//����
	if ( !DeCodeUSB(recvBuf, nLen, decodedBuf, &nDecodedLen) )
		return FALSE;
	
	return TRUE;
}
BOOL WINAPI	GetPackageUDisk(unsigned char *pData,int nLen)
{   
	//printf("------into GetPackgeUDisk function-----\n");
   	unsigned char recvBuf[1024];
	unsigned char* decodedBuf; 
	int nDecodedLen;
    
	if (!pData)
	{
		printf("pDate is error\n");
		return FALSE;
	}
	
	decodedBuf = pData;
	if(UDiskGetData(recvBuf,nLen)!=PS_OK)
	{
		printf("UDiskGetDate is error\n");
		return FALSE;
	}
 
	//����
	if ( !DeCodeUSB(recvBuf, nLen, decodedBuf, &nDecodedLen) )
	{
		printf("DeCodeUSB is error\n");
		return FALSE;
	}
	
	return TRUE;
}
BOOL WINAPI    GetPackage(unsigned char *pData,int nLen=64,int nTimeOut=APP_TIME_OUT)
{
	//printf("-----into GetPackage function-----\n");
	if(gnDeviceType==DEVICE_USB)
	{
		return GetPackageUSB(pData,nLen);
	}
	else if(gnDeviceType==DEVICE_COM)
		return GetPackageCom(pData,nTimeOut);
	else if(gnDeviceType==DEVICE_UDisk)
		return GetPackageUDisk(pData,nLen);
	else
		return FALSE;
}

/**********************************************
  FUNCTION:   GetPackageLength
  PURPOSE:    �õ�����������ֵ
  ARGS:
       pData:  ��������ʼ��ַ
  RETURN:
       ����������ֵ       
***********************************************/
int WINAPI GetPackageLength(unsigned char* pData)
{
    // |  ����ʶ   |   ������	  |   ...{����}     |  У����    |
    // |  1 byte   |     2 bytes  |	  ...{������}   |  2 bytes   |

	if (!pData)
		return 0;
	int length = 0;
	length = pData[1]*256 + pData[2] + 1 +2;
	return length;
}

/**********************************************
  FUNCTION:   GetPackageContentLength
  PURPOSE:    �Ӱ���ָ����λ��ȡ�ð����ݳ���ֵ
  ARGS:
       pData:  ��������ʼ��ַ
  RETURN:
       �����ݳ���ֵ       
***********************************************/
int WINAPI GetPackageContentLength(unsigned char* pData)
{

	if (!pData)
		return 0;
	int length = 0;
	length = pData[1]*256 + pData[2];
	return length;
}

/**********************************************
  FUNCTION:   SendPackage
  PURPOSE:    ����
  ARGS:
       pData:  ָ�룬 Ҫ�������ݵ�ַ
  RETURN:
       0:    ʧ��
	   1:    �ɹ�
***********************************************/
BOOL WINAPI    SendPackageCom(int nAddr,unsigned char *pData)
{   
	int iLength;
	int iEncodedLength;
	unsigned char encodedBuf[MAX_PACKAGE_SIZE+20];
	BOOL bSuccess = FALSE;
	 
	DWORD    dwError;

	if (!pData)
		return FALSE;
        ClearCom();
 
	iLength = GetPackageLength(pData);  //�õ�������
	if (iLength>MAX_PACKAGE_SIZE)
		return FALSE;

	//����
	if ( !EnCode(nAddr,pData, iLength, encodedBuf, &iEncodedLength) )
		return FALSE;

	if (iEncodedLength > MAX_PACKAGE_SIZE)  //��������������
		return FALSE;

/*	if ( !ClearCommError(g_hCom, &dwError, &comstat) )
	{
		return FALSE;
	}*/

	//��������
	for (int i=0; i<iEncodedLength; i++)
	{
		if ( !SendByte(encodedBuf[i]) ) 
			return FALSE;
	} 
	
	return TRUE;
}

/**********************************************
  FUNCTION:   SendPackage
  PURPOSE:    ����
  ARGS:
       pData:  ָ�룬 Ҫ�������ݵ�ַ
  RETURN:
       0:    ʧ��
	   1:    �ɹ�
***********************************************/
BOOL WINAPI    SendPackageUSB(int nAddr,unsigned char *pData)
{   
	int iLength;
	int iEncodedLength;
	unsigned char encodedBuf[MAX_PACKAGE_SIZE+20];
	BOOL bSuccess = FALSE;
	 
	DWORD    dwError;

	if (!pData)
		return FALSE;

	iLength = GetPackageLength(pData);  //�õ�������
	if (iLength>MAX_PACKAGE_SIZE)
		return FALSE;

	//����
	if ( !EnCode(nAddr,pData, iLength, encodedBuf, &iEncodedLength) )
		return FALSE;

	if (iEncodedLength > MAX_PACKAGE_SIZE)  //��������������
		return FALSE;

	if(USBDownData(encodedBuf,iEncodedLength)) 
		 return FALSE;
	
	return TRUE;
}
BOOL WINAPI	SendPackageUDisk(int nAddr,unsigned char *pData)
{   
	//printf("----into SendPackageUDisk function----");
	int iLength;
	int iEncodedLength;
	unsigned char encodedBuf[MAX_PACKAGE_SIZE+20];
	BOOL bSuccess = FALSE;
	
	if (!pData)
	{
		printf("pDate is error\n");
		return FALSE;
	}
	
	iLength = GetPackageLength(pData);  //�õ�������
	if (iLength>MAX_PACKAGE_SIZE)
	{
		printf("iLength is too long\n");
		return FALSE;
	}
	
	//����
	if ( !EnCode(nAddr,pData, iLength, encodedBuf, &iEncodedLength) )
	{
		printf("Encode is error\n");
		return FALSE;
	}
	
	if (iEncodedLength > MAX_PACKAGE_SIZE)  //������������
	{
		printf("iEncodedLength is error\n");
		return FALSE;
	}
//mydata_prt(encodedBuf,iEncodedLength);	
	if(UDiskDownData(encodedBuf,iEncodedLength)) 
	{
		printf("UDiskDownDate is error\n");
		return FALSE;
	}
	
	return TRUE;
}
BOOL WINAPI SendPackage(int nAddr,unsigned char*pData)
{
	//printf("======into SendPackage function====\n");
	if(gnDeviceType==DEVICE_USB)
		return SendPackageUSB(nAddr,pData);
	else if(gnDeviceType==DEVICE_COM)
		return SendPackageCom(nAddr,pData);
	else if(gnDeviceType==DEVICE_UDisk)
		{
		return SendPackageUDisk(nAddr,pData);
		}
	else
		return FALSE;

}
/**********************************************
  FUNCTION:   FillPackage
  PURPOSE:    �������ݰ�
  ARGS:       
      pData:         Ҫ�����İ���ַ
	  nPackageType:  ������   
	  nLength:       ���ݰ����ݳ���
	  pContent:      ���ݰ�����
  RETURN:
      0:      ����
      length: ����������
***********************************************/
int WINAPI FillPackage(unsigned char* pData, int nPackageType, int nLength, unsigned char* pContent)
{
    // |  ����ʶ    |   ������	  |   ...{����}     |  
    // |  1 byte    |   2 bytes   |	  ...{������}   | 

	//printf("--------into FillPackage function-----\n");
	int totalLen = 0;
	long checksum = 0;
 
	if (!pData || nLength<0 || nLength>MAX_PACKAGE_SIZE )
	{
		printf("FillPackage error!\n");
		return 0;
	}
	if ( (nPackageType != CMD) && (nPackageType != DATA) && (nPackageType != ENDDATA) )
	{
		printf("FillPackage error\n");
		return 0;
	}
    nLength+=2;
	pData[0] = nPackageType;        //������
	pData[1] = (nLength>>8) & 0xff; //�����ݳ���
	pData[2] = nLength & 0xff;      //�����ݳ���
	if (nLength)
	{
		if (!pContent)
			return 0;
		memcpy(pData+3, pContent, nLength);
	}

	totalLen = nLength + 3;  
	//printf("----totalLEN is %d, FillPackage is end----\n", totalLen );

	return totalLen;
}

/**********************************************
  FUNCTION:   VerifyResposePackage
  PURPOSE:    ����Ӧ������Ч��
  ARGS:       
      pData:  ָ�룬Ӧ������ַ
  RETURN:
	  -3:     ����
      0:      ��ȷ
	  1-20:   ����
***********************************************/
int WINAPI VerifyResponsePackage(unsigned char nPackageType, unsigned char* pData)
{
	//printf("-----into VerifyResponsePackage function-----\n");
	 
	long checkSum = 0;

	if (!pData)
	{
		printf("pDate is error\n");
		return -3;
	}
	if ( pData[0] != nPackageType )
	{
		printf("pData is error\n");
		return -3;
	}

	int iLength;

	iLength = GetPackageLength(pData);  //�õ�������
     
	if (nPackageType == RESPONSE)
	{
		//printf("VerifyResponsePackage success\n");
		return pData[3];  //ȷ����
	}

	return 0;
}

int     WriteBMP(char* file,unsigned char* Input)
{
	
	unsigned char head[1078]={
		/***************************/
		//file header
	    	0x42,0x4d,//file type 
			//0x36,0x6c,0x01,0x00, //file size***
			0x0,0x0,0x0,0x00, //file size***
			0x00,0x00, //reserved
			0x00,0x00,//reserved
			0x36,0x4,0x00,0x00,//head byte***
			/***************************/
			//infoheader
			0x28,0x00,0x00,0x00,//struct size
			
			//0x00,0x01,0x00,0x00,//map width*** 
			0x00,0x00,0x0,0x00,//map width*** 
			//0x68,0x01,0x00,0x00,//map height***
			0x00,0x00,0x00,0x00,//map height***
			
			0x01,0x00,//must be 1
			0x08,0x00,//color count***
			0x00,0x00,0x00,0x00, //compression
			//0x00,0x68,0x01,0x00,//data size***
			0x00,0x00,0x00,0x00,//data size***
			0x00,0x00,0x00,0x00, //dpix
			0x00,0x00,0x00,0x00, //dpiy
			0x00,0x00,0x00,0x00,//color used
			0x00,0x00,0x00,0x00,//color important
			
	};
	
	FILE *fh;

	if( (fh  = fopen( file, "wb" )) == NULL )
           return 0;
 
	int i,j;

 

	long num;
	num=IMAGE_X; head[18]= num & 0xFF;
	num=num>>8;  head[19]= num & 0xFF;
	num=num>>8;  head[20]= num & 0xFF;
	num=num>>8;  head[21]= num & 0xFF;
	
	
	num=IMAGE_Y; head[22]= num & 0xFF;
	num=num>>8;  head[23]= num & 0xFF;
	num=num>>8;  head[24]= num & 0xFF;
	num=num>>8;  head[25]= num & 0xFF;
	

	
	j=0;
	for (i=54;i<1078;i=i+4)
	{
		head[i]=head[i+1]=head[i+2]=j; 
		head[i+3]=0;
		j++;
	}	
	
    fwrite(head,sizeof(char),1078,fh);
 	
	for(  i=0;i<=IMAGE_Y-1; i++ )
	{
 
		fseek(fh,1078*sizeof(char)+(IMAGE_Y-1-i)*IMAGE_X,SEEK_SET);
		 
		fwrite((Input+i*IMAGE_X),sizeof(char),IMAGE_X,fh );
	}
	
 
	fclose(fh);
	
	return 1;
}
void WINAPI Delay(int nTimes)
{
	dDelay(nTimes);
}
//����ͼ���ļ�
BOOL WINAPI MakeBMP(char* strFileName,unsigned char* pImgData)
{
    return WriteBMP(strFileName,pImgData);
}

//��ȡͼ������
BOOL WINAPI  GetDataFromBMP(const char* pFileName, unsigned char* pImageData)
{
	int num = 0;
	int nFileLength = 0;
	//unsigned char pData[IMAGE_X*IMAGE_Y];

	FILE* fp;
	if ( (fp = fopen(pFileName, "rb")) == NULL )
		return FALSE;

	fseek(fp, 1078, SEEK_SET);
	nFileLength = fread(pImageData, sizeof(char), IMAGE_X*IMAGE_Y, fp);

	fclose(fp);
	
	if ( nFileLength <IMAGE_X*IMAGE_Y)
		return FALSE;	
	return TRUE;
}

///////////////////////////////////////////////
//////             ָ��                  //////
///////////////////////////////////////////////

int WINAPI PSOpenDevice(int nDeviceType,int nPortNum,int nPortPara,int nPackageSize)
{
	//printf("*****into PSOpenDevice function******\n");
	printf("nDeviceType is %d\n", nDeviceType);
	gnDeviceType=nDeviceType;
	switch(nPackageSize)
	{	 
		case 0:
			g_nPackageSize=32;
			break;
		case 1:
			g_nPackageSize=64;
			break;
		case 2:
			g_nPackageSize=128;
			break;
		case 3:
			g_nPackageSize=256;
			break;
		default:g_nPackageSize=128;
	}
   if(nDeviceType==DEVICE_USB)
   {
	   return OpenUSB();
   }
   else if(nDeviceType==DEVICE_COM)
   {
	   return OpenCom(nPortNum,nPortPara);
   }
   else if(nDeviceType==DEVICE_UDisk)
   {
	   return OpenUDisk();
   }
   else
	   return 0;//FALSE
}

BOOL WINAPI PSCloseDevice()
{
	if(gnDeviceType==DEVICE_USB)
		return CloseUSB();
	else if(gnDeviceType==DEVICE_COM)
		return CloseCom();
	else if(gnDeviceType==DEVICE_UDisk)
	   return CloseUDisk();
	else
		return 0;//FALSE
}
/**********************************************
 
  PURPOSE:    �Ӵ�����¼��ָ��ͼ����
  ARGS:
        areaBorder[0]:  ����
        areaBorder[1]:  �Ͻ�
        areaBorder[2]:  �½�
        areaBorder[3]:  ����
        areaBorder[4]:  �ҽ�
  RETURN:
***********************************************/
int WINAPI      PSGetImage(int nAddr)
{
	int num;
	unsigned char cCmd[10];
	int result;
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	cCmd[0] = GET_IMAGE;  //������

    num = FillPackage(iSendData, CMD, 1, cCmd);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
        
		return -1;
        
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
  //  printf("result=%d\n",result);
	return result;
}

/**********************************************
 
  PURPOSE:    ����ָ�������ļ�
  ARGS:
      iBufferID:  ��������
  RETURN:
***********************************************/
int  WINAPI     PSGenChar(int nAddr,int iBufferID)
{
	unsigned char cCmd[10];
	int num;
	int result;
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	cCmd[0] = GEN_CHAR;  //ָ��
	cCmd[1] = iBufferID;   //��������

    num = FillPackage(iSendData, CMD, 2, cCmd);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
/*
	if (areaBorder)
	{
		areaBorder[0] = iGetData[HEAD_LENGTH+1];  //����
		areaBorder[1] = iGetData[HEAD_LENGTH+2];  //�Ͻ�
		areaBorder[2] = iGetData[HEAD_LENGTH+3];  //�½�
		areaBorder[3] = iGetData[HEAD_LENGTH+4];  //����
		areaBorder[4] = iGetData[HEAD_LENGTH+5];  //�ҽ�
	}*/

	return result;
} 

/**********************************************
 
  PURPOSE:    ��ȷ�ȶ���������е������ļ�
  ARGS:
  RETURN:
***********************************************/
int  WINAPI     PSMatch(int nAddr,int* iScore)
{
	unsigned char cCmd[10];
	int num;
	int result;
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	cCmd[0] = MATCH;     //ָ��

    num = FillPackage(iSendData, CMD, 1, cCmd);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	if (iScore!=NULL)
	{
		*iScore = iGetData[HEAD_LENGTH+1]<<8;
		*iScore |= iGetData[HEAD_LENGTH+2];
	}

	return result;
}

/**********************************************
 
  PURPOSE:    ����ָ��
  ARGS:
       iBufferID:  ������A��B
	   iStartPage: ������ʼҳ
	   iPageNum:   ����ҳ��
  RETURN:
***********************************************/
int  WINAPI     PSSearch(int nAddr,int iBufferID, int iStartPage, int iPageNum, int *iMbAddress)
{
	unsigned char cContent[10];
	int num;
	int result;
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	if( (iBufferID<1) || (iBufferID>3) ||(iStartPage<0) || (iPageNum<0) )
		return -4;

	cContent[0] = SEARCH;     //ָ��
	cContent[1] = iBufferID;   //��������
	cContent[2] = (iStartPage>>8) & 0xff;  
	cContent[3] = iStartPage & 0xff;
	cContent[4] = (iPageNum>>8) & 0xff;
	cContent[5] = iPageNum & 0xff;

    num = FillPackage(iSendData, CMD, 6, cContent);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	*iMbAddress = iGetData[HEAD_LENGTH+1]<<8;
	*iMbAddress |= iGetData[HEAD_LENGTH+2];
 
	return result;
}
int  WINAPI     PSHighSpeedSearch(int nAddr,int iBufferID, int iStartPage, int iPageNum, int *iMbAddress)
{
	unsigned char cContent[10];
	int num;
	int result;
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);
	
	if( (iBufferID<1) || (iBufferID>3) ||(iStartPage<0) || (iPageNum<0) )
		return -4;
	
	cContent[0] = HIGH_SPEED_SEARCH;     //ָ��
	cContent[1] = iBufferID;   //��������
	cContent[2] = (iStartPage>>8) & 0xff;  
	cContent[3] = iStartPage & 0xff;
	cContent[4] = (iPageNum>>8) & 0xff;
	cContent[5] = iPageNum & 0xff;
	
    num = FillPackage(iSendData, CMD, 6, cContent);  //�������ݰ�
	
	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;
	
	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	
	*iMbAddress = iGetData[HEAD_LENGTH+1]<<8;
	*iMbAddress |= iGetData[HEAD_LENGTH+2];
	
	return result;
}
/**********************************************
 
  PURPOSE:    ��CharBufferA��CharBufferB�е������ļ��ϲ���
              ����ģ�壬��������ModelBuffer��
  ARGS:
  RETURN:
***********************************************/
int  WINAPI     PSRegModule(int nAddr)
{
	int num;
	int result;
	unsigned char cCmd[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	cCmd[0] = REG_MODULE;     //ָ��

    num = FillPackage(iSendData, CMD, 1, cCmd);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	return result;
}

/**********************************************
 
  PURPOSE:    ��BufferID�е�ģ���ļ��浽PageID��flash���ݿ�λ��
  ARGS:
       BufferID: ��������
	   PageID:   ָ�ƿ�λ�ú�
  RETURN:
***********************************************/
int  WINAPI     PSStoreChar(int nAddr,int iBufferID, int iPageID)
{
	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	if( (iBufferID<1) || (iBufferID>3) || (iPageID<0) )
		return -4;

	cContent[0] = STORE_CHAR;     //ָ��
	cContent[1] = iBufferID;   
	cContent[2] = (iPageID>>8) & 0xff;
	cContent[3] = iPageID & 0xff;  

    num = FillPackage(iSendData, CMD, 4, cContent);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	return result;
}	

/**********************************************
 
  PURPOSE:    ��flash���ݿ���ָ��ID�ŵ�ָ��ģ�����뵽ģ�建����ModelBuffer
  ARGS:
		iBufferID:����������
		PageID: ָ�ƿ�ģ����
  RETURN:
***********************************************/
int  WINAPI     PSLoadChar(int nAddr,int iBufferID,int iPageID)
{
	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	if( (iBufferID<1) || (iBufferID>3) || (iPageID<0) )
		return -4;

	cContent[0] = LOAD_CHAR;     //ָ��
	cContent[1] = iBufferID;	//����������
	cContent[2] = (iPageID>>8) & 0xff;
	cContent[3] = iPageID & 0xff;  

    num = FillPackage(iSendData, CMD, 4, cContent);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	return result;
}



/**********************************************
 
  PURPOSE:    �����������е������ļ��ϴ�����λ��
  ARGS:
      iBufferID:        �������ţ�1��2��3
	  pTemplet:         �洢ͼ�����ڴ�ָ��
	  iTempletLength:   ���ݳ���
  RETURN:
***********************************************/
int WINAPI      PSUpChar(int nAddr,int iBufferID, unsigned char* pTemplet, int* iTempletLength)
{
	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	if( (iBufferID<1) || (iBufferID>3) || !pTemplet || !iTempletLength )
		return -4;

	cContent[0] = UP_CHAR;     //ָ��
	cContent[1] = iBufferID;

    num = FillPackage(iSendData, CMD, 2, cContent);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;

	if(gnDeviceType==DEVICE_USB)
	{
		*iTempletLength=512;
		return USBGetData(pTemplet,512);
	}
	else if(gnDeviceType==DEVICE_UDisk)
	{
		*iTempletLength=512;
		return UDiskGetData(pTemplet,512);
	}

	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	if ( result != 0 )
	    return result;

	if(gnDeviceType==DEVICE_USB)
	{
		*iTempletLength=512;
		return USBGetData(pTemplet,512);
	}
	else if(gnDeviceType==DEVICE_UDisk)
	{
		*iTempletLength=512;
		return UDiskGetData(pTemplet,512);
	}

	//����������ݰ�ֱ���յ�����һ�����ݰ�
	int length;
	int totalLen = 0;
	int checkResult = 0;
	DWORD         dwStart, dwStop;
	
	dwStart = GetTickCount();
	do
	{
		memset(iGetData, 0, MAX_PACKAGE_SIZE);
	    if ( !GetPackage(iGetData) )
		    return -2;

	    length = GetPackageContentLength(iGetData)-2;
		result = VerifyResponsePackage(iGetData[0], iGetData);
		if ( result != 0 )
			checkResult = -3;

	    memcpy((void*)(pTemplet+totalLen), (void*)(iGetData+HEAD_LENGTH), length);
		
		totalLen += length;

		dwStop = GetTickCount();
		if ( (dwStop-dwStart) > APP_TIME_OUT )
			return -2;
	}
	while ( iGetData[0] != ENDDATA );
	
	//���ݰ�����
	*iTempletLength = totalLen;

	if ( *iTempletLength == 0 )
		return -2;

	return checkResult;//����ִ�гɹ�
}

 

/**********************************************
 
  PURPOSE:    ����λ�����������ļ���ģ����һ����������
  ARGS:
      iBufferID:        �������ţ�1��2��3
	  pTemplet:         �洢ͼ�����ڴ�ָ��
	  iTempletLength:   ���ݳ���
  RETURN:
***********************************************/
int  WINAPI     PSDownChar(int nAddr,int iBufferID, unsigned char* pTemplet, int iTempletLength)
{
	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	if( (iBufferID<1) || (iBufferID>3) || !pTemplet || (iTempletLength<=0) )
		return -4;

	cContent[0] = DOWN_CHAR;     //ָ��
	cContent[1] = iBufferID;

    num = FillPackage(iSendData, CMD, 2, cContent); //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;

	if(gnDeviceType==DEVICE_USB)
		return USBDownData1(pTemplet,512);
	else if(gnDeviceType==DEVICE_UDisk)
		return UDiskDownData(pTemplet,512);

	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	if ( result != 0 )
	    return result;

	if(gnDeviceType==DEVICE_USB)
		return USBDownData(pTemplet,512);
	else if(gnDeviceType==DEVICE_UDisk)
		return UDiskDownData(pTemplet,512);

	//����������ݰ�
	int totalLen = iTempletLength;
	unsigned char* pData = pTemplet;

	while (totalLen > g_nPackageSize)
	{
		memset(iSendData, 0, MAX_PACKAGE_SIZE);
	    num = FillPackage(iSendData, DATA, g_nPackageSize, pData);  //�������ݰ�

		pData += g_nPackageSize;
		totalLen -= g_nPackageSize;

	    if ( !SendPackage(nAddr,iSendData) )
		    return -1;
	}

	//��������һ�����ݰ�
	
	memset(iSendData, 0, MAX_PACKAGE_SIZE);
    num = FillPackage(iSendData, ENDDATA, totalLen, pData); //�������ݰ�

    if ( !SendPackage(nAddr,iSendData) )
	    return -1;
	
	return 0;//����ִ�гɹ�
}
/**********************************************
 
  PURPOSE:    �����������е������ļ��ϴ�����λ��
  ARGS:
      iBufferID:        �������ţ�1��2��3
	  pFileName:        �ļ���
  RETURN:
***********************************************/
int   WINAPI    PSUpChar2File(int nAddr,int iBufferID, const char* pFileName)
{
	unsigned char pTemplate[2048];
	int nFileLength = 0;
	int result;
	
	memset(pTemplate, 0, 2048);
	result = PSUpChar(nAddr,iBufferID, pTemplate, &nFileLength);
	if (result!=0)
		return result;

	FILE* fp;
	if ( (fp = fopen(pFileName, "wb")) == NULL )
		return -6;

	if ( (result = fwrite(pTemplate, 1, nFileLength, fp)) != nFileLength )
	{
		fclose(fp);
		return -8;
	}

	fclose(fp);

	return 0;
}

/**********************************************
 
  PURPOSE:    ����λ�����������ļ���ģ����һ����������
  ARGS:
      iBufferID:        �������ţ�1��2��3
	  pFileName:        �ļ���
  RETURN:
***********************************************/
int  WINAPI     PSDownCharFromFile(int nAddr,int iBufferID, const char* pFileName)
{
	unsigned char pTemplate[2048];
	int nFileLength = 0;
	int result;
	int num = 0;
	
	memset(pTemplate, 0, 2048);

	FILE* fp;
	if ( (fp = fopen(pFileName, "rb")) == NULL )
		return -7;

	while ( !feof(fp) ) 
	{
		num = fread(pTemplate+nFileLength, sizeof(char), 1024, fp);
		nFileLength += num;
	}
	fclose(fp);

	result = PSDownChar(nAddr,iBufferID, pTemplate, nFileLength);

	return result;
} 

/**********************************************
 
  PURPOSE:    ��ͼ�񻺳����е������ϴ�����λ��
  ARGS:
      pImageData:  ͼ�󻺳�����
  RETURN:
***********************************************/
int   WINAPI    PSUpImage(int nAddr,unsigned char* pImageData, int* iImageLength)
{
	int num;
	int result;
	unsigned char cCmd[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	if( !pImageData )
		return -1;

	cCmd[0] = UP_IMAGE;     //ָ��

    num = FillPackage(iSendData, CMD, 1, cCmd);  //�������ݰ�
		
	if( !SendPackage(nAddr,iSendData) ) 
		return -1;

	if(gnDeviceType==DEVICE_USB)
	{
		*iImageLength=IMAGE_X*IMAGE_Y;
		return USBGetImage(pImageData,IMAGE_Y*IMAGE_X);
	}
	else if(gnDeviceType==DEVICE_UDisk)
	{
		*iImageLength=IMAGE_X*IMAGE_Y;
		return UDiskGetImage(pImageData,IMAGE_Y*IMAGE_X);
	}

	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	if ( result != 0 )
	    return result;



	//����������ݰ�ֱ���յ�����һ�����ݰ�
	int length;
	int totalLen = 0;
	int checkResult = 0;
	int PacketNum=0;

	DWORD         dwStart, dwStop;
	 
	DWORD    dwError;
  
	
	do
	{
		memset(iGetData, 0, MAX_PACKAGE_SIZE);
	    if ( !GetPackage(iGetData) )
		    return -2;

	    length = GetPackageContentLength(iGetData)-2;
		result = VerifyResponsePackage(iGetData[0], iGetData);  //У��Ӧ����
		if ( result != 0 )
			checkResult = -2;

	     memcpy((void*)(pImageData+totalLen), (void*)(iGetData+HEAD_LENGTH), length);
		totalLen += length;
		PacketNum++;
	}
	while ( iGetData[0] != ENDDATA );

	*iImageLength = totalLen;
	
    return checkResult;//����ִ�гɹ�
}	

/**********************************************
  
  PURPOSE:    ��λ������ͼ�����ݸ�ģ��
  ARGS:
  RETURN:
***********************************************/
int  WINAPI     PSGetImgDataFromBMP(const char *pImageFile,unsigned char *pImageData,int *pnImageLen)
{

	if(gnDeviceType==DEVICE_USB || gnDeviceType==DEVICE_UDisk)
	{
        GetDataFromBMP(pImageFile, pImageData);
		return 0;
	}
    unsigned char* pData;
	unsigned char*pNewData;
	pData=new unsigned char[IMAGE_X*IMAGE_Y];
	pNewData=new unsigned char[IMAGE_X*IMAGE_Y];
	 
	
	BOOL ret = GetDataFromBMP(pImageFile, pData);
	if (!ret)
	{
		delete[] pData;
		delete[] pNewData;
		return -7;
	}
	for(int n=0;n<IMAGE_Y;n++)
	{
		memcpy(pNewData+n*IMAGE_X,pData+(IMAGE_Y-1-n)*IMAGE_X,IMAGE_X);
	}

	for(int i=0;i<IMAGE_Y*IMAGE_X/2;i++)
	{
		*(pImageData+i)=(((pNewData[2*i]/16)<<4)&0xf0)+((pNewData[2*i+1]/16)&0x0f);
	}

	*pnImageLen = IMAGE_Y*IMAGE_X/2; 
    delete[] pData;
	delete[] pNewData;
	return 0;
}
/**********************************************
  
  PURPOSE:    ��ͼ�񻺳����е������ϴ�����λ��
  ARGS:
      pImageFile:  ͼ���ļ���
  RETURN:
***********************************************/
int  WINAPI     PSImgData2BMP(unsigned char* pImgData,const char* pImageFile)
{
	unsigned char newData[IMAGE_X*IMAGE_Y]; 
	if(gnDeviceType==DEVICE_USB)
	{
          MakeBMP((char*)pImageFile, pImgData);
		  return 0;
	}

    

	for(int i=0;i<IMAGE_X*IMAGE_Y/2;i++)
	{
		newData[i*2]=(*(pImgData+i)>>4)*16;
		newData[i*2+1]=(*(pImgData+i)&0x0f)*16;
	}

	BOOL ret = MakeBMP((char*)pImageFile, newData);

	 
	if ( !ret )
		return -6;

	return 0;
}

/**********************************************
  FUNCTION:   CmdDownImage
  PURPOSE:    ��λ������ͼ�����ݸ�ģ��
  ARGS:
  RETURN:
***********************************************/
int  WINAPI     PSDownImage(int nAddr,unsigned char *pImageData, int iLength)
{
	int num;
	int result;
	unsigned char cCmd[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
        unsigned char pNewData[IMAGE_X*IMAGE_Y];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	if( !pImageData )
		return -2;

	cCmd[0] = DOWN_IMAGE;     //ָ��

    num = FillPackage(iSendData, CMD, 1, cCmd);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;

	if(gnDeviceType==DEVICE_USB || gnDeviceType==DEVICE_UDisk)
	{
		for(int n=0;n<IMAGE_Y;n++)
		{
			memcpy(pNewData+n*IMAGE_X,pImageData+(IMAGE_Y-1-n)*IMAGE_X,IMAGE_X);
		}
		if(gnDeviceType==DEVICE_USB)
			return	USBDownImage(pNewData,0x12000);
		else if(gnDeviceType==DEVICE_UDisk)
			return  UDiskDownImage(pNewData,0x12000);
	}

	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	if ( result != 0 )
	    return result;

 

	//����������ݰ�
	int totalLen = iLength;
	unsigned char* pData = pImageData;

	while (totalLen > g_nPackageSize)
	{
		memset(iSendData, 0, MAX_PACKAGE_SIZE);
	    num = FillPackage(iSendData, DATA, g_nPackageSize, pData);  //�������ݰ�

		pData += g_nPackageSize;
		totalLen -= g_nPackageSize;

	    if ( !SendPackage(nAddr,iSendData) )
		    return -1;
	}

	//��������һ�����ݰ�
	
	memset(iSendData, 0, MAX_PACKAGE_SIZE);
    num = FillPackage(iSendData, ENDDATA, totalLen, pData);  //�������ݰ�

    if ( !SendPackage(nAddr,iSendData) )
	    return -1;
	
	return 0;//����ִ�гɹ�
}


/**********************************************
 
  PURPOSE:    ɾ��һ��ģ�� 
  ARGS:
      PageID: ָ�ƿ�ģ����
  RETURN:
***********************************************/
int  WINAPI     PSDelChar(int nAddr,int iStartPageID,int nDelPageNum)
{
	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	if((iStartPageID<0) || nDelPageNum<=0)
		return -4;

	cContent[0] = DEL_CHAR;     //ָ��
	cContent[1] = (iStartPageID>>8) & 0xff;
	cContent[2] = iStartPageID & 0xff;  
	cContent[3] = (nDelPageNum>>8) &0xff;
	cContent[4] = nDelPageNum & 0xff;

    num = FillPackage(iSendData, CMD, 5, cContent);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	return result;
}

/**********************************************
  FUNCTION:  CmdEraseAllTemplet 
  PURPOSE:   ɾ��flash���ݿ�������ָ��ģ�� 
  ARGS:
  RETURN:
***********************************************/
int  WINAPI     PSEmpty(int nAddr)
{
	int num;
	int result;
	unsigned char cCmd[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	cCmd[0] = EMPTY;  

    num = FillPackage(iSendData, CMD, 1, cCmd);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	return result;
}

/**********************************************
   
  PURPOSE:    ��ȡģ���Ĳ����� 64�ֽ�
  ARGS:
  RETURN:
***********************************************/
int  WINAPI     PSReadParTable(int nAddr,unsigned char* pParTable)
{
	int num;
	int result;
	unsigned char cCmd[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	if (!pParTable)
		return -2;

	cCmd[0] = READ_PAR_TABLE;  

    num = FillPackage(iSendData, CMD, 1, cCmd);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	if ( result != 0 )
		return result;

	memcpy((void*)pParTable, (void*)(iGetData+HEAD_LENGTH+1), 16);  //������16�ֽ�

    return 0;
}
 
 
int  WINAPI     PSEnroll(int nAddr,int* nID)
{
	int num;
	int result;
	unsigned char cCmd[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	cCmd[0] = ENROLL;  

    num = FillPackage(iSendData, CMD, 1, cCmd);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	*nID = iGetData[HEAD_LENGTH+1]<<8;
	*nID |= iGetData[HEAD_LENGTH+2];
 

    return result;
}
 

/**********************************************
 
  PURPOSE:    ����ģ�����ֿ���
  ARGS:
      pPassword:  ����
  RETURN:
***********************************************/
int  WINAPI     PSSetPwd(int nAddr,unsigned char* pPassword)
{
	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	cContent[0] = SET_PWD;     //ָ��
	cContent[1] = pPassword[0];
	cContent[2] = pPassword[1];
	cContent[3] = pPassword[2];
	cContent[4] = pPassword[3];

    num = FillPackage(iSendData, CMD, 5, cContent);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	return result;
}

/**********************************************
 
  PURPOSE:    ��֤ģ�����ֿ���
  ARGS:
      pPassword:  ����
  RETURN:
***********************************************/
int  WINAPI    PSVfyPwd(int nAddr,unsigned char* pPassword)
{
	//printf("=========PSVfyPwd is begin=======\n");
	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	cContent[0] = VFY_PWD;     //ָ��
	cContent[1] = pPassword[0];
	cContent[2] = pPassword[1];
	cContent[3] = pPassword[2];
	cContent[4] = pPassword[3];
	//printf("cContent is %s", cContent);

    num = FillPackage(iSendData, CMD, 5, cContent);  //�������ݰ�
	//printf("FillPackage return num is %d\n", num);

	if( !SendPackage(nAddr,iSendData) ) 
	{
		printf("SendPackage function is error!\n");
		return -1;
	}
	if( !GetPackage(iGetData,64,1000) )  
	{
		printf("GetPackage function is error!\n");
		return -2;
	}


	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	//printf("VerifyResponsePackage return is %d\n", result);

	return result;
}	

/**********************************************
  FUNCTION:   CmdReset
  PURPOSE:    ģ����λ������ϵͳ��ʼ����
              ��λ���յ�Ӧ�����������ٵ�100ms�����ٷ�ָ��������������֤�豸���ֿ��
  ARGS:
  RETURN:
***********************************************/
int  WINAPI     PSIdentify(int nAddr,int *iMbAddress)
{
	int num;
	int result;
	unsigned char cCmd[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	cCmd[0] = IDENTIFY;     //ָ��

    num = FillPackage(iSendData, CMD, 1, cCmd);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	*iMbAddress = iGetData[HEAD_LENGTH+1]<<8;
	*iMbAddress |= iGetData[HEAD_LENGTH+2];

	return result;
}
 
/**********************************************
 
  PURPOSE:    д���±�
  ARGS:
	  UserContent:  �û���Ϣ
  RETURN:
***********************************************/
int	 WINAPI	 PSWriteInfo(int nAddr,int nPage,unsigned char* UserContent)
{
	int num;
	int result;
	unsigned char cContent[256];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);
	
	if (!UserContent)
		return -1;
	
	memset(cContent, 0, 10);
	cContent[0] = WRITE_NOTEPAD;  
	cContent[1] = nPage;
	memcpy((void*)(cContent+2), (void*)UserContent, 32);  //�û���Ϣ32���ֽ�
	
    num = FillPackage(iSendData, CMD, 34, cContent);  //�������ݰ�
	
	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;
	
	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	return result;

}
/**********************************************
 
  PURPOSE:    ����±�
  ARGS:
	  UserContent:  �û���Ϣ
  RETURN:
***********************************************/
int	 WINAPI	 PSReadInfo(int nAddr,int nPage,unsigned char* UserContent)
{
	int num;
	int result;
	unsigned char cContent[2];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[1024];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);
	
	if (!UserContent)
		return -1;
	
	cContent[0] = READ_NOTEPAD;  
	cContent[1] = nPage; 
	
    num = FillPackage(iSendData, CMD, 2, cContent);  //�������ݰ�
	
	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData,512) )  
		return -2;
	
	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	if(result!=0)
		return result;
	memcpy(UserContent,&iGetData[HEAD_LENGTH+1],32);
	return result;

}

/**********************************************
   
  PURPOSE:    д���ݵ���������ĳ���Ĵ���
  ARGS:
      iRegAddr:   ��������ַ
	  iRegValue:  ֵ
  RETURN:
***********************************************/
int   WINAPI    PSWriteReg(int nAddr,int iRegAddr,int iRegValue)
{
	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);

	cContent[0] = WRITE_REG;     //ָ��
	cContent[1] = iRegAddr;
	cContent[2] = iRegValue;

    num = FillPackage(iSendData, CMD, 3, cContent);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����

	return result;
}

int WINAPI PSSetBaud(int nAddr,int nBaudNum)
{
	return PSWriteReg(nAddr,REG_BAUD,nBaudNum);
}

int WINAPI PSSetSecurLevel(int nAddr,int nLevel)
{
	return PSWriteReg(nAddr,REG_SECURE_LEVEL,nLevel);
}

int WINAPI PSSetPacketSize(int nAddr,int nSize)
{
	int nRet=PSWriteReg(nAddr,REG_PACKETSIZE,nSize);
	if(nRet==0)
	{
		switch(nSize) {
		case 0:
			g_nPackageSize=32;
			break;
		case 1:
			g_nPackageSize=64;
			break;
		case 2:
			g_nPackageSize=128;
			break;
		case 3:
			g_nPackageSize=256;
			break;
		default:g_nPackageSize=128;
		}
	}
	return nRet;

}

int WINAPI PSGetRandomData(int nAddr,unsigned char* pRandom)
{
	int num;
	int result;
	unsigned char cContent[2];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);
	
	if (!pRandom)
		return -1;
	
	cContent[0] = GET_RANDOM;  
	
    num = FillPackage(iSendData, CMD, 1, cContent);  //�������ݰ�
	
	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;
	
	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	if(result!=0)
		return result;
	memcpy(pRandom,&iGetData[HEAD_LENGTH+1],4);
	return result;
}

int WINAPI PSSetChipAddr(int nAddr,unsigned char* pChipAddr)
{
	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);
	
	cContent[0] = SET_CHIPADDR;     //ָ��
	cContent[1] = pChipAddr[0];
	cContent[2] = pChipAddr[1];
	cContent[3] = pChipAddr[2];
	cContent[4] = pChipAddr[3];
	
    num = FillPackage(iSendData, CMD, 5, cContent);  //�������ݰ�
	
	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;
	
	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	
	return result;
}

int WINAPI PSPortControl(int nAddr,BOOL bOpen)
{
	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);
	
	cContent[0] = PORT_CONTROL;     //ָ��
	cContent[1] = bOpen;
 
	
    num = FillPackage(iSendData, CMD, 2, cContent);  //�������ݰ�
	
	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;
	
	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	
	return result;
}

int WINAPI      PSReadInfPage(int nAddr, unsigned char* pInf)
{
	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);
	
 
	cContent[0] = READ_INFPAGE;     //ָ��
 
	
    num = FillPackage(iSendData, CMD, 1, cContent);  //�������ݰ�
	
	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	
	if(gnDeviceType==DEVICE_USB)
	{
		return USBGetData(pInf,512);
	}
	else if(gnDeviceType==DEVICE_UDisk)
	{
		return UDiskGetData(pInf,512);
	}
	
	if( !GetPackage(iGetData) )  
		return -2;
	
	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	
	if ( result != 0 )
		return result;
	
	if(gnDeviceType==DEVICE_USB)
	{ 
		return USBGetData(pInf,512);
	}
	else if(gnDeviceType==DEVICE_UDisk)
	{
		return UDiskGetData(pInf,512);
	}
	
	//����������ݰ�ֱ���յ�����һ�����ݰ�
	int length;
	int totalLen = 0;
	int checkResult = 0;
	int dwStart, dwStop;
	
	dwStart = GetTickCount();
	do
	{
		memset(iGetData, 0, MAX_PACKAGE_SIZE);
		if ( !GetPackage(iGetData) )
			return -2;
		
		length = GetPackageContentLength(iGetData)-2;
		result = VerifyResponsePackage(iGetData[0], iGetData);
		if ( result != 0 )
			checkResult = -3;
		
		memcpy((void*)(pInf+totalLen), (void*)(iGetData+HEAD_LENGTH), length);
		
		totalLen += length;
		
		dwStop = GetTickCount();
		if ( (dwStop-dwStart) > APP_TIME_OUT )
			return -2;
	}
	while ( iGetData[0] != ENDDATA );
	
 
	
	if ( totalLen == 0 )
		return -2;
	
	return checkResult;//����ִ�гɹ�
}


int WINAPI PSGenBinImage(int nAddr, int nImgType)
{
   	int num;
	int result;
	unsigned char cContent[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);
 
	
	cContent[0] = GEN_BINIMAGE;     //ָ��
	cContent[1] = nImgType;	//����������
 
	
    num = FillPackage(iSendData, CMD, 2, cContent);  //�������ݰ�
	
	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;
	
	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	
	return result;
}

int  WINAPI     PSBurnCode(int nAddr,int nType,unsigned char *pImageData, int iLength)
{
	int num;
	int result;
        unsigned char Temp;
	unsigned char cCmd[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE); 

	if( !pImageData )
		return -2; 

	cCmd[0] = BURN_CODE;     //ָ��
	cCmd[1] = nType;

    num = FillPackage(iSendData, CMD, 2, cCmd);  //�������ݰ�

	if( !SendPackage(nAddr,iSendData) ) 
		return -1;

	 

	for(int i=0;i<iLength/2;i++)
	{
		
		Temp=*(pImageData+i*2);
        *(pImageData+i*2)=*(pImageData+i*2+1);
		*(pImageData+i*2+1)=Temp;
	}

	if(gnDeviceType==DEVICE_USB || gnDeviceType==DEVICE_UDisk)
	{       
		unsigned char pNewData[IMAGE_X*IMAGE_Y]={0};
		memcpy(pNewData,pImageData,iLength);
		if(gnDeviceType==DEVICE_USB)
			result = USBDownImage(pNewData,0x12000);
		else if(gnDeviceType==DEVICE_UDisk)
			result = UDiskDownImage(pNewData,0x12000);
		else
			result = -1;
		//Sleep(5000);//��ʱ��дFlash�����Ҫ��ʱһ��ʱ��
		return result;
	}

	if( !GetPackage(iGetData) )  
		return -2;

	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ���

	if ( result != 0  && result!=0xf0)
	    return result;

 

	//����������ݰ�
	int totalLen = iLength;
	unsigned char* pData = pImageData;

	while (totalLen > g_nPackageSize)
	{
		memset(iSendData, 0, MAX_PACKAGE_SIZE);
	    num = FillPackage(iSendData, DATA, g_nPackageSize, pData);  //�������ݰ�

		pData += g_nPackageSize;
		totalLen -= g_nPackageSize;

	    if ( !SendPackage(nAddr,iSendData) )
		    return -1;
	}

	//��������һ�����ݰ�
	
	memset(iSendData, 0, MAX_PACKAGE_SIZE);
    num = FillPackage(iSendData, ENDDATA, totalLen, pData);  //�������ݰ�

    if ( !SendPackage(nAddr,iSendData) )
	    return -1;
	
	return 0;//����ִ�гɹ�
}
int  WINAPI     PSTemplateNum(int nAddr,int *iMbNum)
{
	int num;
	int result;
	unsigned char cCmd[10];
	unsigned char iSendData[MAX_PACKAGE_SIZE], iGetData[MAX_PACKAGE_SIZE];
	memset(iSendData,0,MAX_PACKAGE_SIZE);
	memset(iGetData,0,MAX_PACKAGE_SIZE);
	
	cCmd[0] = TEMPLATE_NUM;     //ָ��
	
    num = FillPackage(iSendData, CMD, 1, cCmd);  //�������ݰ�
	
	if( !SendPackage(nAddr,iSendData) ) 
		return -1;
	if( !GetPackage(iGetData) )  
		return -2;
	
	result = VerifyResponsePackage(RESPONSE, iGetData);  //У��Ӧ����
	*iMbNum = iGetData[HEAD_LENGTH+1]<<8;
	*iMbNum |= iGetData[HEAD_LENGTH+2];
	
	return result;
}

/**********************************************
 
  PURPOSE:    ���ش�����Ϣ
  ARGS:       
      nErrCode:   ������Ϣ����
  RETURN:
      ������Ϣ����
***********************************************/
char*  WINAPI       PSErr2Str(int nErrCode)
{
	switch (nErrCode)
	{
	case -1:
		strcpy(g_sErrorString, "Send package error");//�������ݰ�ʧ��
		break;
	case -2:
		strcpy(g_sErrorString, "Receive package error"); //�������ݰ�ʧ��
		break;
	case -3:
		strcpy(g_sErrorString, "Sum error");
		break;
	case -4:
		strcpy(g_sErrorString, "Buffer ID outside or Invalid flash templet ID"); //�������ų�����Χ��flashģ������Ч
		break;
	case -5:
		strcpy(g_sErrorString, "Invalid security level"); //��ȫ�ȼ���Ч
		break;
	case -6:
		strcpy(g_sErrorString, "Create specified file error");//����ָ���ļ�ʧ��
		break;
	case -7:
		strcpy(g_sErrorString, "Specified file is not exisit"); //ָ���ļ�������
		break;
	case -8:
		strcpy(g_sErrorString, "Invalid file size"); //�ļ���С���Ϸ�
		break;
	case -9:
		strcpy(g_sErrorString, "Allocate memory error");//�ڴ�����ʧ��
		break;
	
	case 0:
		strcpy(g_sErrorString, "Execute success");//ִ�гɹ�
		break;
	case 1:
		strcpy(g_sErrorString, "Receive package error");//���ݰ����մ���
		break;
	case 2:
		strcpy(g_sErrorString, "There is no finger on the sensor");  //��������û����ָ
		break;
	case 3:
		strcpy(g_sErrorString, "Enroll finger print image error"); //¼��ָ��ͼ��ʧ��
		break;
	case 4:
		strcpy(g_sErrorString, "Too thin fingerprint");//ָ��̫��
		break;
	case 5:
		strcpy(g_sErrorString, "Too faint fingerprint");//ָ��̫��
		break;
	case 6:
		strcpy(g_sErrorString, "Too disorderly fingerprint");//ָ��̫��
		break;
	case 7:
		strcpy(g_sErrorString, "Too little fingerprint character point ");//ָ��������̫��
		break;
	case 8:
		strcpy(g_sErrorString, "No matching fingerprint");//ָ�Ʋ�ƥ��
		break;
	case 9:
		strcpy(g_sErrorString, "No fingerprint searched"); //û������ָ��
		break;
	case 10:
		strcpy(g_sErrorString, "Combine character error");//�����ϲ�ʧ��
		break;
	case 11:
		strcpy(g_sErrorString, "Out of range");
		break;
	case 12:
		strcpy(g_sErrorString, "Read templet from fingerprint library error");//��ָ�ƿ���ģ������
		break;
	case 13:
		strcpy(g_sErrorString, "Upload character failed");//�ϴ�����ʧ��
		break;
	case 14:
		strcpy(g_sErrorString, "Module can't receive succedent package"); //ģ�鲻�ܽ��պ������ݰ�
		break;
	case 15:
		strcpy(g_sErrorString, "Upload image failed");//�ϴ�ͼ��ʧ��
		break;
	case 16:
		strcpy(g_sErrorString, "Delete templet failed ");//ɾ��ģ��ʧ��
		break;
	case 17:
		strcpy(g_sErrorString, "Clear fingerprint failed"); //����ָ�ƿ�ʧ��
		break;
	case 18:
		strcpy(g_sErrorString, "Can not into sleep status");
		break;
	case 19:
		strcpy(g_sErrorString, "Password error"); //�����ȷ
		break;
	case 20:
		strcpy(g_sErrorString, "Reset system error");//ϵͳ��λʧ��
		break;
	case 21:
		strcpy(g_sErrorString, "Invalid fingerprint image");//��Чָ��ͼ��
		break;
	case 22:
		strcpy(g_sErrorString, "Update online failed"); //��������ʧ��
		break;
	case 23:
		strcpy(g_sErrorString, "Need move finger");
		break;
	case 24:
		strcpy(g_sErrorString, "Indicate that read or write Flash error"); //��ʾ��дFLASH����
		break;
	case 25:
		strcpy(g_sErrorString, "Undefined error"); //δ��������
		break;
	case 26:
		strcpy(g_sErrorString, "Invalid register number"); //��Ч�Ĵ�����
		break;
	case 27:
		strcpy(g_sErrorString, "Register set error");
		break;
	case 28:
		strcpy(g_sErrorString, "Specify notepad page error "); //���±�ҳ��ָ������
		break;
	case 29:
		strcpy(g_sErrorString, "Operate port error"); //�˿ڲ���ʧ��
		break;
	case 30:
		strcpy(g_sErrorString, "Enroll automatically error"); //�Զ�ע�ᣨenroll��ʧ��
		break;
	case 31:
		strcpy(g_sErrorString, "Fingerprint library is full"); //ָ�ƿ���
		break;
	 
	default:
		strcpy(g_sErrorString, "Unknown error"); //δ֪����
		break;
	}

	return g_sErrorString;

}


int WINAPI PSUserMatch(unsigned char *SrcFile,unsigned char *DstFile,unsigned char SecuLevel)
{
	int iRet;
//    iRet = UserMatch(SrcFile,DstFile,SecuLevel);
	return iRet;
	//return UserMatch(SrcFile,DstFile,SecuLevel);
}
