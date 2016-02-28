#include <windows.h>
#include <math.h>

#define PI 3.14159265358979323846

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
	float X;
	float Y;
};

struct Vec3
{
	float X;
	float Y;
	float Z;
};

struct Edge
{
	int edge1;
	int edge2;
};

inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs)
{
	Vec2 result = { lhs.X + rhs.X, lhs.Y + rhs.Y };
	return result;
}

inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs)
{
	Vec2 result = { lhs.X - rhs.X, lhs.Y - rhs.Y };
	return result;
}

inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs)
{
	Vec3 result = { lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z};
	return result;
}

inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs)
{
	Vec3 result = { lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z};
	return result;
}

inline Vec3 operator*(const Vec3& lhs, const float& rhs)
{
	Vec3 result = { lhs.X * rhs, lhs.Y * rhs, lhs.Z * rhs };
	return result;
}

inline Vec3 operator*(const float& lhs, const Vec3& rhs)
{
	return rhs * lhs;
}

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

inline void Swap(float* value1, float* value2)
{
	float temp = *value1;
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

void DrawLine(float X1, float Y1, float X2, float Y2, Color color, unsigned char* Buffer)
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

		for (float i = X1; i < X2; i++)
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

		for (float i = Y1; i < Y2; i++)
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

void DrawPolygon(Vec2* polygon, int vertexCount, Color color, unsigned char* Buffer)
{
	for (int i = 0; i < vertexCount; i++)
	{
		DrawLine(polygon[i].X, polygon[i].Y, polygon[(i + 1) % vertexCount].X, polygon[(i + 1) % vertexCount].Y, color, Buffer);
	}
}

void DrawPolygon(Vec3* polygon, Edge* edges, int edgeCount, Color color, unsigned char* Buffer)
{
	//TODO(Simon): Projection!
	for (int i = 0; i < edgeCount; i++)
	{
		DrawLine(polygon[edges[i].edge1].X, polygon[edges[i].edge1].Y, polygon[edges[i].edge2].X, polygon[edges[i].edge2].Y, color, Buffer);
	}
}

void RotatePolygon(Vec2* polygon, Vec2* rotatedPolygon, int vertexCount, Vec2 center, float degrees)
{
	float radians = degrees * (PI/180);

	for (int i = 0; i < vertexCount; i++)
	{
		Vec2 vec = polygon[i];
		vec = vec - center;

		Vec2 rotated = { 0 };
		rotated.X = vec.X * cos(radians) - vec.Y * sin(radians);
		rotated.Y = vec.Y * cos(radians) + vec.X * sin(radians);

		vec = rotated + center;
		rotatedPolygon[i] = vec;
	}
}

void RotatePolygonX(Vec3* polygon, Vec3* rotatedPolygon, int vertexCount, Vec3 center, float degrees)
{
	float radians = degrees * (PI/180);

	memcpy(rotatedPolygon, polygon, vertexCount * sizeof(Vec3));

	for (int i = 0; i < vertexCount; i++)
	{
		Vec3 vec = rotatedPolygon[i];
		vec = vec - center;

		vec.Y = vec.Y * (float)cos(radians) - vec.Z * (float)sin(radians);
		vec.Z = vec.Y * (float)sin(radians) + vec.Z * (float)cos(radians);

		vec = vec + center;
		rotatedPolygon[i] = vec;
	}
}

void RotatePolygonY(Vec3* polygon, Vec3* rotatedPolygon, int vertexCount, Vec3 center, float degrees)
{
	float radians = degrees * (PI/180);

	memcpy(rotatedPolygon, polygon, vertexCount * sizeof(Vec3));

	for (int i = 0; i < vertexCount; i++)
	{
		Vec3 vec = rotatedPolygon[i];
		vec = vec - center;

		vec.X = vec.X * (float)cos(radians) + vec.Z * (float)sin(radians);
		vec.Z = -vec.X * (float)sin(radians) + vec.Z * (float)cos(radians);

		vec = vec + center;
		rotatedPolygon[i] = vec;
	}
}

void RotatePolygonZ(Vec3* polygon, Vec3* rotatedPolygon, int vertexCount, Vec3 center, float degrees)
{
	float radians = degrees * (PI/180);

	memcpy(rotatedPolygon, polygon, vertexCount * sizeof(Vec3));

	for (int i = 0; i < vertexCount; i++)
	{
		Vec3 vec = rotatedPolygon[i];
		vec = vec - center;

		vec.X = vec.X * (float)sin(radians) - vec.Y * (float)cos(radians);
		vec.Y = vec.X * (float)cos(radians) + vec.Y * (float)sin(radians);

		vec = vec + center;
		rotatedPolygon[i] = vec;
	}
}

void ScalePolygon(Vec2* polygon, Vec2* scaledPolygon, int vertexCount, Vec2 center, float scaleX, float scaleY)
{
	for (int i = 0; i < vertexCount; i++)
	{
		Vec2 vec = polygon[i];
		vec = vec - center;

		Vec2 scaled = { 0 };
		scaled.X = vec.X * scaleX;
		scaled.Y = vec.Y * scaleY;

		vec = scaled + center;
		scaledPolygon[i] = vec;
	}
}

void TranslatePolygon(Vec2* polygon, Vec2* translatedPolygon, int vertexCount, float translateX, float translateY)
{
	for (int i = 0; i < vertexCount; i++)
	{
		Vec2 vec = polygon[i];

		Vec2 translated = { 0 };
		translated.X = vec.X + translateX;
		translated.Y = vec.Y + translateY;

		translatedPolygon[i] = translated;
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

	ShowWindow(MainWindow, nShowCmd);

	// define our bitmap info
	BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
	BitMapInfo.bmiHeader.biWidth = BufferWidth;
	BitMapInfo.bmiHeader.biHeight = BufferHeight;
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

		Vec3 polygon[] = {
			{200, 200, 100},
			{200, 400, 100},
			{400, 400, 100},
			{400, 200, 100},
			{200, 200, 300},
			{200, 400, 300},
			{400, 400, 300},
			{400, 200, 300},
		};

		Vec2 polygon2[] {
			{600, 600},
			{600, 800},
			{800, 800},
			{800, 600},
		};

		Edge edges[] = {
			{0, 1},
			{1, 2},
			{2, 3},
			{3, 0},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 4},
			{0, 4},
			{1, 5},
			{2, 6},
			{3, 7},
		};


		int vertexCount = sizeof(polygon) / sizeof(*polygon);
		int edgeCount = sizeof(edges) / sizeof(*edges);

		Vec3* rotatedPolygon = (Vec3*)malloc(vertexCount * sizeof(Vec3));
		Vec3* scaledPolygon = (Vec3*)malloc(vertexCount * sizeof(Vec3));
		Vec3* translatedPolygon = (Vec3*)malloc(vertexCount * sizeof(Vec3));

		Vec3 center = { 300, 300, 200 };
		static Vec3 degrees = { 0, 0, 0 };
		//degrees.X += 0.25;
		//degrees.Y += 0.25;
		degrees.Z += 0.25;

		RotatePolygonX(polygon, rotatedPolygon, vertexCount, center, degrees.X);
		RotatePolygonY(rotatedPolygon, rotatedPolygon, vertexCount, center, degrees.Y);
		RotatePolygonZ(rotatedPolygon, rotatedPolygon, vertexCount, center, degrees.Z);

		DrawPolygon(rotatedPolygon, edges, edgeCount, color, BackBuffer);

		free(rotatedPolygon);
		free(scaledPolygon);
		free(translatedPolygon);

		HDC dc = GetDC(MainWindow);
		StretchDIBits(dc,
			0, 0, BufferWidth, BufferHeight,
			0, 0, BufferWidth, BufferHeight,
			BackBuffer, (BITMAPINFO*)&BitMapInfo,
			DIB_RGB_COLORS, SRCCOPY);
		ReleaseDC(MainWindow, dc);
	}

	return EXIT_SUCCESS;
}
