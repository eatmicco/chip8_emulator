#include "pixel_renderer.h"
#include <SFML/Graphics.hpp>

namespace chip8
{
	PixelRenderer::PixelRenderer()
	{
		PopulateRects();
	}

	PixelRenderer::~PixelRenderer()
	{
		ClearRects();
	}

	void PixelRenderer::Render(sf::RenderWindow *window)
	{
		for (int i = 0; i < PIXEL_COUNT; i++)
		{
			if (this->pixel_map_[i])
			{
				window->draw(rects_[i]);
			}
		}
	}

	void PixelRenderer::ClearRects()
	{
		rects_.clear();
	}

	void PixelRenderer::PopulateRects()
	{
		int xpos, ypos;
		ypos = 0;
		for (int y = 0; y < CHIP8_PIXEL_HEIGHT; y++)
		{
			xpos = 0;
			for (int x = 0; x < CHIP8_PIXEL_WIDTH; x++)
			{
				sf::RectangleShape rect_shape(sf::Vector2f(PIXEL_SCALE, PIXEL_SCALE));
				rect_shape.setPosition(xpos, ypos);
				rects_.push_back(rect_shape);
				xpos += PIXEL_SCALE;
			}
			ypos += PIXEL_SCALE;
		}
	}

	void PixelRenderer::SetPixels(const unsigned char *new_pixels)
	{
		if (new_pixels)
		{
			for (int i = 0; i < PIXEL_COUNT; i++)
			{
				this->pixel_map_[i] = new_pixels[i];
			}
		}
	}
}
