#ifndef PIXEL_RENDERER_H
#define PIXEL_RENDERER_H

#include "defines.h"
#include <vector>

namespace sf
{
	class RenderWindow;
	class RectangleShape;
}

namespace chip8
{
	class PixelRenderer
	{
	private:
		unsigned char pixel_map_[PIXEL_COUNT];
		std::vector<sf::RectangleShape> rects_;

		void ClearRects();
		void PopulateRects();
	public:
		PixelRenderer();
		~PixelRenderer();

		void Render(sf::RenderWindow *window);
		void SetPixels(const unsigned char *new_pixels);
	};
}

#endif //PIXEL_RENDERER_H