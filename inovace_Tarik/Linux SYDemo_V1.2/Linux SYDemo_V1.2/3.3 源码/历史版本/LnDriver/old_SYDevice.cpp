 
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "SYDevice.h"
#include "pusb.h"

#include "serial.h"

#include "pusb-linux.h"

//ԐȽ
#define ST_VENDOR  0x0453 //0x0fbc
#define ST_PRODUCT 0x9005//0x3090



//vendor=0204 ProdID=6025
#define SU_VENDOR 0x2109    //0x0fbc
#define SU_PRODUCT 0x7638    //0x3090

//ϞȽ
//#define SU_VENDOR  0x0fbc	
//#define SU_PRODUCT 0x3090
//#define ST_VENDOR  0x0AEC
//#define ST_PRODUCT 0x3080

//#define ST_VENDOR  0x071B
//#define ST_PRODUCT 0x3102
////////////////////////////////////////////////
/////           ���ڲ���                  //////
////////////////////////////////////////////////

 int       g_nComPort     = 0;     //���ں�
 int       g_nBaudRate    = 57600; //������

const int   WAITTIME_PER_BYTE =  100;   //��ÿ���ֽ�ʱ��(ms)
static pusb_device_t fdusb = NULL;
/**********************************************
  FUNCTION:   InitCom
  PURPOSE:    ��ʼ������
  ARGS:
       comPort:    ���ں�,��1��8
	   baudRate:   ������,0-9600,1-19200,2-38400   3-57600  4-115200  ȱʡΪ3-57600
  RETURN:
       0:    ʧ��
	   1:    �ɹ�
***********************************************/
BOOL   OpenCom(int comPort, int baudRate)
{
	int baud;
 
	baud=baudRate*9600; 


	//��¼������Ϣ
	g_nBaudRate = baud;
	g_nComPort = comPort;

	if( OpenComPort(comPort,baud,8,"2",0)==0)
       {
                ClearComPort();
                
		 return TRUE;
       }

	return FALSE;
}

/**********************************************
  FUNCTION:   ReleaseCom
  PURPOSE:    �رմ���
***********************************************/
BOOL   CloseCom()
{
	CloseComPort();
	return TRUE;
}
 
void ClearCom()
{
  ClearComPort();
}

/**********************************************
  FUNCTION:   GetByte
  PURPOSE:    �Ӵ��ڶ�һ���ֽ�
  ARGS:
       tranChar�� Ҫ���յ��ֽ�
  RETURN:
       0:    ʧ��
	   1:    �ɹ�
***********************************************/
BOOL   GetByte(unsigned char* tranChar)
{
	if (tranChar == NULL)
		return FALSE;

   if(ReadComPort(tranChar,1)==-1)
	   return FALSE;

    return TRUE;
}

/**********************************************
  FUNCTION:   SendByte
  PURPOSE:    �򴮿�дһ���ֽ�
  ARGS:
       tranChar�� Ҫ���͵��ֽ�
  RETURN:
       0:    ʧ��
	   1:    �ɹ�
***********************************************/
BOOL  SendByte(unsigned char tranChar)
{
   
    unsigned char pBuf[2]={0};
    pBuf[0]=tranChar;
    if(WriteComPort(pBuf,1)!=1)
		return FALSE;

    return TRUE;
} 



BOOL CloseUSB();

BOOL OpenUSB()
{
	CloseUSB();
	assert(fdusb == NULL);
	
	fdusb = pusb_search_open(ST_VENDOR,ST_PRODUCT);
	if (fdusb == NULL) {
		return 0;
	}
	return 1;
}

BOOL CloseUSB()
{
	if(fdusb != NULL) {
		pusb_close(fdusb);
		fdusb = NULL;
		return 1;
	}
	return 0;
}

void Delay(int nTimes)
{
  usleep(nTimes);
  return;
 }
