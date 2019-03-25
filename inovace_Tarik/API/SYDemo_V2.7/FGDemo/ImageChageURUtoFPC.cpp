
#include "stdafx.h"
#include "ImageChangeURUtoFPC.h"
/*
**************************************************************************************
名 称: void ImageChangeURUtoFPC(unsigned char *iopImage)
功 能: URU图像转换为FPC的图像格式,225X313-->256X288
参 数:  iopImage-输入输出，图像数据
返回值:	无
**************************************************************************************
*/
void ImageChangeURUtoFPC(unsigned char *iopImage)
{	
	unsigned char ImageNew[282*286];
	int OriX=225,OriY=313,OldX = 282,OldY = 286;
	int NewX = 256,NewY = 288;
	int i,j,m,n;
	int offX,offY;
	float ratioX,ratioY;
	offX = -(NewX-OldX)/2;
	offY = -(OldY-NewY)/2;
	
	ratioX = 152*1.0f/121;
	ratioY = 152*1.0f/166;
	
	//图像分辨率变化
	ImageZoom(iopImage,OriX,OriY,ImageNew,OldX,OldY,ratioX,ratioY);
	
	//图像裁剪为标准尺寸
	memset(iopImage,FILL_COLOR,256*288);
	for(m=offY,i=0; i<OldY; i++,m++)
	{
		for(j=offX, n=0; n<NewX; j++,n++)
			iopImage[m*NewX+n] = ImageNew[i*OldX+j];
	}

	//垂直镜像
	VertMirror(iopImage,NewX,NewY);
}


/*
**************************************************************************************
名 称: void VertMirror(unsigned char*iopImage,int iX, int iY)
功 能: 图像垂直镜像
参 数:  iopImage-输入输出，图像数据
		iX-输入，图像宽度
		iY-输入，图像高度
返回值:	无
**************************************************************************************
*/
void VertMirror(unsigned char*iopImage,int iX, int iY)
{
	int i,j;
	unsigned char ctmp;
	
	for(i=0; i<iY/2; i++)
	{
		for(j=0; j<iX; j++)
		{
			ctmp = iopImage[i*iX+j];
			iopImage[i*iX+j] = iopImage[(iY-1-i)*iX+j];
			iopImage[(iY-1-i)*iX+j] = ctmp;
		}
	}
}

/*
**************************************************************************************
名 称: unsigned char bilinear_Dou(unsigned char *pImage,long lWidth,long lHeight, double x,double y)
功 能: 双线性差值
参 数:  pImage-原始图像数据
		lWidth-原始图像宽度
		lHeight-原始图像高度
		pImageNew-原始缩放后的图像数据
		x-参考像素横坐标位置
		y-参考像素纵坐标位置
返回值:	差值后像素灰度值
**************************************************************************************
*/
unsigned char bilinear_Dou(unsigned char *pImage,long lWidth,long lHeight, double x,double y)
{
	double fx1,fx2,fy1,fy2;
	unsigned char pix11,pix12,pix21,pix22;
	long x1,y1;
	int gray;
	
	if( (x<0) || (x>=lWidth) || (y<0) || (y>=lHeight))
		return FILL_COLOR;	//背景色
	
	x1 = (long)x+1;
	y1 = (long)y+1;
	if(x1>=lWidth)
		x1 = lWidth-1;
	if(y1>=lHeight)
		y1 = lHeight-1;	
	
	pix11 = pImage[(long)y*lWidth+(long)x];
	pix12 = pImage[(long)y*lWidth+(long)x1];
	pix21 = pImage[(long)y1*lWidth+(long)x];
	pix22 = pImage[(long)y1*lWidth+(long)x1];
	
	fx2 = x-(long)x;
	fx1 = 1-fx2;
	fy2 = y-(long)y;
	fy1 = 1-fy2;
	
	gray = (int)( pix11*fy1*fx1 + pix12*fy1*fx2 + pix21*fy2*fx1 + pix22*fy2*fx2+0.5);
	if(gray >255)
		gray = 255;
	if(gray < 0)
		gray = 0;
	
	return (unsigned char)gray;
}


/*
**************************************************************************************
名 称: void ImageZoom(unsigned char *pImageOld,int oldX,int oldY,unsigned char *pImageNew,int newX,int newY,float ratio)
功 能: 图像缩放，根据给定缩放比例缩放图像，并根据给定的缩放后图像尺寸进行截取或者填充
参 数:	pImageOld - 原始图像缓冲区
		oldX - 原始图像宽度
		oldY - 原始图像高度
		pImageNew - 缩放后的图像缓冲区
		newX - 缩放后图像宽度	
		newY - 缩放后图像高
		ratio - 图像缩放比例			  
返回值:	  无
**************************************************************************************
*/
void ImageZoom(unsigned char *pImageOld,int oldX,int oldY,unsigned char *pImageNew,
			   int newX,int newY,float ratioX,float ratioY)
{
	int i,j;
	int NewX_R,NewY_R;	//真实的图像放大或者缩小后的宽度和高度
	int m,n;
	int mstart,nstart,istart,jstart;
	
	memset(pImageNew,FILL_COLOR,sizeof(unsigned char)*newX*newY);
	if(ratioX < 0.0001 || ratioY < 0.0001)
		return ;
	//新的图像大小
	NewX_R = (int)(ratioX*oldX);
	NewY_R = (int)(ratioY*oldY);
	
	mstart = nstart = 0;
	istart = (NewY_R-newY)/2;
	jstart = (NewX_R-newX)/2;
	
	//istart，jstart大于0则进行截取，小于0则进行填充
	if(istart<0)
	{
		mstart = -istart;
		istart = 0;
	}
	if(jstart < 0)
	{
		nstart = -jstart;
		jstart = 0;
	}		
	
	for(i=istart,m=mstart ; i<NewY_R && m<newY; i++,m++)
	{
		for(j=jstart,n=nstart; j<NewX_R && n<newX; j++,n++)
		{
			pImageNew[m*newX+n] = bilinear_Dou(pImageOld,oldX,oldY,j/ratioX+0.5,i/ratioY+0.5);
		}
	}
}