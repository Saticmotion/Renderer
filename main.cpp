#include <windows.h>
#include <math.h>

int Running = 1;
int BufferWidth = 1600;
int BufferHeight = 900;
int BytesPerPixel = 4;
unsigned char* BackBuffer;


typedef struct dibinfo_s
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD          acolors[256];
} dibinfo_t;

dibinfo_t BitMapInfo = { 0 };

inline int Offset(int Width, int X, int Y)
{
	return Y * Width + X;
}

void DrawRect(int X, int Y, int Width, int Height, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char* Buffer)
{
	if (X < 0)
		X = 0;

	if (Y < 0)
		Y = 0;

	if ((X + Width) > BufferWidth)
	{
		Width = BufferWidth - X;
	}

	if ((Y + Height) > BufferHeight)
	{
		Height = BufferHeight - Y;
	}

	unsigned int Color = ((Red << 16) | (Green << 8) | Blue);

	// move to first pixel
	Buffer += (BufferWidth * BytesPerPixel * Y) + (X*BytesPerPixel);

	int* BufferWalker = (int*)Buffer;
	for (int HeightWalker = 0; HeightWalker < Height; HeightWalker++)
	{
		for (int WidthWalker = 0; WidthWalker < Width; WidthWalker++)
		{
			*BufferWalker = Color;
			BufferWalker++;
		}

		Buffer += BufferWidth * BytesPerPixel;
		BufferWalker = (int*)Buffer;
	}
}

void DrawLine(int X1, int Y1, int X2, int Y2, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char* Buffer)
{
	float yIncrease = (Y2 - (float)Y1)/(X2 - (float)X1);
	float currentY = Y1;
	int* BufferWalker = (int*)Buffer;

	for (int i = X1; i < X2; i++)
	{
		int offset1 = Offset(BufferWidth, i, floor(currentY));
		int offset2 = Offset(BufferWidth, i, floor(currentY + 1));
		float frac1 = 1 - (currentY - floor(currentY));
		float frac2 = 1 - frac1;

		int currentColor1 = BufferWalker[offset1];
		int currentColor2 = BufferWalker[offset2];

		int red1 = (Red * frac1) + ((currentColor1 >> 16) * (1 - frac1));
		int red2 = (Red * frac2) + ((currentColor2 >> 16) * (1 - frac2));
		int green1 = (Green * frac1) + ((currentColor1 & 0x0000ff00 >> 8) * (1 - frac1));
		int green2 = (Green * frac2) + ((currentColor2 & 0x0000ff00 >> 8) * (1 - frac2));
		int blue1 = (Blue * frac1) + ((currentColor1 & 0x000000ff) * (1 - frac1));
		int blue2 = (Blue * frac2) + ((currentColor2 & 0x000000ff) * (1 - frac2));

		unsigned int Color1 = ((red1 << 16) | (green1 << 8) | blue1);
		unsigned int Color2 = ((red2 << 16) | (green2 << 8) | blue2);

		BufferWalker[offset1] = Color1;
		BufferWalker[offset2] = Color2;
		currentY += yIncrease;
	}
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;

	switch (uMsg)
	{
	case WM_DESTROY:
		Running = 0;
		break;

	case WM_KEYUP:
		break;

	default:
		Result = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return Result;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// define our window class
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "Module 3";

	RegisterClassExA(&wc);

	DWORD dwExStyle = 0;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW;

	BOOL Fullscreen = TRUE;

	if (Fullscreen)
	{
		DEVMODE dmScreenSettings = { 0 };
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = BufferWidth;
		dmScreenSettings.dmPelsHeight = BufferHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
		{
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP;
		}
		else
		{
			Fullscreen = FALSE;
		}
	}

	// create rectangle for window
	RECT r = { 0 };
	r.right = BufferWidth;
	r.bottom = BufferHeight;
	AdjustWindowRectEx(&r, dwStyle, 0, dwExStyle);

	// create our window
	HWND MainWindow = CreateWindowEx(
		dwExStyle, "Module 3",
		"Lesson 3.3", dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		r.right - r.left, r.bottom - r.top,
		NULL, NULL,
		hInstance, 0);

	if (Fullscreen)
		SetWindowLong(MainWindow, GWL_STYLE, 0);

	ShowWindow(MainWindow, nShowCmd);

	// define our bitmap info
	BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
	BitMapInfo.bmiHeader.biWidth = BufferWidth;
	BitMapInfo.bmiHeader.biHeight = -BufferHeight;
	BitMapInfo.bmiHeader.biPlanes = 1;
	BitMapInfo.bmiHeader.biBitCount = 8 * BytesPerPixel;
	BitMapInfo.bmiHeader.biCompression = BI_RGB;

	BackBuffer = (unsigned char*)malloc(BufferWidth * BufferHeight * BytesPerPixel);

	MSG msg;
	while (Running)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		int *MemoryWalker = (int*)BackBuffer;
		for (int Height = 0; Height < BufferHeight; Height++)
		{
			for (int Width = 0; Width < BufferWidth; Width++)
			{
				*MemoryWalker++ = 0;
			}
		}

		DrawLine(100, 100, 800, 600, 255, 255, 255, BackBuffer);

		HDC dc = GetDC(MainWindow);
		StretchDIBits(dc,
			0, 0, BufferWidth, BufferHeight,
			0, 0, BufferWidth, BufferHeight,
			BackBuffer, (BITMAPINFO*)&BitMapInfo,
			DIB_RGB_COLORS, SRCCOPY);
		DeleteDC(dc);
	}

	return EXIT_SUCCESS;
}