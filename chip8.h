#ifndef CHIP8_H
#define CHIP8_H

#include <string>

namespace chip8
{
	class Chip8
	{
	private:
		unsigned char memory_[4096];

		unsigned short opcode_;

		// Registers
		unsigned char v_[16];

		// Register to store memory addresses
		unsigned short i_;

		unsigned char delay_timer_;
		unsigned char sound_timer_;

		// Program Counter
		unsigned short pc_;

		unsigned short stack_[16];
		// Stack Pointer
		unsigned short sp_;

		bool keys_[16];

		unsigned char *gfx_;
		bool need_redraw_;
	public:
		Chip8();
		~Chip8();

		void Init();
		void LoadGame(const std::string &game_name);
		void Cycle();
		void SetKeyState(unsigned int key, bool state);

		bool GetNeedRedraw();
		void SetNeedRedraw(bool redraw);

		const unsigned char *GetGraphics();
	};
}

#endif //CHIP8_H