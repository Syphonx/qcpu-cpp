//
// 
//

#pragma once

enum class EDrawMode : uint8_t
{
	Disabled,
	Enabled
};

enum class EDisplayMode : uint8_t
{
	Vectron,
	Optron
};

enum class EColourMode : uint8_t
{
	Brightness,
	Binary
};

struct Display
{
	Display(const uint16_t width, const uint16_t height, const EDisplayMode displayMode)
		: m_Width(width)
		, m_Height(height)
		, m_DisplayMode(displayMode)
		, m_Pixels()
		, m_Colour(65535)
	{
		Init();

		m_DisplayMode = EDisplayMode::Vectron;
		m_DrawMode = EDrawMode::Enabled;
		m_ColourMode = EColourMode::Brightness;
		m_CursorX = 0;
		m_CursorY = 0;
	}

	void Init()
	{
		m_Pixels.resize(GetBufferSize());
		std::fill(m_Pixels.begin(), m_Pixels.end(), 0);
	}

	const std::vector<uint8_t>& GetPixels() const
	{
		return m_Pixels;
	}

	void SetDrawMode(const EDrawMode drawMode)
	{
		m_DrawMode = drawMode;
	}

	void MoveCursor(uint16_t x, uint16_t y)
	{
		switch (m_DrawMode)
		{
			case EDrawMode::Enabled:
			{
				DrawLine(m_CursorX, m_CursorY, x, y);
			}
			break;

			default:
			case EDrawMode::Disabled:
			{
			}
			break;
		}

		m_CursorX = x;
		m_CursorY = y;
	}

	void SetColour(EColourMode mode, uint16_t colour)
	{
		m_ColourMode = mode;
		m_Colour = colour;
	}

private:

	int32_t GetPixelWidth() const
	{
		return 4; // RGBA
	}

	int32_t GetPixelDepth() const
	{
		return 8 + 8 + 8 + 8; // RGBA
	}

	int32_t GetBufferSize() const
	{
		return m_Height * GetPitch();
	}

	int32_t GetPitch() const
	{
		return m_Width * GetPixelWidth();
	}

	int32_t GetIndex(uint16_t x, uint16_t y) const
	{
		return (m_Width * 4 * y) + x * 4;
	}

	void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
	{
		switch (m_ColourMode)
		{
			default:
			case EColourMode::Brightness:
			{
				uint8_t colour = GetColourBrightness();
				DrawLine(WrapPixel(x0), WrapPixel(y0), WrapPixel(x1), WrapPixel(y1), colour, colour, colour, colour);
			}
			break;

			case EColourMode::Binary:
			{
				if (m_Colour == 0)
				{
					DrawLine(WrapPixel(x0), WrapPixel(y0), WrapPixel(x1), WrapPixel(y1), 255, 255, 255, 255);	// White
				}
				else
				{
					DrawLine(WrapPixel(x0), WrapPixel(y0), WrapPixel(x1), WrapPixel(y1), 255, 0, 0, 255);		// Red
				}
			}
			break;
		}
	}

	void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		bool steep = false;
		if (std::abs(x0 - x1) < std::abs(y0 - y1))
		{
			std::swap(x0, y0);
			std::swap(x1, y1);
			steep = true;
		}
		if (x0 > x1)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		int16_t dx = x1 - x0;
		int16_t dy = y1 - y0;
		int16_t derror2 = std::abs(dy) * 2;
		int16_t error2 = 0;
		int16_t y = y0;
		for (int16_t x = x0; x <= x1; x++)
		{
			if (steep)
			{
				DrawPixel(y, x, r, g, b, a);
			}
			else
			{
				DrawPixel(x, y, r, g, b, a);
			}
			error2 += derror2;
			if (error2 > dx)
			{
				y += (y1 > y0 ? 1 : -1);
				error2 -= dx * 2;
			}
		}
	}

	uint8_t GetColourBrightness() const
	{
		return (m_Colour / 65535) * 255;
	}

	uint16_t RemapRange(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
	{
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	}

	uint16_t WrapPixel(uint16_t pixel)
	{
		switch (m_DisplayMode)
		{
			default:
			case EDisplayMode::Optron:
			case EDisplayMode::Vectron:
			{
				return (pixel / 65536) * m_Width;
				// return pixel % m_Width;
			}
			break;
		}
	}

	void DrawPixel(uint16_t x, uint16_t y)
	{
		switch (m_ColourMode)
		{
			default:
			case EColourMode::Brightness:
			{
				uint8_t colour = GetColourBrightness();
				DrawPixel(WrapPixel(x), WrapPixel(y), colour, colour, colour, colour);
			}
			break;

			case EColourMode::Binary:
			{
				if (m_Colour == 0)
				{
					DrawPixel(WrapPixel(x), WrapPixel(y), 255, 255, 255, 255);	// White
				}
				else
				{
					DrawPixel(WrapPixel(x), WrapPixel(y), 255, 0, 0, 255);		// Red
				}
			}
			break;
		}
	}

	void DrawPixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		const uint32_t offset = GetIndex(x, y);
		m_Pixels[offset + 0] = b;				// b
		m_Pixels[offset + 1] = g;				// g
		m_Pixels[offset + 2] = r;				// r
		m_Pixels[offset + 3] = a;				// a
	}

	uint16_t					m_Width;
	uint16_t					m_Height;
	std::vector<uint8_t>		m_Pixels;

	EDisplayMode				m_DisplayMode;
	EDrawMode					m_DrawMode;
	EColourMode					m_ColourMode;

	uint16_t					m_Colour;
	uint16_t					m_CursorX;
	uint16_t					m_CursorY;
};