
#ifndef _PUSB_LINUX_C_
#define _PUSB_LINUX_C_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "pusb-linux.h"
// should use
// static int page_size = sysconf(_SC_PAGESIZE);
#include "page.h"

#include "pusb.h"

/* FIXME */
#define __u8 uint8_t
#define __u16 uint16_t
#define __u32 uint32_t

/******************************************************************************
*	Structures
******************************************************************************/

struct tagUSB_device_t
{
	int fd;
};

struct tagUSB_endpoint_t
{
	int fd;
	int ep;
};

static const char usb_path[] = "/proc/bus/usb";

/* Device descriptor */
struct usb_device_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u16 bcdUSB;
	__u8  bDeviceClass;
	__u8  bDeviceSubClass;
	__u8  bDeviceProtocol;
	__u8  bMaxPacketSize0;
	__u16 idVendor;
	__u16 idProduct;
	__u16 bcdDevice;
	__u8  iManufacturer;
	__u8  iProduct;
	__u8  iSerialNumber;
	__u8  bNumConfigurations;
} __attribute__ ((packed));

/*****************************************************************************
*	Local Prototypes
*****************************************************************************/

static int test_file(const char *path, int vendorID, int productID);
static int usbfs_search(const char *path, int vendorID, int productID);
static pusb_device_t make_device(int fd);

/*****************************************************************************
*	Library functions
*****************************************************************************/



/*
* Function     : pusb_search_open
* Return value : NULL on error, a valid pusb_device_t on success
* Description  : 
*/
pusb_device_t pusb_search_open(int vendorID, int productID)
{
	int fd;

	if ((fd = usbfs_search("/proc/bus/usb",vendorID,productID)) < 0)
		return(NULL);

	return(make_device(fd));

}

/*
* Function     : pusb_open
* Return value : NULL on error, a valid pusb_device_t on success
* Description  : Opens the USB device pointed by path
*/
pusb_device_t pusb_open(const char *path)
{
	int fd; 

	if ((fd = open(path, O_RDWR)) < 0)
	{
		printf("pusb_open is fail\n");
		return(NULL);
	}

	return(make_device(fd));

}

/*
* Function     : pusb_close
* Return value : 0 on success, -1 on error (errno is set)
* Description  : Closes the USB dev
*/
int pusb_close(pusb_device_t dev)
{

	int ret;

	ret = close(dev->fd);
	free(dev);

	return(ret);

}

/*
* Function     : pusb_control_msg
* Return value : ioctl returned value (see linux usb docs)
* Description  : sends a control msg urb to the device
*/
int pusb_control_msg(	pusb_device_t dev,
			int request_type,
			int request,
			int value,
			int index, 
			unsigned char *buf,
			int size,
			int timeout)
{

	int ret;
	struct usbdevfs_ctrltransfer ctrl;

	ctrl.requesttype = request_type;
	ctrl.request     = request;
	ctrl.value       = value;
	ctrl.index       = index;
	ctrl.length      = size;
	ctrl.timeout     = timeout;
	ctrl.data        = buf;

	ret = ioctl(dev->fd,USBDEVFS_CONTROL,&ctrl);

	return(ret);

}

