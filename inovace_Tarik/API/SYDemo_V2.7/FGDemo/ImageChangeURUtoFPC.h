
//ImageChangeURUtoFPC.h

#ifndef ImageChangeURUtoFPC_H_2010
#define ImageChangeURUtoFPC_H_2010
///////////////////////////////////////////////////



#define FILL_COLOR 255	//填充灰度值
unsigned char bilinear_Dou(unsigned char *pImage,long lWidth,long lHeight, double x,double y);
void ImageZoom(unsigned char *pImageOld,int oldX,int oldY,unsigned char *pImageNew,int newX,int newY,float ratioX,float ratioY);
void VertMirror(unsigned char*iopImage,int iX, int iY);


/*
**************************************************************************************
名 称: void ImageChangeURUtoFPC(unsigned char *iopImage)
功 能: URU图像转换为FPC的图像格式,225X313-->256X288
参 数:  iopImage-输入输出，图像数据
返回值:	无
**************************************************************************************
*/
void ImageChangeURUtoFPC(unsigned char *iopImage);




///////////////////////////////////////////////////
#endif	//ImageChangeURUtoFPC_H_2010