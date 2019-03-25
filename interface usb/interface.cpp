#include <windows.h>
#include <iostream>
#include <cstdio>

using namespace std;


typedef int (__stdcall *opendevice)(HANDLE* pHandle, int nDeviceType,int iCom,int iBaud,int nPackageSize,int iDevNum);
typedef int (__stdcall *getimageenroll)(HANDLE hHandle,int nAddr);
typedef int (__stdcall *upload)(HANDLE hHandle,int nAddr,unsigned char* pImageData, int* iImageLength); 
typedef int (__stdcall *count)(HANDLE hHandle,int nAddr,int *iMbNum);
typedef int (__stdcall *im2bmp)(unsigned char* pImgData,const char* pImageFile);




int main()
{
  HINSTANCE hGetProcIDDLL = LoadLibrary("C:\\Users\\ahsan\\Desktop\\fingerprint\\inovace_Tarik\\TEST Demo_V2.7\\TEST Demo_V2.7\\SynoAPIEx.dll");

  if (!hGetProcIDDLL) {
    cout << "could not load the dynamic library" << endl;
    return EXIT_FAILURE;
  }

  //resolve function address here
  opendevice open = (opendevice)GetProcAddress(hGetProcIDDLL, "PSOpenDeviceEx");
  getimageenroll getImage = (getimageenroll) GetProcAddress(hGetProcIDDLL, "PSGetImage");
  upload up = (upload) GetProcAddress(hGetProcIDDLL, "PSUpImage");
  count countTemp= (count) GetProcAddress(hGetProcIDDLL, "PSTemplateNum");
  im2bmp saveImage = (im2bmp) GetProcAddress(hGetProcIDDLL, "PSImgData2BMP");
  if (!open || !getImage || !up || !countTemp || !saveImage) {
    std::cout << "could not locate the function" << std::endl;
    return EXIT_FAILURE;
  }
  HANDLE hHandle;
  std::cout << "open returned " << open(&hHandle,2,1,1,2,0) << std::endl;
  cout << "Handle " << hHandle << endl;

  int * num = new int;

  int nAddr = 0xFFFFFFFF;
  if(countTemp(hHandle,nAddr,num)==0){
    cout << "fpt in library: " << *num << endl;
  }


  unsigned char* pImageData = new unsigned char[256*288];
  int iImageLength;

  while(true){
    if(getImage(hHandle,nAddr)==0){
        if(up(hHandle,nAddr,pImageData, &iImageLength)==0){
          if(iImageLength != 0){
            cout << "size "<< iImageLength << endl;
            saveImage(pImageData,"C:\\Users\\ahsan\\Desktop\\a.bmp");
            break;
          }
        }
    }
  }
  return EXIT_SUCCESS;
}