/*
* Function     : pusb_set_configuration
* Return value : ioctl returned value (see linux-usb docs)
* Description  : cf function name
*/
int pusb_set_configuration(pusb_device_t dev, int config)
{

	int ret;

	ret = ioctl(dev->fd,USBDEVFS_SETCONFIGURATION,&config);

	return(ret);

}
/************************************************************/
struct usbdevfs_connectinfo{
	unsigned int devnum;
	unsigned char slow;
};
int pusb_connectInfo(pusb_device_t dev,int devnum)
{
	struct usbdevfs_connectinfo stf;
	stf.devnum = devnum;
	stf.slow = 0;
	int ret = ioctl(dev->fd,USBDEVFS_CONNECTINFO,&stf);
	return ret;
}
int pusb_clr_halt(pusb_device_t dev,int ifg)
{
	int ret = ioctl(dev->fd,USBDEVFS_CLEAR_HALT,&ifg);
	return ret;
}
int pusb_connect(pusb_device_t dev)
{
	int ret = ioctl(dev->fd,USBDEVFS_CONNECT);
	return ret;
}
int pusb_disconnect(pusb_device_t dev)
{
	int ret = ioctl(dev->fd,USBDEVFS_DISCONNECT);
	return ret;
}
int pusb_reset(pusb_device_t dev)
{
	int ret;
	ret = ioctl(dev->fd,USBDEVFS_RESET);
	return ret;
}
int pusb_test(pusb_device_t dev,int epnum,const unsigned char* buf,int size)
{
	int ret,received=0;
	struct usbdevfs_bulktransfer bulk;
	do{
		bulk.ep	= (epnum&0xF)|USB_DIR_OUT;
		bulk.len	= (size>PAGE_SIZE) ? PAGE_SIZE:size;
		bulk.timeout	= 1000;
		bulk.data	= (unsigned char*)buf;

		do{
			ret = ioctl(dev->fd,USBDEVFS_BULK,&bulk);
		}while(ret<0 && errno==EINTR);
	
	if(ret<0)
		return(ret);
	buf		+= ret;
	size		-= ret;
	received	+= ret;
	}while(ret==bulk.len && size>0);
	
	return(received);	
}
/*int pusb_resetep(pusb_device_t dev, int st)
{
	int ret;
	ret = ioctl(dev->fd,USBDEVFS_RESETEP,&st);
	return ret;
}*/
/******************************************************************/
/*
* Function     : pusb_set_interface
* Return value : ioctl returned value (see linux-usb docs)
* Description  : see function name
*/
int pusb_set_interface(pusb_device_t dev, int interface, int alternate)
{

	struct usbdevfs_setinterface setintf;
	int ret;

	setintf.interface = interface;
	setintf.altsetting = alternate;

	ret = ioctl(dev->fd,USBDEVFS_SETINTERFACE,&setintf);

	return(ret);

}

/*
* Function     : pusb_endpoint_open
* Return value : NULL on error, a valid ep on success
* Description  : see function name
*/
pusb_endpoint_t pusb_endpoint_open(pusb_device_t dev, int epnum, int flags)
{

	pusb_endpoint_t ep;

	if ((ep = (pusb_endpoint_t) malloc(sizeof(*ep))) == NULL)
		return(NULL);

	ep->fd = dev->fd;
	ep->ep = epnum & 0xf;

	return(ep);

}

/*
* Function     : pusb_endpoint_rw_no_timeout
* Return value : ioctl returned value
* Description  : Writes or Read from an usb end point (without timeout value)
*/
int pusb_endpoint_rw_no_timeout(int fd,
				int ep,
				const unsigned char *buf,
				int size)
{

	struct usbdevfs_urb urb, * purb;
	int ret;

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

	if (ret < 0)
		return(ret);

	do {
		ret = ioctl(fd,USBDEVFS_REAPURB,&purb);
	} while(ret < 0 && errno == EINTR);

	if(ret < 0)
		return(ret);

/*
	if(purb != &urb)
		fprintf(stderr, "purb=%p, &urb=%p\n",purb,&urb);

	if(purb->buffer != buf)
		fprintf(stderr, "purb->buffer=%p, buf=%p\n",purb->buffer,buf);
*/

	return (purb->status < 0) ? purb->status : purb->actual_length;

}

