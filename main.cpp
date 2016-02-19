#include <windows.h>
#include <math.h>

int Running = 1;
int BufferWidth = 1600;
int BufferHeight = 900;
int BytesPerPixel = 4;
unsigned char* BackBuffer;

struct dibinfo
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD          acolors[256];
};

union Color
{
	int argb;
	struct
	{
		unsigned char red;
		unsigned char green;
		unsigned char blue;
		unsigned char alpha;
	};
};

struct Vec2
{
	int X;
	int Y;
};

dibinfo BitMapInfo = { 0 };

inline int Offset(int Width, int X, int Y)
{
	return Y * Width + X;
}

inline Color MixColors(Color color1, Color color2, float color1frac)
{
	Color newColor = { 0 };

	newColor.red = (color1.red * color1frac) + (color2.red * (1 - color1frac));
	newColor.green = (color1.green * color1frac) + (color2.green * (1 - color1frac));
	newColor.blue = (color1.blue * color1frac) + (color2.blue * (1 - color1frac));

	return newColor;
}

inline void Swap(int* value1, int* value2)
{
	int temp = *value1;
	*value1 = *value2;
	*value2 = temp;
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

void DrawLine(int X1, int Y1, int X2, int Y2, Color color, unsigned char* Buffer)
{
	
	float yIncrease = (Y2 - (float)Y1)/(X2 - (float)X1);

	if (yIncrease < 1 && yIncrease > -1)
	{
		if (X2 < X1)
		{
			Swap(&Y2, &Y1);
			Swap(&X2, &X1);
		}

		float currentY = Y1;
		int* BufferWalker = (int*)Buffer;

		for (int i = X1; i < X2; i++)
		{
			int offset1;
			int offset2; 
			float frac1;
			float frac2;

			offset1 = Offset(BufferWidth, i, floor(currentY));
			offset2 = Offset(BufferWidth, i, floor(currentY + 1));

			frac1 = 1 - (currentY - floor(currentY));
			frac2 = 1 - frac1;

			Color currentColor1 = { BufferWalker[offset1] };
			Color currentColor2 = { BufferWalker[offset2] };

			Color Color1 = MixColors(color, currentColor1, frac1);
			Color Color2 = MixColors(color, currentColor2, frac2);

			BufferWalker[offset1] = Color1.argb;
			BufferWalker[offset2] = Color2.argb;
			currentY += yIncrease;
		}
	}
	else
	{
		if (Y2 < Y1)
		{
			Swap(&X2, &X1);
			Swap(&Y2, &Y1);
		}

		float xIncrease = (X2 - (float)X1)/(Y2 - (float)Y1);
		float currentX = X1;
		int* BufferWalker = (int*)Buffer;

		for (int i = Y1; i < Y2; i++)
		{
			int offset1;
			int offset2; 
			float frac1;
			float frac2;

			offset1 = Offset(BufferWidth, floor(currentX), i);
			offset2 = Offset(BufferWidth, floor(currentX + 1), i);

			frac1 = 1 - (currentX - floor(currentX));
			frac2 = 1 - frac1;

			Color currentColor1 = { BufferWalker[offset1] };
			Color currentColor2 = { BufferWalker[offset2] };

			Color Color1 = MixColors(color, currentColor1, frac1);
			Color Color2 = MixColors(color, currentColor2, frac2);

			BufferWalker[offset1] = Color1.argb;
			BufferWalker[offset2] = Color2.argb;
			currentX += xIncrease;
		}
	}
}

void DrawPolygon(Vec2* polygon, int vbo_size, Color color, unsigned char* Buffer )
{
	for (int i = 0; i < vbo_size; i++)
	{
		DrawLine(polygon[i].X, polygon[i].Y, polygon[(i + 1) % vbo_size].X, polygon[(i + 1) % vbo_size].Y, color, Buffer);
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
				*MemoryWalker++ = 0x00000000;
			}
		}

		Color color = {0};
		color.red = 255;
		color.green = 255;
		color.blue = 255;

		Vec2 polygon[] = {
			{100, 100},
			{100, 200},
			{200, 200},
			{200, 100}
		};

		int size = sizeof(polygon) / sizeof(*polygon); 

		DrawPolygon(polygon, size, color, BackBuffer);

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