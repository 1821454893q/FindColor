#define MYDLL_API _declspec(dllexport)_stdcall
#include "mydll.h"


int jyh::myBmp(HBITMAP hBitmap, const char* filePath)
{
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	int pixcelBytes = bitmap.bmHeight * bitmap.bmWidthBytes;
	bmfh.bfType = 'B' | 'M' << 8; //WORD  ��ȡ�����ֽ� һ���ֽڰ�λ  Windows��С�� ���ڴ�Ӻ�����ֽڿ�ʼ ���������MB
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmfh.bfSize = bmfh.bfOffBits + pixcelBytes;

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = bitmap.bmWidth;
	bmih.biHeight = -bitmap.bmHeight;
	bmih.biPlanes = bitmap.bmPlanes;
	bmih.biBitCount = bitmap.bmBitsPixel;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = pixcelBytes;
	bmih.biXPelsPerMeter = bmih.biYPelsPerMeter = bmih.biClrUsed = bmih.biClrImportant = 0;

	FILE* fd;
	if (int errno_t = fopen_s(&fd, filePath, "wb+") != 0)
	{
		LOG(L"errno_t = %d", errno_t);
		return -1;
	}

	fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1, fd);
	fwrite(&bmih, sizeof(BITMAPINFOHEADER), 1, fd);

	PBYTE pByte = (PBYTE)malloc(pixcelBytes);
	GetBitmapBits(hBitmap, pixcelBytes, pByte);
	fwrite(pByte, pixcelBytes, 1, fd);

	fclose(fd);
	free(pByte);
	return 0;
}

MYDLL_API mytime::mytime(void)
{
	QueryPerformanceFrequency(&CPUFrequency);
	ZeroMemory(&BeginTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&EndTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&Interval, sizeof(double));
}

void MYDLL_API mytime::Begin()
{
	QueryPerformanceCounter(&BeginTime);
}

void MYDLL_API mytime::End()
{
	QueryPerformanceCounter(&EndTime);
	Interval = ((double)EndTime.QuadPart - (double)BeginTime.QuadPart) / (double)CPUFrequency.QuadPart;
}

void MYDLL_API myDIB::BindHwnd(HWND hwnd)
{
	DeleteObject(m_DIBMemDC);
	m_bindHwnd = hwnd;
	RECT rect;
	GetWindowRect(m_bindHwnd, &rect);
	m_cxClient = rect.right - rect.left;
	m_cyClient = rect.bottom - rect.top;

	HDC hdc = GetDC(m_bindHwnd);
	m_DIBMemDC = CreateCompatibleDC(hdc);
	ReleaseDC(m_bindHwnd, hdc);
}

void MYDLL_API myDIB::CaptureRange(WORD x, WORD y, WORD w, WORD h)
{
	m_captureX = x;
	m_captureY = y;
	m_captureW = w;
	m_captureH = h;

	m_lineBits = m_captureW * 4; //ʹ��ARGB?

	ZeroMemory(&m_bmi, sizeof(BITMAPINFO));
	m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bmi.bmiHeader.biWidth = m_captureW;
	//������ ͼƬ��������
	m_bmi.bmiHeader.biHeight = -m_captureH;
	m_bmi.bmiHeader.biPlanes = 1;
	m_bmi.bmiHeader.biBitCount = 32;
	m_bmi.bmiHeader.biCompression = BI_RGB;
	m_bmi.bmiHeader.biSizeImage = m_captureW * m_captureH * 4;
	m_bmi.bmiHeader.biXPelsPerMeter = m_bmi.bmiHeader.biYPelsPerMeter = m_bmi.bmiHeader.biClrUsed =
		m_bmi.bmiHeader.biClrImportant = 0;

	DeleteObject(m_DIBhBitmap);
	m_DIBhBitmap = CreateDIBSection(NULL, &m_bmi, DIB_RGB_COLORS, (void**)&m_pByteBtis, NULL, 0);
	SelectObject(m_DIBMemDC, m_DIBhBitmap);

	m_BitsTotal = m_bmi.bmiHeader.biSizeImage;
}

void MYDLL_API myDIB::testbmp()
{
	InterceptGrating();
	FILE* fd;
	fopen_s(&fd, "test.bmp", "wb+");
	BITMAPFILEHEADER hmfh;
	hmfh.bfType = 'B' | 'M' << 8;
	hmfh.bfReserved1 = 0;
	hmfh.bfReserved2 = 0;
	hmfh.bfOffBits = sizeof(BITMAPINFO) + sizeof(BITMAPFILEHEADER);
	hmfh.bfSize = hmfh.bfOffBits + m_BitsTotal;

	fwrite(&hmfh, sizeof(BITMAPFILEHEADER), 1, fd);
	fwrite(&m_bmi, sizeof(BITMAPINFO), 1, fd);

	fwrite(m_pByteBtis, m_BitsTotal, 1, fd);
	fclose(fd);
}


void myDIB::RanlePixel(WORD Ranle)
{
	int XY = 2 * Ranle - 1;
	m_RanlePixel.clear();
	int index = 0;
	for (int x = 0; x < XY; x++)
	{
		for (int y = 0; y < XY; y++)
		{
			int x1 = (x - Ranle) + 1;
			int y2 = (y - Ranle) + 1;
			m_RanlePixel.push_back(vector<int>());
			m_RanlePixel[index].push_back(x1);
			m_RanlePixel[index++].push_back(y2);
		}
	}
}