/*
* Function     : pusb_endpoint_rw
* Return value : ioctl returned value
* Description  : Writes or Read from an usb end point (with timeout value)
*/
int pusb_endpoint_rw(
			int fd,
			int ep,
			const unsigned char * buf,
			int size,
			int timeout)
{

	struct usbdevfs_bulktransfer bulk;
	int ret, received = 0;
	//printf("=====into pusb_endpoint_rw function=====\n");

	do {
		//printf("pusb_endpoint_rw while\n");
		bulk.ep      = ep;
		bulk.len     = (size > PAGE_SIZE)?PAGE_SIZE:size;
		bulk.timeout = timeout;
		bulk.data    = (unsigned char*)buf;

		do {
			//printf("begin ioctl\n");
			ret = ioctl(fd,USBDEVFS_BULK,&bulk);
		} while (ret < 0 && errno == EINTR);

		if (ret < 0)
		{	perror("ioctl");
			//printf("ioctl error return is %d\n", ret );
			return(ret);
		}
		buf      += ret;
		size     -= ret;
		received += ret;

	} while(ret==bulk.len && size>0);
	
	return(received);
}

/*
* Function     : pusb_endpoint_write
* Return value : same as pusb_endpoint_rw
* Description  : wrapper to the pusb_endpoint_rw
*/
int pusb_endpoint_write(pusb_endpoint_t ep, 
			const unsigned char *buf,
			int size,
			int timeout)
{
	//printf("---into pusb_endpoint_write function---\n");

	if(timeout == 0)
		return pusb_endpoint_rw_no_timeout(ep->fd,ep->ep|USB_DIR_OUT,buf,size);
	//printf("time is not equal 0\n");

	return(pusb_endpoint_rw(ep->fd,ep->ep|USB_DIR_OUT,buf,size,timeout));

}

/*
* Function     : pusb_endpoint_read
* Return value : same as pusb_endpoint_rw
* Description  : wrapper to the pusb_endpoint_rw
*/
int pusb_endpoint_read(	pusb_endpoint_t ep, 
			unsigned char *buf,
			int size,
			int timeout)
{

	if(timeout == 0)
		return(pusb_endpoint_rw_no_timeout(ep->fd,ep->ep|USB_DIR_IN,buf,size));

	return(pusb_endpoint_rw(ep->fd,ep->ep|USB_DIR_IN,buf,size,timeout));

}

/*
* Function     : pusb_endpoint_close
* Return value : 0
* Description  : Close the end pont given in parameter
*/
int pusb_endpoint_close(pusb_endpoint_t ep)
{
	/* nothing to do on the struct content */
	free(ep);

	return(0);

}

/*
* Function     : pusb_claim_interface
* Return value : ioctl returned value
* Description  : Claims an interface for use
*/
int pusb_claim_interface(pusb_device_t dev, int interface)
{

	return(ioctl(dev->fd, USBDEVFS_CLAIMINTERFACE,&interface));

}

/*
* Function     : pusb_release_interface
* Return value : ioctl returned value
* Description  : Release the usb interface
*/
int pusb_release_interface(pusb_device_t dev, int interface)
{

	return(ioctl(dev->fd,USBDEVFS_RELEASEINTERFACE,&interface));

}
 
int pusb_ioctl (pusb_device_t dev,int interface,int code,void *data)
{
	//printf("------into pusb_ioctl function-----\n");
	struct usbdevfs_ioctl ctrl;

	ctrl.ifno = interface;
	ctrl.ioctl_code = code;
	ctrl.data = data;

	return(ioctl(dev->fd,USBDEVFS_IOCTL,&ctrl));
}

/*****************************************************************************
*	Local functions
*****************************************************************************/

