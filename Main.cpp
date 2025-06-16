#include <iostream>
#include <stdint.h>
#include <vector>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <time.h> 
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>

class Chip_8 {



	

	 

	

	 

	void OPC0xEX9E(uint16_t OPC) {
		uint16_t XIndex = getXfromOPC(OPC);
		uint16_t key = registers[XIndex] % 16;
		if (keypad[key] == true) {
			programCounter += 2;
		}
	}

	void OPC0xEXA1(uint16_t OPC) {
		uint16_t XIndex = getXfromOPC(OPC);
		uint16_t key = registers[XIndex] % 16;
		if (keypad[key] == false) {
			programCounter += 2;
		}
	}

	void OPC0xFX07(uint16_t OPC) {
		uint16_t XIndex = getXfromOPC(OPC);
		registers[XIndex] = delayTimer;
	}

	void OPC0xFX15(uint16_t OPC) {
		uint16_t XIndex = getXfromOPC(OPC);
		delayTimer = registers[XIndex];
	}

	void OPC0xFX18(uint16_t OPC) {
		uint16_t XIndex = getXfromOPC(OPC);
		soundTimer = registers[XIndex];
	}

	void OPC0xFX1E(uint16_t OPC) {
		uint16_t XIndex = getXfromOPC(OPC);
		indexRegister += registers[XIndex];
	}

	void OPC0xFX0A(uint16_t OPC) {
		uint16_t XIndex = getXfromOPC(OPC);
		if (isKeyReleased != true)
		{
			programCounter -= 2;
		}
		else {
			registers[XIndex] = keyPressed;
		}
	}

	void OPC0XFX29(uint16_t OPC) {
		uint16_t XIndex = getXfromOPC(OPC);
		indexRegister = (FONT_START + (registers[XIndex] % 16) * 5);
	}

	void OPC0xFX33(uint16_t OPC) {

		uint16_t XIndex = getXfromOPC(OPC);
		uint8_t value = registers[XIndex];

		memory[indexRegister + 2] = value % 10;
		value = value / 10;

		memory[indexRegister + 1] = value % 10;
		value = value / 10;

		memory[indexRegister] = value % 10;
	}

	void OPC0xFX55(uint16_t OPC, ShiftMode shiftmode) {
		uint16_t XIndex = getXfromOPC(OPC);

		for (int i = 0; i <= XIndex; i++)
		{
			memory[indexRegister + i] = registers[i];
		}

		if (shiftmode == COSMAC_VIP)
		{
			indexRegister += XIndex + 1;
		}
	}

	void OPC0xFX65(uint16_t OPC, ShiftMode shiftmode) {
		uint16_t XIndex = getXfromOPC(OPC);

		for (int i = 0; i <= XIndex; i++)
		{
			registers[i] = memory[indexRegister + i];
		}
		if (shiftmode == COSMAC_VIP)
		{
			indexRegister += XIndex + 1;
		}
	}

