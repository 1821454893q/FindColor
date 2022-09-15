#pragma once
#ifdef MYDLL_API
#else
#define MYDLL_API  _declspec(dllimport)_stdcall
#endif // MYDLL_API
#include <Windows.h>
#include <stdio.h>
#include <vector>
#include <mutex>
#include <algorithm>

using namespace std;


#define LOG(fromat,...) wchar_t wch[512];\
wsprintf(wch,fromat,__VA_ARGS__);\
MessageBox(NULL,wch,L"调试信息",MB_OKCANCEL);

class jyh
{
public:
	int MYDLL_API myBmp(HBITMAP hBitmap, const char* filePath);
private:
};

/*微秒计算*/
class  mytime
{
private:
	LARGE_INTEGER BeginTime;
	LARGE_INTEGER EndTime;
	LARGE_INTEGER CPUFrequency;
public:
	MYDLL_API mytime();
	double Interval;
public:
	void MYDLL_API Begin();
	void MYDLL_API End();
};


class  myDIB
{
public:
	~myDIB()
	{
		DeleteObject(m_DIBMemDC);
		DeleteObject(m_DIBhBitmap);
	}
	void MYDLL_API BindHwnd(HWND hwnd);
	void MYDLL_API CaptureRange(WORD x, WORD y, WORD w, WORD h);
	void MYDLL_API testbmp();
	BOOL MYDLL_API MatchingGrating(const RECT& rect, const char* rgbcolor, POINT& point, const int& deviation = 13, const int& RanPixel = 1);
	BOOL MYDLL_API LMouseClick(WORD x, WORD y, int delay = 10);
	BOOL MYDLL_API RMouseClick(WORD x, WORD y, int delay = 10);
	HWND MYDLL_API GetMainHwnd();
private:
	void RanlePixel(WORD);
	long myAbs(int nums);
	void InterceptGrating();
	BOOL CalulatePoint(const LONG& positionX, const LONG& positionY, const DWORD& nums, const DWORD& BitCount, const int& deviation);
	void CalculateColor(const char* ch);
	BOOL LoopPixel(int x, int y, const DWORD& colorBitCount, const int& deviation);
	PBYTE m_pByteBtis; //DIB像素的指针
	DWORD m_BitsTotal; //像素的总大小
	HBITMAP m_DIBhBitmap;
	BITMAPINFO m_bmi;
	DWORD m_lineBits; //单行的字节
	HDC m_DIBMemDC;
	HWND m_bindHwnd;
	long m_cxClient, m_cyClient;
	WORD m_captureX, m_captureY, m_captureW, m_captureH;

	vector<RGBTRIPLE> m_color;
	vector<POINT> m_point;
	vector<vector<int>> m_RanlePixel;
};