/*
* Function     : test_file
* Return value : -1 on error, a valid filedescriptor on success
* Description  : Try to open the file and get USB device information,
*                if it's ok, check if it matches vendorID & productID
*/
static int test_file(const char *path, int vendorID, int productID)
{
	//printf("---------into test_file()---------\n");
	//printf("vendorID is %04X\n", vendorID);
	//printf("vendorID is %04X\n", productID);


	int fd;
	struct usb_device_descriptor desc;
	
	if((fd = open(path, O_RDWR)) == -1) {
		printf("open(path,O_RDWR) is error!\n");
		perror(path);
		return(-1);
	}
	//printf("open file is success\n");
  
	if(read(fd,&desc,sizeof(desc)) == sizeof(desc))  {

	//	printf("read file is success!\n");
		/*
		* Great, we read something
		* check, it match the correct structure
		*/
		if(desc.bLength == sizeof(desc)) {
			//printf("desc.blength == desc\n");
			 // printf( "=== %s ===\n",path);
			 //printf( "  bLength         = %u\n",desc.bLength);
			 //printf( "  bDescriptorType = %u\n",desc.bDescriptorType);
			 // printf( "  bcdUSB          = %04x\n",desc.bcdUSB);
			  printf( "  idVendor        = %04x\n",desc.idVendor);
			  printf( "  idProduct       = %04x\n",desc.idProduct);
			 //printf( "  bcdDevice       = %04x\n",desc.bcdDevice);
			/*	  
			  fprintf(stderr, "=== %s ===\n",path);
			  fprintf(stderr, "  bLength         = %u\n",desc.bLength);
			  fprintf(stderr, "  bDescriptorType = %u\n",desc.bDescriptorType);
			  fprintf(stderr, "  bcdUSB          = %04x\n",desc.bcdUSB);
			  fprintf(stderr, "  idVendor        = %04x\n",desc.idVendor);
			  fprintf(stderr, "  idProduct       = %04x\n",desc.idProduct);
			  fprintf(stderr, "  bcdDevice       = %04x\n",desc.bcdDevice);
			*/
			if(	vendorID == desc.idVendor &&
				productID == desc.idProduct)
			{
				//printf("vid pid is equal! and return ok\n");
				return(fd);
			}
		}

	}
	
	close(fd);
	//printf("-------exit test_file() function------- \n");

	return(-1);

}

/*
* Function     : usbfs_search
* Return value : -1 on error, a valid filedescriptor on success
* Description  : Search for a vendorID, productID.
*/
static int usbfs_search(const char *path, int vendorID, int productID)
{

	int result = -1;
	
	DIR * dir;
	struct dirent * dirp;
	
	//printf("into usbfs_search()\n");
	if((dir = opendir(path)) == NULL) {
		perror(path);
		return(-1);
	}
	
	while((dirp=readdir(dir)) != NULL) {

		//printf("=============into while===========\n");	

		struct stat statbuf;
		char file[PATH_MAX+1];
		
		if (strlen(dirp->d_name) != 3)
		{
			continue;
		}
		//printf("strlen(filename) is 3 ok\n");

		if (!isdigit(dirp->d_name[0]) ||
		    !isdigit(dirp->d_name[1]) ||
		    !isdigit(dirp->d_name[2]))
		{
			continue;
		}
		//printf("name is digit ok\n");

		sprintf(file,"%s/%s",path,dirp->d_name);
		

		if (stat(file,&statbuf) != 0) {
			perror(file);
			continue;
		}
		//printf("stat() is ok\n");
		
		if (S_ISDIR(statbuf.st_mode)) {

			//printf("if it's a dir,then open\n");

			if((result = usbfs_search(file,vendorID,productID)) < 0)
			{
				continue;
			}
			else
				break;
		}
		
		if (S_ISREG(statbuf.st_mode)) {

			//printf("if is a REG file\n");

			if ((result=test_file(file,vendorID,productID)) < 0)					
			{
				//printf("into test_file()\n");
				continue;
			}
			else
				break;
		}

	}
	//printf("usbfs_search end !\n");
	
	closedir(dir);

	//printf("dir is close!\n");

	return(result);

}

/*
* Function     : make_device
* Return value : NULL on error, a valid pusb_device_t
* Description  : Allocates a pusb_device_t data structure
*/
static pusb_device_t make_device(int fd)
{

	//printf("+++++++into make_device function++++++\n");
	pusb_device_t dev;

	if((dev = (pusb_device_t)malloc(sizeof(*dev))) == NULL) {
		close (fd);
		return(NULL);
	}
	
	dev->fd = fd;

	//printf("return is %d\n", dev->fd);
	return(dev);

}

#endif /* _PUSB_LINUX_C_ */
