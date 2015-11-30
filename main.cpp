#include <SFML/Graphics.hpp>
#include "defines.h"
#include "pixel_renderer.h"
#include "chip8.h"

using namespace chip8;

#define FPS 720

Chip8 *engine;
PixelRenderer *renderer;
sf::RenderWindow *window;

void Init()
{
	engine = new Chip8();
	renderer = new PixelRenderer();
}

void UpdateKeyStates()
{
	engine->SetKeyState(0x1, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1));
	engine->SetKeyState(0x2, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2));
	engine->SetKeyState(0x3, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3));
	engine->SetKeyState(0xC, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4));

	engine->SetKeyState(0x4, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q));
	engine->SetKeyState(0x5, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W));
	engine->SetKeyState(0x6, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E));
	engine->SetKeyState(0xD, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R));

	engine->SetKeyState(0x7, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A));
	engine->SetKeyState(0x8, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S));
	engine->SetKeyState(0x9, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D));
	engine->SetKeyState(0xE, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F));

	engine->SetKeyState(0xA, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z));
	engine->SetKeyState(0x0, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X));
	engine->SetKeyState(0xB, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C));
	engine->SetKeyState(0xF, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::V));

}

void Cleanup()
{
	delete window;
	window = nullptr;
	delete renderer;
	renderer = nullptr;
	delete engine;
	engine = nullptr;
}

int main(int argc, char** argv)
{
	bool step_mode = false;
	bool step = false;
	bool fast_mode = false;

	Init();

	if (argc <= 1)
	{
		return 1;
	}
	else
	{
		std::string filename = std::string(argv[1]);
		engine->LoadGame(filename);
	}

	window = new sf::RenderWindow(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "CHIP8");
	window->setFramerateLimit(FPS);

	static float refresh_speed = 1.0 / FPS;
	sf::Clock clock;

	while (window->isOpen())
	{
		sf::Event event;
		while (window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window->close();
			}
			else if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Equal)
				{
					// Debug here
				}
				else if (event.key.code == sf::Keyboard::Num0)
				{
					step_mode = !step_mode;
				}
				else if (event.key.code == sf::Keyboard::N && step_mode)
				{
					step = true;
				}
				else if (event.key.code == sf::Keyboard::G)
				{
					fast_mode = !fast_mode;
				}
			}
		}

		if ((!step_mode && clock.getElapsedTime().asSeconds() >= refresh_speed) || (step_mode && step) || fast_mode)
		{
			UpdateKeyStates();

			engine->Cycle();

			// Update
			bool need_redraw = engine->GetNeedRedraw();
			if (need_redraw)
			{
				window->clear();
				auto pixels = engine->GetGraphics();
				renderer->SetPixels(pixels);
				renderer->Render(window);
				window->display();
				engine->SetNeedRedraw(false);
			}
			clock.restart();
			if (step_mode)
			{
				step = false;
			}
		}

		if (!step_mode && !fast_mode)
		{
			sf::sleep(sf::milliseconds(1000 / FPS));
		}
	}

	Cleanup();
	
	return 0;
}	