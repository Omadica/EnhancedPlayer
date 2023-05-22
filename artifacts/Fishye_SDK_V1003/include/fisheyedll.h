#pragma once
#define DLLDIR  __declspec(dllimport)

enum { FISHEYE_EQUIDISTANT = 0, FISHEYE_STEREOGRAPHIC = 1, FISHEYE_EQUISOLID = 2};
enum { FISHEYE_NORMAL = 0, FISHEYE_FLIP = 1, FISHEYE_MIRROR = 2, FISHEYE_ROTATE = 3, FISHEYE_CLOCKWISE = 4, FISHEYE_COUNTERCLOCKWISE = 5 };
enum { FISHEYE_BOTTOMUP = 0, FISHEYE_TOPDOWN = 1};

extern "C"{
int DLLDIR __stdcall Fisheye_NewObj();
int DLLDIR __stdcall Fisheye_SetParameters(int hd, int view_w, int view_h, int cx, int cy, int diameter, float focalLength, int projection, int rotation, const BITMAPINFOHEADER * input_header);
int DLLDIR __stdcall Fisheye_Dewarp(int hd, int pan, int tilt, float zoom, const BITMAPINFOHEADER * input_header, const unsigned char *input_bmp, BITMAPINFOHEADER * output_header, unsigned char *output, int out_imgW, int out_imgH);
int DLLDIR __stdcall Fisheye_Wall_Dewarp(int hd, int longitude, int latitude, float zoom, const BITMAPINFOHEADER * input_header, const unsigned char *input_bmp, BITMAPINFOHEADER * output_header, unsigned char *output, int out_imgW, int out_imgH);
int DLLDIR __stdcall Fisheye_360_Standard(int hd, int alpha, int beta, int isClockwise, int inStart, int inEnd, const BITMAPINFOHEADER * input_header, const unsigned char *input_bmp, BITMAPINFOHEADER * output_header, unsigned char *output, int out_imgW, int out_imgH);
int DLLDIR __stdcall Fisheye_360_Mercator(int hd, int alpha, int beta, int isClockwise, const BITMAPINFOHEADER * input_header, const unsigned char *input_bmp, BITMAPINFOHEADER * output_header, unsigned char *output, int out_imgW, int out_imgH);
int DLLDIR __stdcall Fisheye_360_Standard_Get_OutImgW(int hd, int beta, int inStart, int inEnd, int out_imgH, int * out_imgWPt);
int DLLDIR __stdcall Fisheye_360_Standard_Get_OutImgH(int hd, int beta, int inStart, int inEnd, int out_imgW, int * out_imgHPt);
int DLLDIR __stdcall Fisheye_GetPTFromOutImg(int hd, int x, int y, int *panPt, int *tiltPt);
int DLLDIR __stdcall Fisheye_GetPosFromOutImg(int hd, int x, int y, int *longitudePt, int * latitudePt);
int DLLDIR __stdcall Fisheye_GetPTValue(int hd, int x, int y, int *panPt, int *tiltPt);
int DLLDIR __stdcall Fisheye_GetPosValue(int hd, int x, int y, int *longitudePt, int * latitudePt);
int DLLDIR __stdcall Fisheye_Free(int hd);
int DLLDIR __stdcall Fisheye_Wall_180_Panorama(int hd, const BITMAPINFOHEADER * input_header, const unsigned char *input_bmp, BITMAPINFOHEADER * output_header, unsigned char *output, int out_imgW, int out_imgH);
int DLLDIR __stdcall Fisheye_SetOptionInt(int hd, const char * paramName, int value);
}
