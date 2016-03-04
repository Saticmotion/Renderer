#include <windows.h>
#include <math.h>
#include "math_r.cpp"

#define PI 3.14159265358979323846
typedef unsigned char uchar;

int Running = 1;
int BufferWidth = 1600;
int BufferHeight = 900;
Vec3 ScreenCenter = {BufferWidth / 2.0f, BufferHeight / 2.0f, 0};
int BytesPerPixel = 4;
uchar* BackBuffer;

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
		uchar blue;
		uchar green;
		uchar red;
		uchar alpha;
	};
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

inline void Swap(float* value1, float* value2)
{
	float temp = *value1;
	*value1 = *value2;
	*value2 = temp;
}

void ClearScreen(uchar* backBuffer, int bufferWidth, int bufferHeight, Color color)
{
	int *MemoryWalker = (int*)backBuffer;
	for (int Height = 0; Height < bufferHeight; Height++)
	{
		for (int Width = 0; Width < bufferWidth; Width++)
		{
			*MemoryWalker++ = color.argb;
		}
	}
}

void DrawRect(int X, int Y, int Width, int Height, uchar Red, uchar Green, uchar Blue, uchar* Buffer)
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

void DrawLine(Vec2 p1, Vec2 p2, Color color, uchar* Buffer)
{

	float yIncrease = (p2.Y - (float)p1.Y)/(p2.X - (float)p1.X);

	if (yIncrease < 1 && yIncrease > -1)
	{
		if (p2.X < p1.X)
		{
			Swap(&p2.Y, &p1.Y);
			Swap(&p2.X, &p1.X);
		}

		float currentY = p1.Y;
		int* BufferWalker = (int*)Buffer;

		for (float i = p1.X; i < p2.X; i++)
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
		if (p2.Y < p1.Y)
		{
			Swap(&p2.X, &p1.X);
			Swap(&p2.Y, &p1.Y);
		}

		float xIncrease = (p2.X - (float)p1.X)/(p2.Y - (float)p1.Y);
		float currentX = p1.X;
		int* BufferWalker = (int*)Buffer;

		for (float i = p1.Y; i < p2.Y; i++)
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

void DrawPolygon(Vec2* polygon, int vertexCount, Color color, uchar* Buffer)
{
	for (int i = 0; i < vertexCount; i++)
	{
		DrawLine(polygon[i], polygon[(i + 1) % vertexCount], color, Buffer);
	}
}

void DrawPolygon(Vec3* polygon, Edge* edges, int edgeCount, Color color, uchar* Buffer)
{
	for (int i = 0; i < edgeCount; i++)
	{
		Vec3 p1 = polygon[edges[i].edge1];
		Vec3 p2 = polygon[edges[i].edge2];

		float fov = 90 * (PI / 180);
		Matrix4x4 projection = { 0 };

		projection.a11 = 1.0f / (((float)BufferWidth / BufferHeight) * tan(fov / 2.0f));
		projection.a22 = 1.0f / (tan(fov / 2.0f));
		projection.a33 = 1;
		projection.a43 = 1;

		p1 = projection * p1;
		p1 = p1 / p1.Z;
		p1.X = ((p1.X + 1.0f) / 2.0f) * BufferWidth;
		p1.Y = ((p1.Y + 1.0f) / 2.0f) * BufferHeight;

		p2 = projection * p2;
		p2 = p2 / p2.Z;
		p2.X = ((p2.X + 1.0f) / 2.0f) * BufferWidth;
		p2.Y = ((p2.Y + 1.0f) / 2.0f) * BufferHeight;

		DrawLine(p1.XY, p2.XY, color, Buffer);
	}
}

void RotatePolygon(Vec2* polygon, Vec2* rotatedPolygon, int vertexCount, Vec2 center, float degrees)
{
	float radians = degrees * (PI/180);
	Matrix2x2 rotationMatrix = {
		(float)cos(radians),
		(float)-sin(radians),
		(float)sin(radians),
		(float)cos(radians)
	};

	for (int i = 0; i < vertexCount; i++)
	{
		Vec2 vec = polygon[i];
		vec = vec - center;

		vec = rotationMatrix * vec;

		vec = vec + center;
		rotatedPolygon[i] = vec;
	}
}

void RotatePolygonX(Vec3* polygon, Vec3* rotatedPolygon, int vertexCount, Vec3 center, float degrees)
{
	float radians = degrees * (PI/180);

	Matrix3x3 rotationMatrix = { 0 };
	rotationMatrix.a11 = 1;
	rotationMatrix.a22 = (float)cos(radians);
	rotationMatrix.a23 = (float)-sin(radians);
	rotationMatrix.a32 = (float)sin(radians);
	rotationMatrix.a33 = (float)cos(radians);

	for (int i = 0; i < vertexCount; i++)
	{
		Vec3 vec = polygon[i];
		vec = vec - center;

		vec = rotationMatrix * vec;

		vec = vec + center;
		rotatedPolygon[i] = vec;
	}
}

void RotatePolygonY(Vec3* polygon, Vec3* rotatedPolygon, int vertexCount, Vec3 center, float degrees)
{
	float radians = degrees * (PI/180);

	Matrix3x3 rotationMatrix = { 0 };
	rotationMatrix.a11 = (float)cos(radians);
	rotationMatrix.a13 = (float)sin(radians);
	rotationMatrix.a22 = 1;
	rotationMatrix.a31 = (float)-sin(radians);
	rotationMatrix.a33 = (float)cos(radians);

	for (int i = 0; i < vertexCount; i++)
	{
		Vec3 vec = polygon[i];
		vec = vec - center;

		vec = rotationMatrix * vec;

		vec = vec + center;
		rotatedPolygon[i] = vec;
	}
}

void RotatePolygonZ(Vec3* polygon, Vec3* rotatedPolygon, int vertexCount, Vec3 center, float degrees)
{
	float radians = degrees * (PI/180);

	Matrix3x3 rotationMatrix = { 0 };
	rotationMatrix.a11 = (float)cos(radians);
	rotationMatrix.a12 = (float)-sin(radians);
	rotationMatrix.a21 = (float)sin(radians);
	rotationMatrix.a22 = (float)cos(radians);
	rotationMatrix.a33 = 1;

	for (int i = 0; i < vertexCount; i++)
	{
		Vec3 vec = polygon[i];
		vec = vec - center;

		vec = rotationMatrix * vec;

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

void ScalePolygon(Vec3* polygon, Vec3* scaledPolygon, int vertexCount, Vec3 center, float scaleX, float scaleY, float scaleZ)
{
	for (int i = 0; i < vertexCount; i++)
	{
		Vec3 vec = polygon[i];
		vec = vec - center;

		Vec3 scaled = { 0 };
		scaled.X = vec.X * scaleX;
		scaled.Y = vec.Y * scaleY;
		scaled.Z = vec.Z * scaleY;

		vec = scaled + center;
		scaledPolygon[i] = vec;
	}
}

void TranslatePolygon(Vec2* polygon, Vec2* translatedPolygon, int vertexCount, Vec2 translate)
{
	for (int i = 0; i < vertexCount; i++)
	{
		translatedPolygon[i] = polygon[i] + translate;
	}
}

void TranslatePolygon(Vec3* polygon, Vec3* translatedPolygon, int vertexCount, Vec3 translate)
{
	for (int i = 0; i < vertexCount; i++)
	{
		translatedPolygon[i] = polygon[i] + translate;
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

	default:
		Result = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return Result;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "Module 3";

	RegisterClassExA(&wc);

	DWORD dwExStyle = 0;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW;

	RECT r = { 0 };
	r.right = BufferWidth;
	r.bottom = BufferHeight;
	AdjustWindowRectEx(&r, dwStyle, 0, dwExStyle);

	HWND MainWindow = CreateWindowEx(
		dwExStyle, "Module 3",
		"Lesson 3.3", dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		r.right - r.left, r.bottom - r.top,
		NULL, NULL,
		hInstance, 0);

	ShowWindow(MainWindow, nShowCmd);

	BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
	BitMapInfo.bmiHeader.biWidth = BufferWidth;
	BitMapInfo.bmiHeader.biHeight = BufferHeight;
	BitMapInfo.bmiHeader.biPlanes = 1;
	BitMapInfo.bmiHeader.biBitCount = 8 * BytesPerPixel;
	BitMapInfo.bmiHeader.biCompression = BI_RGB;

	BackBuffer = (uchar*)malloc(BufferWidth * BufferHeight * BytesPerPixel);

	MSG msg;
	while (Running)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Color bgColor = {0};
		bgColor.red = 39;
		bgColor.green = 40;
		bgColor.blue = 34;

		Color lineColor = {0};
		lineColor.red = 166;
		lineColor.green = 226;
		lineColor.blue = 46;

		ClearScreen(BackBuffer, BufferWidth, BufferHeight, bgColor);

		Vec3 center = { 0, 0, 0 };

		Vec3 polygon[] = {
			{ 0.607f,  0.000f,  0.795f },
			{ 0.188f,  0.577f,  0.795f },
			{-0.491f,  0.357f,  0.795f },
			{-0.491f, -0.357f,  0.795f },
			{ 0.188f, -0.577f,  0.795f },
			{ 0.982f,  0.000f,  0.188f },
			{ 0.304f,  0.934f,  0.188f },
			{-0.795f,  0.577f,  0.188f },
			{-0.795f, -0.577f,  0.188f },
			{ 0.304f, -0.934f,  0.188f },
			{ 0.795f,  0.577f, -0.188f },
			{-0.304f,  0.934f, -0.188f },
			{-0.982f,  0.000f, -0.188f },
			{-0.304f, -0.934f, -0.188f },
			{ 0.795f, -0.577f, -0.188f },
			{ 0.491f,  0.357f, -0.795f },
			{-0.188f,  0.577f, -0.795f },
			{-0.607f,  0.000f, -0.795f },
			{-0.188f, -0.577f, -0.795f },
			{ 0.491f, -0.357f, -0.795f },
		};

		Edge edges[] = {
			{0,1},
			{1,2},
			{2,3},
			{3,4},
			{4,0},

			{0,1},
			{1,6},
			{6,10},
			{10,5},
			{5,0},

			{1,2},
			{2,7},
			{7,11},
			{11,6},
			{6,1},

			{2,3},
			{3,8},
			{8,12},
			{12,7},
			{7,2},

			{3,4},
			{4,9},
			{9,13},
			{13,8},
			{8,3},

			{4,0},
			{0,5},
			{5,14},
			{14,9},
			{9,4},

			{15,16},
			{16,11},
			{11,6},
			{6,10},
			{10,15},

			{16,17},
			{17,12},
			{12,7},
			{7,11},
			{11,16},

			{17,18},
			{18,13},
			{13,8},
			{8,12},
			{12,17},

			{18,19},
			{19,14},
			{14,9},
			{9,13},
			{13,18},

			{19,15},
			{15,10},
			{10,5},
			{5,14},
			{14,19},

			{15,16},
			{16,17},
			{17,18},
			{18,19},
			{19,15},
		};

		int vertexCount = sizeof(polygon) / sizeof(*polygon);
		int edgeCount = sizeof(edges) / sizeof(*edges);

		//Vec3* rotatedPolygon = (Vec3*)calloc(vertexCount * sizeof(Vec3), sizeof(Vec3));

		static Vec3 degrees = { 0, 0, 0 };
		degrees.X -= 0.3;
		degrees.Z -= 0.25;

		static Vec3 translation = { 0, 0, 10 };
		static float translationIncrease = 0.03f;
		translation.X += translationIncrease;

		if (translation.X > 10 || translation.X < -10)
		{
			translationIncrease = -translationIncrease;
		}

		RotatePolygonX(polygon, polygon, vertexCount, center, degrees.X);
		RotatePolygonZ(polygon, polygon, vertexCount, center, degrees.Z);
		ScalePolygon(polygon, polygon, vertexCount, center, 2, 2, 2);
		TranslatePolygon(polygon, polygon, vertexCount, translation);

		DrawPolygon(polygon, edges, edgeCount, lineColor, BackBuffer);

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