//----Send ctrl package to device for get result---------
BOOL SendCtrlPackage(int nRequestType,int nCmdLen)
{
   //send
  unsigned char pData[32]={0};
  int nSent = 0;
  int i;

  //send cmd request
  for (i=0; i<100; ++i)   {
    
    nSent = pusb_control_msg(fdusb,0xc0,nRequestType, 
		     nCmdLen, 0, pData, 2, 500);

    if (nSent >= 2) {
      break;
    Delay(100);
    }
  }
  if(i >= 100) 
  {
     // printf("ctrl error\n");
      return  FALSE;
  }

  return TRUE;
}
BOOL SendCtrlPackageUDisk(int nReqTp, int nReq,int nCmdLen,unsigned char *pData)
{
   //send
//  unsigned char pData[32]={0};
  int nSent = 0;
  int i;

  //send cmd request
  for (i=0; i<10; ++i)   {
    
    nSent = pusb_control_msg(fdusb,nReqTp,nReq, 
		     nCmdLen, 0, pData, 31, 500);

    if (nSent >= 2) {
      break;
    Delay(100);
    }
  }
  if(i >= 10) 
  {
      printf("ctrl error\n");
      return  FALSE;
  }

  return TRUE;
}
int BulkRecvPackage(unsigned char* DataBuf,int nLen,int nTimeOut)
{
	pusb_endpoint_t pedt;
        pedt=pusb_endpoint_open(fdusb,0x81,0);//0x01
	if(pedt==NULL) return -2;
       
        int nRecLen=pusb_endpoint_read(pedt,DataBuf,nLen,nTimeOut);
       // printf("bulk recv %d\n",nRecLen);

	if(nRecLen!=nLen)
	{//printf("bulk recv error\n");
		pusb_endpoint_close(pedt);
		return -3;
	}
        pusb_endpoint_close(pedt);

	return 0;

}
int BulkSendPackage(unsigned char* DataBuf,int nLen,int nTimeOut)
{
	pusb_endpoint_t pedt;
        pedt=pusb_endpoint_open(fdusb,0x02,0);//0x82
	if(pedt==NULL) return -2;

         int nSendLen=pusb_endpoint_write(pedt,DataBuf,nLen,nTimeOut);

   //      printf("Bulk send %d\n",nSendLen);

	if(nSendLen!=nLen)
	{//printf("bulk send error\n");
		pusb_endpoint_close(pedt);
		return -3;
	}
	pusb_endpoint_close(pedt);

	return 0;

}
//�հ�����Щ��һ�㶼��64�ֽڣ�
 
int USBGetData(unsigned char* DataBuf,int nLen)
{
	int nRet;

	/*if (!OpenUSB())
	{
		CloseUSB();
		return -1;
	}
	*/
 
   if(!SendCtrlPackage(1,nLen))
	   return -1;
   // Delay(100);
   
   nRet=BulkRecvPackage(DataBuf,nLen,10000);
	//CloseUSB();
 
	return nRet;	
}
//������������Щ��һ�㶼��64�ֽڣ���ͨ���������һ����Ҫ���صĿ��ư���Ȼ��ͨ��Bulk����64�ֽڵ�������ʽ�� 
int USBDownData(unsigned char *DataBuf,int nLen)
{
	int nRet;

	/*if (!OpenUSB())
	{
		CloseUSB();
		return -1;
	}
	*/
   if(!SendCtrlPackage(0,nLen))
	   return -1;
   //Delay(100);

   nRet=BulkSendPackage(DataBuf,nLen,10000);
	//CloseUSB();
 
	return nRet;	
}
//Down Char�Ƚ�����
int USBDownData1(unsigned char *image,int nLen)
{	
   int nRet;

	/*if (!OpenUSB())
	{
		CloseUSB();
		return -1;
	}
	*/
   if(!SendCtrlPackage(1,nLen))
	   return -1;

   nRet=BulkSendPackage(image,nLen,8000);
	//CloseUSB();
 
	return nRet;	
} 

int DeviceConnected()
{
	BOOL b = OpenUSB();
	CloseUSB();
	return b?0:-1;
}

