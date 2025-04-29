#include <iostream>
#include <stdint.h>
#include <vector>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <time.h> 
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>


enum RegistersNames
{
	VO, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF
};

enum MemoryMap
{
	PROGRAM_START = 0x200,
	FONT_START = 0x50,
	FONT_END = 0x9F,
};

enum ShiftMode
{
	COSMAC_VIP,
	SUPER_CHIP,
	UNKNOWN,
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

uint16_t opcode{};
uint8_t memory[4096] {};
uint16_t programCounter{};
uint16_t indexRegister {};
std::vector<uint16_t> stack {};
uint8_t delayTimer {};
uint8_t soundTimer{};
uint8_t registers[16]{};
uint32_t display[64 * 32]{};

uint8_t font[80]{ 0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
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

uint64_t NSperFrame { 1'000'000'000 / 60 };

bool keypad[16]{}; 
bool isKeyPressed{};
uint8_t keyPressed{};
bool isKeyReleased{};

extern SDL_Window* gWindow;

bool init();
void update();
void clearScreen();
void close();


void LoadFont() {

	for (int i = 0; i < 80; i++)
	{
		memory[FONT_START + i] = font[i];
	}
}

bool OpenROM(const std::string& pathROM) {

	std::ifstream ROMfile{pathROM, std::ios::binary | std::ios::ate };

	if (ROMfile.is_open())
	{
		int size{ static_cast<int>(ROMfile.tellg()) };
		char* buffer = new char[size];

		ROMfile.seekg(0, std::ios::beg);
		ROMfile.read(buffer, size);

		for (int i = 0; i < size; i++)
		{
			memory[PROGRAM_START + i] = buffer[i];
		}
		programCounter = PROGRAM_START;
		return true;
	}

	return false;
}

void fetch() {
	opcode = memory[programCounter];
	opcode = opcode << 8;
	opcode = opcode | memory[programCounter + 1];
	programCounter += 2;
}

void OPC0x00E0() {
	clearScreen();
}

void OPC0x00EE() {
	programCounter = stack.back();
	stack.pop_back();
}

void OPC0x2NNN(uint16_t OPC) {
	stack.push_back(programCounter);
	programCounter = OPC & 0x0FFF;

}

void OPC0x1(uint16_t OPC) {
	programCounter = OPC & 0x0FFF;
}

void OPC0x3XNN(uint16_t OPC) {

	uint16_t tempIndex = (OPC & 0x0F00) >> 8;
	bool condition = (registers[tempIndex] == (OPC & 0x00FF));
	if (condition)
	{
		programCounter += 2;
	}

}

void OPC0x4XNN(uint16_t OPC) {
	uint16_t tempIndex = (OPC & 0x0F00) >> 8;
	bool condition = (registers[tempIndex] != (OPC & 0x00FF));
	if (condition)
	{
		programCounter += 2;
	}
}

void OPC0x5XY0(uint16_t OPC) {
	uint16_t tempXIndex = (OPC & 0x0F00) >> 8;
	uint16_t tempYIndex = (OPC & 0x00F0) >> 4;

	if (registers[tempXIndex] == registers[tempYIndex])
	{
		programCounter += 2;
	}

}

void OPC0x9XY0(uint16_t OPC) {
	uint16_t tempXIndex = (OPC & 0x0F00) >> 8;
	uint16_t tempYIndex = (OPC & 0x00F0) >> 4;

	if (registers[tempXIndex] != registers[tempYIndex])
	{
		programCounter += 2;
	}

}

void OPC0x6(uint16_t OPC) {
	registers[((OPC & 0x0F00) >> 8)] = (OPC & 0x00FF);
}

void OPC0x7(uint16_t OPC) {
	registers[((OPC & 0xF00) >> 8)] += (OPC & 0x00FF);
}

void OPC0x8XY0(uint16_t OPC) {
	uint16_t tempXIndex = (OPC & 0x0F00) >> 8;
	uint16_t tempYIndex = (OPC & 0x00F0) >> 4;
	registers[tempXIndex] = registers[tempYIndex];
}

void OPC0x8XY1(uint16_t OPC, ShiftMode shiftmode) {
	uint16_t tempXIndex = (OPC & 0x0F00) >> 8;
	uint16_t tempYIndex = (OPC & 0x00F0) >> 4;

	registers[tempXIndex] = registers[tempXIndex] | registers[tempYIndex];
	if (shiftmode == COSMAC_VIP)
	{
		registers[VF] = 0;
	}
}

void OPC0x8XY2(uint16_t OPC, ShiftMode shiftmode) {
	uint16_t tempXIndex = (OPC & 0x0F00) >> 8;
	uint16_t tempYIndex = (OPC & 0x00F0) >> 4;

	registers[tempXIndex] = registers[tempXIndex] & registers[tempYIndex];
	if (shiftmode == COSMAC_VIP)
	{
		registers[VF] = 0;
	}
}

void OPC0x8XY3(uint16_t OPC, ShiftMode shiftmode) {
	uint16_t tempXIndex = (OPC & 0x0F00) >> 8;
	uint16_t tempYIndex = (OPC & 0x00F0) >> 4;

	registers[tempXIndex] = registers[tempXIndex] ^ registers[tempYIndex];
	if (shiftmode == COSMAC_VIP)
	{
		registers[VF] = 0;
	}
}

void OPC0x8XY4(uint16_t OPC) {
	uint16_t tempXIndex = (OPC & 0x0F00) >> 8;
	uint16_t tempYIndex = (OPC & 0x00F0) >> 4;

	bool overflow = registers[tempXIndex] + registers[tempYIndex] > 255;

	registers[tempXIndex] = registers[tempXIndex] + registers[tempYIndex];

	registers[VF] = 0x0;

	if (overflow)
	{
		registers[VF] = 0x1;
	}
}

void OPC0x8XY5(uint16_t OPC) {
	uint16_t tempXIndex = (OPC & 0x0F00) >> 8;
	uint16_t tempYIndex = (OPC & 0x00F0) >> 4;

	bool underflow = registers[tempXIndex] < registers[tempYIndex];
	registers[tempXIndex] = registers[tempXIndex] - registers[tempYIndex];

	if (underflow)
	{
		registers[VF] = 0x0;
	}
	else {
		registers[VF] = 0x1;
	}

	
}

void OPC0x8XY7(uint16_t OPC) {
	uint16_t tempXIndex = (OPC & 0x0F00) >> 8;
	uint16_t tempYIndex = (OPC & 0x00F0) >> 4;

	bool underflow = registers[tempYIndex] < registers[tempXIndex];
	registers[tempXIndex] = registers[tempYIndex] - registers[tempXIndex];

	if (underflow)
	{
		registers[VF] = 0x0;
	}
	else {
		registers[VF] = 0x1;
	}

}

void OPC0x8XY6(uint16_t OPC, ShiftMode shiftmode ) {

	uint16_t XIndex = (OPC & 0x0F00) >> 8 ;
	uint16_t Yindex = (OPC & 0x00F0) >> 4 ;


	if (shiftmode == COSMAC_VIP)
	{
		registers[XIndex] = registers[Yindex];
	}

	uint16_t shiftvalue = registers[XIndex] & 0x01;

	registers[XIndex] = registers[XIndex] >> 1;

	if (shiftvalue == 1)
	{
		registers[VF] = 0x1;
	}
	else
	{
		registers[VF] = 0x0;
	}
	
}

void OPC0x8XYE(uint16_t OPC, ShiftMode shiftmode) {

	uint16_t XIndex = (OPC & 0x0F00) >> 8;
	uint16_t Yindex = (OPC & 0x00F0) >> 4;
	uint16_t valueX = registers[XIndex];
	uint16_t valueY = registers[Yindex];

	if (shiftmode == COSMAC_VIP)
	{
		registers[XIndex] = registers[Yindex];
	}

	uint16_t shiftvalue = ((registers[XIndex] >> 7) & 0x1);

	registers[XIndex] = registers[XIndex] << 1;

	if (shiftvalue == 1)
	{
		registers[VF] = 0x1;
	}
	else
	{
		registers[VF] = 0x0;
	}

	
}

void OPC0xA(uint16_t OPC) {
	indexRegister = OPC & 0x0FFF;
}

void OPC0xB(uint16_t OPC, ShiftMode shiftmode) {
	
	uint16_t NNN = OPC & 0x0FFF;

	if (shiftmode == COSMAC_VIP)
	{
		programCounter = NNN + registers[0];
	}
	else {
		uint16_t XIndex =  OPC & 0x0F00 >> 4 ;
		programCounter = NNN + registers[XIndex];
	}
}

void OPC0xC(uint16_t OPC) {

	uint16_t XIndex= (OPC & 0x0F00) >> 8 ;
	uint16_t NN= OPC & 0x00FF ;
	registers[XIndex] = (rand() % 256) & NN;
}

void OPC0xD(uint16_t OPC) {
	//coordinate
	uint8_t x = registers[((OPC & 0x0F00) >> 8)] & 63;
	uint8_t y = registers[((OPC & 0x00F0) >> 4)] & 31;

	uint8_t n = OPC & 0x000F; //sprite height
	std::cout << "drawing at: " << std::dec << static_cast<int>(x) << " & " << static_cast<int>(y) << " for " << static_cast<int>(n) << " rows!\n";
	registers[VF] = 0;

	for (int row = 0; row < n; row++)
	{
		uint8_t spriteData = memory[indexRegister + row];

		if (y + row > 31)
		{
			break;
		}

		for (int column = 0; column < 8; column++)
		{
			if (x + column > 63)
			{
				break;
			}

			bool isPixelSpriteOn = (spriteData & (0x80 >> column)) != 0;

			if (!isPixelSpriteOn)
			{
				continue;
			}

			int coordinates = (y + row) * 64 + (x + column);


			if (display[coordinates] && isPixelSpriteOn)
			{
				registers[VF] = 1;

			}
			display[coordinates] ^= 1;
		}
	}

	
}

void OPC0xEX9E(uint16_t OPC) {
	uint16_t XIndex = (OPC & 0x0F00) >> 8;
	uint16_t key = registers[XIndex] % 16;
	if (keypad[key] == true) {
		programCounter += 2;
	}
}

void OPC0xEXA1(uint16_t OPC) {
	uint16_t XIndex = (OPC & 0x0F00) >> 8;
	uint16_t key = registers[XIndex] % 16;
	if (keypad[key] == false) {
		programCounter += 2;
	}
}

void OPC0xFX07(uint16_t OPC) {
	uint16_t XIndex = (OPC & 0x0F00) >> 8;
	registers[XIndex] = delayTimer;
}

void OPC0xFX15(uint16_t OPC) {
	uint16_t XIndex = (OPC & 0x0F00) >> 8;
	delayTimer = registers[XIndex];
}

void OPC0xFX18(uint16_t OPC) {
	uint16_t XIndex = (OPC & 0x0F00) >> 8;
	soundTimer = registers[XIndex];
}

void OPC0xFX1E(uint16_t OPC) {
	uint16_t XIndex = (OPC & 0x0F00) >> 8;
	indexRegister += registers[XIndex];
}

void OPC0xFX0A(uint16_t OPC) {
	uint16_t XIndex = (OPC & 0x0F00) >> 8;
	if (isKeyReleased != true)
	{
		programCounter -= 2;
	}
	else {
		registers[XIndex] = keyPressed;
	}
}

void OPC0xFX33(uint16_t OPC) {

	uint16_t XIndex = (OPC & 0x0F00) >> 8;
	uint8_t value = registers[XIndex];

	memory[indexRegister + 2]= value % 10;
	value = value / 10;

	memory[indexRegister + 1] = value % 10;
	value = value / 10;

	memory[indexRegister] = value % 10;
}

void OPC0xFX55(uint16_t OPC, ShiftMode shiftmode) {
	uint16_t XIndex = (OPC & 0x0F00) >> 8;

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
	uint16_t XIndex = (OPC & 0x0F00) >> 8;

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

	if (OPC == 0x00E0)
	{
		//clear display
		clearScreen();
		for (int row = 0; row < 32; row++)
		{
			for (int column = 0; column < 64; column++)
			{
				display[column + 64 * (row)] = 0;
			}
		}
		std::cout << "display cleared\n";
	}

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
			OPC0x1(OPC);
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
			OPC0x6(OPC);
			break;
		case 0x7: //register add (no carry)
			OPC0x7(OPC);
			break;
		case 0x8:
			switch ((OPC & 0x000F))
			{
			case 0x0000:
				OPC0x8XY0(OPC);
				break;
			case 0x0001:
				OPC0x8XY1(OPC, COSMAC_VIP);
				break;
			case 0x0002:
				OPC0x8XY2(OPC, COSMAC_VIP);
				break;
			case 0x0003:
				OPC0x8XY3(OPC, COSMAC_VIP);
				break;
			case 0x0004:
				OPC0x8XY4(OPC);
				break;
			case 0x0005:
				OPC0x8XY5(OPC);
				break;
			case 0x0006:
				OPC0x8XY6(OPC, COSMAC_VIP);
				break;
			case 0x0007:
				OPC0x8XY7(OPC);
				break;
			case 0x000E:
				OPC0x8XYE(OPC, COSMAC_VIP);
				break;
			default:
				break;
			}
			break;
		case 0xA: //index register set
			OPC0xA(OPC);
			break;
		case 0xD: //display
			OPC0xD(OPC);
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
			case 0x0033:
				OPC0xFX33(OPC);
				break;
			case 0x0055:
				OPC0xFX55(OPC, COSMAC_VIP);
				break;
			case 0x0065:
				OPC0xFX65(OPC, COSMAC_VIP);
				break;
			default:
				break;
			}
			break;
		default:
			break;
			break;
	}
}

int main(int argc, char* args[]) {

	srand(static_cast<unsigned int>(time(NULL)));
	const std::string ROMpath { "C:/Users/sorgi/Desktop/CH8ROMS/Space Invaders.ch8" };
	LoadFont();

	if (!OpenROM(ROMpath)) {
		std::cout << "problems opening the ROM! Retry";
		return 0;
	}
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

