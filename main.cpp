#include <windows.h>
#include <math.h>
#include "math_r.cpp"

#define PI 3.14159265358979323846

int Running = 1;
int BufferWidth = 1600;
int BufferHeight = 900;
Vec3 ScreenCenter = {BufferWidth / 2.0f, BufferHeight / 2.0f, 0};
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
		unsigned char blue;
		unsigned char green;
		unsigned char red;
		unsigned char alpha;
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
		Vec3 p1 = polygon[edges[i].edge1];
		Vec3 p2 = polygon[edges[i].edge2];

		float factor = 100;
#if 0
		p1 = p1 - ScreenCenter;
		p1.X = p1.X * (1 / (p1.Z / factor));
		p1.Y = p1.Y * (1 / (p1.Z / factor));
		p1 = p1 + ScreenCenter;

		p2 = p2 - ScreenCenter;
		p2.X = p2.X * (1 / (p2.Z / factor));
		p2.Y = p2.Y * (1 / (p2.Z / factor));
		p2 = p2 + ScreenCenter;
#else
	#if 1
		p1 = p1 - ScreenCenter;
		p1.X = (p1.X * factor) / (p1.Z + factor);
		p1.Y = (p1.Y * factor) / (p1.Z + factor);
		p1 = p1 + ScreenCenter;

		p2 = p2 - ScreenCenter;
		p2.X = (p2.X * factor) / (p2.Z + factor);
		p2.Y = (p2.Y * factor) / (p2.Z + factor);
		p2 = p2 + ScreenCenter;			
	#else
		float fov = 90;

		p1 = p1 - ScreenCenter;
		p1.X = p1.X / (3.55 * p1.Z * tan(fov / 2));
		p1.Y = p1.Y / (p1.Z * tan(fov / 2));;
		p1 = p1 + ScreenCenter;

		p2 = p2 - ScreenCenter;
		p2.X = p2.X / (3.55 * p2.Z * tan(fov / 2));
		p2.Y = p2.Y / (p2.Z * tan(fov / 2));
		p2 = p2 + ScreenCenter;	
	#endif
#endif
		DrawLine(p1.X, p1.Y, p2.X, p2.Y, color, Buffer);
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
		color.red = 0;
		color.green = 0;
		color.blue = 255;

		Vec3 center = { 800, 450, 5.5 };
		
		Vec3 polygon[] = {
			{850,363, 1},
			{750,363, 1},
			{700,450, 1},
			{750,537, 1},
			{850,537, 1},
			{900,450, 1},

			{850,363, 10},
			{750,363, 10},
			{700,450, 10},
			{750,537, 10},
			{850,537, 10},
			{900,450, 10},
		};

		Vec3 polygon2[] = {
			{600, 463, 10},
			{500, 463, 10},
			{450, 550, 10},
			{500, 637, 10},
			{600, 637, 10},
			{650, 550, 10},

			{600, 463, 100},
			{500, 463, 100},
			{450, 550, 100},
			{500, 637, 100},
			{600, 637, 100},
			{650, 550, 100},
		};

		Edge edges[] = {
			{0, 1},
			{1, 2},
			{2, 3},
			{3, 4},
			{4, 5},
			{5, 0},

			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 6},

			{0, 6},
			{1, 7},
			{2, 8},
			{3, 9},
			{4, 10},
			{5, 11},

		};

		int vertexCount = sizeof(polygon) / sizeof(*polygon);
		int edgeCount = sizeof(edges) / sizeof(*edges);

		Vec3* rotatedPolygon = (Vec3*)calloc(vertexCount * sizeof(Vec3), sizeof(Vec3));
		Vec3* scaledPolygon = (Vec3*)calloc(vertexCount * sizeof(Vec3), sizeof(Vec3));
		Vec3* translatedPolygon = (Vec3*)calloc(vertexCount * sizeof(Vec3), sizeof(Vec3));

		static Vec3 degrees = { 0, 0, 0 };
		degrees.X -= 0.25;
		degrees.Y += 0.25;
		degrees.Z -= 0.25;

		static float scale = 1;
		static float scaleIncrease = 0.005;
		scale += scaleIncrease;
		if (scale > 2 || scale < 0.5f)
		{
			scaleIncrease = -scaleIncrease;
		}

		static int green = 0;
		static int greenIncrease = 1;
		green += greenIncrease;
		if (green > 254 || green < 1)
		{
			greenIncrease = -greenIncrease;
		}		
		color.green = green;

		static int red = 255;
		static int redIncrease = -1;
		red += redIncrease;
		if (red > 254 || red < 1)
		{
			redIncrease = -redIncrease;
		}		
		color.red = red;

		static Vec3 translate = { 0, 0, 0 };
		static float translateIncrease = 0.5f;
		translate.X += translateIncrease;
		if (translate.X > 600 || translate.X < -600)
		{
			translateIncrease = -translateIncrease;
		}



		//RotatePolygonX(polygon, rotatedPolygon, vertexCount, center, degrees.X);
		//RotatePolygonY(rotatedPolygon, rotatedPolygon, vertexCount, center, degrees.Y);
		RotatePolygonZ(polygon, rotatedPolygon, vertexCount, center, degrees.Z);
		TranslatePolygon(rotatedPolygon, translatedPolygon, vertexCount, translate);

		//ScalePolygon(rotatedPolygon, scaledPolygon, vertexCount, center, scale,scale, scale);

		DrawPolygon(translatedPolygon, edges, edgeCount, color, BackBuffer);
		DrawPolygon(polygon2, edges, edgeCount, color, BackBuffer);
/*	
		Vec2 polygon[] = {
			{450, 450},
			{450, 550},
			{550, 550},
			{550, 450},
		};

		Vec2 center = {500, 500};

		int vertexCount = sizeof(polygon) / sizeof(*polygon);

		Vec2* rotatedPolygon = (Vec2*)calloc(vertexCount * sizeof(Vec2), sizeof(Vec2));

		RotatePolygon(polygon, rotatedPolygon, vertexCount, center, 90);
		DrawPolygon(rotatedPolygon, vertexCount, color, BackBuffer);
*/
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