//��ȡͼ�����ݣ�����λ�ȡ
int USBGetImage(unsigned char* DataBuf,int nLen)
{	
	int nRet;

	/*if (!OpenUSB())
	{
		CloseUSB();
		return -1;
	}*/
   if(!SendCtrlPackage(1,nLen))
	   return -1;

   nRet=BulkRecvPackage(DataBuf,nLen/2,8000);

   if(nRet!=0) return nRet;

   nRet=BulkRecvPackage(DataBuf+nLen/2,nLen/2,8000); 
 
	//CloseUSB();
 
	return nRet;	
}
//����ͼ�����ݣ����Ĵ��´�
int USBDownImage(unsigned char *DataBuf,int nLen)
{	
	int nRet;

	/*if (!OpenUSB())
	{
		CloseUSB();
		return -1;
	}*/
	

   if(!SendCtrlPackage(1,nLen))
	   return -1;

   nRet=BulkSendPackage(DataBuf,nLen/4,8000);

   if(nRet!=0) return nRet;

   nRet=BulkSendPackage(DataBuf+nLen/4,nLen/4,8000); 

   if(nRet!=0) return nRet;

    nRet=BulkSendPackage(DataBuf+nLen/2,nLen/4,8000); 

   if(nRet!=0) return nRet;

    nRet=BulkSendPackage(DataBuf+nLen*3/4,nLen/4,8000); 

	//CloseUSB();
 
	return nRet;		

}

///////////////////////////////////////////////////////////////////////////
//ϞȽ²¿·ԍ
///////////////

//´򿫉豸
BOOL OpenUDisk()
{
	printf("-=-=-=-=-=into openudisk function-=-=-=-\n");
	CloseUDisk();
	assert(fdusb == NULL);
	
	fdusb = pusb_search_open(SU_VENDOR,SU_PRODUCT);
	printf("pusb_serch_open function is success\n");
	if (fdusb == NULL) {
		printf("pusb_search_open is fail\n");
		return FALSE;
	}
//	int ret = pusb_reset(fdusb);//pusb_set_configuration(fdusb,0);
//	printf("ret = %d\n",ret);
	printf("openudisk function is success,here\n");
	return TRUE;
}

//¹ر։豸
BOOL CloseUDisk()
{
	if(fdusb != NULL) {
		pusb_close(fdusb);
		fdusb = NULL;
		return TRUE;
	}
	return TRUE;
}

//½Ԋ֊�nt UDiskGetData(unsigned char* DataBuf, int nLen)
{//ģʽ DO_CBW + DI_DATA + DI_CSW
	int ret = -1,i=0;
	unsigned char do_CBW[33] = {0x55,0x53,0x42,0x43,
								0xb0,0xfa,0x69,0x86,
								0x00,0x00,0x00,0x00,
								0x80,0x00,0x0a,0x85,
								0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00};
	unsigned char di_CSW[16] = {0x00};

	do_CBW[8] = nLen & 0xff;
	do_CBW[9] = (nLen>>8) & 0xff;
	do_CBW[10]= (nLen>>16)& 0xff;
	do_CBW[11]= (nLen>>24)& 0xff;

	//DO_CBW
//	if(!SendCtrlPackageUDisk(1,nLen,do_CBW))
//		return -31;
	ret = BulkSendPackage(do_CBW,31,8000);
	if(ret!=0){
		printf("1...UDiskGetData fail!\n");
		return -31;
	}

	//DI_DATA
	ret = BulkRecvPackage(DataBuf,nLen,10000);
	if(ret!=0)
		return -31;

	//DI_CSW
	ret = BulkRecvPackage(di_CSW,13,10000);
	if(di_CSW[3]!=0x53 || di_CSW[12]!=0x00)
		return -31;//UDisk½Ԋ֊�񋈉di_CSW[3]=0x43;
	for(i=0; i<12; i++){
		if(di_CSW[i]!=do_CBW[i])
			return -31;
	}

	return 0;
}