	void decode(uint16_t OPC) {

		switch (((OPC & 0xF000) >> 12))
		{
		case 0x0:
			switch (OPC)
			{
			case 0x00E0: // clear screen
				OPC0x00E0();
				break;
			case 0x00EE: //return from subroutine
				OPC0x00EE();
				break;
			default:
				break;
			}
			break;
		case 0x1: //Jump
			OPC0x1NNN(OPC);
			break;
		case 0x2: //Unconditional Jump
			OPC0x2NNN(OPC);
			break;
		case 0x3: //conditional jump
			OPC0x3XNN(OPC);
			break;
		case 0x4: //conditional jump
			OPC0x4XNN(OPC);
			break;
		case 0x5: //conditional jump
			OPC0x5XY0(OPC);
			break;
		case 0x9: //conditional jump
			OPC0x9XY0(OPC);
			break;
		case 0x6: //register set
			OPC0x6XNN(OPC);
			break;
		case 0x7: //register add (no carry)
			OPC0x7XNN(OPC);
			break;
		case 0x8:
			switch ((OPC & 0x000F))
			{
			case 0x0000:
				OPC0x8XY0(OPC);
				break;
			case 0x0001:
				OPC0x8XY1(OPC, shiftmode);
				break;
			case 0x0002:
				OPC0x8XY2(OPC, shiftmode);
				break;
			case 0x0003:
				OPC0x8XY3(OPC, shiftmode);
				break;
			case 0x0004:
				OPC0x8XY4(OPC);
				break;
			case 0x0005:
				OPC0x8XY5(OPC);
				break;
			case 0x0006:
				OPC0x8XY6(OPC, shiftmode);
				break;
			case 0x0007:
				OPC0x8XY7(OPC);
				break;
			case 0x000E:
				OPC0x8XYE(OPC, shiftmode);
				break;
			default:
				break;
			}
			break;
		case 0xA: //index register set
			OPC0xANNN(OPC);
			break;
		case 0xB:
			OPC0xBNNN(OPC, shiftmode);
			break;
		case 0xC:
			OPC0xCXNN(OPC);
			break;
		case 0xD: //display
			OPC0xDXYN(OPC);
			break;
		case 0xE:
			switch (OPC & 0x00FF)
			{
			case 0x009E:
				OPC0xEX9E(OPC);
				break;
			case 0x00A1:
				OPC0xEXA1(OPC);
				break;
			default:
				break;
			}
			break;
		case 0xF:
			switch ((OPC & 0x00FF))
			{
			case 0x0007:
				OPC0xFX07(OPC);
				break;
			case 0x0015:
				OPC0xFX15(OPC);
				break;
			case 0x0018:
				OPC0xFX18(OPC);
				break;
			case 0x001E:
				OPC0xFX1E(OPC);
				break;
			case 0x000A:
				OPC0xFX0A(OPC);
				break;
			case 0x0029:
				OPC0XFX29(OPC);
				break;
			case 0x0033:
				OPC0xFX33(OPC);
				break;
			case 0x0055:
				OPC0xFX55(OPC, shiftmode);
				break;
			case 0x0065:
				OPC0xFX65(OPC, shiftmode);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
};

enum RegistersNames
{
	VO, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF
};


SDL_Scancode chip8_keymap[16] = {
	SDL_SCANCODE_X, // 0
	SDL_SCANCODE_1, // 1
	SDL_SCANCODE_2, // 2
	SDL_SCANCODE_3, // 3
	SDL_SCANCODE_Q, // 4
	SDL_SCANCODE_W, // 5
	SDL_SCANCODE_E, // 6
	SDL_SCANCODE_A, // 7
	SDL_SCANCODE_S, // 8
	SDL_SCANCODE_D, // 9
	SDL_SCANCODE_Z, // A
	SDL_SCANCODE_C, // B
	SDL_SCANCODE_4, // C
	SDL_SCANCODE_R, // D
	SDL_SCANCODE_F, // E
	SDL_SCANCODE_V  // F
};




uint64_t NSperFrame { 1'000'000'000 / 60 };

extern SDL_Window* gWindow;

bool init();
void update();
void clearScreen();
void close();
void drawScaledPixel(int column, int row, bool pixelON);

























int main(int argc, char* args[]) {



	std::cout << "\n";

	if (!init()) {
		std::cout << "problems initializing SDL! Retry";
	}
	else {

		bool quit{ false };
		SDL_Event e;
		SDL_zero(e);

		while (!quit)
		{
			uint64_t loopStartNS = SDL_GetTicksNS();

			const bool* keyStates = SDL_GetKeyboardState(nullptr);

			for (int i = 0; i < 16; i++)
			{
				keypad[i] = keyStates[chip8_keymap[i]];

				if (keypad[i] == true && isKeyPressed != true)
				{
					isKeyPressed = true;
					keyPressed = i;
					isKeyReleased = false;
				}

				if (isKeyPressed == true && keypad[keyPressed] == false && isKeyReleased != true)
				{
					isKeyReleased = true;
					isKeyPressed = false;
				}
			}

			
			while (SDL_PollEvent(&e)) {

				if (e.type == SDL_EVENT_QUIT)
				{
					quit = true;
				}

				if (e.type == SDL_EVENT_KEY_DOWN) {

				}
			}



			if (delayTimer > 0)
			{
				delayTimer--;
			}
			
			if (soundTimer > 0)
			{
				soundTimer--;
			}

			// 11 loop
			for (int i = 0; i < 11; i++)
			{
				fetch();
				decode(opcode);
				/*if (((opcode & 0xF000) >> 12) == 0xD)
				{
					break;
				}*/
			}

			isKeyReleased = false;

			update();
			

			uint64_t loopTimeNS{ SDL_GetTicksNS() - loopStartNS };

			if (loopTimeNS < NSperFrame)
			{
				SDL_DelayNS(NSperFrame - loopTimeNS);
			}

		}
	}

	close();
	return 0;
}

