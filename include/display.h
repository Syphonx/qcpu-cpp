//
//	Display
//

#pragma once

#include <algorithm>
#include <execution>
#include <stdint.h>
#include <vector>

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
	Display(const uint16_t width, const uint16_t height, const EDisplayMode displayMode);

	void Init();
	void Tick();
	void Flush();

	const std::vector<uint8_t>& GetPixels() const;

	void SetDrawMode(const EDrawMode drawMode);
	void MoveCursor(uint16_t x, uint16_t y);
	void SetColour(EColourMode mode, uint16_t colour);
	bool ShouldFlush() const;
	void ClearFlush();

private:

	int32_t GetPixelWidth() const;
	int32_t GetPixelDepth() const;
	int32_t GetBufferSize() const;
	int32_t GetPitch() const;
	int32_t GetIndex(uint16_t x, uint16_t y) const;

	void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
	void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	uint8_t GetColourBrightness() const;
	uint16_t RemapRange(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
	uint16_t WrapPixel(uint16_t pixel);
	void DrawPixel(uint16_t x, uint16_t y);
	void DrawPixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

	uint16_t m_Width;
	uint16_t m_Height;
	std::vector<uint8_t> m_Pixels;

	EDisplayMode m_DisplayMode;
	EDrawMode m_DrawMode;
	EColourMode m_ColourMode;

	uint8_t m_Flush : 1;
	uint16_t m_Colour;
	uint16_t m_CursorX;
	uint16_t m_CursorY;
};