//·¢̍˽¾ۍ
int UDiskDownData(unsigned char* pBuf, int nLen)
{//ģʽ DO_CBW + DO_CMD + DI_CSW
	int ret = -1,i;
	unsigned char do_CBW[33] = {0x55,0x53,0x42,0x43,
					  			0xb0,0xfa,0x69,0x86,
								0x00,0x00,0x00,0x00,
								0x00,0x00,0x0a,0x86,
								0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00};
	unsigned char di_CSW[16] = {0x00};

	do_CBW[8] = nLen & 0xff;
	do_CBW[9] = (nLen>>8) & 0xff;
	do_CBW[10]= (nLen>>16)& 0xff;
	do_CBW[11]= (nLen>>24)& 0xff;

	//DO_CBW
//	if(!SendCtrlPackageUDisk(0,nLen,do_CBW))
//		return -32;
	ret = BulkSendPackage(do_CBW,31,10000);
	if(ret!=0){
		printf("1...UDiskDownData fail!\n");
		for(i=0; i<31; i++)
			printf("0x%02X ",do_CBW[i]);
		printf("\n");
		return -32;//UDisk·¢̍˽¾ݴ
	}

	//DO_CMD
	ret = BulkSendPackage(pBuf,nLen,10000);
	if(ret!=0)
		return -32;//UDisk·¢̍˽¾ݴ

	//DI_CSW
	ret = BulkRecvPackage(di_CSW,13,10000);
	if(di_CSW[3]!=0x53 || di_CSW[12]!=0x00)
		return -32;//UDisk·¢̍˽¾ݴ
	di_CSW[3]=0x43;
	for(i=0; i<12; i++){
		if(di_CSW[i]!=do_CBW[i])
			return -32;
	}

	return 0;
}
#include <errno.h>
#include <sys/ioctl.h>
struct tagUSB_device_t
{
	int fd;
};