BOOL myDIB::LoopPixel(int x, int y, const DWORD& colorBitCount, const int& deviation)
{
	for (size_t nums = 1; nums < m_color.size(); nums++)
	{
		for (size_t PixelCount = 0; PixelCount < m_RanlePixel.size(); PixelCount++)
		{
			if (CalulatePoint(x + m_point[nums].x + m_RanlePixel[PixelCount][0], y + m_point[nums].y + m_RanlePixel[PixelCount][1], nums, colorBitCount, deviation))
			{
				if (nums == m_color.size() - 1) return TRUE;
				break;
			}
			if (PixelCount == m_RanlePixel.size() - 1) return FALSE;
		}
	}
	return FALSE;
}


BOOL MYDLL_API myDIB::MatchingGrating(const RECT& rect, const char* rgbcolor, POINT& point, const int& deviation, const int& RanPixel)
{
	InterceptGrating();
	CalculateColor(rgbcolor);
	RanlePixel(RanPixel);
	DWORD colorBitCount = m_bmi.bmiHeader.biBitCount / 8;
	for (long x = rect.left; x < rect.right; x++)
	{
		for (long y = rect.top; y < rect.bottom; y++)
		{
			//�ҵ���һ������
			if (CalulatePoint(x, y, 0, colorBitCount, deviation)) //����ҵ��˵�һ����ɫ
			{
				//������������  �����ϵĻ�
				if (LoopPixel(x, y, colorBitCount, deviation))
				{
					point.x = x;
					point.y = y;
					return TRUE;
				}
			}
		}
	}
	point.x = -1;
	point.y = -1;
	return FALSE;
}

BOOL MYDLL_API myDIB::LMouseClick(WORD x, WORD y, int delay)
{
	if (PostMessage(m_bindHwnd, WM_LBUTTONDOWN, MK_LBUTTON, y << 16 | x))
	{
		::Sleep(delay);
		if (PostMessage(m_bindHwnd, WM_LBUTTONUP, MK_LBUTTON, y << 16 | x))
			return TRUE;
	}
	return FALSE;
}

BOOL MYDLL_API myDIB::RMouseClick(WORD x, WORD y, int delay)
{
	if (PostMessage(m_bindHwnd, WM_RBUTTONDOWN, MK_RBUTTON, y << 16 | x))
	{
		::Sleep(delay);
		if (PostMessage(m_bindHwnd, WM_RBUTTONUP, MK_RBUTTON, y << 16 | x))
			return TRUE;
	}
	return FALSE;
}

HWND MYDLL_API myDIB::GetMainHwnd()
{
	return m_bindHwnd;
}

long myDIB::myAbs(int nums)
{
	if (nums < 0)
		return -nums;
	return nums;
}

void myDIB::InterceptGrating()
{
	HDC hdc = GetDC(m_bindHwnd);
	BitBlt(m_DIBMemDC, m_captureX, m_captureY, m_captureW, m_captureH, hdc, m_captureX, m_captureY, SRCCOPY);
	ReleaseDC(m_bindHwnd, hdc);
}

BOOL myDIB::CalulatePoint(const LONG& positionX, const LONG& positionY, const DWORD& nums, const DWORD& BitCount, const int& deviation)
{
	if (positionX < 0 || positionY < 0 || positionX >= m_cxClient || positionY >= m_cyClient)
		return FALSE;

	byte b = *(m_pByteBtis + ((positionY * m_lineBits) + (positionX * BitCount)));
	byte g = *(m_pByteBtis + ((positionY * m_lineBits) + (positionX * BitCount) + 1));
	byte r = *(m_pByteBtis + ((positionY * m_lineBits) + (positionX * BitCount) + 2));

	if (myAbs(m_color[nums].rgbtBlue - b) < deviation)
		if (myAbs(m_color[nums].rgbtGreen - g) < deviation)
			if (myAbs(m_color[nums].rgbtRed - r) < deviation)
				return TRUE;

	return FALSE;
}

//��ȡ��ɫ�ַ����ķ���
void myDIB::CalculateColor(const char* ch)
{
	m_point.clear();
	m_color.clear();
	int i = 0;
	while (ch[++i] != '\0')
	{
		char temp[8];
		memset(&temp, 0, 8);
		POINT p;
		RGBTRIPLE rgb;
		int j = 0;
		while (ch[i] != '|')
		{
			temp[j++] = ch[i++];
		}
		p.x = atol(temp);
		memset(&temp, 0, 8);
		i++;
		j = 0;
		while (ch[i] != '|')
		{
			temp[j++] = ch[i++];
		}
		p.y = atol(temp);
		i += 3;
		int ax = ch[i] < 58 ? ch[i++] - 48 : ch[i++] - 87;
		int ab = ch[i] < 58 ? ch[i++] - 48 : ch[i++] - 87;
		rgb.rgbtRed = ax * 16 + ab;
		ax = ch[i] < 58 ? ch[i++] - 48 : ch[i++] - 87;
		ab = ch[i] < 58 ? ch[i++] - 48 : ch[i++] - 87;
		rgb.rgbtGreen = ax * 16 + ab;
		ax = ch[i] < 58 ? ch[i++] - 48 : ch[i++] - 87;
		ab = ch[i] < 58 ? ch[i++] - 48 : ch[i++] - 87;
		rgb.rgbtBlue = ax * 16 + ab;
		m_point.push_back(p);
		m_color.push_back(rgb);
	}
}
