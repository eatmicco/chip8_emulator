#include "chip8.h"
#include "defines.h"
#include <ctime>
#include <fstream>
#include <iostream>

namespace chip8
{
	unsigned char chip8_fontset[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	Chip8::Chip8()
	{
		Init();
	}

	Chip8::~Chip8()
	{
		delete[] gfx_;
		gfx_ = nullptr;
	}

	void Chip8::Init()
	{
		opcode_ = 0;

		for (int i = 0; i < 4096; i++)
		{
			if (i < 80)
			{
				memory_[i] = chip8_fontset[i];
			}
			else
			{
				memory_[i] = 0;
			}
		}

		for (int i = 0; i < 16; i++)
		{
			v_[i] = 0;
		}

		i_ = 0;
		pc_ = 0x200;

		gfx_ = new unsigned char[PIXEL_COUNT];
		for (unsigned int i = 0; i < PIXEL_COUNT; i++)
		{
			gfx_[i] = 0;
		}
		need_redraw_ = true;

		delay_timer_ = 0;
		sound_timer_ = 0;

		sp_ = 0;

		for (unsigned int i = 0; i < 16; i++)
		{
			keys_[i] = false;
		}

		srand((unsigned int)time(NULL));
	}

	void Chip8::LoadGame(const std::string &game_name)
	{
		char *rom = nullptr; // We will store the rom in temporary area
		unsigned long size = 0;
		static unsigned int start_pos = 0x200;	// Programs start in 0x200 in chip8 normally
		std::ifstream input;
		//prepare f to throw if failbit gets set
		std::ios_base::iostate exceptionMask = input.exceptions() | std::ios::failbit;
		input.exceptions(exceptionMask);

		try {
			input.open(game_name, std::ios::binary);
		}
		catch (std::ios_base::failure& e) {
			std::cerr << e.what() << '\n';
		}

		if (input && input.is_open())
		{
			input.seekg(0, std::ios::end);	// Go to the end of the file
			size = (unsigned long)input.tellg(); // Record pos so that we can get the size of the file
			if (size + start_pos >= 4096)
			{
				std::cout << "Error: " << game_name << " is to large to load into memory." << std::endl;
			}
			else
			{
				input.seekg(0, std::ios::beg); // Rewind back to the start
				rom = new char[size + 1]; // Allocate some memory to load the rom into
				input.read(rom, size);
			}
			input.close();
		}
		else
		{
			std::cout << "Error: problem loading " << game_name << std::endl;
		}

		if (rom != nullptr)
		{
			for (unsigned int i = start_pos; i < 4096 && i - start_pos < size; i++)
			{
				// Fill up the memory with the rom
				memory_[i] = (unsigned char)rom[i - start_pos];
			}
			std::cout << "Loaded " << game_name << std::endl;
			delete[] rom;
			rom = nullptr;
		}
	}

	void Chip8::Cycle()
	{
		// Fetch two successive bytes and merge them to get the actual code
		opcode_ = memory_[pc_] << 8 | memory_[pc_ + 1];

		// Decode and Execute
		switch (opcode_ & 0xF000)
		{
		case 0x0000:
			// 0x0NNN SYS addr
			// Jump to a machine code routine at NNN
			// This instruction is only used on the old computers on which Chip-8 was originally implemented. It is ignored by modern interpreters
			switch (opcode_ & 0x00F0)
			{
			case 0x00C0:
				// 0x00CN SCD nibble
				// Scroll down N lines
				// When in chip8 mode scrolls down N/2 lines, when in SuperChip mode scroll down N lines
				// TODO
				pc_ += 2;
				break;
			}

			switch (opcode_ & 0x00FF)
			{
			case 0x00E0:
				// 0x00E0 CLS
				// Clear screen
				for (unsigned int i = 0; i < PIXEL_COUNT; i++)
					gfx_[i] = 0;
				need_redraw_ = true;
				pc_ += 2;
				break;
			case 0x00EE:
				// 0x00EE RET
				// Return from a subroutine
				sp_--; // Decrement stack pointer
				pc_ = stack_[sp_];	// Set the program counter to the old position
				pc_ += 2;
				break;
			case 0x00FB:
				// 0x00FB SCR
				// Scroll 4 pixels right in SuperChip mode, or 2 pixels in Chip8 mode
				// TODO
				pc_ += 2;
				break;
			case 0x00FC:
				// 0x00FC SCL
				// Scroll 4 pixels left in SuperChip mode, or 2 pixels in Chip8 mode
				// TODO
				pc_ += 2;
				break;
			case 0x00FD:
				// 0x00FD EXIT
				// Exit the emulator
				// TODO
				std::cout << "Exit!" << std::endl;
				break;
			case 0x00FE:
				// 0x00FE LOW
				// Set emulator to normal Chip8 resolution, 64 x 32
				// TODO
				pc_ += 2;
				break;
			case 0x00FF:
				// 0x00FF HIGH
				// Set emulator to SuperChip resolution 128 x 64
				// TODO
				pc_ += 2;
				break;
			default:
				break;
			}
			break;
		case 0x1000:
			// 0x1NNN JP addr
			// Jump to location NNN
			// Set the pc to location NNN
			// THe interpreter sets the program counter to NNN
			pc_ = opcode_ & 0x0FFF;
			break;
		case 0x2000:
			// 0x2NNN CALL addr
			// Call the subroutine at NNN
			// The interpreter increments the stack pointers, then puts the 
			// current PC on the top of the stack. The PC is then set to NNN
			stack_[sp_] = pc_; // Store the current position on the stack
			sp_++;	// Increment the stack pointer
			pc_ = opcode_ & 0x0FFF;
			break;
		case 0x3000:
			// 0x3XKK SE Vx, byte
			// Skip next intstruction if Vx = KK
			// The interpreter compares register Vx to KK, and if they are equal
			// increments the program counter by 2 (2 step means 4 bytes)
			if (v_[(opcode_ & 0x0F00) >> 8] == (opcode_ & 0x00FF))
			{
				// >> 8 shifts the result 8 bits right, making it a 1 byte value
				// KK doesn't need shifting since it is already on the right
				pc_ += 4;
			}
			else
			{
				pc_ += 2;
			}
			break;
		case 0x4000:
			// 0x4XKK SNE Vx, byte
			// Skip next instruction if Vx != KK
			// The interpreter compares register Vx to KK, and if they are not equal,
			// increments the program counter by 2 (2 steps means 4 bytes)
			if (v_[(opcode_ & 0x0F00) >> 8] != (opcode_ & 0x00FF))
			{
				pc_ += 4;
			}
			else
			{
				pc_ += 2;
			}
			break;
		case 0x5000:
			// 0x5XY0 SE Vx, Vy
			// Skip next instruction if Vx = Vy
			// The interpreter compares register Vx to register Vy, and if they are equal,
			// increments the program counter by 2 (2 means 4 bytes)
			if (v_[(opcode_ & 0x0F00) >> 8] == v_[(opcode_ & 0x00F0) >> 4])
			{
				pc_ += 4;
			}
			else
			{
				pc_ += 2;
			}
			break;
		case 0x6000:
			// 0x6XKK LD Vx, byte
			// Set Vx = KK
			// The interpreter puts the value KK into register Vx
			v_[(opcode_ & 0x0F00) >> 8] = opcode_ & 0x00FF;
			pc_ += 2;
			break;
		case 0x7000:
			// 0x7XKK ADD Vx, byte
			// Set Vx = Vx + KK
			// Adds the value KK to the value of register Vx, then stores the result in Vx
			v_[(opcode_ & 0x0F00) >> 8] += opcode_ & 0x00FF;
			pc_ += 2;
			break;
		case 0x8000:
			// 0x8??? opcodes
			switch (opcode_ & 0x000F)
			{
			case 0x0000:
				// 0x8XY0 LD Vx, Vy
				// Set Vx = Vy
				// Stores the value of register Vy in register Vx
				v_[(opcode_ & 0x0F00) >> 8] = v_[(opcode_ & 0x00F0) >> 4];
				pc_ += 2;
				break;
			case 0x0001:
				// 0x8XY1 OR Vx, Vy
				// Set Vx = Vx OR Vy
				// Performs a bitwise OR on the values of Vx and Vy, then stores
				// the result in Vx. A bitwise OR compares the corresponding bits
				// from two values, and if either bit is 1, then the same bit in 
				// result is also 1. Otherwise, it is 0
				v_[(opcode_ & 0x0F00) >> 8] |= v_[(opcode_ & 0x00F0) >> 4];
				pc_ += 2;
				break;
			case 0x0002:
				// 0x8XY2 AND Vx, Vy
				// Set Vx = Vx AND Vy
				// Performs a bitwise AND on the values of Vx and Vy, then stores
				// the result in Vx. A bitwise AND compares the corresponding bits
				// from two values, and if both bits are 1, then the same bit in the
				// result is also 1. Otherwise, it is 0
				v_[(opcode_ & 0x0F00) >> 8] &= v_[(opcode_ & 0x00F0) >> 4];
				pc_ += 2;
				break;
			case 0x0003:
				// 0x8XY3 XOR Vx, Vy
				// Set Vx = Vx XOR Vy
				// Bitwise XOR is ^=
				v_[(opcode_ & 0x0F00) >> 8] ^= v_[(opcode_ & 0x00F0) >> 4];
				pc_ += 2;
				break;
			case 0x0004:
				// 0x8XY4 ADD Vx, Vy
				// Set Vx = Vx + Vy, set VF = carry
				// The values of Vx and Vy are added together. If the result is greater than
				// 8 bits (i.e. > 255) VF is set to 1, otherwise 0. Only the lowest 8 bits of the
				// result are kept, and stored in Vx.
				if (v_[(opcode_ & 0x00F0) >> 4] > (0xFF - v_[(opcode_ & 0x0F00) >> 8]))
				{
					v_[0xF] = 1;	// There is a carry
				}
				else
				{
					v_[0xF] = 0;	// There is no carry
				}
				v_[(opcode_ & 0x0F00) >> 8] += v_[(opcode_ & 0x00F0) >> 4]; // Set the value
				pc_ += 2;
				break;
			case 0x0005:
				// 0x8XY5 SUB Vx, Vy
				// Set Vx = Vx - Vy, set VF = NOT borrow
				// If Vx > Vy, the VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, 
				// and then the results stored in Vx.
				if (v_[(opcode_ & 0x00F0) >> 4] > v_[(opcode_ & 0x0F00) >> 8])
				{
					v_[0xF] = 0; // There is a borrow
				}
				else
				{
					v_[0xF] = 1;
				}
				v_[(opcode_ & 0x0F00) >> 8] -= v_[(opcode_ & 0x00F0) >> 4]; // Set the value
				pc_ += 2;
				break;
			case 0x0006:
				// 0x8XY6 SHR Vx {, Vy}
				// Set Vx = Vx SHR 1.
				// If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0.
				// The Vx is divided by 2
				// Shift Vx to the right. Setting VF to 1 if the least significant bit is a 1
				// else setting it to 0
				v_[0xF] = v_[(opcode_ & 0x0F00) >> 8] & 0x1;

				// Shorthand for shiftingand setting
				v_[(opcode_ & 0x0F00) >> 8] >>= 1; // Set the value
				pc_ += 2;
				break;
			case 0x0007:
				// 0x8XY7 SUBN Vx, Vy
				// Set Vx = Vy - Vx, Set VF = NOT borrow
				// If Vy > Vx, the VF is set to 1, otherwise 0. Then Vx is subtracted from Vy,
				// and the results stored in Vx.
				if (v_[(opcode_ & 0x00F0) >> 4] > v_[(opcode_ & 0x0F00) >> 8])
				{
					v_[0xF] = 1;
				}
				else
				{
					v_[0xF] = 0; // There is a borrow
				}

				v_[(opcode_ & 0x0F00) >> 8] = v_[(opcode_ & 0x00F0) >> 4] - v_[(opcode_ & 0x0F00) >> 8];
				pc_ += 2;
				break;
			case 0x000E:
				// 0x8XYE SHL Vx {, Vy}
				// Set Vx = Vx SHL 1
				// If the most significant bit of Vx is 1, then VF is set to 1, otherwise to 0.
				// Then Vx is multiplied by 2.
				v_[0xF] = v_[(opcode_ & 0x0F00) >> 8] >> 7;

				// Shorthand for shifting and setting
				v_[(opcode_ & 0x0F00) >> 8] <<= 1; 
				pc_ += 2;
				break;
			default:
				break;
			}
			break;
		case 0x9000:
			// 0x9XY0 SNE Vx, Vy
			// Skip next instruction if Vx != Vy
			// The values of Vx and Vy are compared, and if they are not equal,
			// the program counter is increased by 2 (4 bytes)
			if (v_[(opcode_ & 0x0F00) >> 8] != v_[(opcode_ & 0x00F0) >> 4])
			{
				pc_ += 4;
			}
			else
			{
				pc_ += 2;
			}
			break;
		case 0xA000:
			// 0xANNN LD I, addr
			// Set I = NNN
			// The value of register I is set to NNN
			i_ = opcode_ & 0x0FFF;
			pc_ += 2;
			break;
		case 0xB000:
			// 0xBNNN JP V0, addr
			// Jump to location NNN + V0
			// The program counter is set to NNN plus the value of V0.
			pc_ = (opcode_ & 0x0FFF) + v_[0x0];
			break;
		case 0xC000:
			// 0xCXKK - RND Vx, byte
			// Set Vx = random byte AND KK
			// The interpreter generates a random number from 0 to 255, which is then 
			// ANDed with the value KK. The results are stored in Vx. See instruction
			// 0x8XY2 for more information about AND
			v_[(opcode_ & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode_ & 0x00FF);
			pc_ += 2;
			break;
		case 0xD000:
			{
				// 0xDXYN DRW Vx, Vy, nibble
				// Display N-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
				// The interpreter reads N bytes from memory, startingat the address stored in I.
				// These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
				// Sprites are XORed onto the existing screen. If this causes any pixels to be erased,
				// VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of
				// of it is outside the coordinates of the display, it wraps around to the opposite side
				// of the screen. See instruction 0x8XY3 for more information on XOR, 

				// TODO: Add support for 8*16 and 16*16 sprites when using height of 0 (for Chip8 and SuperChip)
				unsigned short x = v_[(opcode_ & 0x0F00) >> 8];
				unsigned short y = v_[(opcode_ & 0x00F0) >> 4];
				unsigned short height = opcode_ & 0x000F;
				v_[0xF] = 0;	// set Vf to 0, will be set to 1 if any collisions occur

				unsigned short pixel;
				// For each row of the sprite
				for (unsigned int yline = 0; yline < height; yline++)
				{
					pixel = memory_[i_ + yline];

					// For each pixel in the row
					for (unsigned int xline = 0; xline < 8; xline++)
					{
						// Check if there is a pixel that needs drawing present at that x and y in the sprite
						if ((pixel & (0x80 >> xline)) != 0)
						{
							// Check if there is a sprite already there in our graphics
							if (gfx_[(x + xline + ((y + yline) * CHIP8_PIXEL_WIDTH))] == 1)
							{
								v_[0xF] = 1; // If there is set the flag
							}

							gfx_[(x + xline + ((y + yline) * CHIP8_PIXEL_WIDTH))] ^= 1; // XOR onto the screen
						}
					}
				}
				need_redraw_ = true;
				pc_ += 2;
			}
			break;
		case 0xE000:
			// 0xENNN
			switch (opcode_ & 0x00FF)
			{
			case 0x009E:
				// 0xEX9E SKP Vx
				// Skip next instruction if key with the value of Vx is spressed
				// Checks the keyboard, and if the key corresponding to the value
				// of Vx is currently in the down position, PC is increased by 2.
				if (keys_[v_[(opcode_ & 0x0F00) >> 8]])
				{
					pc_ += 4;
				}
				else
				{
					pc_ += 2;
				}
				break;
			case 0x00A1:
				// 0xEXA1 SKNP Vx
				// Skip next instruction if key with the value of Vx is not pressed
				if (!keys_[v_[(opcode_ & 0x0F00) >> 8]])
				{
					pc_ += 4;
				}
				else
				{
					pc_ += 2;
				}
				break;
			default:
				break;
			}
			break;
		case 0xF000:
			// 0xFNNN
			switch (opcode_ & 0x00FF)
			{
			case 0x0007:
				// 0xFX07 LD Vx, DT
				// Set Vx = delay timer value
				// The value of DT is placed into Vx
				v_[(opcode_ & 0x0F00) >> 8] = delay_timer_;
				pc_ += 2;
				break;
			case 0x000A:
				{
					// 0xFX0A LD Vx, K
					// Wait for keypress, store the value of the key in Vx
					// All execution stops until a key is pressed, then the value
					// of that key is stored in Vx.
					bool key_pressed = false;
					for (unsigned int i = 0; i < 16; i++)
					{
						if (keys_[i])
						{
							key_pressed = true;
							v_[(opcode_ & 0x0F00) >> 8] = i;
							break;
						}
					}
					if (!key_pressed)
					{
						return; // Finish this instruction, essentially causing it to wait since pc_ hasn't been incremented
					}
					pc_ += 2;
				}
				break;
			case 0x0015:
				// 0xFX15 LD DT, Vx
				// Set delay timer = Vx
				// DT is set equal to the value of Vx
				delay_timer_ = v_[(opcode_ & 0x0F00) >> 8];
				pc_ += 2;
				break;
			case 0x0018:
				// 0xFX18 LD ST, Vx
				// Set sound timer = Vx
				// ST is set equal to the value of Vx
				sound_timer_ = v_[(opcode_ & 0x0F00) >> 8];
				pc_ += 2;
				break;
			case 0x001E:
				// 0xFX1E ADD I, Vx
				// Set I = I + Vx
				// The values of I and Vx are added, and the results are stored in I
				if (i_ + v_[(opcode_ & 0x0F00) >> 8] > 0xFFF)
				{
					v_[0xF] = 1;
				}
				else
				{
					v_[0xF] = 0;
				}

				i_ += v_[(opcode_ & 0x0F00) >> 8];
				pc_ += 2;

				// Not sure if this should set the carry or not, 
				// I don't think it should since it's manipulating the I register
				break;
			case 0x0029:
				// 0xFX29 LD F, Vx
				// Set I = location of sprite for digit Vx
				// The value of I is set to the location for the hexadecimal sprite
				// corresponding to the value of Vx.
				i_ = v_[(opcode_ & 0x0F00) >> 8] * 0x5;	// Sprites are 8*5
				pc_ += 2;
				break;
			case 0x0030:
				// 0xFX30 LD HF, Vx
				// Set I = location of SuperChip sprite for value of Vx
				// TODO
				//I = V[(opcode & 0x0F00) >> 8] * 0xA; // sprites are 8*10
				pc_ += 2;
				break;
			case 0x0033:
				// 0xFX33 LD B, Vx
				// Store BCD representation of Vx in memory locations I, I+1, and I+2
				// The interpreter takes the decimal value of Vx, and places the hundreds 
				// digit in memory at location in I, the tens digit at location I+1, and
				// the ones digit at location I+2
				memory_[i_] = v_[(opcode_ & 0x0F00) >> 8] / 100;
				memory_[i_ + 1] = (v_[(opcode_ & 0x0F00) >> 8] / 10) % 10;
				memory_[i_ + 2] = (v_[(opcode_ & 0x0F00) >> 8] % 100) % 10;
				pc_ += 2;
				break;
			case 0x0055:
				// 0xFX55 LD [I], Vx
				// Store registers V0 through Vx in memory starting at location I
				// The interpreter copies the values of registers V0 through Vx into memory,
				// starting at the address in I.
				for (unsigned int i = 0; i <= ((opcode_ & 0x0F00) >> 8); i++)
				{
					memory_[i_ + 1] = v_[i];
				}

				// Not sure on this line as it was found in an example emulator but the doc I have doesn't mention incrementing I
				i_ += ((opcode_ & 0x0F00) >> 8) + 1; // Increment I so it's pointing after all the newly inserted values
				
				pc_ += 2;
				break;
			case 0x0065:
				// 0xFX65 LD Vx, [I]
				// Read registers V0 through Vx from memory starting at location I.
				// The interpreter reads values from memory starting at location I into registers V0 through Vx
				for (unsigned int i = 0; i <= ((opcode_ & 0x0F00) >> 8); i++)
				{
					v_[i] = memory_[i_ + 1];
				}

				// Not sure on this line as it was found in an example emulator but the doc I have doesn't mention incrementing I
				i_ += ((opcode_ & 0x0F00) >> 8) + 1; // Increment I so it's pointing after all the newly inserted values

				pc_ += 2;

				break;
			case 0x0075:
				// 0xFX75 LD R, Vx
				// HP48 Save Flag
				// TODO
				pc_ += 2;
				break;

			case 0x0085:
				// 0xFX75 LD Vx, R
				// HP48 Load Flag
				// TODO
				pc_ += 2;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}

		if (delay_timer_ > 0) delay_timer_--;
		if (sound_timer_ > 0) sound_timer_--;
	}

	void Chip8::SetKeyState(unsigned int key, bool state)
	{
		keys_[key] = state;
	}

	bool Chip8::GetNeedRedraw()
	{
		return need_redraw_;
	}

	void Chip8::SetNeedRedraw(bool redraw)
	{
		need_redraw_ = redraw;
	}

	const unsigned char *Chip8::GetGraphics()
	{
		return gfx_;
	}
}