struct tagUSB_endpoint_t
{
	int fd;
	int ep;
};
int mydata_prt(const void *data, int length)
{
    int i;
    unsigned char *dp = (unsigned char *)data;

    if(dp == NULL)
    {
        printf("Warning: data pointer is null!\n");
        return -1;
    }

    printf("Data list(size = %d):", length);

    for(i = 0; i < length; i++)
    {
        if(i%16 == 0)
            printf("\n[%04x] ", i);
        if(i%8 == 0)
            printf(" ");
        printf("%02x ", *dp++);
    }
    printf("\n");

    return 0;
}
int TestData()
{
	int ret = -1,i;

	unsigned char do_CBW[33] = {0x55,0x53,0x42,0x43,
					0xb0,0xfa,0x69,0x86,//0x5f,0xd9,0x00,0x00,	//
					0x10,0x00,0x00,0x00,//0x24,0x00,0x00,0x00,	//
					0x00,0x00,0x10,0xFF,	//0x00,0x00,0x0a,0x86,//
	0x13,0x00,0x10,0x00, 0x57,0x45,0x4C,0x4C,	//0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,//
 	0x43,0x4F,0x4D,0x39, 0x39,0x38,0xFF,0x00};//0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00};//
	unsigned char di_CSW[16] = {0x00};

         unsigned char pBuf[17]={0xef,0x01,0xff,0xff, 0xff,0xff,0x01,0x00,
                                 0x07,0x13,0x00,0x00, 0x00,0x00,0x00,0x1b};

	if(!OpenUDisk()){
		printf("Open UDisk failed!\n");
	       return 0;
	}
	printf("Open UDisk success!\n");

if(0)
{
    struct usbdevfs_getdriver mydriver;

	int ret;

	 ret = ioctl(fdusb->fd, USBDEVFS_GETDRIVER, &mydriver);

     if(ret)
		 perror("get driver");

	 printf("USBDEVFS GETDRIVER ret=%d, interface=%d, name=%s\n", ret, mydriver.interface, mydriver.driver);
}
//ret = USBDownData( pBuf, 3);
if(1)
{
struct usbdevfs_ioctl {
	    int ifno;       /* interface 0..N ; negative numbers reserved */
	    int ioctl_code; /* MUST encode size + direction of data so the
                         * macros in <asm/ioctl.h> give correct values */
	    void    *data;      /* param buffer (in, or out) */
};
	int ret;
	struct usbdevfs_ioctl  myctl;
	 myctl.ifno = 0;
	 myctl.ioctl_code = USBDEVFS_DISCONNECT;

	 ret = ioctl(fdusb->fd, USBDEVFS_IOCTL, &myctl);
	 if(ret < 0)
	   perror("USBDEVFS_IOCTL DISCONNECT");

	 printf("USBDEVFS_IOCTL DISCONNECT ret=%d \n", ret);
}
usleep(10000);
//sleep(5);
if(1)
{
	int interface = 1;
	for(i = 0; i < interface; i++)
	{
	ret = ioctl(fdusb->fd, USBDEVFS_CLAIMINTERFACE, &i); 
	if(ret)
	         perror("USBDEVFS_CLAIMINTERFACE");

     printf("USBDEVFS_CLAIMINTERFACE ret=%d\n", ret);
	}
}

if(0)
{
    struct usbdevfs_getdriver mydriver;

	int ret;

	 ret = ioctl(fdusb->fd, USBDEVFS_GETDRIVER, &mydriver);

     if(ret)
		 perror("get driver");

	 printf("USBDEVFS_GETDRIVER ret=%d, interface=%d, name=%s\n", ret, mydriver.interface, mydriver.driver);
}



if(0)
{
struct usbdevfs_ioctl {
	    int ifno;       /* interface 0..N ; negative numbers reserved */
	    int ioctl_code; /* MUST encode size + direction of data so the
                         * macros in <asm/ioctl.h> give correct values */
	    void    *data;      /* param buffer (in, or out) */
};
	int ret;
	struct usbdevfs_ioctl  myctl;
	 myctl.ifno = 0;
	 myctl.ioctl_code = USBDEVFS_CONNECT;

	 ret = ioctl(fdusb->fd, USBDEVFS_IOCTL, &myctl);
	 if(ret < 0)
	   perror("USBDEVFS_IOCTL CONNECT");

	   printf("USBDEVFS_IOCTL CONNECT ret=%d \n", ret);

}

if(1)
{
	int ret;

	ret = ioctl(fdusb->fd, USBDEVFS_RESET);
	if(ret < 0)
		perror("reset");

	printf("USBDEVFS_RESET reset ret = %d\n", ret);
}



if(0)
{
	struct usbdevfs_bulktransfer bulk;
	char buf[12];
	int ret;
	pusb_endpoint_t pedt;
	pedt=pusb_endpoint_open(fdusb,0x02,0);

	   	bulk.ep      = pedt->ep|USB_DIR_OUT;
		bulk.len     = 12;
		bulk.timeout = 10000;
		bulk.data    = (unsigned char*)buf;

	ret = ioctl(fdusb->fd,USBDEVFS_BULK,&bulk);

	if(ret < 0)
		perror("bulk trans");

	printf("bulk ret= %d\n", ret);
}

if(0)
{
if(!OpenUDisk()){
	        printf("Open UDisk failed!\n");
			           return 0;
					       }

printf("Open UDisk success!\n");
}
//ret = USBDownData( pBuf, 3);
     //ret = USBDownData( pBuf, 3);
	// printf("send size = %d\n", ret);
    //ret = write(fdusb->fd, "1234567890OK", 12);
	//printf("send size = %d\n", ret);

	//ret = read(fdusb->fd, pBuf, 12);
	  //  printf("read size = %d\n", ret);

//	ret = pusb_test(fdusb,0x02,pBuf,17);
//	printf("send Ctrl. ret = %d\n",ret);
//	if(ret<0)
 //		return -1;
	

//	if(!CloseUDisk()){
//		printf("Close UDisk failed!\n");
//	       return 0;
//	}
//	printf("Close UDisk success!\n");
//	return 0;
	
//DO_CBW
//	if(!SendCtrlPackageUDisk(0x80,0x00,31,do_CBW)){
//		printf("1...SendCtrlPackageUDisk failed!\n");
//		return -32;
//	}

/*	pusb_endpoint_t pedt;
	pedt=pusb_endpoint_open(fdusb,0x02,0);
	if(pedt==NULL) 
		return -2;
	int nSendLen;//=pusb_endpoint_write(pedt,do_CBW,31,0);
////////////////////////////////////////////////////////
	int fd = pedt->fd;
	int ep = pedt->ep | USB_DIR_OUT;
	BYTE* buf = do_CBW;
	int size = 31;
	struct usbdevfs_urb urb, * purb;
	memset(&urb,0,sizeof(urb));
	urb.type          = USBDEVFS_URB_TYPE_BULK;
	urb.endpoint      = ep;
	urb.flags         = 0;
	urb.buffer        = (unsigned char*)buf;
	urb.buffer_length = size;
	urb.signr         = 0;
	do {
		ret = ioctl(fd,USBDEVFS_SUBMITURB,&urb);
	} while(ret < 0 && errno == EINTR);
	printf("ret1 = %d\n",ret);
	if (ret < 0)
		return(ret);

	do {
		ret = ioctl(fd,USBDEVFS_REAPURB,&purb);
	} while(ret < 0 && errno == EINTR);
	printf("ret2 = %d",ret);
	if(ret < 0)
		return(ret);

	if(purb != &urb)
		printf("purb=%p, &urb=%p\n",purb,&urb);
	if(purb->buffer != buf)
		printf("purb->buffer=%p, buf=%p\n",purb->buffer,buf);///

	nSendLen = (purb->status < 0) ? purb->status : purb->actual_length;
////////////////////////////////////////////////////////
	printf("Bulk send %d\n",nSendLen);
	if(nSendLen!=31)
	{
		printf("bulk send error\n");
		pusb_endpoint_close(pedt);
		return -3;
	}
	pusb_endpoint_close(pedt);*/
//int pusb_test(pusb_device_t dev,int epnum,const unsigned char* buf,int size);
//	ret = pusb_test(fdusb,0x01,do_CBW,31);

	//ret = pusb_connect(fdusb);
	//printf("connect = %d\n",ret);

	//ret = pusb_connectInfo(fdusb,0);
	//printf("connectInfo = %d\n",ret);

	//ret = pusb_disconnect(fdusb);

	//printf("disconnectInfo = %d\n",ret);

//	ret = pusb_reset(fdusb);
//	printf("ret = %d\n",ret);

//	ret = usb_bulk_write(fdusb,0x02,do_CBW,31,200);
//	printf("write_bytes = %d",ret);
/*
	ret = BulkSendPackage(do_CBW,31,1000);
	if(ret!=0){
		perror("ret");
		printf("1...BulkSendPackage failed!\n   ret = %d\n",ret);
		return -32;//UDisk·¢̍˽¾ݴ
	}
//mydata_prt(do_CBW,31);
	printf("send success\n");
	//DO_CMD
	ret = BulkSendPackage(pBuf,16,10000);
	if(ret!=0)
		return -32;//UDisk·¢̍˽¾ݴ
//mydata_prt(pBuf,16);
        printf("send data success\n");
	//DI_CSW
	ret = BulkRecvPackage(di_CSW,13,10000);
   printf("recv data success\n");
mydata_prt(di_CSW,13);
	if(di_CSW[3]!=0x53 || di_CSW[12]!=0x00)
		return -32;//UDisk·¢̍˽¾ݴ
	di_CSW[3]!=0x43;
	for(i=0; i<12; i++){
		if(di_CSW[i]!=do_CBW[i])
			return -32;
	}
////////////////////////////*/
	ret = UDiskDownData(pBuf,16);
	if(ret!=0){
		return -1;
	}
	printf("Send Succ!\n");
	unsigned char pRev[64]={0};
	ret = UDiskGetData(pRev,16);
	if(ret!=0)
		return -1;
	printf("Recv Succ!\n");
mydata_prt(pRev,16);

	ret = UDiskDownData(pBuf,16);
	if(ret!=0){
		return -1;
	}
	printf("Send Succ!\n");
	ret = UDiskGetData(pRev,16);
	if(ret!=0)
		return -1;
	printf("Recv Succ!\n");
mydata_prt(pRev,16);

CloseUDisk();
	return 0;
}
//»򈢍¼б
int UDiskGetImage(unsigned char* Img, int nLen)
{
	int iTmpLen = nLen;
	iTmpLen = nLen/2;
	if( UDiskGetData(Img,iTmpLen)!=0 )
		return -33;//»򈢍¼бʧ°ڍ
	return UDiskGetData(Img+iTmpLen,iTmpLen);
}

//Ђ՘ͼбµ½Flash
int UDiskDownImage(unsigned char* Img, int nLen)
{
	int iTmpLen = nLen;
	iTmpLen = nLen/2;
	if( UDiskDownData(Img,iTmpLen)!=0 )
		return -34;//Ђ՘ͼбʧ°ڍ
	return UDiskDownData(Img+iTmpLen,iTmpLen);
}
